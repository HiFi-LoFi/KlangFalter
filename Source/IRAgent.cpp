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

#include "IRManager.h"
#include "Parameters.h"
#include "PluginProcessor.h"
#include "SpinningScopedLock.h"

#include <algorithm>


IRAgent::IRAgent(IRManager& manager, size_t inputChannel, size_t outputChannel) :
  ChangeNotifier(),
  _manager(manager),
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
  _levelMeasurement(),
  _eqLo(CookbookEq::LoShelf, Parameters::EqLowFreq.getMinValue(), Parameters::EqLowQ.getMinValue()),
  _eqHi(CookbookEq::HiShelf, Parameters::EqLowFreq.getMaxValue(), Parameters::EqLowQ.getMinValue())
{
  initialize();
}


IRAgent::~IRAgent()
{
  clear();
}


IRManager& IRAgent::getManager() const
{
  return _manager;
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
  PluginAudioProcessor& processor = _manager.getProcessor();
  const float loFreq = processor.getParameter(Parameters::EqLowFreq);
  const float loQ = processor.getParameter(Parameters::EqLowQ);
  const float hiFreq = processor.getParameter(Parameters::EqHighFreq);
  const float hiQ = processor.getParameter(Parameters::EqHighQ);
  _eqLo.setFreqAndQ(loFreq, loQ);
  _eqHi.setFreqAndQ(hiFreq, hiQ);
  
  const float eqSampleRate = static_cast<float>(_manager.getConvolverSampleRate());
  const size_t eqBlockSize = _manager.getConvolverBlockSize();
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
    _levelMeasurement.reset();
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
  _manager.updateConvolvers();
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
  _manager.notifyAboutChange();
}


void IRAgent::process(const float* input, float* output, size_t len, float autoGain)
{
  const float Epsilon = 0.0001f;
  
  PluginAudioProcessor& processor = _manager.getProcessor();
  
  const bool eqLoOn = processor.getParameter(Parameters::EqLowOn);
  float eqLoFreq = -1.0f;
  float eqLoQ = -1.0f;
  float eqLoDecibels = 0.0f;
  if (eqLoOn)
  {
    eqLoFreq = processor.getParameter(Parameters::EqLowFreq);
    eqLoQ = processor.getParameter(Parameters::EqLowQ);
    eqLoDecibels = processor.getParameter(Parameters::EqLowDecibels);
  }
  
  const bool eqHiOn = processor.getParameter(Parameters::EqHighOn);
  float eqHiFreq = -1.0f;
  float eqHiQ = -1.0f;
  float eqHiDecibels = 0.0f;
  if (eqHiOn)
  {
    eqHiFreq = processor.getParameter(Parameters::EqHighFreq);
    eqHiQ = processor.getParameter(Parameters::EqHighQ);
    eqHiDecibels = processor.getParameter(Parameters::EqHighDecibels);
  }
  
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
  
  if (eqLoOn)
  {
    _eqLo.setFreqAndQ(eqLoFreq, eqLoQ);
    _eqLo.setGain(eqLoDecibels);
    _eqLo.filterOut(output, len);
  }
  
  if (eqHiOn)
  {
    _eqHi.setFreqAndQ(eqHiFreq, eqHiQ);
    _eqHi.setGain(eqHiDecibels);
    _eqHi.filterOut(output, len);
  }
  
  _levelMeasurement.process(output, len);
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


float IRAgent::getLevel() const
{
  return _levelMeasurement.getLevel();
}

