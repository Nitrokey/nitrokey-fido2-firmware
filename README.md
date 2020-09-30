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

For convenience, the first FIDO operation is automatically accepted within two seconds after connecting Nitrokey FIDO2. In this case touching the touch button is not required.

To avoid accidental and malicious reset of the Nitrokey, the required touch confirmation time for the FIDO2 reset operation is longer and with a distinct LED behavior (red LED light) than normal operations.

For convenience, multiple operations can be accepted with a single touch. For this, keep the touch button touched for up to 10 seconds.



| LED Color                    | Event                                                                   | Time Period                                                               | Comments                                                                                                                                                                                                                                          |
|------------------------------|-------------------------------------------------------------------------|---------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Any \(blinking\)             | Awaiting for touch                                                      | Until touch is confirmed or timed out                                     |                                                                                                                                                                                                                                                   |
| Any \(blinking faster\)      | Touch detected, counting seconds                                        | Until touch is confirmed or timed out                                     |                                                                                                                                                                                                                                                   |
| White \(blinks\)              | Touch request for FIDO registration or authentication operation      |                                                                           | Requires 1 second touch to complete; timeout is usually about 30 seconds                                                                                                                                                                                                               |
|  Yellow \(blinks\)            | Touch request for configuration operation                             |                                                                           | Requires 5 seconds touch to complete; e.g. used for activating firmware update mode                                                                                                                                                                                                              |
| Red \(blinks\)                | Touch request for reset operation                                     | Available only during the very first 10 seconds after Nitrokey is powered | Requires 5 seconds touch to complete; e.g. used for FIDO2 reset operation                                                                                                                                                                                                              |
| Green \(constant\)           | Touch accepted, Nitrokey is active and accepting further FIDO2 operations | After touch was registered, 10 seconds timeout                            | For the FIDO registration or authentication operations after a confirmation Nitrokey enters into "activation" mode, auto\-accepting any following mentioned operations until touch button is released, but not longer than 10 seconds                               |
| Blue \(constant\)            | Touch consumed \- accepted and used up by the operation                 | Until touch is released                                                   | Touch consumption here means, that without releasing the touch button, and touching again the Nitrokey will not confirm any new operations                                                                                                          |
| White <br/> \(single blink\) | Nitrokey ready to work                                                    | 0\.5 seconds after powering up                                            |                                                                                                                                                                                                                                                   |
| - \(no LED signal\)            | Nitrokey is idle                              |  |  |
| - \(no LED signal\)            | Auto\-accept single FIDO registration or authentication operation                              | Within first 2 seconds after powering up                                        | Nitrokey is automatically accepting any single FIDO registration or authentication operation upon insertion event \- the latter is treated as an equivalent of the touch button registration signal \(user presence\); the configuration/reset operations are not accepted |
| All colors                   | Nitrokey is in Firmware Update mode                                       | Active until firmware update operation is successful, or until reinsertion | If the firmware update fails, the Nitrokey will stay in the this mode until the firmware is written correctly                                                                                           |


### Nitrokey FIDO2 Reset with Windows 10
To avoid accidental and malicious reset of the Nitrokey, the required touch confirmation time for the FIDO2 reset operation is longer and with a distinct LED behavior (red LED light) than normal operations.

If the total taken time for execution will be more than 10 seconds, the Windows OS' user interface will report failure. Reset operation is executed on the Nitrokey even after the latter is reported, as long as the user's touch will be registered before the Nitrokey's internal operation timeout (touch confirmation is shown with the blue color).

<details>
<summary>Technical details</summary>

The FIDO2 reset operation under Windows consist of two operations:
1. single FIDO2 operation
2. actual FIDO2 reset operation

The first operation should be automatically accepted upon insertion, and user should be required to only confirm the second operation. See "VM notes" below for additional information.
</details>

#### Example scenarios
##### Successful
1. User activates FIDO2 reset in the Windows 10 user interface
1. User is asked to remove and insert the Nitrokey once again
2. User is asked to touch the Nitrokey
5. Nitrokey starts blinking red
6. User is asked to single touch the Nitrokey
7. User touches the Nitrokey for 5 seconds
8. Nitrokey flashes blue LED, user releases the touch
9. UI reports success

##### Timeout failure
1. User activates FIDO2 reset in the Windows 10 user interface
1. User is asked to remove and insert the Nitrokey once again
2. User is asked to touch the Nitrokey
3. Nitrokey starts blinking red
4. User is not confirming the operation within 10 seconds
5. UI reports failure


##### VM notes
In case where the Nitrokey is attached through a Virtual Machine instance to Windows 10 OS, the powering up and enumeration times might be different, which could result in the very first FIDO operation not being accepted automatically. In such case it has to be confirmed by hand.

Similarly, the reset operation could fail, if the OS will send the reset operation request after passing the first 10 seconds of Nitrokey being powered up.


# Development (No Hardware Needed)

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
