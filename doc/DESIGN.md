# Design record

## Hardware v1.1 measurements - LED driver

Signal rise time is ~85ns, which is about 10MHz. No overshoot or ringing visible on a 50MHz scope.

At 3.5v, with main LED full on the board draws ~340mA. This increases to ~390mA at 3.2v, and ~440mA at 3.0v.

## Hardware v1.3 calculations

### PIR sensitivity adjust

The `SENS` pin is used to adjust the sensitivity of the motion sensor. 0V is maximum sensitivity, and 1/4 _ VDD is minimum. In this revision, the PIR is powered by a 3.0V LDO, so the minimum sensitivity will be at 1/4 _ 3.0V = 0.75V.

The PIR `SENS` pin has an equivalent 120kΩ pulldown (measured on two devices, pretty consistent, at least at the same temperature). I added a 1MΩ pulldown resistor to improve stability across PIR devices and temperatures. This pulldown likely comes from the PIR's internal ADC.

To allow adjusting sensitivity in software, three resistors are connected to the pulldown via MCU pins. These form a variable voltage divider. They're sized at 1MΩ and 2MΩ. This means that the possible resistance and voltage divider output values (at 3.4V battery level) are:

| Resistance | Output Voltage | Percent |
| ---------- | -------------- | ------- |
| 2 MΩ       | 173 mV         | 23%     |
| 1 MΩ       | 329 mV         | 44%     |
| 667 kΩ     | 470 mV         | 63%     |
| 400 kΩ     | 718 mV         | 96%     |

There could be moderate variation in the PIR's equivalent pulldown between devices and at different temperatures. The sensitivity adjustment is pretty coarse anyway, so this is acceptable. If this does prove to be an issue, we could use a low-power voltage buffer, such as `TLV8541DBVR`, which consumes 500nA.

### Battery charger temperature sensor

The `BQ25170DSGR` uses an NTC thermistor to measure the temperature of the battery, and will only charge when the temperature is in a certain range. [The battery that I've used so far](https://www.batteryspace.com/LiFePO4-18650-Rechargeable-Cell-3.2V-1800-mAh.aspx) may be charged between 0°C and 50°C. Since the thermistor is just surface-mounted to the PCB, and not in direct contact with the battery, we'll adjust this range to roughly 5°C-45°C.

From [the charger IC datasheet](https://www.ti.com/lit/ds/symlink/bq25170.pdf), the nominal bias current is 38μA. `V_COLD` is 1.04V, and `V_HOT` is 188mV. So, the resistances we need are:

- Cold: `1.04V / 38μA = 27368Ω`
- Hot: `.188V / 38μA = 4947Ω`

If we use [this Sunlord 10k 3950K thermistor](https://jlcpcb.com/partdetail/Sunlord-SDNT1608X103F3950FTF/C279936), then it will yield the range 4°C-42°C - close enough!

We can disable charging by pulling the TS pin below 40mV (minimum). We'll connect this to an MCU pin. Most of the pins have 50nA leakage, which is negligible relative to the 38μA bias current. If we ignore the resistance of the thermistor, then we need `.04V / 38μA = 1052Ω` at most. So, if we use a 1kΩ resistor, it'll be sufficient.

### Part availability and sourcing

The previous revision used a few ICs from Chinese brands, which I couldn't find a source for other than LCSC. This presents moderate risk - if LCSC stops carrying them, then we'd need to use a different part, likely requiring a redesign.

#### USB-to-serial converter

The previous revision used the CH340X USB-to-UART serial converter chip. While this chip is commonly used, it is not available from Western retailers such as Digikey and Mouser. It _is_ available through LCSC, a reputable Chinese supplier. I found it at [another Chinese supplier, HQOnline](https://www.hqonline.com/product-detail/usb-chips-wch-ch340x-2500405741) - owned by NextPCB. While I'm not familiar with them, it's at least a second source. There is still some risk from using parts only available from Chinese distributors, such as from export restrictions or tariffs.

Sparkfun uses the CH340 series in some of their products - see [their guide on installing the drivers](https://learn.sparkfun.com/tutorials/how-to-install-ch340-drivers/all). This is a decent sign that it's available enough. I also found [this thread on EEVblog](https://www.eevblog.com/forum/beginners/ch340-ch330-for-commercial-crowdfunding-product/), where multiple people report using CH340s successfully in their projects.

An alternative would be the [Silicon Labs CP2102N](https://www.digikey.com/en/products/detail/silicon-labs/CP2102N-A02-GQFN28R/9863480). However, the chip is larger (QFN28 vs MSOP-10), and about twice the cost ($1.03 vs $0.39 at qty 100 from LCSC, at revision time).

#### 18650 battery holder

The design uses the [`BH-18650-A1AJ005` 18650 holder](https://jlcpcb.com/partdetail/MYOUNG-BH_18650A1AJ005/C5290176) from manufacturer MYOUNG. The only primary source I could find this from was LCSC. However, JLC doesn't currently list 18650 holders from any other manufacturers, so we'll stick with this as long as we're using JLC's turnkey assembly service. While it may take a layout update if we need to switch to a different holder, the functionality will not change, so this is lower risk.

#### Tactile switch

The design uses a tactile switch for the reset button. This shouldn't strictly be needed, since the CH340X can reset the MCU. However, it is nice to have, and is very cheap (about $0.02 each). This is available both from LCSC and HQOnline, and as with the battery holder, would be straightforward to replace if necessary.

#### White LEDs

As with the above parts, the white LEDs (`HL-AM-2835H421W-S1-08-HR3` from HONGLITRONIC) are not available from Western distributors. However, they are a common size (2835 metric) with common specs (~3.2V forward voltage, 60mA rated current), and [drop-in alternatives](https://www.digikey.com/en/products/detail/ams-osram-usa-inc/GW-JTLMS1-CM-G7H2-XX58-1-60-R33/13680961) are easily available from Western distributors.

#### PIR sensor

The design uses a PIR sensor, `XYC-PIR224S-S1` from NewOpto. This part is not available from Western distributors, and I couldn't find it from Asian distributors other than LCSC. If we limit this to only complete modules with a digital interface, then the cheapest Western-available part is the [ZiLOG `ZDP323`](https://www.mouser.com/ProductDetail/ZiLOG/ZDP323B1). As of writing, it is $2.57 each at qty 100, versus $0.47 each at qty 100 for the NewOpto. From what I can tell, using an analog sensor and implementing the supporting circuitry needed would be pretty complex, and not significantly cheaper anyway. So, we'll stick with the NewOpto for now, and revisit if it becomes a problem.
