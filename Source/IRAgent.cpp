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

#include "IRAgent.h"

#include "Parameters.h"
#include "PluginProcessor.h"
#include "SpinningScopedLock.h"

#include <algorithm>


IRAgent::IRAgent(PluginAudioProcessor& processor, size_t inputChannel, size_t outputChannel) :
  ChangeNotifier(),
  _processor(processor),
  _inputChannel(inputChannel),
  _outputChannel(outputChannel),
  _mutex(),
  _file(File::nonexistent),
  _fileSampleCount(0),
  _fileChannelCount(0),
  _fileSampleRate(0.0),
  _fileChannel(0),
  _irBuffer(nullptr),
  _convolverMutex(),
  _convolver(nullptr),
  _fadeFactor(0.0),
  _fadeIncrement(0.0),
  _eqLo(CookbookEq::LoShelf, Parameters::EqLowFreq.getMinValue(), 1.0f),
  _eqHi(CookbookEq::HiShelf, Parameters::EqLowFreq.getMaxValue(), 1.0f)
{
  initialize();
}


IRAgent::~IRAgent()
{
  clear();
}


PluginAudioProcessor& IRAgent::getProcessor() const
{
  return _processor;
}


size_t IRAgent::getInputChannel() const
{
  return _inputChannel;
}


size_t IRAgent::getOutputChannel() const
{
  return _outputChannel;
}


void IRAgent::initialize()
{
  const float eqSampleRate = static_cast<float>(_processor.getConvolverSampleRate());
  const size_t eqBlockSize = _processor.getConvolverHeadBlockSize();
  
  _eqLo.setFreq(_processor.getParameter(Parameters::EqLowFreq));
  _eqHi.setFreq(_processor.getParameter(Parameters::EqHighFreq));
  
  _eqLo.prepareToPlay(eqSampleRate, eqBlockSize);
  _eqHi.prepareToPlay(eqSampleRate, eqBlockSize);
}


void IRAgent::clear()
{
  fadeOut();
  waitForFadeOut();
  setConvolver(nullptr);
  {
    ScopedLock lock(_mutex);
    _file = File::nonexistent;
    _fileSampleCount = 0;
    _fileChannelCount = 0;
    _fileSampleRate = 0.0;
    _fileChannel = 0;
    _irBuffer = nullptr;
  }
  propagateChange();
}


void IRAgent::setFile(const File& file, size_t fileChannel)
{
  AudioFormatManager formatManager;
  formatManager.registerBasicFormats();
  ScopedPointer<AudioFormatReader> audioFormatReader(formatManager.createReaderFor(file));
  {
    ScopedLock lock(_mutex);
    if (audioFormatReader)
    {
      if (_file == file && _fileChannel == fileChannel)
      {
        return;
      }
      _file = file;
      _fileSampleCount = static_cast<size_t>(audioFormatReader->lengthInSamples);
      _fileChannelCount = static_cast<size_t>(audioFormatReader->numChannels);
      _fileSampleRate = audioFormatReader->sampleRate;
      _fileChannel = fileChannel;
    }
    else
    {
      _file = File::nonexistent;
      _fileSampleCount = 0;
      _fileChannelCount = 0;
      _fileSampleRate = 0.0;
      _fileChannel = 0;
    }
  }
  propagateChange();
  updateConvolver();
}


File IRAgent::getFile() const
{
  ScopedLock lock(_mutex);
  return _file;
}


size_t IRAgent::getFileChannelCount() const
{
  ScopedLock lock(_mutex);
  return _fileChannelCount;
}


size_t IRAgent::getFileSampleCount() const
{
  ScopedLock lock(_mutex);
  return _fileSampleCount;
}


double IRAgent::getFileSampleRate() const
{
  ScopedLock lock(_mutex);
  return _fileSampleRate;
}


size_t IRAgent::getFileChannel() const
{
  ScopedLock lock(_mutex);
  return _fileChannel;
}


FloatBuffer::Ptr IRAgent::getImpulseResponse() const
{
  ScopedLock lock(_mutex);
  return _irBuffer;
}


void IRAgent::updateConvolver()
{
  _processor.updateConvolvers();
}


void IRAgent::resetIR(const FloatBuffer::Ptr& irBuffer, Convolver* convolver)
{
  {
    ScopedLock lock(_mutex);
    _irBuffer = irBuffer;
  }

  {
    ScopedPointer<Convolver> conv(convolver);
    {
      // Make sure that the convolver mutex is locked as short as
      // possible and that all destruction and deallocation happens
      // outside of the lock because this might block the audio thread
      ScopedLock convolverLock(_convolverMutex);
      if (_convolver != conv)
      {
        _convolver.swapWith(conv);
      }
    }
  }

  propagateChange();
}


CriticalSection& IRAgent::getConvolverMutex()
{
  return _convolverMutex;
}


Convolver* IRAgent::getConvolver()
{
  return _convolver.get();
}
                  

void IRAgent::setConvolver(Convolver* convolver)
{
  ScopedPointer<Convolver> conv(convolver);
  {
    // Make sure that the convolver mutex is locked as short as
    // possible and that all destruction and deallocation happens
    // outside of the lock because this might block the audio thread
    ScopedLock convolverLock(_convolverMutex);
    if (_convolver != conv)
    {
      _convolver.swapWith(conv);
    }
  }
  conv = nullptr;
}


void IRAgent::propagateChange()
{
  notifyAboutChange();
  _processor.notifyAboutChange();
}


void IRAgent::process(const float* input, float* output, size_t len, float autoGain)
{
  const float Epsilon = 0.0001f;
  
  // Lock the convolver mutex by spinning because this method
  // is called in the realtime audio thread...
  SpinningScopedLock<juce::CriticalSection> convolverLock(_convolverMutex);
  
  if (_convolver && (_fadeFactor > Epsilon || ::fabs(_fadeIncrement) > Epsilon))
  {
    _convolver->process(input, output, len);

    if (::fabs(autoGain-1.0f) > 0.000001f)
    {
      for (size_t i=0; i<len; ++i)
      {
        output[i] *= autoGain;
      }
    }
    
    if (::fabs(_fadeIncrement) > Epsilon || _fadeFactor < (1.0-Epsilon))
    {
      for (size_t i=0; i<len; ++i)
      {
        _fadeFactor = std::max(0.0f, std::min(1.0f, _fadeFactor+_fadeIncrement));
        output[i] *= _fadeFactor;
      }
      if (_fadeFactor < Epsilon || _fadeFactor > (1.0-Epsilon))
      {
        _fadeIncrement = 0.0;
      }
    }
  }
  else
  {
    ::memset(output, 0, len * sizeof(float));
    _fadeFactor = 0.0;
    _fadeIncrement = 0.0;
  }
  
  if (_processor.getParameter(Parameters::EqLowOn))
  {
    _eqLo.setFreq(_processor.getParameter(Parameters::EqLowFreq));
    _eqLo.setGain(_processor.getParameter(Parameters::EqLowDecibels));
    _eqLo.filterOut(output, len);
  }
  
  if (_processor.getParameter(Parameters::EqHighOn))
  {
    _eqHi.setFreq(_processor.getParameter(Parameters::EqHighFreq));
    _eqHi.setGain(_processor.getParameter(Parameters::EqHighDecibels));
    _eqHi.filterOut(output, len);
  }
}


void IRAgent::fadeIn()
{
  _fadeIncrement = +0.005f;
}


void IRAgent::fadeOut()
{
  _fadeIncrement = -0.005f;
}


bool IRAgent::waitForFadeOut(size_t waitTimeMs)
{
  for (size_t i=0; i < waitTimeMs && _fadeFactor >= 0.0001; ++i)
  {
    Thread::sleep(1);
  }
  return (_fadeFactor < 0.0001);
}
