// ==================================================================================
// Copyright (c) 2012 HiFi-LoFi
//
// This is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// ==================================================================================

#include "PluginProcessor.h"

#include "IRAgent.h"
#include "Persistence.h"
#include "Settings.h"
#include "FFTConvolver/Configuration.h"
#include "FFTConvolver/Sample.h"
#include "UI/KlangFalterEditor.h"

#include <algorithm>


//==============================================================================
PluginAudioProcessor::PluginAudioProcessor() :
  AudioProcessor(),
  ChangeBroadcaster(),
  _irManager(),
  _sampleRate(0.0),
  _wetBuffer(1, 0),
  _convolutionBuffers(),
  _parameters(),
  _eqLo(4, CookbookEq(CookbookEq::LoShelf, 0, 1.0f)),
  _eqHi(4, CookbookEq(CookbookEq::HiShelf, 20000, 1.0f)),
  _levelMeasurementsDry(2),
  _levelMeasurementsWet(2),
  _settings()
{
  _parameters.insert(std::make_pair(DryOn, Parameter("DryOn", 1.0f)));
  _parameters.insert(std::make_pair(Dry, Parameter("Dry", 1.0f)));
  
  _parameters.insert(std::make_pair(WetOn, Parameter("WetOn", 1.0f)));
  _parameters.insert(std::make_pair(Wet, Parameter("Wet", 1.0f)));
  
  _parameters.insert(std::make_pair(AutoGainOn, Parameter("Autogain On", 1.0f)));
  _parameters.insert(std::make_pair(AutoGain, Parameter("Autogain", 1.0f)));

  _parameters.insert(std::make_pair(EqLowOn, Parameter("Low On", 0.0f)));
  _parameters.insert(std::make_pair(EqLowFreq, Parameter("Low Freq", 20.0f)));
  _parameters.insert(std::make_pair(EqLowGainDb, Parameter("Low Gain", 0.0f)));
  _parameters.insert(std::make_pair(EqLowQ, Parameter("Low Q", 1.0f)));
  
  _parameters.insert(std::make_pair(EqHighOn, Parameter("High On", 0.0f)));
  _parameters.insert(std::make_pair(EqHighFreq, Parameter("High Freq", 20000.0f)));
  _parameters.insert(std::make_pair(EqHighGainDb, Parameter("High Gain", 0.0f)));
  _parameters.insert(std::make_pair(EqHighQ, Parameter("High Q", 1.0f)));

  _irManager = new IRManager(*this, 2, 2);
}


PluginAudioProcessor::~PluginAudioProcessor()
{
  PluginAudioProcessor::releaseResources();
}


//==============================================================================
const String PluginAudioProcessor::getName() const
{
#ifdef JucePlugin_Name
  return JucePlugin_Name;
#else
  return String("Plugin Test");
#endif
}

int PluginAudioProcessor::getNumParameters()
{
  return _parameters.size();
}

float PluginAudioProcessor::getParameter(int index)
{
  auto it = _parameters.find(static_cast<ParameterId>(index));
  jassert(it != _parameters.end());
  return it->second.getValue();
}

void PluginAudioProcessor::setParameter(int index, float newValue)
{
  auto it = _parameters.find(static_cast<ParameterId>(index));
  jassert(it != _parameters.end());
  if (it != _parameters.end() && ::fabs(it->second.getValue()-newValue) > 0.0000001)
  {
    it->second.setValue(newValue);
    sendChangeMessage();
  }
}

const String PluginAudioProcessor::getParameterName(int index)
{
  auto it = _parameters.find(static_cast<ParameterId>(index));
  jassert(it != _parameters.end());
  return it->second.getName();
}

const String PluginAudioProcessor::getParameterText(int index)
{
  auto it = _parameters.find(static_cast<ParameterId>(index));
  jassert(it != _parameters.end());
  return juce::String(it->second.getValue());
}

const String PluginAudioProcessor::getInputChannelName(int channelIndex) const
{
    return String (channelIndex + 1);
}

const String PluginAudioProcessor::getOutputChannelName(int channelIndex) const
{
    return String (channelIndex + 1);
}

bool PluginAudioProcessor::isInputChannelStereoPair(int /*index*/) const
{
    return true;
}

bool PluginAudioProcessor::isOutputChannelStereoPair(int /*index*/) const
{
    return true;
}

bool PluginAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool PluginAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

int PluginAudioProcessor::getNumPrograms()
{
    return 0;
}

int PluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PluginAudioProcessor::setCurrentProgram (int /*index*/)
{
}

const String PluginAudioProcessor::getProgramName (int /*index*/)
{
    return String::empty;
}

void PluginAudioProcessor::changeProgramName (int /*index*/, const String& /*newName*/)
{
}


//==============================================================================
void PluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
  // Play safe to be clean
  releaseResources();
  
  // Prepare convolution
  _sampleRate = sampleRate;
  _irManager->setConvolverSampleRate(_sampleRate);
  _wetBuffer.setSize(2, samplesPerBlock);  
  
  for (size_t i=0; i<_irManager->getAgentCount(); ++i)
  {
    _convolutionBuffers.push_back(new fftconvolver::SampleBuffer(samplesPerBlock));
  }
  
  // EQ
  for (size_t i=0; i<_eqLo.size(); ++i)
  {
    _eqLo[i].prepareToPlay(sampleRate, samplesPerBlock);
  }
  for (size_t i=0; i<_eqHi.size(); ++i)
  {
    _eqHi[i].prepareToPlay(sampleRate, samplesPerBlock);
  }
}


void PluginAudioProcessor::releaseResources()
{
  for (size_t i=0; i<_convolutionBuffers.size(); ++i)
  {
    delete _convolutionBuffers[i];
  }
  _convolutionBuffers.clear();
  _wetBuffer.setSize(1, 0, false, true, false);
  _sampleRate = 0.0;

  for (size_t i=0; i<_eqLo.size(); ++i)
  {
    _eqLo[i].cleanup();
  }
  for (size_t i=0; i<_eqHi.size(); ++i)
  {
    _eqHi[i].cleanup();
  }
  
  sendChangeMessage();
}


void PluginAudioProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& /*midiMessages*/)
{ 
  // Preparation
  const bool wetOn = (getParameter(WetOn) >= 0.5f);
  const bool dryOn = (getParameter(DryOn) >= 0.5f);
  const float factorWet = getParameter(Wet);
  const float factorDry = getParameter(Dry);

  const float eqLoOn = (getParameter(EqLowOn) >= 0.5f);
  const float eqLoFreq = getParameter(EqLowFreq);
  const float eqLoGainDb = getParameter(EqLowGainDb);
  const float eqLoQ = getParameter(EqLowQ);
  
  const float eqHiOn = (getParameter(EqHighOn) >= 0.5f);
  const float eqHiFreq = getParameter(EqHighFreq);
  const float eqHiGainDb = getParameter(EqHighGainDb);
  const float eqHiQ = getParameter(EqHighQ);
  
  const int numInputChannels = getNumInputChannels();
  const int numOutputChannels = getNumOutputChannels();
  const size_t samplesToProcess = buffer.getNumSamples();
  
  _wetBuffer = buffer;
  _wetBuffer.applyGain(0, samplesToProcess, factorWet);

  buffer.applyGain(0, samplesToProcess, factorDry);

  if (numInputChannels == 1)
  {    
    _levelMeasurementsDry[0].process(buffer.getSampleData(0), samplesToProcess);
    _levelMeasurementsDry[1].reset();
  }
  else if (numInputChannels == 2)
  {
    _levelMeasurementsDry[0].process(buffer.getSampleData(0), samplesToProcess);
    _levelMeasurementsDry[1].process(buffer.getSampleData(1), samplesToProcess);
  }

  float* channelData0 = nullptr;
  float* channelData1 = nullptr;

  if (numInputChannels == 1)
  {    
    channelData0 = _wetBuffer.getSampleData(0);
    channelData1 = _wetBuffer.getSampleData(0);
  }
  else if (numInputChannels == 2)
  {
    channelData0 = _wetBuffer.getSampleData(0);
    channelData1 = _wetBuffer.getSampleData(1);
  }
  
  // Convolution
  if (dryOn == false)
  {
    buffer.clear();
  }

  if (channelData0 && channelData1)
  {
    IRAgent* irAgent00 = _irManager->getAgent(0, 0);
    IRAgent* irAgent01 = _irManager->getAgent(0, 1);
    IRAgent* irAgent10 = _irManager->getAgent(1, 0);
    IRAgent* irAgent11 = _irManager->getAgent(1, 1);
    
    fftconvolver::SampleBuffer* convolutionBuffer00 = nullptr;
    fftconvolver::SampleBuffer* convolutionBuffer01 = nullptr;
    fftconvolver::SampleBuffer* convolutionBuffer10 = nullptr;
    fftconvolver::SampleBuffer* convolutionBuffer11 = nullptr;

    float autoGain = 1.0f;
    if (getParameter(AutoGainOn) > 0.5f)
    {
      autoGain = getParameter(AutoGain);
    }
    
    // Convolve
    if (irAgent00 && irAgent00->getConvolver())
    {
      convolutionBuffer00 = _convolutionBuffers[0];
      irAgent00->process(channelData0, convolutionBuffer00->data(), samplesToProcess, autoGain);

      if (eqLoOn)
      {
        CookbookEq& eqLo = _eqLo[0];
        eqLo.setFreqAndQ(eqLoFreq, eqLoQ);
        eqLo.setGain(eqLoGainDb);
        eqLo.filterOut(convolutionBuffer00->data(), samplesToProcess);
      }

      if (eqHiOn)
      {
        CookbookEq& eqHi = _eqHi[0];
        eqHi.setFreqAndQ(eqHiFreq, eqHiQ);
        eqHi.setGain(eqHiGainDb);
        eqHi.filterOut(convolutionBuffer00->data(), samplesToProcess);
      }
      
      if (wetOn)
      {
        buffer.addFrom(0, 0, convolutionBuffer00->data(), samplesToProcess, 1.0f);
      }
    }
    
    if (irAgent01 && irAgent01->getConvolver())
    {
      convolutionBuffer01 = _convolutionBuffers[1];
      irAgent01->process(channelData0, convolutionBuffer01->data(), samplesToProcess, autoGain);
      
      if (eqLoOn)
      {
        CookbookEq& eqLo = _eqLo[1];
        eqLo.setFreqAndQ(eqLoFreq, eqLoQ);
        eqLo.setGain(eqLoGainDb);
        eqLo.filterOut(convolutionBuffer01->data(), samplesToProcess);
      }
      
      if (eqHiOn)
      {
        CookbookEq& eqHi = _eqHi[1];
        eqHi.setFreqAndQ(eqHiFreq, eqHiQ);
        eqHi.setGain(eqHiGainDb);
        eqHi.filterOut(convolutionBuffer01->data(), samplesToProcess);
      }
      
      if (wetOn)
      {
        buffer.addFrom(1, 0, convolutionBuffer01->data(), samplesToProcess, 1.0f);
      }
    }
    
    if (irAgent10 && irAgent10->getConvolver())
    {
      convolutionBuffer10 = _convolutionBuffers[2];
      irAgent10->process(channelData1, convolutionBuffer10->data(), samplesToProcess, autoGain);
      
      if (eqLoOn)
      {
        CookbookEq& eqLo = _eqLo[2];
        eqLo.setFreqAndQ(eqLoFreq, eqLoQ);
        eqLo.setGain(eqLoGainDb);
        eqLo.filterOut(convolutionBuffer10->data(), samplesToProcess);
      }
      
      if (eqHiOn)
      {
        CookbookEq& eqHi = _eqHi[2];
        eqHi.setFreqAndQ(eqHiFreq, eqHiQ);
        eqHi.setGain(eqHiGainDb);
        eqHi.filterOut(convolutionBuffer10->data(), samplesToProcess);
      }
      
      if (wetOn)
      {
        buffer.addFrom(0, 0, convolutionBuffer10->data(), samplesToProcess, 1.0f);
      }
    }
    
    if (irAgent11 && irAgent11->getConvolver())
    {
      convolutionBuffer11 = _convolutionBuffers[3];
      irAgent11->process(channelData1, convolutionBuffer11->data(), samplesToProcess, autoGain);
      
      if (eqLoOn)
      {
        CookbookEq& eqLo = _eqLo[3];
        eqLo.setFreqAndQ(eqLoFreq, eqLoQ);
        eqLo.setGain(eqLoGainDb);
        eqLo.filterOut(convolutionBuffer11->data(), samplesToProcess);
      }
      
      if (eqHiOn)
      {
        CookbookEq& eqHi = _eqHi[3];
        eqHi.setFreqAndQ(eqHiFreq, eqHiQ);
        eqHi.setGain(eqHiGainDb);
        eqHi.filterOut(convolutionBuffer11->data(), samplesToProcess);
      }
      
      if (wetOn)
      {
        buffer.addFrom(1, 0, convolutionBuffer11->data(), samplesToProcess, 1.0f);
      }
    }  
  }
  
  // In case we have more outputs than inputs, we'll clear any output
  // channels that didn't contain input data, (because these aren't
  // guaranteed to be empty - they may contain garbage).
  for (int i=numInputChannels; i<numOutputChannels; ++i)
  {
    buffer.clear (i, 0, buffer.getNumSamples());
  }
}

//==============================================================================
bool PluginAudioProcessor::hasEditor() const
{
    return true;
}

AudioProcessorEditor* PluginAudioProcessor::createEditor()
{
  return new KlangFalterEditor(this);
}

//==============================================================================
void PluginAudioProcessor::getStateInformation (MemoryBlock& destData)
{
  const juce::File irDirectory = _settings.getImpulseResponseDirectory();
  juce::ScopedPointer<juce::XmlElement> element(SaveState(irDirectory, *this));
  if (element)
  {
    copyXmlToBinary(*element, destData);
  }
}

void PluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{  
  juce::ScopedPointer<juce::XmlElement> element(getXmlFromBinary(data, sizeInBytes));
  if (element)
  {
    const juce::File irDirectory = _settings.getImpulseResponseDirectory();
    LoadState(irDirectory, *element, *this);
  }
}


//==============================================================================


double PluginAudioProcessor::getSampleRate() const
{
  return _sampleRate;
}


IRManager& PluginAudioProcessor::getIRManager()
{
  return *_irManager;
}


const IRManager& PluginAudioProcessor::getIRManager() const
{
  return *_irManager;
}


float PluginAudioProcessor::getLevelDry(size_t channel) const
{
  return (channel < _levelMeasurementsDry.size()) ? _levelMeasurementsDry[channel].getLevel() : 0.0f;
}


Settings& PluginAudioProcessor::getSettings()
{
  return _settings;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginAudioProcessor();
}
