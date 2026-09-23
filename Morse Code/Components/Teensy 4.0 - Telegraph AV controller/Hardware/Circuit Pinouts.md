# Telegraph for Physics Lab Circuit Pinous

*One circuit board per side of `telegraph`, two circuit boards total 

*PT2811 pinout found in `/telegraph/Components/Teensy 4.1 - Telegraph AV controller/PRJC8211 Audio Shield/`

## Teensy 4.0 -> PT2811 (hardware-connected, not in custom circuit)
1. Pin 7 -> DIN(data)
2. Pin 20 -> WS (serial bus)
3. Pin 21 -> BCK (bit clock)
4. 3.3V -> VCC
5. GND -> GND

## PT2811 -> Amp 
1. Audio jack -> Audio input 

## Teensy 4.0 -> Circuit board
1. GND -> GND
2. VCC -> 5V 
3. 

## Buttons -> Teensy(? JST failsafe(s))
1. 12V -> 3.3V(?) -> Pin 
2. GND -> GND

## Teensy 4.0 -> LED Fixtures (to 2-pin Phoenix connectors)
1. Pin -> LED
2. GND -> LED GND

## Light Fixutres
1. 12V to 12V 


# PCB Codes (OMSI_Basics Library in Fusion)

1. JST_CONN_02 for Ethernet port failsafe(s)
2. RJ45 Ethernet port 
3. 

# ?'s
1. The LEDs are 12-pin, how are they connecting to the Teensy, and how are they getting powered?
