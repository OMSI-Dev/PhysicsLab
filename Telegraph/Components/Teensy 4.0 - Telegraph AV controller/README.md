# Telegraph for Physics Lab


## Summary of interactives
The `telegraph` project emulates an analog system with a digital system. A `PT8211` audio shield [from PJRC](https://www.pjrc.com/store/pt8211_kit.html) is mounted on a `Teensy 4.0`, hosted on a custom circuit board. 

## PT8211 and the Teensy 4.0
PRJC's original `PT8211` shield design is for the `Teensy 3.x` footprint. The `Teensy 4.0` is used in this project to fit this requirement. 

Physical modifications are needed if a `Teensy 4.1` board is used. 

### Assembling the PT8211 shield
PRJC's `PT8211 T4` variation is used for  `telegraph`. 

Refer to [this guide](https://www.pjrc.com/store/pt8211_kit.html), and ensure the `T4` variation is used. 

## Objectives of interactives

1. Read short & long analog pulses from two opposing buttons
2. Output high-resolution PWM fades to two opposing LEDs, emulating old incandescent light-bulbs

## Summary of components
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

1. Assemble `PT8211` to mount on Teensy
`Teensy 4.1 to PT8211`

2. Run tests in `/Telegraph/Documentation/Firmware Tests/0_Sine Wave/`

2. Create PCB Schematics 
```

    a) Map inputs/outputs
    b) Map all components
    c) Map component circuit pinouts
    d) Map circuit connections
    e) Create Fusion file 
  ```

## TODO FIRMWARE

1. Use bounce library to do a simple debounce, and to measure the duration of a buttonpress.
2. Measure buttons analog input to detect longer and shorter presses 
3. Send PWM output to two LED fixtures 

