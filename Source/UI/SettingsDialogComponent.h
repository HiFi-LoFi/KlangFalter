/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  25 Nov 2012 3:27:48pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

#ifndef __JUCER_HEADER_SETTINGSDIALOGCOMPONENT_SETTINGSDIALOGCOMPONENT_301CD402__
#define __JUCER_HEADER_SETTINGSDIALOGCOMPONENT_SETTINGSDIALOGCOMPONENT_301CD402__

//[Headers]     -- You can add your own extra header files here --
#include "../JuceLibraryCode/JuceHeader.h"

#include "../PluginProcessor.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class SettingsDialogComponent  : public Component,
                                 public ComboBoxListener,
                                 public ButtonListener
{
public:
    //==============================================================================
    SettingsDialogComponent (PluginAudioProcessor& processor);
    ~SettingsDialogComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged);
    void buttonClicked (Button* buttonThatWasClicked);



    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    PluginAudioProcessor& _processor;
    //[/UserVariables]

    //==============================================================================
    GroupComponent* _performanceGroupComponent;
    Label* _blockSizeHeaderLabel;
    ComboBox* _blockSizeComboBox;
    GroupComponent* _irDirectoryGroupComponent;
    TextButton* _irDirectoryBrowseButton;
    Label* _irDirectoryLabel;
    GroupComponent* _aboutGroupComponent;
    Label* _nameVersionLabel;
    Label* _copyrightLabel;
    HyperlinkButton* _licenseHyperlink;


    //==============================================================================
    // (prevent copy constructor and operator= being generated..)
    SettingsDialogComponent (const SettingsDialogComponent&);
    const SettingsDialogComponent& operator= (const SettingsDialogComponent&);
};


#endif   // __JUCER_HEADER_SETTINGSDIALOGCOMPONENT_SETTINGSDIALOGCOMPONENT_301CD402__
