/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  13 Oct 2012 12:04:16pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...

#include "../DecibelScaling.h"

//[/Headers]

#include "KlangFalterEditor.h"
#include "IRComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...


template<typename T>
T SnapValue(const T& val, const T& snapValue, const T& sensitivity)
{
  return (::fabs(val - snapValue) < sensitivity) ? snapValue : val;
}


//[/MiscUserDefs]

//==============================================================================
KlangFalterEditor::KlangFalterEditor (PluginAudioProcessor* ownerFilter)
    : AudioProcessorEditor(ownerFilter),
      _decibelScale (0),
      _irTabComponent (0),
      _stretchHeaderLabel (0),
      _stretchSlider (0),
      _reverseButton (0),
      _levelMeterDry (0),
      _levelMeterWet (0),
      _dryLevelLabel (0),
      _wetLevelLabel (0),
      _drySlider (0),
      _decibelScale2 (0),
      _wetSlider (0),
      _browseButton (0),
      _irBrowserComponent (0),
      _settingsButton (0),
      _stretchLabel (0),
      _beginHeaderLabel (0),
      _beginSlider (0),
      _beginLabel (0),
      _autoGainButton (0),
      _dryButton (0),
      _wetButton (0)
{
    addAndMakeVisible (_decibelScale = new DecibelScale());
    _decibelScale->setName (L"DecibelScale");

    addAndMakeVisible (_irTabComponent = new TabbedComponent (TabbedButtonBar::TabsAtTop));
    _irTabComponent->setTabBarDepth (30);
    _irTabComponent->addTab (L"1-1", Colour (0xffe5e5f0), new IRComponent(), true);
    _irTabComponent->addTab (L"1-2", Colour (0xffe5e5f0), new IRComponent(), true);
    _irTabComponent->addTab (L"2-1", Colour (0xffe5e5f0), new IRComponent(), true);
    _irTabComponent->addTab (L"2-2", Colour (0xffe5e5f0), new IRComponent(), true);
    _irTabComponent->setCurrentTabIndex (0);

    addAndMakeVisible (_stretchHeaderLabel = new Label (String::empty,
                                                        L"Stretch"));
    _stretchHeaderLabel->setFont (Font (15.0000f, Font::plain));
    _stretchHeaderLabel->setJustificationType (Justification::centred);
    _stretchHeaderLabel->setEditable (false, false, false);
    _stretchHeaderLabel->setColour (Label::textColourId, Colour (0xff202020));
    _stretchHeaderLabel->setColour (TextEditor::textColourId, Colour (0xff202020));
    _stretchHeaderLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (_stretchSlider = new Slider (String::empty));
    _stretchSlider->setRange (0, 2, 0);
    _stretchSlider->setSliderStyle (Slider::RotaryVerticalDrag);
    _stretchSlider->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
    _stretchSlider->setColour (Slider::thumbColourId, Colour (0xff8080c0));
    _stretchSlider->setColour (Slider::rotarySliderFillColourId, Colour (0xff8080c0));
    _stretchSlider->addListener (this);

    addAndMakeVisible (_reverseButton = new ToggleButton (L"ReverseButton"));
    _reverseButton->setButtonText (L"Reverse");
    _reverseButton->addListener (this);
    _reverseButton->setColour (ToggleButton::textColourId, Colour (0xff202020));

    addAndMakeVisible (_levelMeterDry = new LevelMeter());
    _levelMeterDry->setName (L"LevelMeterDry");

    addAndMakeVisible (_levelMeterWet = new LevelMeter());
    _levelMeterWet->setName (L"LevelMeterWet");

    addAndMakeVisible (_dryLevelLabel = new Label (L"DryLevelLabel",
                                                   L"-inf"));
    _dryLevelLabel->setFont (Font (15.0000f, Font::plain));
    _dryLevelLabel->setJustificationType (Justification::centredRight);
    _dryLevelLabel->setEditable (false, false, false);
    _dryLevelLabel->setColour (Label::textColourId, Colour (0xff202020));
    _dryLevelLabel->setColour (TextEditor::textColourId, Colours::black);
    _dryLevelLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (_wetLevelLabel = new Label (L"WetLevelLabel",
                                                   L"-inf"));
    _wetLevelLabel->setFont (Font (15.0000f, Font::plain));
    _wetLevelLabel->setJustificationType (Justification::centredRight);
    _wetLevelLabel->setEditable (false, false, false);
    _wetLevelLabel->setColour (Label::textColourId, Colour (0xff202020));
    _wetLevelLabel->setColour (TextEditor::textColourId, Colours::black);
    _wetLevelLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (_drySlider = new Slider (L"DrySlider"));
    _drySlider->setRange (0, 10, 0);
    _drySlider->setSliderStyle (Slider::LinearVertical);
    _drySlider->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
    _drySlider->addListener (this);

    addAndMakeVisible (_decibelScale2 = new DecibelScale());
    _decibelScale2->setName (L"DecibelScale");

    addAndMakeVisible (_wetSlider = new Slider (L"WetSlider"));
    _wetSlider->setRange (0, 10, 0);
    _wetSlider->setSliderStyle (Slider::LinearVertical);
    _wetSlider->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
    _wetSlider->addListener (this);

    addAndMakeVisible (_browseButton = new TextButton (String::empty));
    _browseButton->setButtonText (L"Show Browser");
    _browseButton->setConnectedEdges (Button::ConnectedOnBottom);
    _browseButton->addListener (this);
    _browseButton->setColour (TextButton::buttonOnColourId, Colour (0xffbcbcff));

    addAndMakeVisible (_irBrowserComponent = new IRBrowserComponent());

    addAndMakeVisible (_settingsButton = new TextButton (String::empty));
    _settingsButton->setButtonText (L"Settings");
    _settingsButton->setConnectedEdges (Button::ConnectedOnRight | Button::ConnectedOnTop);
    _settingsButton->addListener (this);

    addAndMakeVisible (_stretchLabel = new Label (String::empty,
                                                  L"100%"));
    _stretchLabel->setFont (Font (15.0000f, Font::plain));
    _stretchLabel->setJustificationType (Justification::centred);
    _stretchLabel->setEditable (false, false, false);
    _stretchLabel->setColour (Label::textColourId, Colour (0xff202020));
    _stretchLabel->setColour (TextEditor::textColourId, Colours::black);
    _stretchLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (_beginHeaderLabel = new Label (String::empty,
                                                      L"Begin"));
    _beginHeaderLabel->setFont (Font (15.0000f, Font::plain));
    _beginHeaderLabel->setJustificationType (Justification::centred);
    _beginHeaderLabel->setEditable (false, false, false);
    _beginHeaderLabel->setColour (Label::textColourId, Colour (0xff202020));
    _beginHeaderLabel->setColour (TextEditor::textColourId, Colour (0xff202020));
    _beginHeaderLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (_beginSlider = new Slider (String::empty));
    _beginSlider->setRange (0, 2, 0);
    _beginSlider->setSliderStyle (Slider::RotaryVerticalDrag);
    _beginSlider->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
    _beginSlider->setColour (Slider::thumbColourId, Colour (0xff8080c0));
    _beginSlider->setColour (Slider::rotarySliderFillColourId, Colour (0xff8080c0));
    _beginSlider->addListener (this);
    _beginSlider->setSkewFactor (0.5);

    addAndMakeVisible (_beginLabel = new Label (String::empty,
                                                L"0ms"));
    _beginLabel->setFont (Font (15.0000f, Font::plain));
    _beginLabel->setJustificationType (Justification::centred);
    _beginLabel->setEditable (false, false, false);
    _beginLabel->setColour (Label::textColourId, Colours::black);
    _beginLabel->setColour (TextEditor::textColourId, Colours::black);
    _beginLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (_autoGainButton = new ToggleButton (String::empty));
    _autoGainButton->setButtonText (L"Autogain 0.0dB");
    _autoGainButton->addListener (this);
    _autoGainButton->setToggleState (true, false);
    _autoGainButton->setColour (ToggleButton::textColourId, Colour (0xff202020));

    addAndMakeVisible (_dryButton = new ToggleButton (String::empty));
    _dryButton->setButtonText (L"Dry");
    _dryButton->addListener (this);
    _dryButton->setToggleState (true, false);
    _dryButton->setColour (ToggleButton::textColourId, Colour (0xff202020));

    addAndMakeVisible (_wetButton = new ToggleButton (String::empty));
    _wetButton->setButtonText (L"Wet");
    _wetButton->addListener (this);
    _wetButton->setToggleState (true, false);
    _wetButton->setColour (ToggleButton::textColourId, Colour (0xff202020));


    //[UserPreSize]

    setLookAndFeel(&_customLookAndFeel);

    //[/UserPreSize]

    setSize (760, 330);


    //[Constructor] You can add your own custom stuff here..

    _processor = ownerFilter;
    if (_processor)
    {
      IRManager& irManager = _processor->getIRManager();
      _processor->addChangeListener(this);
      irManager.addChangeListener(this);
      IRComponent* irComponent00 = dynamic_cast<IRComponent*>(_irTabComponent->getTabContentComponent(0));
      IRComponent* irComponent01 = dynamic_cast<IRComponent*>(_irTabComponent->getTabContentComponent(1));
      IRComponent* irComponent10 = dynamic_cast<IRComponent*>(_irTabComponent->getTabContentComponent(2));
      IRComponent* irComponent11 = dynamic_cast<IRComponent*>(_irTabComponent->getTabContentComponent(3));
      irComponent00->init(&irManager, 0, 0);
      irComponent01->init(&irManager, 0, 1);
      irComponent10->init(&irManager, 1, 0);
      irComponent11->init(&irManager, 1, 1);
      _levelMeterDry->setChannelCount(2);
      _levelMeterWet->setChannelCount(4);
      _irBrowserComponent->init(_processor);
      startTimer(100);
    }
    processorChanged();

    _browseButton->setClickingTogglesState(true);

    //[/Constructor]
}

KlangFalterEditor::~KlangFalterEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..

    if (_processor)
    {
      _processor->removeChangeListener(this);
      _processor->getIRManager().removeChangeListener(this);
    }

    //[/Destructor_pre]

    deleteAndZero (_decibelScale);
    deleteAndZero (_irTabComponent);
    deleteAndZero (_stretchHeaderLabel);
    deleteAndZero (_stretchSlider);
    deleteAndZero (_reverseButton);
    deleteAndZero (_levelMeterDry);
    deleteAndZero (_levelMeterWet);
    deleteAndZero (_dryLevelLabel);
    deleteAndZero (_wetLevelLabel);
    deleteAndZero (_drySlider);
    deleteAndZero (_decibelScale2);
    deleteAndZero (_wetSlider);
    deleteAndZero (_browseButton);
    deleteAndZero (_irBrowserComponent);
    deleteAndZero (_settingsButton);
    deleteAndZero (_stretchLabel);
    deleteAndZero (_beginHeaderLabel);
    deleteAndZero (_beginSlider);
    deleteAndZero (_beginLabel);
    deleteAndZero (_autoGainButton);
    deleteAndZero (_dryButton);
    deleteAndZero (_wetButton);


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void KlangFalterEditor::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colour (0xffa6a6b1));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void KlangFalterEditor::resized()
{
    _decibelScale->setBounds (580, 36, 32, 176);
    _irTabComponent->setBounds (12, 8, 542, 204);
    _stretchHeaderLabel->setBounds (104, 216, 84, 24);
    _stretchSlider->setBounds (104, 240, 84, 40);
    _reverseButton->setBounds (208, 8, 88, 24);
    _levelMeterDry->setBounds (616, 36, 12, 176);
    _levelMeterWet->setBounds (704, 36, 24, 176);
    _dryLevelLabel->setBounds (584, 216, 60, 24);
    _wetLevelLabel->setBounds (680, 216, 64, 24);
    _drySlider->setBounds (624, 28, 24, 192);
    _decibelScale2->setBounds (668, 36, 32, 176);
    _wetSlider->setBounds (724, 28, 24, 192);
    _browseButton->setBounds (12, 308, 736, 24);
    _irBrowserComponent->setBounds (12, 332, 736, 288);
    _settingsButton->setBounds (708, 0, 52, 16);
    _stretchLabel->setBounds (104, 276, 84, 24);
    _beginHeaderLabel->setBounds (16, 216, 84, 24);
    _beginSlider->setBounds (16, 240, 84, 40);
    _beginLabel->setBounds (16, 276, 84, 24);
    _autoGainButton->setBounds (436, 236, 134, 24);
    _dryButton->setBounds (596, 236, 52, 24);
    _wetButton->setBounds (696, 236, 56, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void KlangFalterEditor::sliderValueChanged (Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == _stretchSlider)
    {
        //[UserSliderCode__stretchSlider] -- add your slider handling code here..
        if (_processor)
        {
          double sliderVal = _stretchSlider->getValue();
          if (::fabs(sliderVal-1.0) < 0.025)
          {
            sliderVal = 1.0;
            _stretchSlider->setValue(1.0, false);
          }
          _processor->getIRManager().setStretch(sliderVal);
        }
        //[/UserSliderCode__stretchSlider]
    }
    else if (sliderThatWasMoved == _drySlider)
    {
        //[UserSliderCode__drySlider] -- add your slider handling code here..
        const float scale = static_cast<float>(_drySlider->getValue());
        const float gain = SnapValue(DecibelScaling::Scale2Gain(scale), 1.0f, 0.075f);
        if (_processor)
        {
          _processor->setParameterNotifyingHost(PluginAudioProcessor::Dry, gain);
        }
        //[/UserSliderCode__drySlider]
    }
    else if (sliderThatWasMoved == _wetSlider)
    {
        //[UserSliderCode__wetSlider] -- add your slider handling code here..
        const float scale = static_cast<float>(_wetSlider->getValue());
        const float gain = SnapValue(DecibelScaling::Scale2Gain(scale), 1.0f, 0.075f);
        if (_processor)
        {
          _processor->setParameterNotifyingHost(PluginAudioProcessor::Wet, gain);
        }
        //[/UserSliderCode__wetSlider]
    }
    else if (sliderThatWasMoved == _beginSlider)
    {
        //[UserSliderCode__beginSlider] -- add your slider handling code here..
        _processor->getIRManager().setFileBeginSeconds(_beginSlider->getValue());
        //[/UserSliderCode__beginSlider]
    }

    //[UsersliderValueChanged_Post]
    //[/UsersliderValueChanged_Post]
}

void KlangFalterEditor::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]

    const File presetDirectory = File::getCurrentWorkingDirectory();

    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == _reverseButton)
    {
        //[UserButtonCode__reverseButton] -- add your button handler code here..
        if (_processor)
        {
          _processor->getIRManager().setReverse(_reverseButton->getToggleState());
        }
        //[/UserButtonCode__reverseButton]
    }
    else if (buttonThatWasClicked == _browseButton)
    {
        //[UserButtonCode__browseButton] -- add your button handler code here..
        int browserHeight = 0;
        juce::String browseButtonText;
        if (_browseButton->getToggleState())
        {
          browserHeight = 300;
          browseButtonText = juce::String("Hide Browser");
        }
        else
        {
          browserHeight = 0;
          browseButtonText = juce::String("Show Browser");
        }
        setBounds(getBounds().withHeight(_browseButton->getY() + _browseButton->getHeight() + browserHeight));
        _irBrowserComponent->setBounds(_irBrowserComponent->getBounds().withHeight(browserHeight - 10));
        _browseButton->setButtonText(browseButtonText);
        //[/UserButtonCode__browseButton]
    }
    else if (buttonThatWasClicked == _settingsButton)
    {
        //[UserButtonCode__settingsButton] -- add your button handler code here..
        _settingsDialog = nullptr;
        _settingsDialog = new SettingsDialogComponent(*_processor);
        juce::DialogWindow::showDialog("Settings",
                                       _settingsDialog,
                                       this,
                                       juce::Colours::white,
                                       true);
        //[/UserButtonCode__settingsButton]
    }
    else if (buttonThatWasClicked == _autoGainButton)
    {
        //[UserButtonCode__autoGainButton] -- add your button handler code here..
        _processor->setParameter(PluginAudioProcessor::AutoGainOn, _autoGainButton->getToggleState() ? 1.0f : 0.0f);
        //[/UserButtonCode__autoGainButton]
    }
    else if (buttonThatWasClicked == _dryButton)
    {
        //[UserButtonCode__dryButton] -- add your button handler code here..
        _processor->setParameter(PluginAudioProcessor::DryOn, _dryButton->getToggleState() ? 1.0f : 0.0f);
        //[/UserButtonCode__dryButton]
    }
    else if (buttonThatWasClicked == _wetButton)
    {
        //[UserButtonCode__wetButton] -- add your button handler code here..
        _processor->setParameter(PluginAudioProcessor::WetOn, _wetButton->getToggleState() ? 1.0f : 0.0f);
        //[/UserButtonCode__wetButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...

void KlangFalterEditor::processorChanged()
{
  const IRManager* irManager = _processor ? &_processor->getIRManager() : nullptr;
  const double maxFileDuration = irManager ? irManager->getMaxFileDuration() : 0.0;
  const bool irAvailable = (maxFileDuration > 0.0);

  if (_stretchSlider)
  {
    const double stretch = irManager ? irManager->getStretch() : 1.0;
    _stretchSlider->setEnabled(irAvailable);
    _stretchSlider->setRange(0.5, 1.5);
    _stretchSlider->setValue(stretch, false, false);
    _stretchLabel->setText(String(static_cast<int>(100.0*stretch)) + String("%"), true);
  }

  if (_drySlider)
  {
    if (_processor)
    {
      const float gain = _processor->getParameter(PluginAudioProcessor::Dry);
      const float db = DecibelScaling::Gain2Db(gain);
      const float scale = DecibelScaling::Db2Scale(db);
      _drySlider->setEnabled(true);
      _drySlider->setRange(0.0, 1.0);
      _drySlider->setValue(scale, false, false);
      _dryLevelLabel->setText(DecibelScaling::DecibelString(db), true);
    }
    else
    {
      _drySlider->setEnabled(false);
    }
  }

  if (_wetSlider)
  {
    if (_processor)
    {
      const float gain = _processor->getParameter(PluginAudioProcessor::Wet);
      const float db = DecibelScaling::Gain2Db(gain);
      const float scale = DecibelScaling::Db2Scale(db);
      _wetSlider->setEnabled(true);
      _wetSlider->setRange(0.0, 1.0);
      _wetSlider->setValue(scale, false, false);
      _wetLevelLabel->setText(DecibelScaling::DecibelString(db), true);
    }
    else
    {
      _wetSlider->setEnabled(false);
    }
  }

  if (_reverseButton)
  {
    if (irManager)
    {
      _reverseButton->setEnabled(true);
      _reverseButton->setToggleState(irManager->getReverse(), false);
    }
    else
    {
      _reverseButton->setEnabled(false);
      _reverseButton->setToggleState(false, false);
    }
  }

  if (_beginSlider)
  {
    const double fileBeginSeconds = std::min(irManager->getFileBeginSeconds(), maxFileDuration);
    _beginSlider->setEnabled(irAvailable);
    if (maxFileDuration > 0.0)
    {
      _beginSlider->setRange(0.0, maxFileDuration);
    }
    _beginSlider->setValue(fileBeginSeconds, false, false);

    if (_beginLabel)
    {
      _beginLabel->setText(juce::String(fileBeginSeconds * 1000.0, 1) + juce::String("ms"), true);
    }
  }

  if (_autoGainButton)
  {
    const float autoGain = _processor->getParameter(PluginAudioProcessor::AutoGain);
    const float autoGainOn = _processor->getParameter(PluginAudioProcessor::AutoGainOn);
    const juce::String autoGainText = DecibelScaling::DecibelString(DecibelScaling::Gain2Db(autoGain));
    _autoGainButton->setButtonText(juce::String("Autogain ") + autoGainText);
    _autoGainButton->setToggleState(autoGainOn > 0.5, false);
  }

  if (_dryButton)
  {
    _dryButton->setToggleState(_processor->getParameter(PluginAudioProcessor::DryOn) > 0.5, false);
  }

  if (_wetButton)
  {
    _wetButton->setToggleState(_processor->getParameter(PluginAudioProcessor::WetOn) > 0.5, false);
  }
}


void KlangFalterEditor::changeListenerCallback(ChangeBroadcaster* /*source*/)
{
  processorChanged();
}


void KlangFalterEditor::timerCallback()
{
  float levelDry1 = 0.0f;
  float levelDry2 = 0.0f;
  float levelWet00 = 0.0f;
  float levelWet01 = 0.0f;
  float levelWet10 = 0.0f;
  float levelWet11 = 0.0f;
  if (_processor)
  {
    IRManager& irManager = _processor->getIRManager();
    levelDry1 = _processor->getLevelDry(0);
    levelDry2 = _processor->getLevelDry(1);
    levelWet00 = irManager.getAgent(0, 0)->getLevel();
    levelWet01 = irManager.getAgent(0, 1)->getLevel();
    levelWet10 = irManager.getAgent(1, 0)->getLevel();
    levelWet11 = irManager.getAgent(1, 1)->getLevel();
  }
  _levelMeterDry->setLevel(0, levelDry1);
  _levelMeterDry->setLevel(1, levelDry2);
  _levelMeterWet->setLevel(0, levelWet00);
  _levelMeterWet->setLevel(1, levelWet01);
  _levelMeterWet->setLevel(2, levelWet10);
  _levelMeterWet->setLevel(3, levelWet11);
}


//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="KlangFalterEditor" componentName=""
                 parentClasses="public AudioProcessorEditor, public ChangeListener, public Timer"
                 constructorParams="PluginAudioProcessor* ownerFilter" variableInitialisers="AudioProcessorEditor(ownerFilter)"
                 snapPixels="4" snapActive="1" snapShown="1" overlayOpacity="0.330000013"
                 fixedSize="1" initialWidth="760" initialHeight="330">
  <BACKGROUND backgroundColour="ffa6a6b1"/>
  <GENERICCOMPONENT name="DecibelScale" id="6dd7ac2ee661b784" memberName="_decibelScale"
                    virtualName="" explicitFocusOrder="0" pos="580 36 32 176" class="DecibelScale"
                    params=""/>
  <TABBEDCOMPONENT name="IRTabComponent" id="697fc3546f1ab7f1" memberName="_irTabComponent"
                   virtualName="" explicitFocusOrder="0" pos="12 8 542 204" orientation="top"
                   tabBarDepth="30" initialTab="0">
    <TAB name="1-1" colour="ffe5e5f0" useJucerComp="1" contentClassName=""
         constructorParams="" jucerComponentFile="IRComponent.cpp"/>
    <TAB name="1-2" colour="ffe5e5f0" useJucerComp="1" contentClassName=""
         constructorParams="" jucerComponentFile="IRComponent.cpp"/>
    <TAB name="2-1" colour="ffe5e5f0" useJucerComp="1" contentClassName=""
         constructorParams="" jucerComponentFile="IRComponent.cpp"/>
    <TAB name="2-2" colour="ffe5e5f0" useJucerComp="1" contentClassName=""
         constructorParams="" jucerComponentFile="IRComponent.cpp"/>
  </TABBEDCOMPONENT>
  <LABEL name="" id="ff104b46d553eb03" memberName="_stretchHeaderLabel"
         virtualName="" explicitFocusOrder="0" pos="104 216 84 24" textCol="ff202020"
         edTextCol="ff202020" edBkgCol="0" labelText="Stretch" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="36"/>
  <SLIDER name="" id="e6fe992b37e74eba" memberName="_stretchSlider" virtualName=""
          explicitFocusOrder="0" pos="104 240 84 40" thumbcol="ff8080c0"
          rotarysliderfill="ff8080c0" min="0" max="2" int="0" style="RotaryVerticalDrag"
          textBoxPos="NoTextBox" textBoxEditable="1" textBoxWidth="80"
          textBoxHeight="20" skewFactor="1"/>
  <TOGGLEBUTTON name="ReverseButton" id="7f5e152dc8dfd08c" memberName="_reverseButton"
                virtualName="" explicitFocusOrder="0" pos="208 8 88 24" txtcol="ff202020"
                buttonText="Reverse" connectedEdges="0" needsCallback="1" radioGroupId="0"
                state="0"/>
  <GENERICCOMPONENT name="LevelMeterDry" id="93270230a2db62e0" memberName="_levelMeterDry"
                    virtualName="" explicitFocusOrder="0" pos="616 36 12 176" class="LevelMeter"
                    params=""/>
  <GENERICCOMPONENT name="LevelMeterWet" id="e4867bf99a47726a" memberName="_levelMeterWet"
                    virtualName="" explicitFocusOrder="0" pos="704 36 24 176" class="LevelMeter"
                    params=""/>
  <LABEL name="DryLevelLabel" id="892bd8ba7f961215" memberName="_dryLevelLabel"
         virtualName="" explicitFocusOrder="0" pos="584 216 60 24" textCol="ff202020"
         edTextCol="ff000000" edBkgCol="0" labelText="-inf" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="34"/>
  <LABEL name="WetLevelLabel" id="3469fbc38286d2b6" memberName="_wetLevelLabel"
         virtualName="" explicitFocusOrder="0" pos="680 216 64 24" textCol="ff202020"
         edTextCol="ff000000" edBkgCol="0" labelText="-inf" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="34"/>
  <SLIDER name="DrySlider" id="3694f3553dea94b" memberName="_drySlider"
          virtualName="" explicitFocusOrder="0" pos="624 28 24 192" min="0"
          max="10" int="0" style="LinearVertical" textBoxPos="NoTextBox"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1"/>
  <GENERICCOMPONENT name="DecibelScale" id="f3824f7df1d2ea95" memberName="_decibelScale2"
                    virtualName="" explicitFocusOrder="0" pos="668 36 32 176" class="DecibelScale"
                    params=""/>
  <SLIDER name="WetSlider" id="e50054d828347fbd" memberName="_wetSlider"
          virtualName="" explicitFocusOrder="0" pos="724 28 24 192" min="0"
          max="10" int="0" style="LinearVertical" textBoxPos="NoTextBox"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1"/>
  <TEXTBUTTON name="" id="e5cc4d9d88fb6d29" memberName="_browseButton" virtualName=""
              explicitFocusOrder="0" pos="12 308 736 24" bgColOn="ffbcbcff"
              buttonText="Show Browser" connectedEdges="8" needsCallback="1"
              radioGroupId="0"/>
  <GENERICCOMPONENT name="" id="5388ff2994f22af6" memberName="_irBrowserComponent"
                    virtualName="" explicitFocusOrder="0" pos="12 332 736 288" class="IRBrowserComponent"
                    params=""/>
  <TEXTBUTTON name="" id="53a50811080a676c" memberName="_settingsButton" virtualName=""
              explicitFocusOrder="0" pos="708 0 52 16" buttonText="Settings"
              connectedEdges="6" needsCallback="1" radioGroupId="0"/>
  <LABEL name="" id="51bcb70beb24f3cf" memberName="_stretchLabel" virtualName=""
         explicitFocusOrder="0" pos="104 276 84 24" textCol="ff202020"
         edTextCol="ff000000" edBkgCol="0" labelText="100%" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="36"/>
  <LABEL name="" id="59911bc6fa006837" memberName="_beginHeaderLabel"
         virtualName="" explicitFocusOrder="0" pos="16 216 84 24" textCol="ff202020"
         edTextCol="ff202020" edBkgCol="0" labelText="Begin" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="36"/>
  <SLIDER name="" id="5e1bc6ab0a48dea8" memberName="_beginSlider" virtualName=""
          explicitFocusOrder="0" pos="16 240 84 40" thumbcol="ff8080c0"
          rotarysliderfill="ff8080c0" min="0" max="2" int="0" style="RotaryVerticalDrag"
          textBoxPos="NoTextBox" textBoxEditable="1" textBoxWidth="80"
          textBoxHeight="20" skewFactor="0.5"/>
  <LABEL name="" id="b32110895dcec8f5" memberName="_beginLabel" virtualName=""
         explicitFocusOrder="0" pos="16 276 84 24" textCol="ff000000"
         edTextCol="ff000000" edBkgCol="0" labelText="0ms" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="36"/>
  <TOGGLEBUTTON name="" id="dd1ecfa2e279dd6" memberName="_autoGainButton" virtualName=""
                explicitFocusOrder="0" pos="436 236 134 24" txtcol="ff202020"
                buttonText="Autogain 0.0dB" connectedEdges="0" needsCallback="1"
                radioGroupId="0" state="1"/>
  <TOGGLEBUTTON name="" id="8628927d4ebd1925" memberName="_dryButton" virtualName=""
                explicitFocusOrder="0" pos="596 236 52 24" txtcol="ff202020"
                buttonText="Dry" connectedEdges="0" needsCallback="1" radioGroupId="0"
                state="1"/>
  <TOGGLEBUTTON name="" id="91ab6a3d6477618f" memberName="_wetButton" virtualName=""
                explicitFocusOrder="0" pos="696 236 56 24" txtcol="ff202020"
                buttonText="Wet" connectedEdges="0" needsCallback="1" radioGroupId="0"
                state="1"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif
