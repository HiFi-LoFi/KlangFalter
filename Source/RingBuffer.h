#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stddef.h>
#include <string.h>

#include <algorithm>


template<typename T>
class RingBuffer
{
public:
  explicit RingBuffer(size_t size, size_t initialFill = 0) :
    _size(0),
    _data(nullptr),
    _begin(0),
    _end(0)
  {
    init(size, initialFill);
  }


  virtual ~RingBuffer()
  {
    destroy();
  }
  
  
  void destroy()
  {
    delete _data;
    _data = nullptr;
    _size = 0;
    _begin = 0;
    _end = 0;
  }
  
  
  void init(size_t size, size_t initialFill = 0)
  {
    destroy();
    
    if (initialFill > size)
    {
      assert(false);
      return;
    }
    
    if (size > 0)
    {
      _data = new T[size+1];
      _size = size;
      _begin = 0;
      _end = 0;
      if (initialFill > 0)
      {
        _end = initialFill;
        ::memset(_data, 0, initialFill * sizeof(T));
      }
    }
  }


  size_t getSize() const
  {
    return _size - 1;
  }


  size_t getWriteSpace() const
  {
    size_t writeSpace = 0;
    size_t begin = _begin;
    size_t end = _end;
    if (begin == end)
    {
      writeSpace = _size - 1;
    }
    else if (end < begin)
    {
      writeSpace = begin - end - 1;
    }
    else
    {
      size_t space1 = (_size - 1) - end;
      size_t space2 = begin;
      writeSpace = space1 + space2;
    }
    return writeSpace;
  }


  size_t getReadSpace() const
  {
    size_t readSpace = 0;
    size_t begin = _begin;
    size_t end = _end;
    if (begin == end)
    {
      readSpace = 0;
    }
    else if (begin < end)
    {
      readSpace = end - begin;
    }
    else
    {
      size_t space1 = _size - begin;
      size_t space2 = end;
      readSpace = space1 + space2;
    }
    return readSpace;
  }


  size_t write(const T* data, size_t length)
  {
    size_t end = _end;
    size_t writeLen = std::min(getWriteSpace(), length);
    if (writeLen > 0)
    {
      if (writeLen <= _size - end)
      {
        memcpy(_data+end, data, writeLen*sizeof(T));
        _end += writeLen;
      }
      else
      {
        size_t writeLen1 = _size - end;
        size_t writeLen2 = writeLen - writeLen1;
        memcpy(_data+end, data, writeLen1*sizeof(T));
        memcpy(_data, data+writeLen1, writeLen2*sizeof(T));
        _end = writeLen2;
      }
    }
    return writeLen;
  }


  size_t read(T* data, size_t length)
  {
    size_t begin = _begin;
    size_t readLen = std::min(getReadSpace(), length);
    if (readLen > 0)
    {
      if (readLen <= _size - begin)
      {
        memcpy(data, _data+begin, readLen*sizeof(T));
        _begin += readLen;
      }
      else
      {
        size_t readLen1 = _size - begin;
        size_t readLen2 = readLen - readLen1;
        memcpy(data, _data+begin, readLen1*sizeof(T));
        memcpy(data+readLen1, _data, readLen2*sizeof(T));
        _begin = readLen2;
      }
    }
    return readLen;
  }

private:
  size_t _size;
  T* _data;
  volatile size_t _begin;
  volatile size_t _end;

  // Prevent uncontrolled usage
  RingBuffer(const RingBuffer&);
  RingBuffer& operator=(const RingBuffer&);
};

#endif // RINGBUFFER_H
