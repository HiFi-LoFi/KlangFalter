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

#include "Parameters.h"

#include "DecibelScaling.h"


const BoolParameterDescriptor Parameters::WetOn(0,
                                                "Wet On",
                                                "",
                                                ParameterDescriptor::Automatable,
                                                true);

const FloatParameterDescriptor Parameters::WetDecibels(1,
                                                       "Wet",
                                                       "dB",
                                                       ParameterDescriptor::Automatable,
                                                       0.0f,
                                                       DecibelScaling::MinScaleDb(),
                                                       DecibelScaling::MaxScaleDb());
                                              
const BoolParameterDescriptor Parameters::DryOn(2,
                                                "Dry On",
                                                "",
                                                ParameterDescriptor::Automatable,
                                                true);

const FloatParameterDescriptor Parameters::DryDecibels(3,
                                                       "Dry",
                                                       "dB",
                                                       ParameterDescriptor::Automatable,
                                                       0.0f,
                                                       DecibelScaling::MinScaleDb(),
                                                       DecibelScaling::MaxScaleDb());

const BoolParameterDescriptor Parameters::AutoGainOn(4,
                                                     "Autogain On",
                                                     "",
                                                     ParameterDescriptor::Automatable,
                                                     true);
 
const FloatParameterDescriptor Parameters::AutoGainDecibels(5,
                                                            "Autogain",
                                                            "dB",
                                                            ParameterDescriptor::NotAutomatable,
                                                            0.0f,
                                                            DecibelScaling::MinScaleDb(),
                                                            DecibelScaling::MaxScaleDb());

const BoolParameterDescriptor Parameters::EqLowOn(6,
                                                  "EQ Low On",
                                                  "",
                                                  ParameterDescriptor::Automatable,
                                                  true);

const FloatParameterDescriptor Parameters::EqLowFreq(7,
                                                     "EQ Low Freq",
                                                     "Hz",
                                                     ParameterDescriptor::Automatable,
                                                     20.0f,
                                                     20.0f,
                                                     2000.0f);

const FloatParameterDescriptor Parameters::EqLowDecibels(8,
                                                         "EQ Low Gain",
                                                         "dB",
                                                         ParameterDescriptor::Automatable,
                                                         0.0f,
                                                         -30.0f,
                                                         +30.0f);
                                                   
const FloatParameterDescriptor Parameters::EqLowQ(9,
                                                  "EQ Low Slope",
                                                  "",
                                                  ParameterDescriptor::Automatable,
                                                  1.0f,
                                                  1.0f,
                                                  2.0f);

const BoolParameterDescriptor Parameters::EqHighOn(10,
                                                   "EQ High On",
                                                   "",
                                                   ParameterDescriptor::Automatable,
                                                   true);
                                                   
const FloatParameterDescriptor Parameters::EqHighFreq(11,
                                                      "EQ High Freq",
                                                      "Hz",
                                                      ParameterDescriptor::Automatable,
                                                      20000.0f,
                                                      2000.0f,
                                                      20000.0f);
                                                     
const FloatParameterDescriptor Parameters::EqHighDecibels(12,
                                                          "EQ High Gain",
                                                          "dB",
                                                          ParameterDescriptor::Automatable,
                                                          0.0f,
                                                          -30.0,
                                                          +30.0);
                                                       
const FloatParameterDescriptor Parameters::EqHighQ(13,
                                                   "EQ High Slope",
                                                   "",
                                                   ParameterDescriptor::Automatable,
                                                   1.0f,
                                                   1.0f,
                                                   2.0f);
