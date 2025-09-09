# open-motion-light

A battery-powered motion sensor light.

Status: in development (as of July 2025). The current hardware revision, v1.2, mostly works, but has a few small issues. I'm hoping that the next revision will be the "final" version before starting small-scale production for sale.

Ordering and programming one of these units is a relatively complicated project. I don't recommend it unless you're looking for a learning experience, or have completed similar projects before.

## Programming

To easily update the firmware on your unit, and to configure the settings, go to the management page: https://ademuri.github.io/open-motion-light-manager/. This uses the WebSerial browser API, which is only supported in Chromium-based browsers (e.g. Chrome and Edge).

Otherwise, you can build the code yourself using PlatformIO. To build and upload, from the root of this directory run:

```bash
$ pio run -e default -t upload
```

## Firmware

The firmware for this project is written in C++ using the Arduino framework, and built using PlatformIO. It has comprehensive test coverage, which runs on the desktop (also using PlatformIO). You can run the tests with:

```bash
$ pio test -e native
```

### Architecture

Most use of hardware is wrapped in a hardware abstraction layer (HAL) - a thin wrapper class. This allows writing simple fakes for each of these interfaces, for use in tests. This means that the primary control logic is easily testable.

## Fabrication

This board is designed for PCB fabrication and assembly using JLC PCB. The relevant files are in `hardware/production`:

- `open-motion.light.zip` contains the Gerbers, for PCB assembly.
- `bom.csv` is the bill of materials, which contains the list of components.
- `positions.csv` is the positions file - it lists the exact position and orientation of all placed components.

### PCB parameters

Most of JLC's defaults for PCBs are fine. 1.6mm is the best size for this. White soldermask is preferred, since it reflects more light. I highly recommend selecting the 'LeadFree HASL' surface finish - better not to use lead, and it's only a small cost increase.

### Assembly parameters

This design requires PCBA Type 'Standard', since it needs assembly on both sides. Most of the components are on the top, except the USB port, battery holder, power switch, and reset button. For small quantities, this adds moderate cost, but the USB port would be difficult (although not impossible) to solder by hand. The rest of the parameters can be left as default.
