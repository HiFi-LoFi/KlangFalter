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

#include "ChangeNotifier.h"
#include "Envelope.h"
#include "IRAgent.h"
#include "LevelMeasurement.h"
#include "Parameterset.h"
#include "Settings.h"
#include "FFTConvolver/Sample.h"

#include <map>
#include <vector>


//==============================================================================
/**
*/
class PluginAudioProcessor  : public AudioProcessor, public ChangeNotifier
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
    virtual float getParameter (int index);
    virtual void setParameter (int index, float newValue);
  
    template<typename T>
    T getParameter(const TypedParameterDescriptor<T>& parameter) const
    {
      return _parameterSet.getParameter(parameter);
    }
  
    template<typename T>
    void setParameter(const TypedParameterDescriptor<T>& parameter, T val)
    {
      if (_parameterSet.setParameter(parameter, val))
      {
        notifyAboutChange();
      }
    }
  
    template<typename T>
    void setParameterNotifyingHost(const TypedParameterDescriptor<T>& parameter, T val)
    {
      AudioProcessor::setParameterNotifyingHost(parameter.getIndex(), parameter.convertToNormalized(val));
    }
  
    const juce::String getParameterName (int index);
    const juce::String getParameterText (int index);

    const juce::String getInputChannelName (int channelIndex) const;
    const juce::String getOutputChannelName (int channelIndex) const;
    bool isInputChannelStereoPair (int index) const;
    bool isOutputChannelStereoPair (int index) const;

    bool acceptsMidi() const;
    bool producesMidi() const;
  
    bool silenceInProducesSilenceOut() const;
  
    void numChannelsChanged();

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
    float getLevelDry(size_t channel) const;
    float getLevelWet(size_t channel) const;

    Settings& getSettings();
  
  
    //==============================================================================
    size_t getConvolverHeadBlockSize() const;
    size_t getConvolverTailBlockSize() const;
  
    IRAgent* getAgent(size_t inputChannel, size_t outputChannel) const;
    size_t getAgentCount() const;
    IRAgentContainer getAgents() const;
    
    void reset();
  
    void setStretch(double stretch);
    double getStretch() const;
  
    void setReverse(bool reverse);
    bool getReverse() const;
  
    void setEnvelope(const Envelope& envelope);
    Envelope getEnvelope() const; 
  
    size_t getMaxIRSampleCount() const;
    size_t getMaxFileSampleCount() const;
    double getMaxFileDuration() const;
  
    void setFileBeginSeconds(double fileBeginSeconds);
    double getFileBeginSeconds() const;
  
    void setPredelayMs(double predelayMs);
    double getPredelayMs() const;
  
    void updateConvolvers();
  
private:
    juce::AudioSampleBuffer _wetBuffer;
    std::vector<fftconvolver::SampleBuffer*> _convolutionBuffers;
    ParameterSet _parameterSet;  
    std::vector<LevelMeasurement> _levelMeasurementsDry;
    std::vector<LevelMeasurement> _levelMeasurementsWet;
    Settings _settings;
  
    mutable juce::CriticalSection _convolverMutex;
    IRAgentContainer _agents;
    double _stretch;
    bool _reverse;
    Envelope _envelope;
    size_t _convolverHeadBlockSize;
    size_t _convolverTailBlockSize;
    double _fileBeginSeconds;
    double _predelayMs;
  
    mutable juce::CriticalSection _irCalculationMutex;
    juce::ScopedPointer<juce::Thread> _irCalculation;
  
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginAudioProcessor);
};

#endif // Header guard
