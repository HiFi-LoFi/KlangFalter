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
#include "IRCalculation.h"
#include "Parameters.h"
#include "Persistence.h"
#include "Settings.h"
#include "StereoWidth.h"
#include "UI/KlangFalterEditor.h"

#include <algorithm>


//==============================================================================
PluginAudioProcessor::PluginAudioProcessor() :
  AudioProcessor(),
  ChangeNotifier(),
  _wetBuffer(1, 0),
  _convolutionBuffer(),
  _parameterSet(),
  _levelMeasurementsDry(2),
  _levelMeasurementsWet(2),
  _levelMeasurementsOut(2),
  _settings(),
  _convolverMutex(),
  _agents(),
  _stretch(1.0),
  _reverse(false),
  _envelope(1.0, 1.0),
  _convolverHeadBlockSize(0),
  _convolverTailBlockSize(0),
  _fileBeginSeconds(0.0),
  _predelayMs(0.0),
  _irCalculationMutex(),
  _irCalculation()
{ 
  _parameterSet.registerParameter(Parameters::WetOn);
  _parameterSet.registerParameter(Parameters::WetDecibels);
  _parameterSet.registerParameter(Parameters::DryOn);
  _parameterSet.registerParameter(Parameters::DryDecibels);
  _parameterSet.registerParameter(Parameters::AutoGainOn);
  _parameterSet.registerParameter(Parameters::AutoGainDecibels);
  _parameterSet.registerParameter(Parameters::EqLowOn);
  _parameterSet.registerParameter(Parameters::EqLowFreq);
  _parameterSet.registerParameter(Parameters::EqLowDecibels);
  _parameterSet.registerParameter(Parameters::EqHighOn);
  _parameterSet.registerParameter(Parameters::EqHighFreq);
  _parameterSet.registerParameter(Parameters::EqHighDecibels);
  _parameterSet.registerParameter(Parameters::StereoWidth);

  _agents.push_back(new IRAgent(*this, 0, 0));
  _agents.push_back(new IRAgent(*this, 0, 1));
  _agents.push_back(new IRAgent(*this, 1, 0));
  _agents.push_back(new IRAgent(*this, 1, 1));
}


PluginAudioProcessor::~PluginAudioProcessor()
{
  PluginAudioProcessor::releaseResources();

  for (size_t i=0; i<_agents.size(); ++i)
  {
    delete _agents[i];
  }
  _agents.clear();
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
  return _parameterSet.getParameterCount();
}

float PluginAudioProcessor::getParameter(int index)
{
  return _parameterSet.getNormalizedParameter(index);
}

void PluginAudioProcessor::setParameter(int index, float newValue)
{
  if (_parameterSet.setNormalizedParameter(index, newValue))
  {
    notifyAboutChange();
  }
}

const String PluginAudioProcessor::getParameterName(int index)
{
  return _parameterSet.getParameterDescriptor(index).getName();
}

const String PluginAudioProcessor::getParameterText(int index)
{
  return _parameterSet.getParameterDescriptor(index).getUnit();
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

bool PluginAudioProcessor::silenceInProducesSilenceOut() const
{
  return false;
}

void PluginAudioProcessor::numChannelsChanged()
{
  juce::AudioProcessor::numChannelsChanged();
  notifyAboutChange();
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
void PluginAudioProcessor::prepareToPlay(double /*sampleRate*/, int samplesPerBlock)
{
  // Play safe to be clean
  releaseResources();

  // Prepare convolvers
  {
    juce::ScopedLock convolverLock(_convolverMutex);
    _convolverHeadBlockSize = static_cast<size_t>(std::max(samplesPerBlock, 256));
    _convolverTailBlockSize = static_cast<size_t>(std::max(samplesPerBlock, 4096));
  }

  // Prepare convolution buffers
  _wetBuffer.setSize(2, samplesPerBlock);
  _convolutionBuffer.resize(samplesPerBlock);

  notifyAboutChange();
  updateConvolvers();
}


void PluginAudioProcessor::releaseResources()
{
  _wetBuffer.setSize(1, 0, false, true, false);
  _convolutionBuffer.clear();
  notifyAboutChange();
}


void PluginAudioProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& /*midiMessages*/)
{ 
  // Preparation
  const bool wetOn = getParameter(Parameters::WetOn);
  const bool dryOn = getParameter(Parameters::DryOn);
  const float factorWet = DecibelScaling::Db2Gain(getParameter(Parameters::WetDecibels));
  const float factorDry = DecibelScaling::Db2Gain(getParameter(Parameters::DryDecibels));

  const int numInputChannels = getNumInputChannels();
  const int numOutputChannels = getNumOutputChannels();
  const size_t samplesToProcess = buffer.getNumSamples();

  // Determine channel data
  float* channelData0 = nullptr;
  float* channelData1 = nullptr;
  if (numInputChannels == 1)
  {    
    channelData0 = buffer.getSampleData(0);
    channelData1 = buffer.getSampleData(0);
  }
  else if (numInputChannels == 2)
  {
    channelData0 = buffer.getSampleData(0);
    channelData1 = buffer.getSampleData(1);
  }

  // Convolution
  _wetBuffer.clear();
  if (numInputChannels > 0 && numOutputChannels > 0)
  {
    float autoGain = 1.0f;
    if (getParameter(Parameters::AutoGainOn))
    {
      autoGain = DecibelScaling::Db2Gain(getParameter(Parameters::AutoGainDecibels));
    }

    // Convolve
    IRAgent* irAgent00 = getAgent(0, 0);
    if (irAgent00 && irAgent00->getConvolver() && numInputChannels >= 1 && numOutputChannels >= 1)
    {
      irAgent00->process(channelData0, &_convolutionBuffer[0], samplesToProcess);
      _wetBuffer.addFrom(0, 0, &_convolutionBuffer[0], samplesToProcess, autoGain);
    }

    IRAgent* irAgent01 = getAgent(0, 1);
    if (irAgent01 && irAgent01->getConvolver() && numInputChannels >= 1 && numOutputChannels >= 2)
    {
      irAgent01->process(channelData0, &_convolutionBuffer[0], samplesToProcess);
      _wetBuffer.addFrom(1, 0, &_convolutionBuffer[0], samplesToProcess, autoGain);
    }

    IRAgent* irAgent10 = getAgent(1, 0);
    if (irAgent10 && irAgent10->getConvolver() && numInputChannels >= 2 && numOutputChannels >= 1)
    {      
      irAgent10->process(channelData1, &_convolutionBuffer[0], samplesToProcess);
      _wetBuffer.addFrom(0, 0, &_convolutionBuffer[0], samplesToProcess, autoGain);
    }

    IRAgent* irAgent11 = getAgent(1, 1);
    if (irAgent11 && irAgent11->getConvolver() && numInputChannels >= 2 && numOutputChannels >= 2)
    {
      irAgent11->process(channelData1, &_convolutionBuffer[0], samplesToProcess);
      _wetBuffer.addFrom(1, 0, &_convolutionBuffer[0], samplesToProcess, autoGain);
    }
  }

  // Stereo width
  if (numOutputChannels >= 2)
  {
    StereoWidth(getParameter(Parameters::StereoWidth), _wetBuffer.getSampleData(0), _wetBuffer.getSampleData(1), samplesToProcess);
  }

  // Dry/wet gain
  buffer.applyGain(0, samplesToProcess, factorDry);
  _wetBuffer.applyGain(0, samplesToProcess, factorWet);

  // Level measurement (dry)
  if (numInputChannels == 1)
  {    
    _levelMeasurementsDry[0].process(samplesToProcess, buffer.getSampleData(0));
    _levelMeasurementsDry[1].reset();
  }
  else if (numInputChannels == 2)
  {
    _levelMeasurementsDry[0].process(samplesToProcess, buffer.getSampleData(0));
    _levelMeasurementsDry[1].process(samplesToProcess, buffer.getSampleData(1));
  }

  // Sum wet to dry signal
  if (!dryOn)
  {
    buffer.clear();
  }
  if (wetOn)
  {
    if (numOutputChannels > 0)
    {
      buffer.addFrom(0, 0, _wetBuffer.getSampleData(0), samplesToProcess);
    }
    if (numOutputChannels > 1)
    {
      buffer.addFrom(1, 0, _wetBuffer.getSampleData(1), samplesToProcess);
    }
  }

  // Level measurement (wet/out)
  if (numOutputChannels == 1)
  {
    _levelMeasurementsWet[0].process(samplesToProcess, _wetBuffer.getSampleData(0));
    _levelMeasurementsWet[1].reset();
    _levelMeasurementsOut[0].process(samplesToProcess, buffer.getSampleData(0));
    _levelMeasurementsOut[1].reset();
  }
  else if (numOutputChannels == 2)
  {
    _levelMeasurementsWet[0].process(samplesToProcess, _wetBuffer.getSampleData(0));
    _levelMeasurementsWet[1].process(samplesToProcess, _wetBuffer.getSampleData(1));
    _levelMeasurementsOut[0].process(samplesToProcess, buffer.getSampleData(0));
    _levelMeasurementsOut[1].process(samplesToProcess, buffer.getSampleData(1));
  }

  // In case we have more outputs than inputs, we'll clear any output
  // channels that didn't contain input data, (because these aren't
  // guaranteed to be empty - they may contain garbage).
  for (int i=numInputChannels; i<numOutputChannels; ++i)
  {
    buffer.clear(i, 0, buffer.getNumSamples());
  }
}

//==============================================================================
bool PluginAudioProcessor::hasEditor() const
{
  return true;
}

AudioProcessorEditor* PluginAudioProcessor::createEditor()
{
  return new KlangFalterEditor(*this);
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


float PluginAudioProcessor::getLevelDry(size_t channel) const
{
  return (channel < _levelMeasurementsDry.size()) ? _levelMeasurementsDry[channel].getLevel() : 0.0f;
}


float PluginAudioProcessor::getLevelWet(size_t channel) const
{
  return (channel < _levelMeasurementsWet.size()) ? _levelMeasurementsWet[channel].getLevel() : 0.0f;
}


float PluginAudioProcessor::getLevelOut(size_t channel) const
{
  return (channel < _levelMeasurementsOut.size()) ? _levelMeasurementsOut[channel].getLevel() : 0.0f;
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


// =============================================================================


IRAgent* PluginAudioProcessor::getAgent(size_t inputChannel, size_t outputChannel) const
{
  for (size_t i=0; i<_agents.size(); ++i)
  {
    IRAgent* agent = _agents[i];
    if (agent->getInputChannel() == inputChannel && agent->getOutputChannel() == outputChannel)
    {
      return agent;
    }
  }
  return nullptr;
}


size_t PluginAudioProcessor::getAgentCount() const
{
  return _agents.size();
}


IRAgentContainer PluginAudioProcessor::getAgents() const
{
  return _agents;
}


void PluginAudioProcessor::clearConvolvers()
{
  {
    juce::ScopedLock convolverLock(_convolverMutex);
    _reverse = false;
    _stretch = 1.0;
    _fileBeginSeconds = 0.0;
    _envelope.reset();
  }

  setParameterNotifyingHost(Parameters::EqLowOn, Parameters::EqLowOn.getDefaultValue());
  setParameterNotifyingHost(Parameters::EqLowFreq, Parameters::EqLowFreq.getDefaultValue());
  setParameterNotifyingHost(Parameters::EqLowDecibels, Parameters::EqLowDecibels.getDefaultValue());

  setParameterNotifyingHost(Parameters::EqHighOn, Parameters::EqHighOn.getDefaultValue());
  setParameterNotifyingHost(Parameters::EqHighFreq, Parameters::EqHighFreq.getDefaultValue());
  setParameterNotifyingHost(Parameters::EqHighDecibels, Parameters::EqHighDecibels.getDefaultValue());

  for (size_t i=0; i<_agents.size(); ++i)
  {
    _agents[i]->clear();
  }

  notifyAboutChange();
  updateConvolvers();
}


void PluginAudioProcessor::setStretch(double stretch)
{
  bool changed = false;
  {
    juce::ScopedLock convolverLock(_convolverMutex);
    if (::fabs(_stretch-stretch) > 0.000001)
    {
      _stretch = stretch;
      changed = true;
    }
  }
  if (changed)
  {
    notifyAboutChange();
    updateConvolvers();
  }
}


double PluginAudioProcessor::getStretch() const
{
  juce::ScopedLock convolverLock(_convolverMutex);
  return _stretch;
}


void PluginAudioProcessor::setReverse(bool reverse)
{
  bool changed = false;
  {
    juce::ScopedLock convolverLock(_convolverMutex);
    if (_reverse != reverse)
    {
      _reverse = reverse;
      _envelope.setReverse(reverse);    
      changed = true;
    }
  }
  if (changed)
  {
    notifyAboutChange();
    updateConvolvers();
  }
}


bool PluginAudioProcessor::getReverse() const
{
  juce::ScopedLock convolverLock(_convolverMutex);
  return _reverse;
}


void PluginAudioProcessor::setEnvelope(const Envelope& envelope)
{
  {
    juce::ScopedLock convolverLock(_convolverMutex);
    _envelope = envelope;
  }
  notifyAboutChange();
  updateConvolvers();
}


Envelope PluginAudioProcessor::getEnvelope() const
{
  juce::ScopedLock convolverLock(_convolverMutex);
  return _envelope;
}


void PluginAudioProcessor::setPredelayMs(double predelayMs)
{
  bool changed = false;
  {
    juce::ScopedLock convolverLock(_convolverMutex);
    if (_predelayMs != predelayMs)
    {
      _predelayMs = predelayMs;
      changed = true;
    }
  }
  if (changed)
  {
    notifyAboutChange();
    updateConvolvers();
  }
}


double PluginAudioProcessor::getPredelayMs() const
{
  juce::ScopedLock convolverLock(_convolverMutex);
  return _predelayMs;
}


void PluginAudioProcessor::setFileBeginSeconds(double fileBeginSeconds)
{
  bool changed = false;
  {
    juce::ScopedLock convolverLock(_convolverMutex);
    if (_fileBeginSeconds != fileBeginSeconds)
    {
      _fileBeginSeconds = fileBeginSeconds;
      changed = true;
    }    
  }
  if (changed)
  {
    notifyAboutChange();
    updateConvolvers();
  }
}


double PluginAudioProcessor::getFileBeginSeconds() const
{
  juce::ScopedLock convolverLock(_convolverMutex);
  return std::max(0.0, std::min(getMaxFileDuration(), _fileBeginSeconds));
}


size_t PluginAudioProcessor::getConvolverHeadBlockSize() const
{
  juce::ScopedLock convolverLock(_convolverMutex);
  return _convolverHeadBlockSize;
}


size_t PluginAudioProcessor::getConvolverTailBlockSize() const
{
  juce::ScopedLock convolverLock(_convolverMutex);
  return _convolverTailBlockSize;
}


size_t PluginAudioProcessor::getMaxIRSampleCount() const
{
  size_t maxSampleCount = 0;
  for (auto it=_agents.begin(); it!=_agents.end(); ++it)
  {
    FloatBuffer::Ptr ir = (*it)->getImpulseResponse();
    if (ir)
    {
      maxSampleCount = std::max(maxSampleCount, ir->getSize());
    }
  }
  return maxSampleCount;
}


size_t PluginAudioProcessor::getMaxFileSampleCount() const
{
  juce::ScopedLock convolverLock(_convolverMutex);
  size_t maxSampleCount = 0;
  for (auto it=_agents.begin(); it!=_agents.end(); ++it)
  {
    const size_t sampleCount = (*it)->getFileSampleCount();
    if (sampleCount > maxSampleCount)
    {
      maxSampleCount = sampleCount;
    }
  }
  return maxSampleCount;
}


double PluginAudioProcessor::getMaxFileDuration() const
{
  juce::ScopedLock convolverLock(_convolverMutex);
  double maxDuration = 0.0;
  for (auto it=_agents.begin(); it!=_agents.end(); ++it)
  {
    const size_t sampleCount = (*it)->getFileSampleCount();
    const double sampleRate = (*it)->getFileSampleRate();
    const double duration = static_cast<double>(sampleCount) / sampleRate;
    if (duration > maxDuration)
    {
      maxDuration = duration;
    }
  }
  return maxDuration;
}


void PluginAudioProcessor::updateConvolvers()
{
  juce::ScopedLock irCalculationlock(_irCalculationMutex);
  if (_irCalculation)
  {
    _irCalculation->stopThread(-1);
    _irCalculation = nullptr;
  }
  _irCalculation = new IRCalculation(*this);
}
