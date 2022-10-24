# Spankulator
The Spankulator is a WiFi-enabled multi-function Eurorack module. Its main function is to generate a trio of coordinated analog and digital signals which can be used in an endless variety of ways. While each output generates familiar signals, it is the coordination of these signals and the methods of control that make the Spankulator special.

# Spankulator-Hardware, Spankulator-Panel
These repos contain the materials needed to construct the Spankulator main board and the front panel.
The files are for KiCAD 6.0. Go to the Greenlight Kicad repo to get the needed libraries.
The gerber files work with JLCPC and will most likely work with other board manufacturers.
Assembly instructions are in spankulator_assembly.docx.

If starting from a bare circuit board, the BOM for the Spankulator is in bom/Digi-Key/spankulator_full_bom.xlsx. This file can be directly uploaded to Digi-Key for buying parts.

If using the JLCPCB assembly service, the BOM is split into 2 parts.
- bom/LCSC/spankulator_bom.xlsx : The parts to be assembled by JLCPCB
- bom/Digi-Key/spankulator_bom.xlsx : The remaining parts to be assembled by user

Also, when an assembly service is used, a component position file is required. This file's path is: fab/pos/spankulator_cpl.xlsx

note: The bom and cpl files are auto-generated from the csv BOM files produced by KiCAD. You will need to refresh the data when changes to the schematic that affect the BOM occur.

# Spankulator-Code
This repository contains the elements needed to build code for the Spankulator. 
The code is for the Arduino Nano 33 IoT and is found in the Spankulator folder. Make sure to get the required libraries from the Arduino-Libs repo.

The build files are for Windows. They are simple .bat files but depend on having a working Arduino development system.
Certain variables must be edited to adapt these files to a particular installation. 

Here is a list of the batch files and folders:
 
- make.bat ... compiles the code
- make_n_load.bat ... calls make.bat then uploads the code to the Spankulator
- load_recent.bat ... loads most recent compile
- list_ports.bat ... lists available COM ports
- release.bat ... used to update release number in the code and create release binary zip
- out ... this folder must be created. It holds compile output

# Du Dad
The Du Dad is an addition to the Spankulator that provides these benefits:

- Front Panel USB access
- Power sensing shuts off USB to Arduino when synth power is off
- Reset switch
- USB Power indicator
- Boot Loader / Ext Trig Disabled indicator
- Trig In Latch indicator
- Extremely useful to those who use Terminal Remote mode

The folder structure of the Du Dad is similar to the Spankulator Hardware folder. Currently there is no support for assembly service.