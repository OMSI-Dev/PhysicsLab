# Detecting Dark Matter for Physics Lab 

## Summary of interactives
The `detecting dark matter` project demonstrates a physics simulation by comparing two cylindrical forces of motion each with differing porportions of mass. 

The two pieces are rolled down separate ramp tracks, the first arriving piece being more efficently loaded. 

## Objective of interactives
Two switches are located at the bottom of each ramp. Each switch will trigger an audio file produced by a `wavTrigger`. 

Once a switch is triggered, the sound will play once, and lock out. 

After a switch is untriggered, there will be a 1 second delay, and the lock-out resets for the sound to be triggered again.

# Hardware Assembly

## Summary of components
1. Custom circuit board with parts:
```
    a) (*) Teensy 4.0 
    b) (X1) Terminal for 5V (to 3.3V for Teensy)
    c) (X2) Terminal for Switch #2
    d) (X3) Terminal for Switch #1
    e) (X4) Terminal for WavTrigger input 

Teensy 4.0 talks to wavTrigger over serial 
- Use wavTrigger library in WildCreativity (audio out)

# Assets

1. First place tone / jingle
2. Second place tone / jingle

