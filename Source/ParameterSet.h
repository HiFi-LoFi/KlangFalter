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

#ifndef _PARAMETERSET_H
#define _PARAMETERSET_H

#include <algorithm>
#include <map>


/**
* @class ParameterDescriptor
* @brief Base class for parameter descriptors
*/
class ParameterDescriptor
{
public:
  virtual ~ParameterDescriptor()
  {    
  }
  
  int getIndex() const
  {
    return _index;
  }
  
  const juce::String& getName() const
  {
    return _name;
  }
  
  const juce::String& getUnit() const
  {
    return _unit;
  }
  
  enum AutomationStatus
  {
    Automatable,
    NotAutomatable
  };
  
  AutomationStatus getAutomationStatus() const
  {
    return _automationStatus;
  }
  
protected:
  ParameterDescriptor(int index,
                      const juce::String& name,
                      const juce::String& unit,
                      AutomationStatus automationStatus) :
    _index(index),
    _name(name),
    _unit(unit),
    _automationStatus(automationStatus)
  {
  }
  
private:
  const int _index;
  const juce::String _name;
  const juce::String _unit;
  const AutomationStatus _automationStatus;
};


// =====


/**
* @class TypedParameterDescriptor
* @brief Base class for parameter descriptors which have a specified type
*/
template<typename T>
class TypedParameterDescriptor : public ParameterDescriptor
{
public:
  typedef T ValueType;
  
  virtual float convertToNormalized(ValueType val) const = 0;
  
  virtual ValueType convertFromNormalized(float normalized) const = 0;
  
  virtual ValueType constraintValue(ValueType val) const = 0;
  
  ValueType getDefaultValue() const
  {
    return _defaultValue;
  }
  
  ValueType getMinValue() const
  {
    return _minValue;
  }
  
  ValueType getMaxValue() const
  {
    return _maxValue;
  }
  
protected:
  TypedParameterDescriptor(int index,
                           const juce::String& name,
                           const juce::String& unit,
                           ParameterDescriptor::AutomationStatus automationStatus,
                           T defaultValue,
                           T minValue,
                           T maxValue) :
    ParameterDescriptor(index, name, unit, automationStatus),
    _defaultValue(defaultValue),
    _minValue(minValue),
    _maxValue(maxValue)
  {
  }
  
private:
  const T _defaultValue;
  const T _minValue;
  const T _maxValue;
};


// =====


/**
* @class BoolParameterDescriptor
* @brief Parameter descriptor for boolean parameters
*/
class BoolParameterDescriptor : public TypedParameterDescriptor<bool>
{
public:
  BoolParameterDescriptor(int index,
                          const juce::String& name,
                          const juce::String& unit,
                          ParameterDescriptor::AutomationStatus automationStatus,
                          bool defaultValue,                
                          bool minValue = false,
                          bool maxValue = true) :
    TypedParameterDescriptor<bool>(index, name, unit, automationStatus, defaultValue, minValue, maxValue)
  {
  }
   
  virtual float convertToNormalized(bool val) const
  {
    return (val ? 1.0f : 0.0f);
  }
      
  virtual bool convertFromNormalized(float normalized) const
  {
    return (normalized >= 0.5f);
  }
  
  virtual bool constraintValue(bool val) const
  {
    return val;
  }
};

// =====


/**
* @class FloatParameterDescriptor
* @brief Parameter descriptor for floating point parameters
*/
class FloatParameterDescriptor : public TypedParameterDescriptor<float>
{
public:
  FloatParameterDescriptor(int index,
                           const juce::String& name,
                           const juce::String& unit,
                           ParameterDescriptor::AutomationStatus automationStatus,
                           float defaultValue,
                           float minValue,
                           float maxValue) :
    TypedParameterDescriptor<float>(index, name, unit, automationStatus, defaultValue, minValue, maxValue)
  {
  }
  
  virtual ~FloatParameterDescriptor()
  {
  }

  virtual float convertToNormalized(float val) const
  {
    const float minValue = getMinValue();
    const float maxValue = getMaxValue();
    const float range = maxValue - minValue;
    return (::fabs(range) > 0.0001f) ? ((val-minValue)/range) : 0.0f;
  }

  virtual float convertFromNormalized(float normalized) const
  {
    const float minValue = getMinValue();
    const float maxValue = getMaxValue();
    return minValue + normalized * (maxValue-minValue);
  }
  
  virtual float constraintValue(float val) const
  {
    return std::min(getMaxValue(), std::max(getMinValue(), val));
  }
};


// ============


/**
* @class ParameterSet
* @brief Container for managing a set of parameters
*/
class ParameterSet
{
public:
  ParameterSet() :
    _parameters()
  {
  }
  
  template<typename T>
  void registerParameter(const TypedParameterDescriptor<T>& parameter)
  {
    _parameters[parameter.getIndex()] = std::make_pair(&parameter, parameter.convertToNormalized(parameter.constraintValue(parameter.getDefaultValue())));
  }
  
  template<typename T>
  T getParameter(const TypedParameterDescriptor<T>& parameter) const
  {
    return parameter.convertFromNormalized(_parameters.find(parameter.getIndex())->second.second);
  }
  
  template<typename T>
  bool setParameter(const TypedParameterDescriptor<T>& parameter, T val)
  {
    return setNormalizedParameter(parameter.getIndex(), parameter.convertToNormalized(parameter.constraintValue(val)));
  }
  
  float getNormalizedParameter(int index) const
  {
    return _parameters.find(index)->second.second;
  }
  
  bool setNormalizedParameter(int index, float normalizedVal)
  {
    ParameterMap::iterator it = _parameters.find(index);
    if (::fabs(it->second.second - normalizedVal) > 0.00001f)
    {
      it->second.second = normalizedVal;
      return true;
    }
    return false;
  }
  
  const ParameterDescriptor& getParameterDescriptor(int index) const
  {
    return *(_parameters.find(index)->second.first);
  }
  
  size_t getParameterCount() const
  {
    return _parameters.size();
  }
  
private:
  typedef std::map<int, std::pair<const ParameterDescriptor*, float> > ParameterMap;
  ParameterMap _parameters;
  
  // Prevent uncontrolled usage
  ParameterSet(const ParameterSet&);
  ParameterSet& operator=(const ParameterSet&);
};

#endif // Header guard
