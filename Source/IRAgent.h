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

#ifndef _IRAGENT_H
#define _IRAGENT_H

#include "../JuceLibraryCode/JuceHeader.h"

#include "LevelMeasurement.h"

#include "FFTConvolver/FFTConvolver.h"

#include <vector>


// Forward declarations
class IRManager;


// ====================================================


class FloatBuffer : public juce::ReferenceCountedObject
{
public:
  typedef juce::ReferenceCountedObjectPtr<FloatBuffer> Ptr;

  explicit FloatBuffer(size_t size) :
    juce::ReferenceCountedObject(),
    _buffer(size)
  {
  }

  FloatBuffer(const float* data, size_t size) :
    juce::ReferenceCountedObject(),
    _buffer(data, data+size)
  {
  }

  size_t getSize() const
  {
    return _buffer.size();
  }

  float* data()
  {
    return _buffer.data();
  }

  const float* data() const
  {
    return _buffer.data();
  }
    
private:
  std::vector<float> _buffer;

  // Prevent uncontrolled usage
  FloatBuffer(const FloatBuffer&);
  FloatBuffer& operator=(const FloatBuffer&);
};


// ====================================================


class IRAgent : public ChangeBroadcaster
{
public:
  IRAgent(IRManager& manager, size_t inputChannel, size_t outputChannel);
  virtual ~IRAgent();
  
  // Manager
  IRManager& getManager() const;
  
  // Input/output
  size_t getInputChannel() const;
  size_t getOutputChannel() const;
  
  void clear();
  
  // IR File
  void setFile(const File& file, size_t fileChannel);
  File getFile() const;
  
  size_t getFileSampleCount() const;
  size_t getFileChannelCount() const;
  double getFileSampleRate() const;
  size_t getFileChannel() const;
  
  FloatBuffer::Ptr getImpulseResponse() const;
  
  void fadeIn();
  void fadeOut();
  bool waitForFadeOut(size_t waitTimeMs = 100);
  
  float getLevel() const;
  
  // Convolver
  void updateConvolver();
  void clearConvolver();
  void resetIR(const FloatBuffer::Ptr& irBuffer, fftconvolver::FFTConvolver* convolver);
  
  CriticalSection& getConvolverMutex();
  fftconvolver::FFTConvolver* getConvolver();
  void setConvolver(fftconvolver::FFTConvolver* convolver);
  
  void process(const float* input, float* output, size_t len, float autoGain);
  
private:
  void propagateChange();
  
  IRManager& _manager;
  size_t _inputChannel;
  size_t _outputChannel;
  
  CriticalSection _mutex;
  
  File _file;
  size_t _fileSampleCount;
  size_t _fileChannelCount;
  double _fileSampleRate;
  size_t _fileChannel;
  
  FloatBuffer::Ptr _irBuffer;
  
  CriticalSection _convolverMutex;
  ScopedPointer<fftconvolver::FFTConvolver> _convolver;
  
  float _fadeFactor;
  float _fadeIncrement;
  
  LevelMeasurement _levelMeasurement;
    
  // Prevent uncontrolled usage
  IRAgent(const IRAgent&);
  IRAgent& operator=(const IRAgent&);
};


#endif  // Header guard
