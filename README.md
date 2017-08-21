# Arduino digital polyphonic synth with analog filter

Six voices synth with MIDI input controlled by an arduino microcontroller. 

Synth features:
  - MIDI input
  - 76 key support (21 to 96)
  - Six digital voices:
    - square waveform 50%
    - square waveform 25%  
  - Two envolope generators (EG) with attack, and release controls
  - One digital 8 bit LFO with:
    - 0.5Hz to 30Hz tune control.
    - Sine and exponential waveforms
  - One analog filter with:
	- Cutoff freq and resonance controls.
	- Modulated by LFO and EG2
  - One digital VCA modulated by EG1 and LFO
  - Digital features:
      - Arpeggiator: Repeat until 10 keys pressed at the same time with speed control.
      - Two modes: 
        - MODE 0:EG with attack and release parameters.
        - MODE 1:EG with attack, sustain and release parameters.

  - MIDI controls info:
      - MIDI channel : 1  
      - EG1 Attack   : control number 74
      - EG1 Release  : control number 71
      - EG2 Attack   : control number 73
      - EG2 Release  : control number 72
      - LFO Speed    : control number 75
      - VCA modulation (EG1 or LFO balance) : control number 77
      - VCF modulation (EG2 or LFO balance) : control number 76
      - Arpeggiator speed : control number 78
	  
		
This project contains:
  - Arduino nano Firmware
  - Schematics and PCB 
  
  