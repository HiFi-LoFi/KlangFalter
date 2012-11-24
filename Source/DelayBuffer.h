#ifndef _DELAYBUFFER_H
#define _DELAYBUFFER_H

#include "RingBuffer.h"

#include <string>


template<typename T>
class DelayBuffer
{
public:
  explicit DelayBuffer(size_t delay = 0) :
    _delay(0),
    _buffer()
  {
    init(delay);
  }
  
  virtual ~DelayBuffer()
  {
    destroy();
  }
  
  void destroy()
  {
    _delay = 0;
    _buffer.destroy();
  }
  
  void init(size_t delay)
  {
    destroy();
    
    if (delay > 0)
    {
      _buffer.init(2 * delay, delay);
      _delay = delay;
    }
  }
  
  void proceed(T* data, size_t len)
  {
    if (_delay == 0 || len == 0)
    {
      return;
    }
    
    if (len <= _delay)
    {
      _buffer.write(data, len);
      _buffer.read(data, len);
    }
    else
    {
      const size_t offset = len - _delay;
      _buffer.write(data + offset, _delay);
      ::memmove(_buffer, data + _delay, offset * sizeof(T));
      _buffer.read(data, _delay);
    }
  }
  
private:
  size_t _delay;
  RingBuffer<T> _buffer;
};

#endif // Header guard
