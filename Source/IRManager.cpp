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

#include "IRManager.h"

#include "Convolver.h"
#include "IRAgent.h"
#include "Parameters.h"
#include "PluginProcessor.h"
#include "Settings.h"


class FloatBufferSource : public AudioSource
{
public:
  explicit FloatBufferSource(const FloatBuffer::Ptr& sourceBuffer) :
    juce::AudioSource(),
    _sourceBuffer(sourceBuffer),
    _pos(0)
  {
  }

  virtual void prepareToPlay(int samplesPerBlockExpected, double sampleRate)
  {
    (void) samplesPerBlockExpected;
    (void) sampleRate;
    _pos = 0;
  }

  virtual void releaseResources()
  {
    _pos = 0;
  }

  virtual void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill)
  {
    AudioSampleBuffer* destBuffer = bufferToFill.buffer;
    const int len = std::min(bufferToFill.numSamples, static_cast<int>(_sourceBuffer->getSize()-_pos));
    if (destBuffer)
    {
      for (int channel=0; channel<destBuffer->getNumChannels(); ++channel)
      {
        if (channel == 0 && _sourceBuffer)
        {
          destBuffer->copyFrom(channel, bufferToFill.startSample, _sourceBuffer->data()+_pos, len);
          if (len < bufferToFill.numSamples)
          {
            const int startClear = bufferToFill.startSample + len;
            const int lenClear = bufferToFill.numSamples - len;
            destBuffer->clear(startClear, lenClear);
          }
        }
        else
        {
          destBuffer->clear(channel, bufferToFill.startSample, len);
        }
      }
    }
    _pos += len;
  }

private:
  const FloatBuffer::Ptr& _sourceBuffer;
  size_t _pos;

  FloatBufferSource(const FloatBufferSource&);
  FloatBufferSource& operator=(const FloatBufferSource&);
};


// ===================================================================




class IRCalculation : public juce::Thread
{
public:
  explicit IRCalculation(IRManager& irManager) :
    juce::Thread("IRCalculation"),
    _irManager(irManager)
  {
    startThread();
  }

  virtual ~IRCalculation()
  {
    stopThread(8000);
  }

  virtual void run()
  {
    if (threadShouldExit())
    {
      return;
    }

    // Initiate fade out
    IRAgentContainer agents = _irManager.getAgents();
    for (size_t i=0; i<agents.size(); ++i)
    {
      agents[i]->fadeOut();
    }
    
    // Import the files
    std::vector<FloatBuffer::Ptr> buffers(agents.size(), nullptr);
    std::vector<double> fileSampleRates(agents.size(), 0.0);
    const double fileBeginSeconds = _irManager.getFileBeginSeconds();
    for (size_t i=0; i<agents.size(); ++i)
    {
      const juce::File file = agents[i]->getFile();
      if (file != juce::File::nonexistent)
      {
        double sampleRate;
        FloatBuffer::Ptr buffer = importAudioFile(file, agents[i]->getFileChannel(), fileBeginSeconds, sampleRate);
        if (!buffer || sampleRate < 0.0001 || threadShouldExit())
        {
          return;
        }
        buffers[i] = buffer;
        fileSampleRates[i] = sampleRate;
      }
    }

    // Sample rate
    const double convolverSampleRate = _irManager.getConvolverSampleRate();
    const double stretch = _irManager.getStretch();
    const double stretchSampleRate = convolverSampleRate / stretch;
    for (size_t i=0; i<agents.size(); ++i)
    { 
      if (buffers[i] != nullptr && fileSampleRates[i] > 0.00001)
      {
        FloatBuffer::Ptr resampled = changeSampleRate(buffers[i], fileSampleRates[i], stretchSampleRate);
        if (!resampled || threadShouldExit())
        {
          return;
        }
        buffers[i] = resampled;
      }
    }

    // Unify buffer size
    unifyBufferSize(buffers);
    if (threadShouldExit())
    { 
      return;
    }

    // Calculate auto gain (should be done before applying the envelope!)
    const float autoGain = calculateAutoGain(buffers);
    if (threadShouldExit())
    {
      return;
    }

    // Reverse and envelope
    const Envelope envelope = _irManager.getEnvelope();
    const bool reverse = _irManager.getReverse();
    for (size_t i=0; i<buffers.size(); ++i)
    {
      if (buffers[i] != nullptr)
      {
        if (reverse)
        {
          reverseBuffer(buffers[i]);
          if (threadShouldExit())
          {
            return;
          }
        }
        envelope.apply(buffers[i]->data(), buffers[i]->getSize());
        if (threadShouldExit())
        {
          return;
        }
      }
    }
    
    // Predelay
    const double predelayMs = _irManager.getPredelayMs();
    const size_t predelaySamples = static_cast<size_t>((convolverSampleRate / 1000.0) * predelayMs);
    if (predelaySamples > 0)
    {
      for (size_t i=0; i<buffers.size(); ++i)
      {
        if (buffers[i] != nullptr)
        {
          FloatBuffer::Ptr buffer = new FloatBuffer(buffers[i]->getSize() + predelaySamples);
          ::memset(buffer->data(), 0, predelaySamples * sizeof(float));
          ::memcpy(buffer->data()+predelaySamples, buffers[i]->data(), buffers[i]->getSize() * sizeof(float));
          buffers[i] = buffer;
        }      
      }
    }
    
    // Update convolvers
    const size_t headBlockSize = _irManager.getConvolverBlockSize();
    const size_t tailBlockSize = 8192;
    _irManager.getProcessor().setParameter(Parameters::AutoGain, autoGain);
    for (size_t i=0; i<agents.size(); ++i)
    {
      juce::ScopedPointer<Convolver> convolver(new Convolver());
      if (buffers[i] != nullptr && buffers[i]->getSize() > 0)
      {        
        convolver = new Convolver();
        const bool successInit = convolver->init(headBlockSize, tailBlockSize, buffers[i]->data(), buffers[i]->getSize());
        if (!successInit || threadShouldExit())
        {
          return;
        }
      }

      while (!agents[i]->waitForFadeOut(1))
      {
        if (threadShouldExit())
        {
          return;
        }
      }
      agents[i]->resetIR(buffers[i], convolver.release());
      agents[i]->fadeIn();
    }
  }

private:
  FloatBuffer::Ptr importAudioFile(const File& file, size_t fileChannel, double fileBeginSeconds, double& fileSampleRate) const
  {
    fileSampleRate = 0.0;

    if (file == File::nonexistent)
    {
      return FloatBuffer::Ptr();
    }

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    ScopedPointer<AudioFormatReader> audioFormatReader(formatManager.createReaderFor(file));
    if (!audioFormatReader)
    {
      return FloatBuffer::Ptr();
    }

    const int fileChannels = audioFormatReader->numChannels;
    const int fileLen = static_cast<int>(audioFormatReader->lengthInSamples);
    if (static_cast<int>(fileChannel) >= fileChannels)
    {
      return FloatBuffer::Ptr();
    }

    const size_t startSample = static_cast<size_t>(std::max(0.0, fileBeginSeconds) * audioFormatReader->sampleRate);
    const size_t bufferSize = (startSample < static_cast<size_t>(fileLen)) ? (fileLen - startSample) : 0;
    FloatBuffer::Ptr buffer(new FloatBuffer(bufferSize));

    juce::AudioFormatReaderSource audioFormatReaderSource(audioFormatReader, false);
    audioFormatReaderSource.setNextReadPosition(static_cast<juce::int64>(startSample));
    size_t bufferPos = 0;
    int filePos = static_cast<int>(startSample);
    juce::AudioSampleBuffer importBuffer(fileChannels, 8192);
    while (filePos < fileLen)
    {
      if (threadShouldExit())
      {
        return FloatBuffer::Ptr();
      }
      int loading = std::min(importBuffer.getNumSamples(), fileLen-filePos);
      juce::AudioSourceChannelInfo info;
      info.buffer = &importBuffer;
      info.startSample = 0;
      info.numSamples = loading;
      audioFormatReaderSource.getNextAudioBlock(info);
      ::memcpy(buffer->data()+bufferPos, importBuffer.getSampleData(fileChannel), loading * sizeof(float));
      bufferPos += static_cast<size_t>(loading);
      filePos += loading;
    }

    fileSampleRate = audioFormatReader->sampleRate;
    return buffer;
  }

  void reverseBuffer(FloatBuffer::Ptr& buffer) const
  {
    if (buffer)
    {
      const size_t bufferSize = buffer->getSize();
      if (bufferSize > 0)
      {
        float* a = buffer->data();
        float* b = a + (bufferSize - 1);
        while (a < b)
        {
          std::swap(*a, *b);
          ++a;
          --b;
        }
      }
    }
  }

  FloatBuffer::Ptr changeSampleRate(const FloatBuffer::Ptr& inputBuffer, double inputSampleRate, double outputSampleRate) const
  {
    if (!inputBuffer)
    {
      return FloatBuffer::Ptr();
    }

    if (::fabs(outputSampleRate-inputSampleRate) < 0.0000001)
    {
      return inputBuffer;
    }

    assert(inputSampleRate >= 1.0);
    assert(outputSampleRate >= 1.0);

    const double samplesInPerOutputSample = outputSampleRate / inputSampleRate;
    const int inputSampleCount = inputBuffer->getSize();
    const int outputSampleCount = static_cast<int>(::ceil(static_cast<double>(inputSampleCount) / samplesInPerOutputSample));
    const int blockSize = 8192;

    FloatBufferSource inputSource(inputBuffer);
    ResamplingAudioSource resamplingSource(&inputSource, false, 1);
    resamplingSource.setResamplingRatio(samplesInPerOutputSample);
    resamplingSource.prepareToPlay(blockSize, outputSampleRate);

    FloatBuffer::Ptr outputBuffer(new FloatBuffer(outputSampleCount));
    juce::AudioSampleBuffer blockBuffer(1, blockSize);
    int processed = 0;
    while (processed < outputSampleCount)
    {
      if (threadShouldExit())
      {
        return FloatBuffer::Ptr();
      }

      const int remaining = outputSampleCount - processed;
      const int processing = std::min(blockSize, remaining);

      juce::AudioSourceChannelInfo info;
      info.buffer = &blockBuffer;
      info.startSample = 0;
      info.numSamples = processing;
      resamplingSource.getNextAudioBlock(info);      
      ::memcpy(outputBuffer->data()+static_cast<size_t>(processed), blockBuffer.getSampleData(0), processing * sizeof(float));
      processed += processing;
    }

    resamplingSource.releaseResources();

    return outputBuffer;
  }

  void unifyBufferSize(std::vector<FloatBuffer::Ptr>& buffers) const
  {
    size_t bufferSize = 0;
    for (size_t i=0; i<buffers.size(); ++i)
    {
      if (buffers[i] != nullptr)
      {
        bufferSize = std::max(bufferSize, buffers[i]->getSize());
      }
    }
    for (size_t i=0; i<buffers.size(); ++i)
    {
      if (buffers[i] && buffers[i]->getSize() < bufferSize)
      {
        FloatBuffer::Ptr buffer(new FloatBuffer(bufferSize));
        const size_t copySize = buffers[i]->getSize();
        const size_t padSize = bufferSize - copySize;
        ::memcpy(buffer->data(), buffers[i]->data(), copySize * sizeof(float));
        ::memset(buffer->data() + copySize, 0, padSize * sizeof(float));
        buffers[i] = buffer;
      }
      if (threadShouldExit())
      {
        return;
      }
    }
  }

  float calculateAutoGain(const std::vector<FloatBuffer::Ptr>& buffers) const
  {
    double autoGain = 1.0;
    for (size_t buffer=0; buffer<buffers.size(); ++buffer)
    {
      if (buffers[buffer] != nullptr)
      {
        const float* data = buffers[buffer]->data();
        const size_t len = buffers[buffer]->getSize();
        double sum = 0.0;
        for (size_t i=0; i<len; ++i)
        {
          const double val = static_cast<double>(data[i]);
          sum += val * val;
        }
        const double x = 1.0 / std::sqrt(sum);
        autoGain = std::min(autoGain, x);
        if (threadShouldExit())
        {
          return 0.0f;
        }
      }
    }
    return static_cast<float>(autoGain);
  }

  IRManager& _irManager;

  IRCalculation(const IRCalculation&);
  IRCalculation& operator=(const IRCalculation&);
};


// ==================================================


IRManager::IRManager(PluginAudioProcessor& processor, size_t inputChannels, size_t outputChannels) :
  ChangeBroadcaster(),
  _processor(processor),
  _agents(),
  _mutex(),
  _stretch(1.0),
  _reverse(false),
  _envelope(1.0, 1.0),
  _convolverSampleRate(0.0),
  _convolverBlockSize(processor.getSettings().getConvolverBlockSize()),
  _fileBeginSeconds(0.0),
  _predelayMs(0.0),
  _irCalculationMutex(),
  _irCalculation()
{
  for (size_t inputChannel=0; inputChannel<inputChannels; ++inputChannel)
  {
    for (size_t outputChannel=0; outputChannel<outputChannels; ++outputChannel)
    {
      _agents.push_back(new IRAgent(*this, inputChannel, outputChannel));
    }
  }
}


IRManager::~IRManager()
{
  for (size_t i=0; i<_agents.size(); ++i)
  {
    delete _agents[i];
  }
  _agents.clear();
}


PluginAudioProcessor& IRManager::getProcessor()
{
  return _processor;
}


const PluginAudioProcessor& IRManager::getProcessor() const
{
  return _processor;
}


void IRManager::initialize(double convolverSampleRate, size_t convolverBlockSize)
{
  bool changed = false;
  {
    juce::ScopedLock lock(_mutex);
    if (::fabs(_convolverSampleRate-convolverSampleRate) > 0.00000001 || _convolverBlockSize != convolverBlockSize)
    {
      _convolverSampleRate = convolverSampleRate;
      _convolverBlockSize = convolverBlockSize;
      changed = true;
    }
  }
  if (changed)
  {
    for (size_t i=0; i<_agents.size(); ++i)
    {
      _agents[i]->initialize();
    }
    sendChangeMessage();
    updateConvolvers();
  }
}


IRAgent* IRManager::getAgent(size_t inputChannel, size_t outputChannel) const
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


size_t IRManager::getAgentCount() const
{
  return _agents.size();
}


IRAgentContainer IRManager::getAgents() const
{
  return _agents;
}


void IRManager::reset()
{
  {
    juce::ScopedLock lock(_mutex);
    _reverse = false;
    _stretch = 1.0;
    _fileBeginSeconds = 0.0;
    _envelope.reset();
  }

  for (size_t i=0; i<_agents.size(); ++i)
  {
    _agents[i]->clear();
  }

  sendChangeMessage();
  updateConvolvers();
}


void IRManager::setStretch(double stretch)
{
  bool changed = false;
  {
    juce::ScopedLock lock(_mutex);
    if (::fabs(_stretch-stretch) > 0.000001)
    {
      _stretch = stretch;
      changed = true;
    }
  }
  if (changed)
  {
    sendChangeMessage();
    updateConvolvers();
  }
}


double IRManager::getStretch() const
{
  juce::ScopedLock lock(_mutex);
  return _stretch;
}


void IRManager::setReverse(bool reverse)
{
  bool changed = false;
  {
    juce::ScopedLock lock(_mutex);
    if (_reverse != reverse)
    {
      _reverse = reverse;
      _envelope.setReverse(reverse);    
      changed = true;
    }
  }
  if (changed)
  {
    sendChangeMessage();
    updateConvolvers();
  }
}


bool IRManager::getReverse() const
{
  juce::ScopedLock lock(_mutex);
  return _reverse;
}


void IRManager::setEnvelope(const Envelope& envelope)
{
  {
    juce::ScopedLock lock(_mutex);
    {
      _envelope = envelope;
    }
  }
  sendChangeMessage();
  updateConvolvers();
}


Envelope IRManager::getEnvelope() const
{
  juce::ScopedLock lock(_mutex);
  return _envelope;
}


void IRManager::setPredelayMs(double predelayMs)
{
  bool changed = false;
  {
    juce::ScopedLock lock(_mutex);
    if (_predelayMs != predelayMs)
    {
      _predelayMs = predelayMs;
      changed = true;
    }
  }
  if (changed)
  {
    sendChangeMessage();
    updateConvolvers();
  }
}


double IRManager::getPredelayMs() const
{
  juce::ScopedLock lock(_mutex);
  return _predelayMs;
}


void IRManager::setFileBeginSeconds(double fileBeginSeconds)
{
  bool changed = false;
  {
    juce::ScopedLock lock(_mutex);
    if (_fileBeginSeconds != fileBeginSeconds)
    {
      _fileBeginSeconds = fileBeginSeconds;
      changed = true;
    }    
  }
  if (changed)
  {
    sendChangeMessage();
    updateConvolvers();
  }
}


double IRManager::getFileBeginSeconds() const
{
  juce::ScopedLock lock(_mutex);
  return std::max(0.0, std::min(getMaxFileDuration(), _fileBeginSeconds));
}


double IRManager::getConvolverSampleRate() const
{
  juce::ScopedLock lock(_mutex);
  return _convolverSampleRate;
}


size_t IRManager::getConvolverBlockSize() const
{
  juce::ScopedLock lock(_mutex);
  return _convolverBlockSize;
}


size_t IRManager::getMaxIRSampleCount() const
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


size_t IRManager::getMaxFileSampleCount() const
{
  juce::ScopedLock lock(_mutex);
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


double IRManager::getMaxFileDuration() const
{
  juce::ScopedLock lock(_mutex);
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


void IRManager::updateConvolvers()
{
  juce::ScopedLock irCalculationlock(_irCalculationMutex);
  if (_irCalculation)
  {
    _irCalculation->stopThread(8000);
    _irCalculation = nullptr;
  }
  _irCalculation = new IRCalculation(*this);
}
