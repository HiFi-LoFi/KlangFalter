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

#include "AsyncMultiplyAddEngine.h"


class MultiplyAddThread : public juce::Thread
{
public:
  explicit MultiplyAddThread(AsyncMultiplyAddEngine& multiplyAddEngine) :
    juce::Thread("MultiplyAddThread"),
    _multiplyAddEngine(multiplyAddEngine)
  {
    startThread();
  }
  
  
  virtual ~MultiplyAddThread()
  {
    notify();
    stopThread(1000);
  }
  
  
  virtual void run()
  {
    for (;;)
    {
      wait(-1);
      if (threadShouldExit())
      {
        return;
      }
      _multiplyAddEngine.asyncMultiplyAdd();
    }
  }
  
private:
  AsyncMultiplyAddEngine& _multiplyAddEngine;
  
  MultiplyAddThread(const MultiplyAddThread&);
  MultiplyAddThread& operator=(const MultiplyAddThread&);
};


// =================================================

AsyncMultiplyAddEngine::AsyncMultiplyAddEngine() :
  fftconvolver::MultiplyAddEngine(),
  _thread(),
  _resultReady(true),
  _pairs()
{
}


AsyncMultiplyAddEngine::~AsyncMultiplyAddEngine()
{
  _thread = nullptr;
  _resultReady.signal();
}


void AsyncMultiplyAddEngine::init(const std::vector<fftconvolver::SplitComplex*>& ir)
{ 
  _pairs.reserve(ir.size());
  _resultReady.signal();
  _thread = new MultiplyAddThread(*this);
  fftconvolver::MultiplyAddEngine::init(ir);
}


void AsyncMultiplyAddEngine::setAudio(size_t index, const fftconvolver::SplitComplex& audio)
{
  fftconvolver::MultiplyAddEngine::setAudio(index, audio);
}


void AsyncMultiplyAddEngine::multiplyAdd(const std::vector<Pair>& pairs)
{
  _resultReady.wait();
  _resultReady.reset();
  _pairs = pairs;
  _thread->notify();
}


const fftconvolver::SplitComplex& AsyncMultiplyAddEngine::getResult()
{
  _resultReady.wait();
  return fftconvolver::MultiplyAddEngine::getResult();
}


void AsyncMultiplyAddEngine::asyncMultiplyAdd()
{
  fftconvolver::MultiplyAddEngine::multiplyAdd(_pairs);
  _resultReady.signal();
}
