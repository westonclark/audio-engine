# Audio Engine

> [!NOTE] Currently in development and not all feature are implemented yet

This is a bare bones audio mixing engine designed to be lightweight and performative

- This will be a channel-based engine, much like a digital mixer. I want it to support up to 16 channels at once.
- Eventually there will be a GUI but for now it will be a CLI acting as the GUI thread
- First phase is just to be able to playback multiple audio files at once, sum them to the master, and outputing to core audio
- Each channel will have a selectable input source (either audio file or hardware device) and all the channels will sum together into a stereo output bus.
- There will be a static set of processing effects for each "channel strip", starting out with just gain.
- There will be a unified interface for the input nodes that feed the channels so a channel doesn't know what type of input it is.

```
       ___________________       _____________________
      | Input Node (File) |     | Input Node (Device) |
       ‾‾‾‾‾‾‾‾‾|‾‾‾‾‾‾‾‾‾       ‾‾‾‾‾‾‾‾‾‾|‾‾‾‾‾‾‾‾‾‾
         _______|_______            _______|_______
        | Channel Strip |          | Channel Strip |
         ‾‾‾‾‾‾‾|‾‾‾‾‾‾‾            ‾‾‾‾‾‾‾|‾‾‾‾‾‾‾
                |__________________________|
                             |
                        _____|______
                       | Master Bus |
                        ‾‾‾‾‾|‾‾‾‾‾‾
                      _______|_______
                     | Output Device |
                      ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
```

We will start off with one thread for audio processing and one for the GUI and user input layer. We will use `std::atomic` values to safely update values across the two. CoreAudio operates by passing buffers of samples through audio callback functions, so we will try to design the input nodes to the engine to be consistent with this pattern.

Eventually there will be a thread dedicated to reading the audio file samples ahead of time and filling a ring buffer so that we are not loading the entire file into memory at once.

  I think first steps are
  - Implement master bus
  - Implement channels on the audio engine
  - Implement reading multiple channels at once
  - Implement CLI
  - Implement read ahead threads for audio files
  - Implement Core audio input devices as a channel input

## Core Philosophies

- Zero Dependencies
- Basic C++, no templates, macros or other fancy add ons
