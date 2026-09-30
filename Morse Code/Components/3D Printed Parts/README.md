# 3D Printing

- Calibrated filaments for Elegoo (in Cura)
- Dont change deflection when importing
- Layer lines perpendicular to the final piece's stress/interactivity is ideal, for durability
- Slice your piece to check for overhangs, w

# Strength 
layer height/thickness (0.2-0.24, OR 2.4 for quality) 
More walls = stronger (minumum 3)
Always 'Alternate extra wall' for extra adhesion 
Detect thin walls pnly if things dissapear in slicing

Bottom shell layer # = Top shell layer #

## infill
Strongest is TPMS-FK, and the -D variation 
- mess with infill density andfill multiline # to get density by volume, but strength in infill lines (two lines of infill width)

Filter out tiny gaps OFF if details are lost 

Enable multiline to TWO minimum to have infill wall loops be at least two lines thick

## advanced

Insert solid layers for extra strength. # = solid layers every x layers 

# Quality 

## Seams
Line width settings cn be ignored

* You can do seam paint tool if there are weird seam artifacts. Adjust your brush size. Vertical ON in tool settings for a stright line

# Seam
Seam pos = Aligned 
Staggered inner seams ON
Scarf joint ON (experimental)(eliminated seams)
Ignore wiping

# Precision
- Ignore arc fighting
- Convert to polyholes
- Polyhole twist ON
- Ironing takes time to configure, not necessary unless you want to configure & take time

# Wall generator
- Wall generator: arachnie (fills in gaps, stronger, but slower) good for precise, smaller parts 

# Wall and Surfaces
- Walls printing order: Inner/Outer 

# Bridging
BRIDGE COUNTERBORE HOLES: PARTIALLY BRIDGED, or fully bridged if it's giving you trouble. for overhangs 

# Overhang
Extra perieters on overhngs: helps bridge small bidges

# Speed
Outer wall to 75mm, inner wall as is. Slice and go to speed section 


## Overhang
Slow down for overhang and bridge support. 

# Others
- Brims are usually always necessary 


.step files

# Material 
- PETG for thin parts to be strong, wheras polycarbonate would not. 

- Best rotation for objects in slicer is alnmost always 45 degrees 

## polycarbonate 
it should be dried as it is printed

