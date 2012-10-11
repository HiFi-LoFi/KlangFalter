//
//  main.cpp
//  Convolution
//
//  Created by Uli Haberhauer on 16.01.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <algorithm>
#include <cstring>
#include <vector>

#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "../FFTConvolver.h"


template<typename T>
void SimpleConvolve(const T* input, size_t inLen, const T* ir, size_t irLen, T* output)
{
  if (irLen > inLen)
  {
    SimpleConvolve(ir, irLen, input, inLen, output);
    return;
  }
  
  ::memset(output, 0, (inLen+irLen-1) * sizeof(T));
  
  for (size_t n=0; n<irLen; ++n)
  {
    for (size_t m=0; m<=n; ++m)
    {
      output[n] += ir[m] * input[n-m];
    }
  }
  
  for (size_t n=irLen; n<inLen; ++n)
  {
    for (size_t m=0; m<irLen; ++m)
    {
      output[n] += ir[m] * input[n-m];
    }
  }
  
  for (size_t n=inLen; n<inLen+irLen-1; ++n)
  {
    for (size_t m=n-inLen+1; m<irLen; ++m)
    {
      output[n] += ir[m] * input[n-m];
    }
  }
}


static bool TestConvolver(size_t inputSize, size_t irSize, size_t blockSizeMin, size_t blockSizeMax, bool refCheck)
{
  // Prepare input and IR
  std::vector<fftconvolver::Sample> in(inputSize);
  for (size_t i=0; i<inputSize; ++i)
  {
    in[i] = ::cos(static_cast<float>(i) / 123.0);
  }
  
  std::vector<fftconvolver::Sample> ir(irSize);
  for (size_t i=0; i<irSize; ++i)
  {
    ir[i] = ::sin(static_cast<float>(i) / 79.0);
  }
  
  // Simple convolver
  std::vector<fftconvolver::Sample> outSimple(in.size() + ir.size() - 1, fftconvolver::Sample(0.0));
  if (refCheck)
  {
    SimpleConvolve(&in[0], in.size(), &ir[0], ir.size(), &outSimple[0]);
  }
  
  // Orgami convolver
  std::vector<fftconvolver::Sample> out(in.size() + ir.size() - 1, fftconvolver::Sample(0.0));
  {
    fftconvolver::FFTConvolver convolver;
    convolver.init(blockSizeMax, &ir[0], ir.size());
    std::vector<fftconvolver::Sample> inBuf(blockSizeMax);
    size_t processedOut = 0;
    size_t processedIn = 0;
    while (processedOut < out.size())
    {
      const size_t blockSize = blockSizeMin + (rand() % (1+(blockSizeMax-blockSizeMin))); 
      
      const size_t remainingOut = out.size() - processedOut;
      const size_t remainingIn = in.size() - processedIn;
      
      const size_t processingOut = std::min(remainingOut, blockSize);
      const size_t processingIn = std::min(remainingIn, blockSize);
      
      memset(&inBuf[0], 0, inBuf.size() * sizeof(fftconvolver::Sample));
      if (processingIn > 0)
      {
        memcpy(&inBuf[0], &in[processedIn], processingIn * sizeof(fftconvolver::Sample));
      }
      
      convolver.process(&inBuf[0], &out[processedOut], processingOut);
      
      processedOut += processingOut;
      processedIn += processingIn;
    }
  }
  
  if (refCheck)
  {
    size_t diffSamples = 0;
    for (size_t i=0; i<outSimple.size(); ++i)
    {
      const fftconvolver::Sample a = out[i];
      const fftconvolver::Sample b = outSimple[i];
      
      if (::fabs(a-b) > 0.05)
      {
        ++diffSamples;
      }
    }
    printf("Correctness Test (input %zu, IR %zu, blocksize %zu-%zu) => %s\n", inputSize, irSize, blockSizeMin, blockSizeMax, (diffSamples == 0) ? "[OK]" : "[FAILED]");
    return (diffSamples == 0);
  }
  else
  {
    printf("Performance Test (input %zu, IR %zu, blocksize %zu-%zu) => Completed\n", inputSize, irSize, blockSizeMin, blockSizeMax);
    return true;
  }
}



int main()
{ 
  // Correctness
  TestConvolver(1, 1, 1, 1, true);
  TestConvolver(2, 2, 2, 2, true);
  TestConvolver(3, 3, 3, 3, true);
  
  TestConvolver(3, 2, 2, 2, true);
  TestConvolver(4, 2, 2, 2, true);
  TestConvolver(4, 3, 2, 2, true);
  TestConvolver(9, 4, 3, 3, true);
  TestConvolver(171, 7, 5, 5, true);
  TestConvolver(1979, 17, 7, 7, true);
  TestConvolver(100, 10, 3, 5, true);
  TestConvolver(123, 45, 12, 34, true);
  
  TestConvolver(2, 3, 2, 2, true);
  TestConvolver(2, 4, 2, 2, true);
  TestConvolver(3, 4, 2, 2, true);
  TestConvolver(4, 9, 3, 3, true);
  TestConvolver(7, 171, 5, 5, true);
  TestConvolver(17, 1979, 7, 7, true);
  TestConvolver(10, 100, 3, 5, true);
  TestConvolver(45, 123, 12, 34, true);
  
  TestConvolver(100000, 1234, 100,  128, true);
  TestConvolver(100000, 1234, 100,  256, true);
  TestConvolver(100000, 1234, 100,  512, true);
  TestConvolver(100000, 1234, 100, 1024, true);
  TestConvolver(100000, 1234, 100, 2048, true);
  
  TestConvolver(100000, 4321, 100,  128, true);
  TestConvolver(100000, 4321, 100,  256, true);
  TestConvolver(100000, 4321, 100,  512, true);
  TestConvolver(100000, 4321, 100, 1024, true);
  TestConvolver(100000, 4321, 100, 2048, true);
  
  return 0;
}
