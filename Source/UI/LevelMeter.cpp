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

#include "LevelMeter.h"

#include "CustomLookAndFeel.h"
#include "../DecibelScaling.h"


LevelMeter::LevelMeter() :
  Component(),
  _levels(),
  _colourGradient()
{
}


LevelMeter::~LevelMeter()
{
}


void LevelMeter::paint(Graphics& g)
{
  const int w = getWidth();
  const int h = getHeight();

  const juce::Colour colour = CustomLookAndFeel::GetCustomLookAndFeel(this).getScaleColour();
  g.setColour(colour);
  
  if (!_levels.empty())
  {    
    const int wChannel = static_cast<int>(w / static_cast<float>(_levels.size()));
    for (size_t channel=0; channel<_levels.size(); ++channel)
    {
      const float level = _levels[channel];
      const int levelHeight = static_cast<int>(DecibelScaling::Gain2Scale(level) * h);
      const int x = channel * wChannel;
      const int y = h - levelHeight;
      
      if (levelHeight > 0)
      {
        g.setGradientFill(_colourGradient);
        g.fillRect(x, y, wChannel-1, levelHeight);
      }

      g.setColour(Colours::black);
      g.fillRect(x, 0, wChannel-1, h-levelHeight);
    }
  }  
}


void LevelMeter::resized()
{
  const CustomLookAndFeel& customLookAndFeel = CustomLookAndFeel::GetCustomLookAndFeel(this);
  const float h = static_cast<float>(getHeight());
  const double scalePosZeroDb = static_cast<double>(DecibelScaling::Db2Scale(0.0f));
  const double scalePosMinus40Db = static_cast<double>(DecibelScaling::Db2Scale(-40.0f));
  _colourGradient = ColourGradient(customLookAndFeel.getLevelColourClipping(), 0.0f, 0.0f,
                                   customLookAndFeel.getLevelColourMinusInfDb(), 0.0f, h-1.0f,
                                   false);  
  _colourGradient.addColour(0.999999 - scalePosZeroDb, customLookAndFeel.getLevelColourClipping());
  _colourGradient.addColour(1.000000 - scalePosZeroDb, customLookAndFeel.getLevelColourZeroDb());
  _colourGradient.addColour(1.000000 - scalePosMinus40Db, customLookAndFeel.getLevelColourMinus40Db());
}


void LevelMeter::setChannelCount(size_t channelCount)
{
  _levels.resize(channelCount, 0.0f);
  repaint();
}


void LevelMeter::setLevel(size_t channel, float level)
{
  bool needsRepaint = false;
  if (channel < _levels.size())
  {
    if (::fabs(_levels[channel]-level) > 0.000001)
    {
      _levels[channel] = level;
      needsRepaint = true;
    }
  }
  if (needsRepaint)
  {
    repaint();
  }
}
