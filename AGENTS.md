# open-motion-light

A battery-powered motion sensor light based on the STM32L051K8U microcontroller, built using the Arduino framework and PlatformIO.

## Project Overview

This project implements the firmware for a motion-activated light. It features:
- **Low-power operation:** Designed for battery use with LiFePO4 batteries.
- **Sensors:** VCNL4020 for ambient light and proximity sensing, plus a PIR motion sensor.
- **HAL Architecture:** Hardware interfaces are abstracted into classes (e.g., `VCNL4020`, `PowerController`), allowing for easy testing with fakes.
- **Configuration:** Managed via Protobuf (Nanopb) over USB Serial, with a companion web-based manager.
- **Hardware:** KiCad design files for PCB fabrication (targeted at JLC PCB).

### Main Technologies
- **Language:** C++17
- **Framework:** Arduino (STM32 core)
- **Build System:** PlatformIO
- **Communication:** Protobuf (Nanopb)
- **Testing:** GoogleTest (running on `native` environment)

## Building and Running

### Firmware
To build the firmware for the default STM32 environment:
```bash
pio run -e default
```

To upload the firmware (requires `stm32loader` or ST-Link as configured):
```bash
pio run -e default -t upload
```

### Testing
To run the unit tests on your local machine:
```bash
pio test -e native
```

### Linting
The project uses `clang-format` with the Google style. Use the provided script:
```bash
./lint.sh check   # To verify formatting
./lint.sh format  # To apply formatting
```

## Development Conventions

- **Hardware Abstraction:** Always use the HAL interfaces defined in `lib/` for hardware interactions. This ensures the logic remains testable.
- **Testing:** New features should include unit tests in the `test/` directory, utilizing the `native` environment and fakes where appropriate.
- **Protobufs:** Any changes to the serial communication protocol should be made in `proto/serial.proto`.
- **Style:** Follow the Google C++ style guide. Formatting is enforced via `clang-format`.
- **Copyright:** All source files should include the Google LLC copyright and Apache 2.0 license header.

## Directory Structure

- `src/`: Main application entry point (`open-motion-light.cc`).
- `lib/`: Library modules, including HAL interfaces and hardware-specific implementations.
- `test/`: Unit tests using GoogleTest.
- `proto/`: Protobuf definitions for serial communication.
- `hardware/`: KiCad project and production files.
- `enclosure/`: Mechanical design files (STEP).
- `doc/`: Design and power documentation.
