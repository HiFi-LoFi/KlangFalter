KlangFalter
===========

KlangFalter is a convolution audio plugin.

I started its development a couple of time ago because I couldn't find a free convolution plugin which suited my needs - and because I was curious about audio DSP programming. ;-)

## Features: ##

- Low/no latency convolution plugin
- Easy user interface
- Support for true stereo impulse responses
- Simple high and low shelf EQ
- Freely modifiable envelope for the impulse response
- Basically no limit on impulse response length - as long as your computer can handle it...
- Intuitive and flexible loading of impulse response files

![Screenshot](https://github.com/HiFi-LoFi/KlangFalter/blob/master/Screenshot.png)


### Getting KlangFalter: ###

Please check the [Binary](https://github.com/HiFi-LoFi/KlangFalter/tree/master/Binary) folder.

At the moment, I only build Audio Unit plugins. However, it should be easy to build the plugin for any format supported by the [JUCE](http://www.rawmaterialsoftware.com) framework.

### Very Short Tutorial: ###

#### Loading impulse responses ####

- Click on the "Settings" button to adjust the folder which contains your impulse respone files.
- Click on "Show Browser" and select some impulse response file.
- KlangFalter tries to find matching impulse response files automatically by searching for matching file names (e.g. left/right and true stereo).
- You can load impulse response files for specific channel connections by clicking on the file name beneath the wave form display of the according tab.  
- You can clear the impulse response of a specific channel connection by clicking on the "X" button of the according tab.

KlangFalter doesn't come with any impulse respones, but you can find many free and good impulse response collections on the web. Any file format supported by JUCE should work (currently at least .wav, .mp3, .aiff, .ogg and a couple more).

#### Controls ####
- *Wet/Dry*: Adjustment of the wet and dry signal.
- *Autogain*: "Normalizes" the loaded impulse responses in order to achieve a consistent level.
- *Begin*: Begin of the impulse response.
- *Stretch*: Stretches the impulse response.
- *Predelay*: Delay before the impulse response kicks in.
- *Lo Shelf & Hi Shelf*: EQ
- *Reverse*: Reverts the impulse response for spacy effects.

#### Impulse Response Envelope: ####
- Drag one of the dotted envelope nodes in the wave form display to modify the envelope of the impulse response.
- Double-click somewhere in the wave form display to add a new envelope node.
- Double-click on a envelope node to remove it again.

### Technical Stuff: ###

- Open source ([GPL](http://www.gnu.org/licenses) license)
- SSE optimized algorithm
- Multithreaded convolution engine
- Written in C++
- Based on the great [JUCE](http://www.rawmaterialsoftware.com) framework

### Important: ###

At the moment, KlangFalter is neither really "feature complete" nor absolutely free of software bugs, and there's still much room for improvement. Nevertheless, it runs sufficiently stable at least in Ableton Live 8, which I use myself for making music.