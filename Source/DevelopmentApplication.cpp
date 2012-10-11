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

#include "../JuceLibraryCode/JuceHeader.h"


// This is a dirty trick for switching easily between
// plug-in builds and development application builds
#ifndef JucePlugin_Name


#include "PluginProcessor.h"
#include "UI/KlangFalterEditor.h"

#include <vector>


// ==========================================================================


extern AudioProcessor* JUCE_CALLTYPE createPluginFilter();


// ==========================================================================


class AudioSourceProcessorPlayer : public AudioIODeviceCallback
{
public:
  AudioSourceProcessorPlayer() :
    AudioIODeviceCallback(),
    _audioSourcePlayer(),
    _audioProcessorPlayer(),
    _numberBuffers(0),
    _bufferSize(0),
    _buffers(nullptr)
  {
  }


  virtual ~AudioSourceProcessorPlayer()
  {
    assert(_numberBuffers == 0 && _bufferSize == 0 && _buffers == nullptr);
  }


  void init(AudioSource* audioSource, AudioProcessor* audioProcessor)
  {
    _audioSourcePlayer.setSource(audioSource);
    _audioProcessorPlayer.setProcessor(audioProcessor);
  }


  virtual void audioDeviceIOCallback(const float** inputChannelData,
    int numInputChannels,
    float** outputChannelData,
    int numOutputChannels,
    int numSamples)
  {
    assert(_buffers);
    assert(static_cast<size_t>(numInputChannels) <= _numberBuffers);
    assert(static_cast<size_t>(numOutputChannels) <= _numberBuffers);
    assert(static_cast<size_t>(numSamples) <= _bufferSize);

    _audioSourcePlayer.audioDeviceIOCallback(inputChannelData, numInputChannels, _buffers, numOutputChannels, numSamples);
    _audioProcessorPlayer.audioDeviceIOCallback(const_cast<const float**>(_buffers), numOutputChannels, outputChannelData, numOutputChannels, numSamples);
  }


  virtual void audioDeviceAboutToStart(AudioIODevice* device)
  {
    _numberBuffers = static_cast<size_t>(device->getNumBufferSizesAvailable());
    _buffers = new float*[_numberBuffers];
    for (size_t i=0; i<_numberBuffers; ++i)
    {
      _bufferSize = std::max(_bufferSize, static_cast<size_t>(device->getBufferSizeSamples(static_cast<int>(i))));
    }
    for (size_t i=0; i<_numberBuffers; ++i)
    {
      _buffers[i] = new float[_bufferSize];
    }

    _audioSourcePlayer.audioDeviceAboutToStart(device);
    _audioProcessorPlayer.audioDeviceAboutToStart(device);
  }


  virtual void audioDeviceStopped()
  { 
    _audioSourcePlayer.audioDeviceStopped();
    _audioProcessorPlayer.audioDeviceStopped();

    for (size_t i=0; i<_numberBuffers; ++i)
    {
      delete [] _buffers[i];
    }
    delete [] _buffers;
    _buffers = nullptr;
    _numberBuffers = 0;
    _bufferSize = 0;
  }


  virtual void audioDeviceError(const String& errorMessage)
  {
    _audioSourcePlayer.audioDeviceError(errorMessage);
    _audioProcessorPlayer.audioDeviceError(errorMessage);
  }

private:
  AudioSourcePlayer _audioSourcePlayer;
  AudioProcessorPlayer _audioProcessorPlayer;
  size_t _numberBuffers;
  size_t _bufferSize;
  float** _buffers;
};


// =======================================================


class DevelopmentApplicationWindow : public DocumentWindow
{
public:
    DevelopmentApplicationWindow() :
    DocumentWindow("KlangFalter - Development Application", Colours::black, DocumentWindow::minimiseButton | DocumentWindow::closeButton),
    _editor(nullptr),
    _deviceManager(),  
    _audioFileSource(nullptr),
    _audioProcessor(nullptr),
    _audioSourceProcessorPlayer()  
  {
    // Audio
    AudioFormatManager formatManager;
    FileChooser fileChooser("Choose a file for testing...",
                            File(),
                            formatManager.getWildcardForAllFormats(),
                            true);
    if (fileChooser.browseForFileToOpen() && fileChooser.getResults().size() == 1)
    {
      const File audioFile = fileChooser.getResults().getReference(0);
      formatManager.registerBasicFormats();
      AudioFormatReader* reader = formatManager.createReaderFor(audioFile);
      if (reader)
      { 
        // Audio file
        _audioFileSource = new AudioFormatReaderSource(reader, true);

        // Audio processor
        _audioProcessor = createPluginFilter();
        if (_audioProcessor)
        {
          // UI
          _editor = dynamic_cast<KlangFalterEditor*>(_audioProcessor->createEditor());
          if (_editor)
          {
            setContentNonOwned(_editor, true);  
          }

          _deviceManager.initialise(2, 2, nullptr, true);

          const AudioIODevice* currentDevice = _deviceManager.getCurrentAudioDevice();
          if (!currentDevice)
          {
            const OwnedArray<AudioIODeviceType>& availableDevices = _deviceManager.getAvailableDeviceTypes();
            if (availableDevices.size() > 0)
            {
              AudioIODeviceType* device = availableDevices[1];
              if (device)
              {
                _deviceManager.setCurrentAudioDeviceType(device->getTypeName(), true);
              }
            }
          }        
          _audioSourceProcessorPlayer.init(_audioFileSource, _audioProcessor);
          _deviceManager.addAudioCallback(&_audioSourceProcessorPlayer);

          // Try to reload last session
          const juce::File workingDirectory = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory();
          const juce::File sessionFile = workingDirectory.getChildFile("SavedSession.klangfalter");
          if (sessionFile.existsAsFile())
          {
            juce::MemoryBlock sessionData;
            if (sessionFile.loadFileAsData(sessionData))
            {
              _audioProcessor->setStateInformation(sessionData.getData(), sessionData.getSize());
            }
          }
        }
      }
    }  
  }


  virtual ~DevelopmentApplicationWindow()
  {
    _editor = nullptr;
    _deviceManager.closeAudioDevice();  
    _deviceManager.removeAudioCallback(&_audioSourceProcessorPlayer);
  }


  virtual void closeButtonPressed()
  {
    if (_audioProcessor)
    {
      // Try to reload last session
      const juce::File workingDirectory = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory();
      const juce::File sessionFile = workingDirectory.getChildFile("SavedSession.klangfalter");
      juce::MemoryBlock sessionData;
      _audioProcessor->getStateInformation(sessionData);
      sessionFile.replaceWithData(sessionData.getData(), sessionData.getSize());
    }

    JUCEApplication::quit();
  }

private:
  juce::ScopedPointer<KlangFalterEditor> _editor;  
  AudioDeviceManager _deviceManager;  
  juce::ScopedPointer<AudioFormatReaderSource> _audioFileSource;
  juce::ScopedPointer<AudioProcessor> _audioProcessor;
  AudioSourceProcessorPlayer _audioSourceProcessorPlayer;

};


//==============================================================================


class DevelopmentApplication : public JUCEApplication
{
public:

  DevelopmentApplication() :
    _applicationWindow(nullptr)
  {
  }
  
  virtual ~DevelopmentApplication()
  {
  }
  
  void initialise (const String&)
  {
    _applicationWindow = new DevelopmentApplicationWindow();
    _applicationWindow->setVisible(true);
  }
  
  void shutdown()
  {
    deleteAndZero(_applicationWindow);
  }

  void systemRequestedQuit()
  {
    quit();
  }
  
  const String getApplicationName()
  {
    return "Falter";
  }
  
  const String getApplicationVersion()
  {
    return ProjectInfo::versionString;
  }
  
  bool moreThanOneInstanceAllowed()
  {
    return true;
  }
  
  void anotherInstanceStarted (const String&)
  {
  }
  
private:
  DevelopmentApplicationWindow* _applicationWindow;
};


//==============================================================================


// This macro generates the main() routine that starts the app.
START_JUCE_APPLICATION(DevelopmentApplication)

#endif // JucePlugin_Name
