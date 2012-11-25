/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  25 Nov 2012 2:04:33pm

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
      _performanceGroupComponent (0),
      _blockSizeHeaderLabel (0),
      _blockSizeComboBox (0),
      _irDirectoryGroupComponent (0),
      _irDirectoryBrowseButton (0),
      _irDirectoryLabel (0),
      _aboutGroupComponent (0),
      _nameVersionLabel (0),
      _copyrightLabel (0),
      _licenseHyperlink (0)
{
    addAndMakeVisible (_performanceGroupComponent = new GroupComponent (L"new group",
                                                                        L"Performance"));
    _performanceGroupComponent->setColour (GroupComponent::textColourId, Colour (0xff202020));

    addAndMakeVisible (_blockSizeHeaderLabel = new Label (String::empty,
                                                          L"Convolution Block Size:"));
    _blockSizeHeaderLabel->setFont (Font (15.0000f, Font::plain));
    _blockSizeHeaderLabel->setJustificationType (Justification::centredLeft);
    _blockSizeHeaderLabel->setEditable (false, false, false);
    _blockSizeHeaderLabel->setColour (Label::textColourId, Colour (0xff202020));
    _blockSizeHeaderLabel->setColour (TextEditor::textColourId, Colours::black);
    _blockSizeHeaderLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (_blockSizeComboBox = new ComboBox (String::empty));
    _blockSizeComboBox->setEditableText (false);
    _blockSizeComboBox->setJustificationType (Justification::centredRight);
    _blockSizeComboBox->setTextWhenNothingSelected (String::empty);
    _blockSizeComboBox->setTextWhenNoChoicesAvailable (L"(no choices)");
    _blockSizeComboBox->addItem (L"64", 1);
    _blockSizeComboBox->addItem (L"128", 2);
    _blockSizeComboBox->addItem (L"256", 3);
    _blockSizeComboBox->addItem (L"512", 4);
    _blockSizeComboBox->addItem (L"1024", 5);
    _blockSizeComboBox->addItem (L"2048", 6);
    _blockSizeComboBox->addItem (L"4096", 7);
    _blockSizeComboBox->addItem (L"8192", 8);
    _blockSizeComboBox->addListener (this);

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
                                                    L"Copyright (c) 2012 HiFi-LoFi"));
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


    //[UserPreSize]
    //[/UserPreSize]

    setSize (504, 318);


    //[Constructor] You can add your own custom stuff here..

    if (_nameVersionLabel)
    {
      _nameVersionLabel->setText(ProjectInfo::projectName + juce::String(" - Version ") + ProjectInfo::versionString, false);
    }

    const size_t blockSize = _processor.getSettings().getConvolverBlockSize();
    juce::String blockSizeString(static_cast<unsigned>(blockSize));
    int comboBoxIndex = 0;
    while (comboBoxIndex < _blockSizeComboBox->getNumItems() && _blockSizeComboBox->getItemText(comboBoxIndex) != blockSizeString)
    {
      ++comboBoxIndex;
    }
    _blockSizeComboBox->setSelectedItemIndex(comboBoxIndex);

    if (_irDirectoryLabel)
    {
      _irDirectoryLabel->setText(_processor.getSettings().getImpulseResponseDirectory().getFullPathName(), false);
    }

    //[/Constructor]
}

SettingsDialogComponent::~SettingsDialogComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    deleteAndZero (_performanceGroupComponent);
    deleteAndZero (_blockSizeHeaderLabel);
    deleteAndZero (_blockSizeComboBox);
    deleteAndZero (_irDirectoryGroupComponent);
    deleteAndZero (_irDirectoryBrowseButton);
    deleteAndZero (_irDirectoryLabel);
    deleteAndZero (_aboutGroupComponent);
    deleteAndZero (_nameVersionLabel);
    deleteAndZero (_copyrightLabel);
    deleteAndZero (_licenseHyperlink);


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void SettingsDialogComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colour (0xffa6a6b1));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void SettingsDialogComponent::resized()
{
    _performanceGroupComponent->setBounds (16, 124, 472, 72);
    _blockSizeHeaderLabel->setBounds (24, 148, 192, 24);
    _blockSizeComboBox->setBounds (188, 148, 132, 20);
    _irDirectoryGroupComponent->setBounds (16, 208, 472, 88);
    _irDirectoryBrowseButton->setBounds (356, 256, 114, 24);
    _irDirectoryLabel->setBounds (24, 224, 456, 24);
    _aboutGroupComponent->setBounds (16, 11, 472, 101);
    _nameVersionLabel->setBounds (24, 28, 456, 24);
    _copyrightLabel->setBounds (24, 52, 456, 24);
    _licenseHyperlink->setBounds (24, 76, 456, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void SettingsDialogComponent::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    //[UsercomboBoxChanged_Pre]
    //[/UsercomboBoxChanged_Pre]

    if (comboBoxThatHasChanged == _blockSizeComboBox)
    {
        //[UserComboBoxCode__blockSizeComboBox] -- add your combo box handling code here..
        const int index = _blockSizeComboBox->getSelectedItemIndex();
        const int blockSize = _blockSizeComboBox->getItemText(index).getIntValue();
        _processor.getIRManager().setConvolverBlockSize(blockSize);
        _processor.getSettings().setConvolverBlockSize(blockSize);
        //[/UserComboBoxCode__blockSizeComboBox]
    }

    //[UsercomboBoxChanged_Post]
    //[/UsercomboBoxChanged_Post]
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
            if (_irDirectoryLabel)
            {
              _irDirectoryLabel->setText(directory.getFullPathName(), true);
            }
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
                 initialHeight="318">
  <BACKGROUND backgroundColour="ffa6a6b1"/>
  <GROUPCOMPONENT name="new group" id="ddebbd07ce1b9c08" memberName="_performanceGroupComponent"
                  virtualName="" explicitFocusOrder="0" pos="16 124 472 72" textcol="ff202020"
                  title="Performance"/>
  <LABEL name="" id="62ad5d666b54a84a" memberName="_blockSizeHeaderLabel"
         virtualName="" explicitFocusOrder="0" pos="24 148 192 24" textCol="ff202020"
         edTextCol="ff000000" edBkgCol="0" labelText="Convolution Block Size:"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
  <COMBOBOX name="" id="9e24f95de8031f2e" memberName="_blockSizeComboBox"
            virtualName="" explicitFocusOrder="0" pos="188 148 132 20" editable="0"
            layout="34" items="64&#10;128&#10;256&#10;512&#10;1024&#10;2048&#10;4096&#10;8192"
            textWhenNonSelected="" textWhenNoItems="(no choices)"/>
  <GROUPCOMPONENT name="" id="54a84aa39bb27f4b" memberName="_irDirectoryGroupComponent"
                  virtualName="" explicitFocusOrder="0" pos="16 208 472 88" textcol="ff202020"
                  title="Impulse Response Directory"/>
  <TEXTBUTTON name="" id="b1f2dd9a0266865f" memberName="_irDirectoryBrowseButton"
              virtualName="" explicitFocusOrder="0" pos="356 256 114 24" buttonText="Browse ..."
              connectedEdges="3" needsCallback="1" radioGroupId="0"/>
  <LABEL name="new label" id="3415b13950fc6ae1" memberName="_irDirectoryLabel"
         virtualName="" explicitFocusOrder="0" pos="24 224 456 24" textCol="ff202020"
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
         edTextCol="ff202020" edBkgCol="0" labelText="Copyright (c) 2012 HiFi-LoFi"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="33"/>
  <HYPERLINKBUTTON name="" id="605db3d7e5dbbec" memberName="_licenseHyperlink" virtualName=""
                   explicitFocusOrder="0" pos="24 76 456 24" tooltip="http://www.gnu.org/licenses"
                   buttonText="Licensed under GPL3" connectedEdges="0" needsCallback="0"
                   radioGroupId="0" url="http://www.gnu.org/licenses"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif
