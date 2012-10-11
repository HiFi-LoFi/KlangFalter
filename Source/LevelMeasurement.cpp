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


LevelMeasurement::LevelMeasurement() :
  _level(0.0f),
  _decay(0.9999f)
{
}


void LevelMeasurement::process(const float* data, size_t len)
{
  float level = _level;
  for (size_t i=0; i<len; ++i)
  {
    const float val = data[i];
    if (level < val)
    {
      level = val;
    }
    else if (level > 0.0001)
    {
      level *= _decay;
    }
    else
    {
      level = 0.0f;
    }
  }
  _level = level;
}


float LevelMeasurement::getLevel() const
{
  return _level;
}


void LevelMeasurement::reset()
{
  _level = 0.0f;
}
