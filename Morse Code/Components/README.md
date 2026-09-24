# Telegraph station for Physics Lab

## Summary of interactives
The `morse code` project emulates an analog system with a digital system. 

Two identical telegraph stations are present; two circuit boards, firmware copies, speaker systems, LED domes, etc. 
 

## Objectives of interactives
Two stations will each host a telegraph input. A station will trigger its' opposing station's output components: an LED dome and speaker. 

The LED dome will output high-resolution fades to emulate an old incadescent light bulb. 

## Checkpoints
1. Read inbound short & long analog signals from the opposing button
2. Output short & long sine-wave audio to the `PT8211`, depending on the received input
3. Output short & long visual pulses on the LED dome, depending on the recieved input

# Hardware Assembly
## LED domes
LED domes are internally illuminated by the module [Z-G4-9WW](https://www.digikey.com/en/products/detail/jkl-components-corp/Z-G4-9WW/22677423), G-4 TWO-PIN 12V-24VDC WARM. 

An onboard N-channel mosfet drives the LED's PWM pulses, controlled by Pin 3 on the `Teensy 4.0` controller. 

## Telegraph Inputs
The telegraph inputs are [TODO (yet to be sourced)](github.com), 12V manual momentary contact switch. 

The telegraph works in a series of resistors to bring the logic level voltage down to 3.3V, which can be read by the `Teensy 4.0`. 

```
12V -> 560 Ω -> Pin 14 of Teensy 4.0 -> 220 Ω -> GND
                                          |
                                          V
                                         GND
```

## PT8211 and the Teensy 4.0
A `PT8211` audio shield [from PJRC](https://www.pjrc.com/store/pt8211_kit.html) is mounted on a `Teensy 4.0`, hosted on a custom circuit board.

PRJC's original `PT8211` shield design is for the `Teensy 3.x` footprint. The `Teensy 4.0` is used in this project to fit this requirement. 

Physical modifications are needed if a `Teensy 4.1` board is used. 

### Assembling the PT8211 shield
PRJC's `PT8211 T4` variation is used for  `morse code`. 

Refer to [this guide](https://www.pjrc.com/store/pt8211_kit.html), and ensure the `T4` variation is used. 

## Summary of components
1. Custom circuit board with parts:
```
    a) (*) Teensy 4.0 w/ PT8211 header to amp 
    b) (X1) terminal for LED output 
        - (R5) 10K resistor
        - (Q5) N-Channel mosfet
    c) (X2) terminal for 12V 
        - (C1) 10uF capacitor 
        - (D2) RED LED 
        - (R1) 500 Ohm resistor
        - (Q2) P-Channel mosfet
    d) (X3) terminal for 5V 
        - (C2) 10uF capacitor to 3.3V
        - (C3) 100nF capacitor to 5V
        - (D3) Red LED
        - (R4) 150 Ohm resistor 
        - (Q3) P-Channel mosfet
        - (CR1) 3.3V contage regulator
    e) (X4) Terminal for button output connection 
        - (D4) Green LED 
        - (R6) 10K Ohm resistor to 3.3V
        - (R7) 150 Ohm resistor 
        - (Q6) P-Channel mosfet
    c) (J1) terminal for button input/output line
        - (J2) JST connection 
        - (J3) JST connection
        
```
    (*)= mounted, not soldered on
    
2. Audio output parts:
```
    a) amplifier to speaker
    b) speaker
    c) 24V 5A power supply
```

3. Interactives:
```
    a) 2x buttons
    b) 2x LED fixtures
```

## TODO FIRMWARE

1. Verify functionality with LED modules. Assemble LED circuit board and test against it. 

## TODO Hardware

1. None for now (-:

## Time management

Sept 21-24 Telegraph


