KlangFalter
===========

KlangFalter is a convolution audio plugin, e.g. for usage as convolution reverb.

I started its development a couple of time ago because I couldn't find a convolution reverb plugin which suited my needs - and because I was curious about audio DSP programming. ;-)

## Features: ##

- Zero-latency
- Easy user interface
- Support for true stereo impulse responses
- Simple 2-band EQ (low cut/shelf and high cut/shelf)
- Easy modifiable envelope for the impulse response
- Basically no limit on impulse response length - as long as your computer can handle it...
- Intuitive and flexible loading of impulse response files

![Screenshot](https://raw.github.com/HiFi-LoFi/KlangFalter/master/Screenshot.png)


## Getting KlangFalter: ##

Please check the [Binary](https://github.com/HiFi-LoFi/KlangFalter/tree/master/Binary) folder.
For installation, just download the according .zip file and extract it in your plug-in folder.

At the moment, I only build Audio Unit plugins for Mac. However, an LV2 version for Linux can be found
in the really great [distrho](http://distrho.sourceforge.net) distrho project.


## Very Short Tutorial: ##

#### Loading impulse responses ####
- Click on the "Settings" button to adjust the folder which contains your impulse response files.
- Click on "Show Browser" and select some impulse response files.
- KlangFalter tries to find matching impulse response files automatically by searching for matching file names (e.g. left/right and true stereo).
- You can load impulse response files for specific channel connections by clicking on the file name beneath the wave form display of the according tab.  
- You can clear the impulse response of a specific channel connection by clicking on the "X" button of the according tab.

KlangFalter doesn't come with any impulse respones, but you can find many free and good impulse response collections on the web. Any file format supported by JUCE should work (currently at least .wav, .mp3, .aiff, .ogg and a couple more).

#### Mixing dry and wet signal ####
- *Wet/Dry*: Adjustment of the wet and dry signal.
- *Autogain*: "Normalizes" the loaded impulse responses in order to achieve a consistent level.
- *Stereo Width*: Adjusts the - guess what - stereo width.
- Click on the label on top of the right level display to switch the according level measurement between "out" and "wet" mode.

#### Modifying the impulse response ####
- *Gap*: Additional gap at the begin of the impulse response ("pre-delay").
- *Begin*: Begin of the impulse response.
- *End*: End of the impulse response
- *Stretch*: Stretches the impulse response.
- *Attack Length & Shape*: Modifies the front (attack) of the impulse response.
- *Decay Shape*: Modifies the decay of the impulse response
- *Reverse*: Reverts the impulse response for spacy effects.
- Click on the timeline at the bottom of the waveform display to switch time measurement between time and beats-per-minute (useful e.g. for timing reverbs to the music).

#### EQ ####
- Click on the header of the low resp. high EQ to switch between cut and shelf filter.
- *Freq*: Adjusts the frequency of the according EQ.
- *Gain*: EQ gain (only available if working as shelf filter).


## Technical Stuff: ##

- Open source ([GPL](http://www.gnu.org/licenses) license)
- SSE optimized algorithm
- Multithreaded convolution engine
- Written in C++
- Based on the great [JUCE](http://www.rawmaterialsoftware.com) framework
