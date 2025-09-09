# Power

## Battery

By default, the MCU that we're using has brown-out reset (BOR) enabled, and configured to use a threshold from 2.54 to 2.74 V. This is documented in the programming manual (RM0377), in section `6.2.2 Brown out reset (BOR)`. Note that with regards to BOR, there are two versions of the processor: one which enables it by default at power-on time, and one which disables it. We want the former.
