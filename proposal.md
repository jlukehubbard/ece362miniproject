# Proposal
## Briefly, what are you making and what are you making it out of?
A polyphonic digital synthesizer with MIDI input capabilities. The microcontroller's timers and DAC will be used to synthesize audio, and the USART will be used to interact with MIDI.

The OLED LCD will display the interface for switching between the wavetables stored on the microcontroller.

Components we plan to use:
  - SOC1602A OLED LCD Display

## Briefly, how will you demonstrate it?
We will connect a MIDI controller to our project's MIDI IN and play some music. We will demonstrate that switching between the different timbre presets changes the sound output in a predictable way.

## Briefly, how will we define the criteria for a successful project?
A successful project should:
  - Respond to MIDI input with a sound of the correct pitch, volume, and timbre.
  - Present a workable interface for switching between sounds
  - Remain entirely uncombusted all the while
