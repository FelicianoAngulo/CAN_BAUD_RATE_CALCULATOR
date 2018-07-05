Overview
========
This project was done base on a  SDK FTM input capture project.

Toolchain supported
===================
- MCUXpresso10.1.0

Hardware requirements
=====================
- Mini/micro USB cable
- FRDM-K64F board
- Personal Computer
- Access to a real CAN bus.
- CAN transceiver.

Board settings
==============
This example project does not call for any special hardware configurations.
Although not required, the recommendation is to leave the development board's jumper settings
and configurations in default state when running this example.

Prepare the Project
================
1. Connect a USB cable between the PC host and the OpenSDA USB port on the board.
2. Open a serial terminal with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4. Connect the CAN transceiver CANH and CANL to real CAN bus and Rx line to PTC1 (J1-5)
3. Download the program to the target board.
4. Press SW3 button for calculate baud rate.

Running the Project
================
- These results are displayed/shown on the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~
