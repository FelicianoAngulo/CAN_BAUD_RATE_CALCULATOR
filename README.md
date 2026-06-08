# CAN_BAUD_RATE_CALCULATOR

Implementation of a CAN bus baud rate calculator for the NXP FRDM-K64F development board.

## Overview

This project captures CAN waveform pulse widths using the FRDM-K64F's FTM input capture peripheral and calculates the CAN bus baud rate from the measured bit time. The application also decodes a captured CAN frame to display:

- CAN identifier (standard or extended)
- DLC (Data Length Code)
- Up to 8 data bytes

The capture is triggered from the on-board SW3 push button, and the results are printed to the serial debug console.

## Hardware Requirements

- FRDM-K64F development board
- USB serial connection to the board
- CAN bus physical waveform connected to `FTM0_CH0` on pin `PTC1`
- On-board `SW3` used to start the measurement

## Usage

1. Build and flash the firmware to the FRDM-K64F.
2. Open a serial console at `115200` baud, `8N1`.
3. Press the on-board `SW3` button.
4. The application captures CAN timing pulses and prints:
   - bit time in microseconds
   - calculated baud rate in kbps
   - CAN ID
   - DLC
   - frame data bytes

## Project Structure

- `source/main.c` — board initialization and application entry point
- `source/SWC/BRC_APP.c` — CAN baud rate calculator application logic
- `source/BSW/ECUAL/FTM_ECUAL.c` — FTM input capture driver and pulse-to-time conversion
- `board/` — board, pin mux, and clock configuration support files
- `CMSIS/` — ARM Cortex-M4 CMSIS device headers and startup support
- `drivers/` — NXP peripheral driver implementations

## Notes

- The application assumes the CAN bus waveform is available on the configured capture pin.
- The current implementation performs a series of capture attempts and reports an error if a valid CAN frame cannot be decoded.
- This repository is primarily intended for educational use and demonstration of CAN baud rate measurement on the FRDM-K64F.

