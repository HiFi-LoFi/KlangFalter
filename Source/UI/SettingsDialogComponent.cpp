/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  6 Feb 2013 2:44:47pm

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

#include "../Settings.h"

//[/Headers]

#include "SettingsDialogComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
SettingsDialogComponent::SettingsDialogComponent (PluginAudioProcessor& processor)
    : _processor(processor),
      _irDirectoryGroupComponent (0),
      _irDirectoryBrowseButton (0),
      _irDirectoryLabel (0),
      _aboutGroupComponent (0),
      _nameVersionLabel (0),
      _copyrightLabel (0),
      _licenseHyperlink (0),
      _infoGroupComponent (0),
      _juceVersionPrefixLabel (0),
      _juceVersionLabel (0),
      _numberInputsPrefixLabel (0),
      _numberInputsLabel (0),
      _numberOutputsPrefixLabel (0),
      _numberOutputsLabel (0),
      _sseOptimizationPrefixLabel (0),
      _sseOptimizationLabel (0)
{
    addAndMakeVisible (_irDirectoryGroupComponent = new GroupComponent (String::empty,
                                                                        L"Impulse Response Directory"));
    _irDirectoryGroupComponent->setColour (GroupComponent::textColourId, Colour (0xff202020));

    addAndMakeVisible (_irDirectoryBrowseButton = new TextButton (String::empty));
    _irDirectoryBrowseButton->setButtonText (L"Browse ...");
    _irDirectoryBrowseButton->setConnectedEdges (Button::ConnectedOnLeft | Button::ConnectedOnRight);
    _irDirectoryBrowseButton->addListener (this);

    addAndMakeVisible (_irDirectoryLabel = new Label (L"new label",
                                                      L"<None>"));
    _irDirectoryLabel->setFont (Font (15.0000f, Font::plain));
    _irDirectoryLabel->setJustificationType (Justification::centredLeft);
    _irDirectoryLabel->setEditable (false, false, false);
    _irDirectoryLabel->setColour (Label::textColourId, Colour (0xff202020));
    _irDirectoryLabel->setColour (TextEditor::textColourId, Colour (0xff202020));
    _irDirectoryLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (_aboutGroupComponent = new GroupComponent (String::empty,
                                                                  L"About"));
    _aboutGroupComponent->setColour (GroupComponent::textColourId, Colour (0xff202020));

    addAndMakeVisible (_nameVersionLabel = new Label (String::empty,
                                                      L"KlangFalter - Version <Unknown>"));
    _nameVersionLabel->setFont (Font (15.0000f, Font::plain));
    _nameVersionLabel->setJustificationType (Justification::centredLeft);
    _nameVersionLabel->setEditable (false, false, false);
    _nameVersionLabel->setColour (Label::textColourId, Colour (0xff202020));
    _nameVersionLabel->setColour (TextEditor::textColourId, Colour (0xff202020));
    _nameVersionLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (_copyrightLabel = new Label (String::empty,
                                                    L"Copyright (c) 2013 HiFi-LoFi"));
    _copyrightLabel->setFont (Font (15.0000f, Font::plain));
    _copyrightLabel->setJustificationType (Justification::centredLeft);
    _copyrightLabel->setEditable (false, false, false);
    _copyrightLabel->setColour (Label::textColourId, Colour (0xff202020));
    _copyrightLabel->setColour (TextEditor::textColourId, Colour (0xff202020));
    _copyrightLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (_licenseHyperlink = new HyperlinkButton (L"Licensed under GPL3",
                                                                URL (L"http://www.gnu.org/licenses")));
    _licenseHyperlink->setTooltip (L"http://www.gnu.org/licenses");
    _licenseHyperlink->setButtonText (L"Licensed under GPL3");

    addAndMakeVisible (_infoGroupComponent = new GroupComponent (String::empty,
                                                                 L"Plugin Information"));
    _infoGroupComponent->setColour (GroupComponent::textColourId, Colour (0xff202020));

    addAndMakeVisible (_juceVersionPrefixLabel = new Label (String::empty,
                                                            L"JUCE Version:"));
    _juceVersionPrefixLabel->setFont (Font (15.0000f, Font::plain));
    _juceVersionPrefixLabel->setJustificationType (Justification::centredLeft);
    _juceVersionPrefixLabel->setEditable (false, false, false);
    _juceVersionPrefixLabel->setColour (Label::textColourId, Colour (0xff202020));
    _juceVersionPrefixLabel->setColour (TextEditor::textColourId, Colour (0xff202020));
    _juceVersionPrefixLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (_juceVersionLabel = new Label (String::empty,
                                                      L"<Unknown>"));
    _juceVersionLabel->setFont (Font (15.0000f, Font::plain));
    _juceVersionLabel->setJustificationType (Justification::centredLeft);
    _juceVersionLabel->setEditable (false, false, false);
    _juceVersionLabel->setColour (Label::textColourId, Colour (0xff202020));
    _juceVersionLabel->setColour (TextEditor::textColourId, Colour (0xff202020));
    _juceVersionLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (_numberInputsPrefixLabel = new Label (String::empty,
                                                             L"Input Channels:"));
    _numberInputsPrefixLabel->setFont (Font (15.0000f, Font::plain));
    _numberInputsPrefixLabel->setJustificationType (Justification::centredLeft);
    _numberInputsPrefixLabel->setEditable (false, false, false);
    _numberInputsPrefixLabel->setColour (Label::textColourId, Colour (0xff202020));
    _numberInputsPrefixLabel->setColour (TextEditor::textColourId, Colour (0xff202020));
    _numberInputsPrefixLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (_numberInputsLabel = new Label (String::empty,
                                                       L"<Unknown>"));
    _numberInputsLabel->setFont (Font (15.0000f, Font::plain));
    _numberInputsLabel->setJustificationType (Justification::centredLeft);
    _numberInputsLabel->setEditable (false, false, false);
    _numberInputsLabel->setColour (Label::textColourId, Colour (0xff202020));
    _numberInputsLabel->setColour (TextEditor::textColourId, Colour (0xff202020));
    _numberInputsLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (_numberOutputsPrefixLabel = new Label (String::empty,
                                                              L"Output Channels:"));
    _numberOutputsPrefixLabel->setFont (Font (15.0000f, Font::plain));
    _numberOutputsPrefixLabel->setJustificationType (Justification::centredLeft);
    _numberOutputsPrefixLabel->setEditable (false, false, false);
    _numberOutputsPrefixLabel->setColour (Label::textColourId, Colour (0xff202020));
    _numberOutputsPrefixLabel->setColour (TextEditor::textColourId, Colour (0xff202020));
    _numberOutputsPrefixLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (_numberOutputsLabel = new Label (String::empty,
                                                        L"<Unknown>"));
    _numberOutputsLabel->setFont (Font (15.0000f, Font::plain));
    _numberOutputsLabel->setJustificationType (Justification::centredLeft);
    _numberOutputsLabel->setEditable (false, false, false);
    _numberOutputsLabel->setColour (Label::textColourId, Colour (0xff202020));
    _numberOutputsLabel->setColour (TextEditor::textColourId, Colour (0xff202020));
    _numberOutputsLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (_sseOptimizationPrefixLabel = new Label (String::empty,
                                                                L"SSE Optimization:"));
    _sseOptimizationPrefixLabel->setFont (Font (15.0000f, Font::plain));
    _sseOptimizationPrefixLabel->setJustificationType (Justification::centredLeft);
    _sseOptimizationPrefixLabel->setEditable (false, false, false);
    _sseOptimizationPrefixLabel->setColour (Label::textColourId, Colour (0xff202020));
    _sseOptimizationPrefixLabel->setColour (TextEditor::textColourId, Colour (0xff202020));
    _sseOptimizationPrefixLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (_sseOptimizationLabel = new Label (String::empty,
                                                          L"<Unknown>"));
    _sseOptimizationLabel->setFont (Font (15.0000f, Font::plain));
    _sseOptimizationLabel->setJustificationType (Justification::centredLeft);
    _sseOptimizationLabel->setEditable (false, false, false);
    _sseOptimizationLabel->setColour (Label::textColourId, Colour (0xff202020));
    _sseOptimizationLabel->setColour (TextEditor::textColourId, Colour (0xff202020));
    _sseOptimizationLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));


    //[UserPreSize]
    //[/UserPreSize]

    setSize (504, 372);


    //[Constructor] You can add your own custom stuff here..
    _nameVersionLabel->setText(ProjectInfo::projectName + juce::String(" - Version ") + ProjectInfo::versionString, false);
    _irDirectoryLabel->setText(_processor.getSettings().getImpulseResponseDirectory().getFullPathName(), false);
    _juceVersionLabel->setText(juce::SystemStats::getJUCEVersion(), false);
    _numberInputsLabel->setText(juce::String(_processor.getNumInputChannels()), false);
    _numberOutputsLabel->setText(juce::String(_processor.getNumOutputChannels()), false);
    _sseOptimizationLabel->setText((fftconvolver::SSEOptimized() == true) ? juce::String("Yes") : juce::String("No"), false);
    //[/Constructor]
}

SettingsDialogComponent::~SettingsDialogComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    deleteAndZero (_irDirectoryGroupComponent);
    deleteAndZero (_irDirectoryBrowseButton);
    deleteAndZero (_irDirectoryLabel);
    deleteAndZero (_aboutGroupComponent);
    deleteAndZero (_nameVersionLabel);
    deleteAndZero (_copyrightLabel);
    deleteAndZero (_licenseHyperlink);
    deleteAndZero (_infoGroupComponent);
    deleteAndZero (_juceVersionPrefixLabel);
    deleteAndZero (_juceVersionLabel);
    deleteAndZero (_numberInputsPrefixLabel);
    deleteAndZero (_numberInputsLabel);
    deleteAndZero (_numberOutputsPrefixLabel);
    deleteAndZero (_numberOutputsLabel);
    deleteAndZero (_sseOptimizationPrefixLabel);
    deleteAndZero (_sseOptimizationLabel);


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void SettingsDialogComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colour (0xffe5e5f0));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void SettingsDialogComponent::resized()
{
    _irDirectoryGroupComponent->setBounds (16, 128, 472, 88);
    _irDirectoryBrowseButton->setBounds (356, 176, 114, 24);
    _irDirectoryLabel->setBounds (24, 144, 456, 24);
    _aboutGroupComponent->setBounds (16, 11, 472, 101);
    _nameVersionLabel->setBounds (24, 28, 456, 24);
    _copyrightLabel->setBounds (24, 52, 456, 24);
    _licenseHyperlink->setBounds (24, 76, 456, 24);
    _infoGroupComponent->setBounds (16, 232, 472, 124);
    _juceVersionPrefixLabel->setBounds (24, 252, 140, 24);
    _juceVersionLabel->setBounds (156, 252, 316, 24);
    _numberInputsPrefixLabel->setBounds (24, 276, 140, 24);
    _numberInputsLabel->setBounds (156, 276, 316, 24);
    _numberOutputsPrefixLabel->setBounds (24, 300, 140, 24);
    _numberOutputsLabel->setBounds (156, 300, 316, 24);
    _sseOptimizationPrefixLabel->setBounds (24, 324, 140, 24);
    _sseOptimizationLabel->setBounds (156, 324, 316, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void SettingsDialogComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == _irDirectoryBrowseButton)
    {
        //[UserButtonCode__irDirectoryBrowseButton] -- add your button handler code here..
        juce::FileChooser fileChooser("Impulse Response Directory",
                                      _processor.getSettings().getImpulseResponseDirectory(),
                                      "*",
                                      true);
        if (fileChooser.browseForDirectory() && fileChooser.getResults().size() == 1)
        {
          const juce::File directory = fileChooser.getResults().getReference(0);
          if (directory.exists() && directory.isDirectory())
          {
            _processor.getSettings().setImpulseResponseDirectory(directory);
            _irDirectoryLabel->setText(directory.getFullPathName(), true);
          }
        }
        //[/UserButtonCode__irDirectoryBrowseButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="SettingsDialogComponent"
                 componentName="" parentClasses="public Component" constructorParams="PluginAudioProcessor&amp; processor"
                 variableInitialisers="_processor(processor)" snapPixels="4" snapActive="1"
                 snapShown="1" overlayOpacity="0.330000013" fixedSize="1" initialWidth="504"
                 initialHeight="372">
  <BACKGROUND backgroundColour="ffe5e5f0"/>
  <GROUPCOMPONENT name="" id="54a84aa39bb27f4b" memberName="_irDirectoryGroupComponent"
                  virtualName="" explicitFocusOrder="0" pos="16 128 472 88" textcol="ff202020"
                  title="Impulse Response Directory"/>
  <TEXTBUTTON name="" id="b1f2dd9a0266865f" memberName="_irDirectoryBrowseButton"
              virtualName="" explicitFocusOrder="0" pos="356 176 114 24" buttonText="Browse ..."
              connectedEdges="3" needsCallback="1" radioGroupId="0"/>
  <LABEL name="new label" id="3415b13950fc6ae1" memberName="_irDirectoryLabel"
         virtualName="" explicitFocusOrder="0" pos="24 144 456 24" textCol="ff202020"
         edTextCol="ff202020" edBkgCol="0" labelText="&lt;None&gt;" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <GROUPCOMPONENT name="" id="81749f34274f2c9a" memberName="_aboutGroupComponent"
                  virtualName="" explicitFocusOrder="0" pos="16 11 472 101" textcol="ff202020"
                  title="About"/>
  <LABEL name="" id="4b7ca86a8e675cd7" memberName="_nameVersionLabel"
         virtualName="" explicitFocusOrder="0" pos="24 28 456 24" textCol="ff202020"
         edTextCol="ff202020" edBkgCol="0" labelText="KlangFalter - Version &lt;Unknown&gt;"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
  <LABEL name="" id="b963eaf05cdcbe30" memberName="_copyrightLabel" virtualName=""
         explicitFocusOrder="0" pos="24 52 456 24" textCol="ff202020"
         edTextCol="ff202020" edBkgCol="0" labelText="Copyright (c) 2013 HiFi-LoFi"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
  <HYPERLINKBUTTON name="" id="605db3d7e5dbbec" memberName="_licenseHyperlink" virtualName=""
                   explicitFocusOrder="0" pos="24 76 456 24" tooltip="http://www.gnu.org/licenses"
                   buttonText="Licensed under GPL3" connectedEdges="0" needsCallback="0"
                   radioGroupId="0" url="http://www.gnu.org/licenses"/>
  <GROUPCOMPONENT name="" id="25ac040a541cb0e7" memberName="_infoGroupComponent"
                  virtualName="" explicitFocusOrder="0" pos="16 232 472 124" textcol="ff202020"
                  title="Plugin Information"/>
  <LABEL name="" id="c4a4ccf3c53f694f" memberName="_juceVersionPrefixLabel"
         virtualName="" explicitFocusOrder="0" pos="24 252 140 24" textCol="ff202020"
         edTextCol="ff202020" edBkgCol="0" labelText="JUCE Version:" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <LABEL name="" id="e47fab678f6315f0" memberName="_juceVersionLabel"
         virtualName="" explicitFocusOrder="0" pos="156 252 316 24" textCol="ff202020"
         edTextCol="ff202020" edBkgCol="0" labelText="&lt;Unknown&gt;"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
  <LABEL name="" id="b930280e91f83049" memberName="_numberInputsPrefixLabel"
         virtualName="" explicitFocusOrder="0" pos="24 276 140 24" textCol="ff202020"
         edTextCol="ff202020" edBkgCol="0" labelText="Input Channels:"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
  <LABEL name="" id="26b792a855f83bf7" memberName="_numberInputsLabel"
         virtualName="" explicitFocusOrder="0" pos="156 276 316 24" textCol="ff202020"
         edTextCol="ff202020" edBkgCol="0" labelText="&lt;Unknown&gt;"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
  <LABEL name="" id="7ecaf5ecb63407f4" memberName="_numberOutputsPrefixLabel"
         virtualName="" explicitFocusOrder="0" pos="24 300 140 24" textCol="ff202020"
         edTextCol="ff202020" edBkgCol="0" labelText="Output Channels:"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
  <LABEL name="" id="69f8c3850d35c1bd" memberName="_numberOutputsLabel"
         virtualName="" explicitFocusOrder="0" pos="156 300 316 24" textCol="ff202020"
         edTextCol="ff202020" edBkgCol="0" labelText="&lt;Unknown&gt;"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
  <LABEL name="" id="ad7d4e6a39bb8ab5" memberName="_sseOptimizationPrefixLabel"
         virtualName="" explicitFocusOrder="0" pos="24 324 140 24" textCol="ff202020"
         edTextCol="ff202020" edBkgCol="0" labelText="SSE Optimization:"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
  <LABEL name="" id="7a6359296851c399" memberName="_sseOptimizationLabel"
         virtualName="" explicitFocusOrder="0" pos="156 324 316 24" textCol="ff202020"
         edTextCol="ff202020" edBkgCol="0" labelText="&lt;Unknown&gt;"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif
