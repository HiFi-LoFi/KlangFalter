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

#ifndef _ASYNCMULTIPLYADD_H
#define _ASYNCMULTIPLYADD_H

#include "../JuceLibraryCode/JuceHeader.h"

#include "fftconvolver/MultiplyAdd.h"


class AsyncMultiplyAddEngine : public fftconvolver::MultiplyAddEngine
{
public:
  AsyncMultiplyAddEngine();
  virtual ~AsyncMultiplyAddEngine();
  
  virtual void init(const std::vector<fftconvolver::SplitComplex*>& ir);  
  virtual void setAudio(size_t index, const fftconvolver::SplitComplex& audio);
  virtual void multiplyAdd(const std::vector<Pair>& pairs);
  virtual const fftconvolver::SplitComplex& getResult();
  
private:
  friend class MultiplyAddThread;
  void asyncMultiplyAdd();
  
  juce::ScopedPointer<juce::Thread> _thread;
  juce::WaitableEvent _resultReady;
  std::vector<Pair> _pairs;
};


#endif // Header guard
