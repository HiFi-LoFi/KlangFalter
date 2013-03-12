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

#include "Processor.h"

#include "IRAgent.h"
#include "IRCalculation.h"
#include "Parameters.h"
#include "Persistence.h"
#include "Settings.h"
#include "UI/KlangFalterEditor.h"

#include <algorithm>


//==============================================================================
Processor::Processor() :
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
  _stereoWidth(),
  _dryOn(Parameters::DryOn.getDefaultValue() ? 1.0f : 0.0f),
  _wetOn(Parameters::WetOn.getDefaultValue() ? 1.0f : 0.0f),
  _dryGain(DecibelScaling::Db2Gain(Parameters::DryDecibels.getDefaultValue())),
  _wetGain(DecibelScaling::Db2Gain(Parameters::WetDecibels.getDefaultValue())),
  _beatsPerMinute(0.0f),
  _irCalculationMutex(),
  _irCalculation()
{ 
  _parameterSet.registerParameter(Parameters::WetOn);
  _parameterSet.registerParameter(Parameters::WetDecibels);
  _parameterSet.registerParameter(Parameters::DryOn);
  _parameterSet.registerParameter(Parameters::DryDecibels);
  _parameterSet.registerParameter(Parameters::AutoGainOn);
  _parameterSet.registerParameter(Parameters::AutoGainDecibels);
  _parameterSet.registerParameter(Parameters::EqLowFreq);
  _parameterSet.registerParameter(Parameters::EqLowDecibels);
  _parameterSet.registerParameter(Parameters::EqHighFreq);
  _parameterSet.registerParameter(Parameters::EqHighDecibels);
  _parameterSet.registerParameter(Parameters::StereoWidth);

  _agents.push_back(new IRAgent(*this, 0, 0));
  _agents.push_back(new IRAgent(*this, 0, 1));
  _agents.push_back(new IRAgent(*this, 1, 0));
  _agents.push_back(new IRAgent(*this, 1, 1));
}


Processor::~Processor()
{
  Processor::releaseResources();

  for (size_t i=0; i<_agents.size(); ++i)
  {
    delete _agents[i];
  }
  _agents.clear();
}


//==============================================================================
const String Processor::getName() const
{
#ifdef JucePlugin_Name
  return JucePlugin_Name;
#else
  return String("Plugin Test");
#endif
}

int Processor::getNumParameters()
{
  return _parameterSet.getParameterCount();
}

float Processor::getParameter(int index)
{
  return _parameterSet.getNormalizedParameter(index);
}

void Processor::setParameter(int index, float newValue)
{
  if (_parameterSet.setNormalizedParameter(index, newValue))
  {
    notifyAboutChange();
  }
}

const String Processor::getParameterName(int index)
{
  return _parameterSet.getParameterDescriptor(index).getName();
}

const String Processor::getParameterText(int index)
{
  return _parameterSet.getParameterDescriptor(index).getUnit();
}

const String Processor::getInputChannelName(int channelIndex) const
{
  return String (channelIndex + 1);
}

const String Processor::getOutputChannelName(int channelIndex) const
{
  return String (channelIndex + 1);
}

bool Processor::isInputChannelStereoPair(int /*index*/) const
{
  return true;
}

bool Processor::isOutputChannelStereoPair(int /*index*/) const
{
  return true;
}

bool Processor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool Processor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool Processor::silenceInProducesSilenceOut() const
{
  return false;
}

void Processor::numChannelsChanged()
{
  juce::AudioProcessor::numChannelsChanged();
  notifyAboutChange();
}

int Processor::getNumPrograms()
{
  return 0;
}

int Processor::getCurrentProgram()
{
  return 0;
}

void Processor::setCurrentProgram (int /*index*/)
{
}

const String Processor::getProgramName (int /*index*/)
{
  return String::empty;
}

void Processor::changeProgramName (int /*index*/, const String& /*newName*/)
{
}


//==============================================================================
void Processor::prepareToPlay(double /*sampleRate*/, int samplesPerBlock)
{
  // Play safe to be clean
  releaseResources();

  // Prepare convolvers
  {
    juce::ScopedLock convolverLock(_convolverMutex);
    _convolverHeadBlockSize = 1;
    while (_convolverHeadBlockSize < static_cast<size_t>(samplesPerBlock))
    {
      _convolverHeadBlockSize *= 2;
    }
    _convolverTailBlockSize = static_cast<size_t>(std::min(samplesPerBlock, 8192));
    _convolverTailBlockSize = std::max(_convolverTailBlockSize, 2 * _convolverTailBlockSize);
  }

  // Prepare convolution buffers
  _wetBuffer.setSize(2, samplesPerBlock);
  _convolutionBuffer.resize(samplesPerBlock);

  // Initialize parameters
  _stereoWidth.initializeWidth(getParameter(Parameters::StereoWidth));

  // Initialize IR agents
  for (size_t i=0; i<_agents.size(); ++i)
  {
    _agents[i]->initialize();
  }

  notifyAboutChange();
  updateConvolvers();
}


void Processor::releaseResources()
{
  _wetBuffer.setSize(1, 0, false, true, false);
  _convolutionBuffer.clear();
  _beatsPerMinute.set(0);

  // De-initialize IR agents
  for (size_t i=0; i<_agents.size(); ++i)
  {
    _agents[i]->clear();
  }

  notifyAboutChange();
}


void Processor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& /*midiMessages*/)
{ 
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
    _stereoWidth.updateWidth(getParameter(Parameters::StereoWidth));
    _stereoWidth.process(_wetBuffer.getSampleData(0), _wetBuffer.getSampleData(1), samplesToProcess);
  }

  // Dry/wet gain
  {
    float dryGain0, dryGain1;
    _dryGain.updateValue(DecibelScaling::Db2Gain(getParameter(Parameters::DryDecibels)));
    _dryGain.getSmoothValues(samplesToProcess, dryGain0, dryGain1);
    buffer.applyGainRamp(0, samplesToProcess, dryGain0, dryGain1);
  }
  {
    float wetGain0, wetGain1;
    _wetGain.updateValue(DecibelScaling::Db2Gain(getParameter(Parameters::WetDecibels)));
    _wetGain.getSmoothValues(samplesToProcess, wetGain0, wetGain1);
    _wetBuffer.applyGainRamp(0, samplesToProcess, wetGain0, wetGain1);
  }

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
  {
    float dryOnGain0, dryOnGain1;
    _dryOn.updateValue(getParameter(Parameters::DryOn) ? 1.0f : 0.0f);
    _dryOn.getSmoothValues(samplesToProcess, dryOnGain0, dryOnGain1);
    buffer.applyGainRamp(0, samplesToProcess, dryOnGain0, dryOnGain1);
  }
  {
    float wetOnGain0, wetOnGain1;
    _wetOn.updateValue(getParameter(Parameters::WetOn) ? 1.0f : 0.0f);
    _wetOn.getSmoothValues(samplesToProcess, wetOnGain0, wetOnGain1);
    if (numOutputChannels > 0)
    {
      buffer.addFromWithRamp(0, 0, _wetBuffer.getSampleData(0), samplesToProcess, wetOnGain0, wetOnGain1);
    }
    if (numOutputChannels > 1)
    {
      buffer.addFromWithRamp(1, 0, _wetBuffer.getSampleData(1), samplesToProcess, wetOnGain0, wetOnGain1);
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

  // Update beats per minute info
  float beatsPerMinute = 0.0f;
  juce::AudioPlayHead* playHead = getPlayHead();
  if (playHead)
  {
    juce::AudioPlayHead::CurrentPositionInfo currentPositionInfo;
    if (playHead->getCurrentPosition(currentPositionInfo))
    {
      beatsPerMinute = static_cast<float>(currentPositionInfo.bpm);
    }
  }
  if (::fabs(_beatsPerMinute.exchange(beatsPerMinute)-beatsPerMinute) > 0.001f)
  {
    notifyAboutChange();
  }
}

//==============================================================================
bool Processor::hasEditor() const
{
  return true;
}

AudioProcessorEditor* Processor::createEditor()
{
  return new KlangFalterEditor(*this);
}

//==============================================================================
void Processor::getStateInformation (MemoryBlock& destData)
{
  const juce::File irDirectory = _settings.getImpulseResponseDirectory();
  juce::ScopedPointer<juce::XmlElement> element(SaveState(irDirectory, *this));
  if (element)
  {
    copyXmlToBinary(*element, destData);
  }
}


void Processor::setStateInformation (const void* data, int sizeInBytes)
{
  juce::ScopedPointer<juce::XmlElement> element(getXmlFromBinary(data, sizeInBytes));
  if (element)
  {
    const juce::File irDirectory = _settings.getImpulseResponseDirectory();
    LoadState(irDirectory, *element, *this);
  }
}


float Processor::getLevelDry(size_t channel) const
{
  return (channel < _levelMeasurementsDry.size()) ? _levelMeasurementsDry[channel].getLevel() : 0.0f;
}


float Processor::getLevelWet(size_t channel) const
{
  return (channel < _levelMeasurementsWet.size()) ? _levelMeasurementsWet[channel].getLevel() : 0.0f;
}


float Processor::getLevelOut(size_t channel) const
{
  return (channel < _levelMeasurementsOut.size()) ? _levelMeasurementsOut[channel].getLevel() : 0.0f;
}


Settings& Processor::getSettings()
{
  return _settings;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
  return new Processor();
}


// =============================================================================


IRAgent* Processor::getAgent(size_t inputChannel, size_t outputChannel) const
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


size_t Processor::getAgentCount() const
{
  return _agents.size();
}


IRAgentContainer Processor::getAgents() const
{
  return _agents;
}


void Processor::clearConvolvers()
{
  {
    juce::ScopedLock convolverLock(_convolverMutex);
    _reverse = false;
    _stretch = 1.0;
    _fileBeginSeconds = 0.0;
    _envelope.reset();
  }

  setParameterNotifyingHost(Parameters::EqLowFreq, Parameters::EqLowFreq.getDefaultValue());
  setParameterNotifyingHost(Parameters::EqLowDecibels, Parameters::EqLowDecibels.getDefaultValue());
  setParameterNotifyingHost(Parameters::EqHighFreq, Parameters::EqHighFreq.getDefaultValue());
  setParameterNotifyingHost(Parameters::EqHighDecibels, Parameters::EqHighDecibels.getDefaultValue());
  setParameterNotifyingHost(Parameters::StereoWidth, Parameters::StereoWidth.getDefaultValue());

  for (size_t i=0; i<_agents.size(); ++i)
  {
    _agents[i]->clear();
  }

  notifyAboutChange();
  updateConvolvers();
}


void Processor::setStretch(double stretch)
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


double Processor::getStretch() const
{
  juce::ScopedLock convolverLock(_convolverMutex);
  return _stretch;
}


void Processor::setReverse(bool reverse)
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


bool Processor::getReverse() const
{
  juce::ScopedLock convolverLock(_convolverMutex);
  return _reverse;
}


void Processor::setEnvelope(const Envelope& envelope)
{
  {
    juce::ScopedLock convolverLock(_convolverMutex);
    _envelope = envelope;
  }
  notifyAboutChange();
  updateConvolvers();
}


Envelope Processor::getEnvelope() const
{
  juce::ScopedLock convolverLock(_convolverMutex);
  return _envelope;
}


void Processor::setPredelayMs(double predelayMs)
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


double Processor::getPredelayMs() const
{
  juce::ScopedLock convolverLock(_convolverMutex);
  return _predelayMs;
}


void Processor::setFileBeginSeconds(double fileBeginSeconds)
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


double Processor::getFileBeginSeconds() const
{
  juce::ScopedLock convolverLock(_convolverMutex);
  return std::max(0.0, std::min(getMaxFileDuration(), _fileBeginSeconds));
}


size_t Processor::getConvolverHeadBlockSize() const
{
  juce::ScopedLock convolverLock(_convolverMutex);
  return _convolverHeadBlockSize;
}


size_t Processor::getConvolverTailBlockSize() const
{
  juce::ScopedLock convolverLock(_convolverMutex);
  return _convolverTailBlockSize;
}


size_t Processor::getMaxIRSampleCount() const
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


size_t Processor::getMaxFileSampleCount() const
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


double Processor::getMaxFileDuration() const
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


void Processor::updateConvolvers()
{
  juce::ScopedLock irCalculationlock(_irCalculationMutex);
  if (_irCalculation)
  {
    _irCalculation->stopThread(-1);
    _irCalculation = nullptr;
  }
  _irCalculation = new IRCalculation(*this);
}


float Processor::getBeatsPerMinute() const
{
  return _beatsPerMinute.get();
}
