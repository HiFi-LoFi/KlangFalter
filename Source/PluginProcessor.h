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

#ifndef _PLUGINPROCESSOR_H
#define _PLUGINPROCESSOR_H

#include "../JuceLibraryCode/JuceHeader.h"

#include "IRManager.h"
#include "LevelMeasurement.h"
#include "Settings.h"
#include "FFTConvolver/Sample.h"

#include <map>
#include <vector>


//==============================================================================
/**
*/
class PluginAudioProcessor  : public AudioProcessor, public ChangeBroadcaster
{
public:
    //==============================================================================
    PluginAudioProcessor();
    virtual ~PluginAudioProcessor();

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void releaseResources();

    void processBlock(juce::AudioSampleBuffer& buffer, MidiBuffer& midiMessages);

    //==============================================================================
    juce::AudioProcessorEditor* createEditor();
    bool hasEditor() const;

    //==============================================================================
    const juce::String getName() const;

    int getNumParameters();

    float getParameter (int index);
    virtual void setParameter (int index, float newValue);

    const juce::String getParameterName (int index);
    const juce::String getParameterText (int index);

    const juce::String getInputChannelName (int channelIndex) const;
    const juce::String getOutputChannelName (int channelIndex) const;
    bool isInputChannelStereoPair (int index) const;
    bool isOutputChannelStereoPair (int index) const;

    bool acceptsMidi() const;
    bool producesMidi() const;

    //==============================================================================
    int getNumPrograms();
    int getCurrentProgram();
    void setCurrentProgram (int index);
    const juce::String getProgramName (int index);
    void changeProgramName (int index, const juce::String& newName);

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData);
    void setStateInformation(const void* data, int sizeInBytes);

  
    //==============================================================================    
    IRManager& getIRManager();
    const IRManager& getIRManager() const;
  
    double getSampleRate() const;
  
    enum ParameterId
    {
      WetOn,
      Wet,
      DryOn,
      Dry,
      AutoGainOn,
      AutoGain,
      EqLowOn,
      EqLowFreq,
      EqLowGainDb,
      EqLowQ,
      EqHighOn,
      EqHighFreq,
      EqHighGainDb,
      EqHighQ
    };
  
    float getLevelDry(size_t channel) const;

    Settings& getSettings();
  
private:
    juce::ScopedPointer<IRManager> _irManager; 
    double _sampleRate;
    juce::AudioSampleBuffer _wetBuffer;
  
    std::vector<fftconvolver::SampleBuffer*> _convolutionBuffers;
  
    class Parameter
    {
    public:
      Parameter() : _name(), _value(std::numeric_limits<float>::max())
      {
      }
      
      Parameter(const String& name, const float val) : _name(name), _value(val)
      {
      }
      
      const String& getName() const
      {
        return _name;
      }
      
      float getValue() const
      {
        return _value;
      }
      
      void setValue(const float newValue)
      {
        _value = newValue;
      } 
      
    private:
      String _name;
      float _value;
    };
  
    std::map<ParameterId, Parameter> _parameters;
  
    std::vector<LevelMeasurement> _levelMeasurementsDry;

    Settings _settings;
  
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginAudioProcessor);
};

#endif // Header guard
