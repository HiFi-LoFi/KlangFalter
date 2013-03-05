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

#ifndef _PARAMETERS_H
#define _PARAMETERS_H

#include "../JuceLibraryCode/JuceHeader.h"

#include "ParameterSet.h"


struct Parameters
{
  static const BoolParameterDescriptor WetOn;
  static const FloatParameterDescriptor WetDecibels;
  
  static const BoolParameterDescriptor DryOn;
  static const FloatParameterDescriptor DryDecibels;
  
  static const BoolParameterDescriptor AutoGainOn;
  static const FloatParameterDescriptor AutoGainDecibels;

  static const BoolParameterDescriptor EqLowOn;
  static const FloatParameterDescriptor EqLowFreq;
  static const FloatParameterDescriptor EqLowDecibels;
  
  static const BoolParameterDescriptor EqHighOn;
  static const FloatParameterDescriptor EqHighFreq;
  static const FloatParameterDescriptor EqHighDecibels;

  static const FloatParameterDescriptor StereoWidth;
};

#endif // Header guard
