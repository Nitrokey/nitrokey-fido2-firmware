# Udev rules installation


This is for Linux systems only. Check out our [official documentation](https://www.nitrokey.com/documentation/installation) for the details.

To install the separate Nitrokey FIDO2 udev rules, allowing access to your key, run

```
make install
```

This should work assuming your system is reasonably up-to-date. If not, try

```
make install-legacy
```

which would install Udev rules in the legacy format.
