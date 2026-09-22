# Telegraph for Physics Lab


## Summary of interactives
This `telegraph` project will emulate an analog system with a digital system. A `PT8211` audio shield [from PJRC](https://www.pjrc.com/store/pt8211_kit.html) is mounted on a `Teensy 4.0`, hosted on a custom circuit board. 

## Using the PT8211 with a Teensy 4.0
The original design of PRJC's `PT8211` is for `Teensy 3.x` boards. The `Teensy 4.0` was selected for this project to fit this form factor. Physical modifications are needed if a `Teensy 4.1` board is used. 

### Assembling the PT8211 with a Teensy 4.0
The `PT8211` has a `T4` variation, indicated by a label on its circuit board. For using a `Teensy 4.0`, verify your `PT8211` board has `T4` labeled on it. Solder all parts included in the kit as labelled on the board.

The `PT8211` will function as a top-mounted audio shield to the `Teensy 4.0` Its onboard audio jack will line directly above the `Teensy 4.0` USB port.

Follow [this guide](https://www.pjrc.com/store/pt8211_kit.html).

## Objective of interactives

1. Read short & long analog pulses from two opposing buttons
2. Output high-resolution PWM fades to two opposing LEDs, emulating old incandescent light-bulbs

## Summary of Components
1. Custom circuit board with parts:
```
    a) Teensy 4.0 w/ PT8211 header to amp
    b) 2x terminal for button input/output
    c) 2x terminal for LED output
    d) 1x terminal for 5V
    e) 1x terminal for 12V 
```
    
2. Audio output parts:
```
    a) amplifier to speaker
    b) speaker
    c) 24V 5A power supply
```

3. Interactives:
```
    a) 2x buttons
    b) 2x LED WS2812(?) fixtures
```
    

## TODO HARDWARE

1. Create PT8211 hat for Teensy
`Teensy 4.1 to PT8211`
    a) Pin 7: Connects to PT8211 Pin 3 (Data In)
    b) Pin 9: Connects to PT8211 Pin 2 (Word Select)
    c) Pin 21: Connects to PT8211 Pn 1 (Bit Clock)

        Hard solder prototype 
        Output to amplifier 
        Run the PT8211Sine example sketch 
    
2. Create PCB Schematics 
    a) Map inputs/outputs
    b) Map all components
    c) Map component circuit pinouts
    d) Map circuit connections
    e) Create Fusion file 

## TODO FIRMWARE

1. Use bounce library to trigger buttons
2. Measure buttons analog input to detect longer and shorter presses 
3. Send PWM output to two LED fixtures 

## TODO MATERIALS

1. Order Digikey Order 
    a) Long headers 
