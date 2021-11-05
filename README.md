# ece362miniproject
Andrew Helton

James Hubbard

Jack Myers

Bartosz Stoppel

## Tentative Feature List
    - 8-voice polyphonic
	Chords
	- MIDI IN/OUT/THRU
	- Waveform presets read from SD card
	Modify stepping through saved wavetables
	- Faux oscilloscope display

## Task list
    - Copy lab 5 code and build into a working setup on new breadboards
	- Make lab 5 8-voice polyphonic
	- Write preset-switching code
    - Write MIDI input/thru code
	- Build MIDI in/thru circuits
	- Figure out how to interface with TFT display to display waveform/scroll
	- Build monitor output (headphone jack)
	- Interface with OLED for preset switching
	- Design/fabricate PCB
	
	
# Proposal
## Briefly, what are you making and what are you making it out of?
A polyphonic digital synthesizer with switchable wavetables loaded from an SD card and MIDI input capabilities. Our uC's timers and DAC will be used to synthesize audio, and the USART will be used to receive MIDI.

The OLED LCD will display the interface for switching between the wavetables stored on the SD card.

If there is time and spare computing power, we would also like to show a graphical representation of the loaded wavetable on the TFT display, which would make use of the uC's SPI peripheral.

Components we plan to use:
  - SOC1602A OLED LCD Display
  - 2.2-inch SPI Module MSP2202 TFT LCD Display

## Briefly, how will you demonstrate it?
We will connect a MIDI controller to our project's MIDI IN and play some music. We will demonstrate that switching between the different timbre presets changes the sound output in a predictable way.

## Briefly, how will we define the criteria for a successful project?
A successful project should:
  - Respond to MIDI input with a sound of the correct pitch, volume, and timbre.
  - Present a workable interface for switching between sounds
  - Remain entirely uncombusted all the while
