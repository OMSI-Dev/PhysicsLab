# Telegraph station for OMSI's Physics Lab

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
4. Emulate comunication between two systems: activate light and sound of opposite system from a local system.

# Hardware Assembly

**LED domes**
LED domes are internally illuminated by the module [Z-G4-9WW](https://www.digikey.com/en/products/detail/jkl-components-corp/Z-G4-9WW/22677423), G-4 TWO-PIN 12V-24VDC WARM. 

An onboard N-channel mosfet drives the LED's PWM pulses, controlled by Pin 3 on the `Teensy 4.0` controller. 

**Telegraph Inputs**
The telegraph inputs are 12V manual momentary contact switches.

The telegraph works in a series of resistors to bring the logic level voltage down to 3.3V, which can be read by the `Teensy 4.0`. 

```
12V -> 560 Ω -> Pin 14 of Teensy 4.0 -> 220 Ω -> GND
                                          |
                                          V
                                         GND
```

**PT8211 and the Teensy 4.0**
A `PT8211` audio shield [from PJRC](https://www.pjrc.com/store/pt8211_kit.html) is mounted on a `Teensy 4.0`, hosted on a custom circuit board.

PRJC's original `PT8211` shield design is for the `Teensy 3.x` footprint. The `Teensy 4.0` is used in this project to fit this requirement. 

Physical modifications are needed if a `Teensy 4.1` board is used. 

### Assembling the PT8211 shield
PRJC's `PT8211 T4` variation is used for  `morse code`. 

Refer to [this guide](https://www.pjrc.com/store/pt8211_kit.html), and ensure the `T4` variation is used. 

***
**Note:** Information below will become outdated! Modifications needed to the circuit board:

- Direct-routed 500 Ohm resistor to signal out

- Jumper header for telegraph A/B designation 

- GND and 3.3V connected to Teensy 

- Labels on Phoenix Connector terminals 

- Rereouted signal in/out to pins 14 and 15

Modifications needed to the firmware:

- Self-identifying telegraph A/B 

- New pinouts for telegraph A/B

```
if i am telegraph A, I recieve via pin 14, and transmit via pin 15

if i am telegraph B, I recieve via pin 15, and tranmit via pin 14
```

***

# Component Specifications

Referencing documentation found in `/Manufacturing`: `V1_Teensy 4.0 - Main Controller PCB.pdf` & `V1_Teensy 4.0 - Main Controller Schematic.pdf`

**Custom circuit board components**
    (*)= mounted, not soldered on

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
    
2. Audio output parts:
```
    a) ZK-502T amplifier to speaker
    b) speaker
    c) 24V 5A power supply
```

3. Interactives:
```
    a) 2x buttons
    b) 2x LED fixtures
```

# Circuit pinouts

*PT2811 pinout found in `/telegraph/Components/Teensy 4.1 - Telegraph AV controller/PRJC8211 Audio Shield/`
*Other general component pinouts found in repository `OMSI-methods`

## Externals
**Teensy 4.0 -> PT2811 Audio Shield**

1. Pin 7 -> DIN(data)
2. Pin 20 -> WS (serial bus)
3. Pin 21 -> BCK (bit clock)
4. 3.3V -> VCC
5. GND -> GND

**PT2811 -> ZK-502T Amplifier**

1. Audio jack -> Audio input 

**Teensy 4.0 -> circuit board**

1. GND -> GND (V1 hotwired...)
2. VCC -> 5V (V1 hotwired...)
3. Pin 14 -> SW_DATA
4. Pin 3 -> LED_DATA

**12V telegraph -> circuit board**

*= signal line connected at this point
1. GND -> GND
2. Signal -> (10kΩ -> 12V) -> (GND -> Green LED -> 500Ω -> *Mosfet -> 12V) -> SW_OUTPUT

**12V LED -> circuit board**

1. VIN -> 12V
2. GND -> (Mosfet -> 10KΩ -> LED_DATA) -> GND

## Power

**12V -> circuit board**

1. VIN -> (P-channel Mosfet -> GND) -> 12V
2. GND -> GND
3. (12V -> 500Ω -> Red indicator LED -> GND)

**5V -> circuit board** 

1. VIN -> (P-chanel Mosfet -> GND) -> (5V) -> 3.3V regulator
2. (5V -> 100nF -> GND)
3. (3.3V -> 10uF -> GND)
4. (5V -> 150Ω -> Red indicator LED -> GND )

## Connections 
**Note:** The header and ethernet port system on this board is designed with precautions of future mechanical failures. On-board headers are bridged with `header jumpers` to the target signal destination on the ethernet port.

Main signals are `SW_OUT` and `SW_IN`, both which get bridged to the signal path options. For example, `SW_OUTPUT` is bridged to `SW_OUT` OR `SW_OUT(2)`. Misc signals are also interfaceable.

**RJ45 Ethernet Port (J1)** 

1. Pin 1 -> SW_OUT(2)
2. Pin 2 -> MISC(4)
3. Pin 3 -> SW_OUT 
4. Pin 4 -> MISC(3)
5. Pin 5 -> SW_IN(2)
6. Pin 6 -> MISC(2)
7. Pin 7 -> SW_IN

### JST connections for ethernet signals**

**JST header J2**

1. SW_OUT
2. SW_OUTPUT
3. SW_OUT(2)

**JST header J3**

1. SW_IN
2. Signal -> (560Ω -> SW_DATA) -> 220Ω -> GND
3. SW_IN(2)

**JST header J4**

1. MISC(1)
2. MISC(2)
3. MISC(3)
4. MISC(4)


