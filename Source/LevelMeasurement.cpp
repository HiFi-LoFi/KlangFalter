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

#include "LevelMeasurement.h"

#include <algorithm>


LevelMeasurement::LevelMeasurement(float decay) :
  _level(0.0f),
  _decay(decay)
{
}


void LevelMeasurement::process(size_t len, const float* data0)
{
  if (len > 0)
  {
    float level = _level.get();
    if (data0)
    {
      for (size_t i=0; i<len; ++i)
      {
        const float val = data0[i];
        if (level < val)
        {
          level = val;
        }
        else if (level > 0.0001f)
        {
          level *= _decay;
        }
        else
        {
          level = 0.0f;
        }
      }
    }
    else
    {
      if (level > 0.0001f)
      {
        for (size_t i=0; i<len; ++i)
        {
          if (level > 0.0001f)
          {
            level *= _decay;
          }
          else
          {
            level = 0.0f;
            break;
          }
        }
      }
    }    
    _level.set(level);
  }
}


void LevelMeasurement::process(size_t len, const float* data0, const float* data1)
{
  if (len > 0 && data0 && data1)
  {
    float level = _level.get();
    for (size_t i=0; i<len; ++i)
    {
      const float val = std::max(data0[i], data1[i]);
      if (level < val)
      {
        level = val;
      }
      else if (level > 0.0001f)
      {
        level *= _decay;
      }
      else
      {
        level = 0.0f;
      }
    }
    _level.set(level);
  }
  else
  {
    process(len, data0);
  }
}


float LevelMeasurement::getLevel() const
{
  return _level.get();
}


void LevelMeasurement::reset()
{
  _level.set(0.0f);
}
