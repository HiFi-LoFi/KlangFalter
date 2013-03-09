/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  8 Mar 2013 4:35:37pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

#ifndef __JUCER_HEADER_KLANGFALTEREDITOR_KLANGFALTEREDITOR_C0B0AF22__
#define __JUCER_HEADER_KLANGFALTEREDITOR_KLANGFALTEREDITOR_C0B0AF22__

//[Headers]     -- You can add your own extra header files here --
#include "../JuceLibraryCode/JuceHeader.h"

#include "CustomLookAndFeel.h"
#include "DecibelScale.h"
#include "IRBrowserComponent.h"
#include "IRComponent.h"
#include "LevelMeter.h"
#include "SettingsDialogComponent.h"
#include "../Processor.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class KlangFalterEditor  : public AudioProcessorEditor,
                           public ChangeNotifier::Listener,
                           public ChangeListener,
                           public Timer,
                           public SliderListener,
                           public ButtonListener
{
public:
    //==============================================================================
    KlangFalterEditor (Processor& processor);
    ~KlangFalterEditor();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void updateUI();
    virtual void changeListenerCallback (ChangeBroadcaster* source);
    virtual void changeNotification();
    virtual void timerCallback();
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
    Processor& _processor;
    juce::ScopedPointer<SettingsDialogComponent> _settingsDialog;
    std::map<std::pair<size_t, size_t>, IRComponent*> _irComponents;
    //[/UserVariables]

    //==============================================================================
    DecibelScale* _decibelScaleDry;
    TabbedComponent* _irTabComponent;
    Label* _stretchHeaderLabel;
    Slider* _stretchSlider;
    LevelMeter* _levelMeterDry;
    Label* _dryLevelLabel;
    Label* _wetLevelLabel;
    Slider* _drySlider;
    DecibelScale* _decibelScaleOut;
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
    Label* _hiFreqLabel;
    Label* _hiGainLabel;
    Label* _hiGainHeaderLabel;
    Label* _hiFreqHeaderLabel;
    Slider* _hiGainSlider;
    Slider* _hiFreqSlider;
    Label* _loFreqLabel;
    Label* _loGainLabel;
    Label* _loGainHeaderLabel;
    Label* _loFreqHeaderLabel;
    Slider* _loGainSlider;
    Slider* _loFreqSlider;
    LevelMeter* _levelMeterOut;
    TextButton* _levelMeterOutLabelButton;
    Label* _levelMeterDryLabel;
    Label* _widthHeaderLabel;
    Slider* _widthSlider;
    Label* _widthLabel;
    Label* _hiEqLabel;
    Label* _loEqLabel;


    //==============================================================================
    // (prevent copy constructor and operator= being generated..)
    KlangFalterEditor (const KlangFalterEditor&);
    const KlangFalterEditor& operator= (const KlangFalterEditor&);
};


#endif   // __JUCER_HEADER_KLANGFALTEREDITOR_KLANGFALTEREDITOR_C0B0AF22__
