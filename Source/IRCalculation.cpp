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

#include "Convolver.h"
#include "DecibelScaling.h"
#include "IRCalculation.h"
#include "IRAgent.h"
#include "Parameters.h"
#include "PluginProcessor.h"

#include <algorithm>


class FloatBufferSource : public AudioSource
{
public:
  explicit FloatBufferSource(const FloatBuffer::Ptr& sourceBuffer) :
    juce::AudioSource(),
    _sourceBuffer(sourceBuffer),
    _pos(0)
  {
  }

  virtual void prepareToPlay(int /*samplesPerBlockExpected*/, double /*sampleRate*/)
  {
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


IRCalculation::IRCalculation(PluginAudioProcessor& processor) :
  juce::Thread("IRCalculation"),
  _processor(processor)
{
  startThread();
}


IRCalculation::~IRCalculation()
{
  stopThread(-1);
}


void IRCalculation::run()
{
  if (threadShouldExit())
  {
    return;
  }

  // Initiate fade out
  IRAgentContainer agents = _processor.getAgents();
  for (size_t i=0; i<agents.size(); ++i)
  {
    agents[i]->fadeOut();
  }
  
  // Import the files
  std::vector<FloatBuffer::Ptr> buffers(agents.size(), nullptr);
  std::vector<double> fileSampleRates(agents.size(), 0.0);
  const double fileBeginSeconds = _processor.getFileBeginSeconds();
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
  const double convolverSampleRate = _processor.getSampleRate();
  const double stretch = _processor.getStretch();
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
  const Envelope envelope = _processor.getEnvelope();
  const bool reverse = _processor.getReverse();
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
  const double predelayMs = _processor.getPredelayMs();
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
  const size_t headBlockSize = _processor.getConvolverHeadBlockSize();
  const size_t tailBlockSize = _processor.getConvolverTailBlockSize();
  _processor.setParameter(Parameters::AutoGainDecibels, DecibelScaling::Gain2Db(autoGain));
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



FloatBuffer::Ptr IRCalculation::importAudioFile(const File& file, size_t fileChannel, double fileBeginSeconds, double& fileSampleRate) const
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

void IRCalculation::reverseBuffer(FloatBuffer::Ptr& buffer) const
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

FloatBuffer::Ptr IRCalculation::changeSampleRate(const FloatBuffer::Ptr& inputBuffer, double inputSampleRate, double outputSampleRate) const
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


void IRCalculation::unifyBufferSize(std::vector<FloatBuffer::Ptr>& buffers) const
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
    if (threadShouldExit())
    {
      return;
    }
    if (buffers[i] && buffers[i]->getSize() < bufferSize)
    {
      FloatBuffer::Ptr buffer(new FloatBuffer(bufferSize));
      const size_t copySize = buffers[i]->getSize();
      const size_t padSize = bufferSize - copySize;
      ::memcpy(buffer->data(), buffers[i]->data(), copySize * sizeof(float));
      ::memset(buffer->data() + copySize, 0, padSize * sizeof(float));
      buffers[i] = buffer;
    }
  }
}


float IRCalculation::calculateAutoGain(const std::vector<FloatBuffer::Ptr>& buffers) const
{
  double autoGain = 1.0;
  for (size_t buffer=0; buffer<buffers.size(); ++buffer)
  {
    if (threadShouldExit())
    {
      return -1.0f;
    }
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
    }
  }
  return static_cast<float>(autoGain);
}

