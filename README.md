# Spankulator-Hardware
This repo contains the materials needed to construct the Spankulator main board.
See the Spankulator-Panel repo to make the front panel. 
The files are for KiCAD 6.0. Go to the Greenlight Kicad repo to get the needed libraries.
The gerber files work with JLCPC and will most likely work with other board manufacturers.
Assembly instructions are in spankulator_assembly.docx.
The BOM for the Spankulator is in bom/Spankulator.xlsx which can be directly uploaded to Digi-Key for buying parts.

# Spankulator-Code
This repository contains the elements needed to build code for the Spankulator. 
The code is for the Arduino Nano 33 IoT and is found in the Spankulator folder.

The build files are for Windows. They are simple .bat files but depend on having a working Arduino development system.
Certain variables must be edited to adapt these files to a particular installation. 

Here is a list of the batch files and folders:
 
- make.bat ... compiles the code
- make_n_load.bat ... calls make.bat then uploads the code to the Spankulator
- load_recent.bat ... loads most recent compile
- list_ports.bat ... lists available COM ports
- release.bat ... used to update release number in the code and create release binary zip
- out ... this folder must be created. It holds compile output
