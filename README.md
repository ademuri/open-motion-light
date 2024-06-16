# open-motion-light

A battery-powered motion sensor light.

## Building software

I added support for the STM32L051K8U on Arduino to Platformio in [this commit](https://github.com/stm32duino/Arduino_Core_STM32/pull/2395). That change isn't in a release yet, so you'll need to manually update it:

```bash
cd /tmp
git clone -b stm32l051k6_8u --single-branch https://github.com/candykingdom/Arduino_Core_STM32.git
cd Arduino_Core_STM32
cp ./* "${HOME?}/.platformio/packages/framework-arduinoststm32/"
```

## Hardware v1.1 measurements

### LED driver

Signal rise time is ~85ns, which is about 10MHz. No overshoot or ringing visible on a 50MHz scope.

At 3.5v, with main LED full on the board draws ~340mA. This increases to ~390mA at 3.2v, and ~440mA at 3.0v.
