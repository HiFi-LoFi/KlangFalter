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

#include "StereoWidth.h"

#include <algorithm>
#include <cmath>


void StereoWidth(float width, float* left, float* right, size_t len)
{
  if (::fabs(width-1.0f) > 0.00001f)
  {
    const float tmp = 1.0f / std::max(1.0f + width, 2.0f);
    const float cm = 1.0f * tmp;
    const float cs = width * tmp;
    for (size_t i=0; i<len; ++i)
    {
      const float m = (right[i] + left[i]) * cm;
      const float s = (right[i] - left[i]) * cs;
      left[i] = m - s;
      right[i] = m + s;
    }
  }
}
