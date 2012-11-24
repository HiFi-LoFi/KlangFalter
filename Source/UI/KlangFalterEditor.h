/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  24 Nov 2012 9:05:28pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

#ifndef __JUCER_HEADER_KLANGFALTEREDITOR_KLANGFALTEREDITOR_C38F55DD__
#define __JUCER_HEADER_KLANGFALTEREDITOR_KLANGFALTEREDITOR_C38F55DD__

//[Headers]     -- You can add your own extra header files here --
#include "../JuceLibraryCode/JuceHeader.h"

#include "CustomLookAndFeel.h"
#include "DecibelScale.h"
#include "IRBrowserComponent.h"
#include "LevelMeter.h"
#include "SettingsDialogComponent.h"
#include "../PluginProcessor.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class KlangFalterEditor  : public AudioProcessorEditor,
                           public ChangeListener,
                           public Timer,
                           public SliderListener,
                           public ButtonListener
{
public:
    //==============================================================================
    KlangFalterEditor (PluginAudioProcessor* ownerFilter);
    ~KlangFalterEditor();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void processorChanged();
    void changeListenerCallback(ChangeBroadcaster* source);
    void timerCallback();
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void sliderValueChanged (Slider* sliderThatWasMoved);
    void buttonClicked (Button* buttonThatWasClicked);



    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    CustomLookAndFeel _customLookAndFeel;
    PluginAudioProcessor* _processor;
    juce::ScopedPointer<SettingsDialogComponent> _settingsDialog;
    //[/UserVariables]

    //==============================================================================
    DecibelScale* _decibelScale;
    TabbedComponent* _irTabComponent;
    Label* _stretchHeaderLabel;
    Slider* _stretchSlider;
    LevelMeter* _levelMeterDry;
    LevelMeter* _levelMeterWet;
    Label* _dryLevelLabel;
    Label* _wetLevelLabel;
    Slider* _drySlider;
    DecibelScale* _decibelScale2;
    Slider* _wetSlider;
    TextButton* _browseButton;
    IRBrowserComponent* _irBrowserComponent;
    TextButton* _settingsButton;
    Label* _stretchLabel;
    Label* _beginHeaderLabel;
    Slider* _beginSlider;
    Label* _beginLabel;
    TextButton* _wetButton;
    TextButton* _dryButton;
    TextButton* _autogainButton;
    TextButton* _reverseButton;
    Label* _predelayHeaderLabel;
    Slider* _predelaySlider;
    Label* _predelayLabel;


    //==============================================================================
    // (prevent copy constructor and operator= being generated..)
    KlangFalterEditor (const KlangFalterEditor&);
    const KlangFalterEditor& operator= (const KlangFalterEditor&);
};


#endif   // __JUCER_HEADER_KLANGFALTEREDITOR_KLANGFALTEREDITOR_C38F55DD__
