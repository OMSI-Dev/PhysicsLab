# Telegraph for Physics Lab


## Summary of interactives
This `telegraph` project will emulate an analog system with a digital system. A `Teensy 4.0` with `PT8211` DAC breakout on a custom circuit board.

## Using the PT8211 with a Teensy 4.0
The original design of the `PT8211` [spec](https://www.pjrc.com/store/pt8211_kit.html) is for `Teensy 3.x` boards. The `Teensy 4.0` was selected for this project to fit this form factor. Physical modifications are needed if a `Teensy 4.1` board is used. 

### Assembling the PT8211 with a Teensy 4.0
The `PT8211` has a `T4` variation as indicated as a label on its circuit board. For using a `Teensy 4.0`, verify your `PT8211` board has `T4` labeled on it. Solder all parts included in the kit as labelled on the board.



## Objective of interactives

1. Read analog pulses from opposing button
2. Output read signals to LED with high-resolution PWM fading to emulate old incandescent lightbulb
3. 

## Summary of Componens
1. Custom circuit board
    a. Unboard Teensy 4.0 
2. 
3. 

## TODO PLANNING

1. Map inputs/outputs
2. Map circuit pinouts 
3. Create Fusion file 

## TODO HARDWARE

1. Create PT8211 hat for Teensy
`Teensy 4.1 to PT8211`
a) Pin 2: Connects to PT8211 Pin 3 (Data In)
b) Pin 3: Connects to PT8211 Pin 2 (Word Select)
c) Pin 4: Connects to PT8211 Pn 1 (Bit Clock)

    Start digikey order for long headers
    Hard solder prototype 

## TODO FIRMWARE

1. Execute
