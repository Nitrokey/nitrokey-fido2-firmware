# Nitrokey FIDO2

Nitrokey FIDO2 is an open source security key which supports FIDO2 and U2F standards for strong two-factor authentication and password-less login. It protects you against phishing and other online attacks. It's based on [Solokey](https://github.com/solokeys/solo).

This repository contains the firmware, including implementations of FIDO2 and U2F (CTAP2 and CTAP) over USB and NFC. The main implementation is for STM32L432, but it is easily portable.

For development no hardware is needed, it also runs as a standalone application for Windows, Linux, and Mac OSX. If you like (or want to learn) hardware instead, you can run the firmware on the NUCLEO-L432KC development board.


# Security

Nitrokey FIDO2 is based on the STM32L432 microcontroller. It offers the following security features.

- True random number generation to guarantee random keys.
- Security isolation so only simple & secure parts of code can handle keys.
- Flash protection from both external use and untrusted code segments.
- 256 KB of memory to support hardened crypto implementations and, later, additional features such as OpenPGP or SSH.
- No NDA needed to develop for.


# Documentation

- [new official documentation](https://docs.nitrokey.com/fido2/)
- [old documentation](https://www.nitrokey.com/documentation/installation)
- [upstream documentation](https://docs.solokeys.io/solo/)



### Touch Button and LED Behavior

See documentation: https://docs.nitrokey.com/fido2/windows/#touch-button-and-led-behavior.

## Nitrokey Reset
See documentation: https://docs.nitrokey.com/fido2/windows/#nitrokey-reset.


## pynitrokey
Take a look at [pynitrokey](https://github.com/Nitrokey/pynitrokey) for key management and client applications development.

# Development

## Simulation (No Hardware Needed)

Clone this repository and build it

```bash
git clone --recurse-submodules https://github.com//Nitrokey/nitrokey-fido2-firmware
cd nitrokey-fido2-firmware
make all
```

This builds the firmware as a standalone application. The application is set up to send and recv USB HID messages over UDP to ease development and reduce need for hardware.

Testing can be done using our fork of Yubico's client software, python-fido2. Solokey's fork of python-fido2 has small changes to make it send USB HID over UDP to the authenticator application. You can install Solokey's fork by running the following:

```bash
pip install -r tools/requirements.txt
```

Run the application:
```bash
./main
```

In another shell, you can run Solokey's [test suite](https://github.com/solokeys/fido2-tests).

You can find more details in Solokey's [documentation](https://docs.solokeys.io/solo/), including how to build on the the NUCLEO-L432KC development board.


## Firmware Build

Firmware can be built in a Docker container by calling:
```text
make docker-build-toolchain
make docker-build-all
```

This will result in a debug and release firmwares in the `./builds` directory.

## Firmware size

For the Nitrokey FIDO2 2.4.0 release, the total binary size is less than 84 kB (22kB bootloader included) in the release (size optimized) mode.
This information can be seen during the build or verified by hand by loading the hex image - both presented below:

```text
# shown during the build - bootloader
Memory region         Used Size  Region Size  %age Used
           flash:       20292 B        20 KB     99.08%
       flash_cfg:           4 B       2040 B      0.20%
             ram:       17896 B        48 KB     36.41%
           sram2:          0 GB        16 KB      0.00%
arm-none-eabi-size bootloader.elf
   text    data     bss     dec     hex filename
  19892     404   17496   37792    93a0 bootloader.elf

# actual image

Memory region         Used Size  Region Size  %age Used
           flash:       62148 B     159736 B     38.91%
             ram:       29680 B        48 KB     60.38%
           sram2:         656 B        16 KB      4.00%

arm-none-eabi-size solo.elf
   text    data     bss     dec     hex filename
  61492     656   29680   91828   166b4 solo.elf

```

Verifying the same in Python:
```python
import intelhex # requires intelhex package (tested 2.3.0)
i = intelhex.IntelHex()
i.loadfile(open('release/nitrokey-fido2-firmware-2.4.0.nitrokey-2-gc073c7a-all-to_flash.hex'))
a = len(i)
print(a)
# 83132
# or
a = list(y-x for x,y in i.segments())
# [20292, 62148, 2, 8, 4, 678]
print(sum(a))
# 83132
```

User data right now is 38 kB, which makes a total of 119 kB minimum flash size required.

From the user data region, 20 kB are reserved for the FIDO2 RKs. Minimal user space required is 8 kB (without the RKs, including attestation certificate/key and device key) but can be shrunk since most of it is wasted in padding.


# Contributors

Contributors are welcome. The ultimate goal is to have a FIDO2 security key supporting USB, NFC, and BLE interfaces, that can run on a variety of MCUs.

Look at the issues to see what is currently being worked on. Feel free to add issues as well.


# License

This software is fully open source.

All software, unless otherwise noted, is dual licensed under Apache 2.0 and MIT.
You may use this software under the terms of either the Apache 2.0 license or MIT license.

All hardware, unless otherwise noted, is dual licensed under CERN and CC-BY-SA.
You may use the hardware under the terms of either the CERN 2.1 license or CC-BY-SA 4.0 license.

All documentation, unless otherwise noted, is licensed under CC-BY-SA.
You may use the documentation under the terms of the CC-BY-SA 4.0 license


[![FOSSA Status](https://app.fossa.com/api/projects/git%2Bgithub.com%2FNitrokey%2Fnitrokey-fido2-firmware.svg?type=large)](https://app.fossa.com/projects/git%2Bgithub.com%2FNitrokey%2Fnitrokey-fido2-firmware?ref=badge_large)

# Where To Buy

Available in our [online shop](https://shop.nitrokey.com/shop/product/nitrokey-fido2-55).
