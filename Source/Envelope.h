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

#ifndef _ENVELOPE_H
#define _ENVELOPE_H

#include <vector>


class Envelope
{
public:
  Envelope();
  Envelope(double yLeft, double yRight);
  Envelope(const Envelope& other);
  virtual ~Envelope();
  Envelope& operator=(const Envelope& other);
  
  size_t insertNode(double x, double y);
  void removeNode(size_t index);
  void reset();
  
  bool isNeutral() const;
  
  void setReverse(bool reverse);
  bool getReverse() const;
  
  size_t getNodeCount() const;
  double getX(size_t index) const;
  double getY(size_t index) const;
  void setX(size_t index, double x);
  void setY(size_t index, double y);
  
  void render(float* envelope, size_t len) const;
  void apply(float* data, size_t len) const;
  
private:
  struct Node
  {
    Node(double x, double y) : _x(x), _y(y)
    {
    }
    double _x;
    double _y;
  };
  
  std::vector<Node> _nodes;
  bool _reverse;
};


#endif // Header guard
