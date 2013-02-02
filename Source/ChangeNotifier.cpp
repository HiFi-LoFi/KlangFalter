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

#include "ChangeNotifier.h"


ChangeNotifier::ChangeNotifier(int notificationInterval) :
  juce::Timer(),
  _listenersMutex(),
  _listeners(),
  _changePending(0)
{
  startTimer(notificationInterval);
}
  

ChangeNotifier::~ChangeNotifier()
{
  stopTimer();
  
  juce::ScopedLock lock(_listenersMutex);
  _listeners.clear();
}
 

void ChangeNotifier::notifyAboutChange()
{
  _changePending.set(1);
}

 
void ChangeNotifier::addNotificationListener(ChangeNotifier::Listener* listener)
{
  if (listener)
  {
    juce::ScopedLock lock(_listenersMutex);
    if (std::find(_listeners.begin(), _listeners.end(), listener) == _listeners.end())
    {
      _listeners.push_back(listener);
    }
  }
}


void ChangeNotifier::removeNotificationListener(ChangeNotifier::Listener* listener)
{
  if (listener)
  {
    juce::ScopedLock lock(_listenersMutex);
    std::vector<Listener*>::iterator it = std::find(_listeners.begin(), _listeners.end(), listener);
    if (it != _listeners.end())
    {
      _listeners.erase(it);
    }
  }
}
  

void ChangeNotifier::timerCallback()
{
  if (_changePending.get() == 1)
  {
    {
      juce::ScopedLock lock(_listenersMutex);    
      // Work on a copy which makes it safe that the callback can add/remove listeners...
      std::vector<Listener*> listeners(_listeners); 
      for (size_t i=0; i<listeners.size(); ++i)
      {
        listeners[i]->changeNotification();
      }
    }
    _changePending.set(0);
  }
}
