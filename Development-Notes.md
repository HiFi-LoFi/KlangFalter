Audio Plugin vs. Development Application
========================================

In order to build an audio plugin resp. a simple development application, the following settings have to be changed in the Introjucer:

## Audio Plugin ##

- Project Type: Audio Plug-In
- JUCE module "juce_audio_plugin_client": Checked

## Development Application ##

- Project Type: Application (GUI)
- JUCE module "juce_audio_plugin_client": Unchecked


Xcode
=====

## Deployment Settings ##

- Base SDK: Current Mac OS
- Mac OS X Deployment Target: Mac OS X 10.5
