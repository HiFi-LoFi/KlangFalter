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

#include "Envelope.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "../JuceLibraryCode/JuceHeader.h"


Envelope::Envelope() :
  _nodes(),
  _reverse(false)
{
  reset();
}


Envelope::Envelope(double yLeft, double yRight) :
  _nodes(),
  _reverse(false)
{
  _nodes.push_back(Node(0.0, yLeft));
  _nodes.push_back(Node(1.0, yRight));
}


Envelope::Envelope(const Envelope& other) :
  _nodes(other._nodes),
  _reverse(other._reverse)
{
}


Envelope::~Envelope()
{
}


Envelope& Envelope::operator=(const Envelope& other)
{
  if (this != &other)
  {
    _nodes = other._nodes;
    _reverse = other._reverse;
  }
  return (*this);
}


bool Envelope::isNeutral() const
{
  return (_nodes.size() == 2 &&
          ::fabs(_nodes[0]._y-1.0) < 0.0000001 &&
          ::fabs(_nodes[1]._y-1.0) < 0.0000001);
}

void Envelope::setReverse(bool reverse)
{
  if (_reverse != reverse)
  {
    _reverse = reverse;
    std::reverse(_nodes.begin(), _nodes.end());
    for (size_t i=0; i<_nodes.size(); ++i)
    {
      _nodes[i]._x = 1.0 - _nodes[i]._x;
    }
  }
}


bool Envelope::getReverse() const
{
  return _reverse;
}


void Envelope::removeNode(size_t index)
{
  if (index > 0 && index < _nodes.size()-1)
  {
    _nodes.erase(_nodes.begin() + index);
  }
}


size_t Envelope::insertNode(double x, double y)
{ 
  x = std::min(1.0, std::max(0.0, x));
  y = std::min(1.0, std::max(0.0, y));
  size_t index = 1;
  while (index < _nodes.size()-1 && _nodes[index]._x < x)
  {
    ++index;
  }
  _nodes.insert(_nodes.begin()+index, Node(x, y));
  return index;
}


void Envelope::reset()
{
  _nodes.clear();
  _nodes.push_back(Node(0.0, 1.0));
  _nodes.push_back(Node(1.0, 1.0));
  _reverse = false;
}


size_t Envelope::getNodeCount() const
{
  return _nodes.size();
}


double Envelope::getX(size_t index) const
{
  if (index >_nodes.size())
  {
    throw std::out_of_range("Index out of range for accessing envelope nodes");
  }
  return _nodes[index]._x;
}


double Envelope::getY(size_t index) const
{
  if (index >_nodes.size())
  {
    throw std::out_of_range("Index out of range for accessing envelope nodes");
  }
  return _nodes[index]._y;
}


void Envelope::setX(size_t index, double x)
{
  if (index == 0 || index == _nodes.size()-1)
  {
    return;
  }
  
  if (index >_nodes.size())
  {
    throw std::out_of_range("Index out of range for accessing envelope nodes");
  }
  
  // Left restriction
  x = std::min(1.0, std::max(0.0, x));
  if (index > 0)
  {
    size_t i = index - 1;
    for (;;)
    {
      if (_nodes[i]._x <= x)
      {
        break;
      }
      _nodes[i]._x = x;
      if (i == 0)
      {
        break;
      }
      --i;
    }
  }
  
  // Right restriction
  for (size_t i=index+1; i<_nodes.size(); ++i)
  {
    if (_nodes[i]._x > x)
    {
      break;
    }
    _nodes[i]._x = x;
  }
  
  // Update the node
  _nodes[index]._x = x;
}


void Envelope::setY(size_t index, double y)
{ 
  if (index >_nodes.size())
  {
    throw std::out_of_range("Index out of range for accessing envelope nodes");
  }
  _nodes[index]._y = std::min(1.0, std::max(0.0, y));
}


void Envelope::render(float* envelope, size_t len) const
{
  // This method renders the gain envelope which corresponds
  // to a linear behaviour of the according decibel values. 
  size_t sample = 0;
  size_t node0 = 0;
  size_t node1 = 1;
  while (sample < len)
  {
    const double x0 = _nodes[node0]._x;
    const double x1 = _nodes[node1]._x;
    const double y0 = Decibels::gainToDecibels(_nodes[node0]._y);
    const double y1 = Decibels::gainToDecibels(_nodes[node1]._y);
    const double deltaX = x1 - x0;
    const double deltaY = y1 - y0;
    ++node0;
    ++node1;
    if (::fabs(deltaX) > 0.00000001)
    {
      double val = y0;
      double inc = deltaY / (deltaX * static_cast<double>(len));
      size_t rangeEnd = static_cast<size_t>(x1 * static_cast<double>(len));
      while (sample < rangeEnd)
      {
        envelope[sample] = static_cast<float>(Decibels::decibelsToGain(val));
        val += inc;
        ++sample;
      }
    }
  }
}


void Envelope::apply(float* data, size_t len) const
{
  std::vector<float> envelope(len);
  render(&envelope[0], len);
  for (size_t i=0; i<len; ++i)
  {
    data[i] *= envelope[i];
  }
}
