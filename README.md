# Nitrokey FIDO2

Nitrokey FIDO2 is an open source security key which supports FIDO2 and U2F standards for strong two-factor authentication and password-less login. It protects you against phishing and other online attacks. It's based on [Solokey](https://github.com/solokeys/solo) and currently under development.

This repo contains the firmware, including implementations of FIDO2 and U2F (CTAP2 and CTAP) over USB and NFC. The main implementation is for STM32L432, but it is easily portable.

For development no hardware is needed, it also runs as a standalone application for Windows, Linux, and Mac OSX. If you like (or want to learn) hardware instead, you can run the firmware on the NUCLEO-L432KC development board.


# Security

Nitrokey FIDO2 is based on the STM32L432 microcontroller. It offers the following security features.

- True random number generation to guarantee random keys.
- Security isolation so only simple & secure parts of code can handle keys.
- Flash protection from both external use and untrusted code segments.
- 256 KB of memory to support hardened crypto implementations and, later, additional features such as OpenPGP or SSH.
- No NDA needed to develop for.


# Development (No Hardware Needed)

Clone this repository and build it

```bash
git clone --recurse-submodules https://github.com/solokeys/solo
cd solo
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


# Documentation

Check out our [official documentation](https://docs.solokeys.io/solo/).


# Contributors

Solo is an upgrade to [U2F Zero](https://github.com/conorpp/u2f-zero). It was born from Conor's passion for making secure hardware, and from our shared belief that security should be open to be trustworthy, in hardware like in software.

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


[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2Fsolokeys%2Fsolo.svg?type=large)](https://app.fossa.io/projects/git%2Bgithub.com%2Fsolokeys%2Fsolo?ref=badge_large)

# Where To Buy

Not available yet. Please subscribe our [newsletter](https://www.nitrokey.com/newsletter) to get informed right away.
