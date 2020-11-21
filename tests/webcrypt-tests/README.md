# Nitrokey Webcrypt Tests



## Setting up

### Tests
1. Python 3.7 with `pipenv`

To install Python dependencies for this project only it suffices to call:
```bash
pipenv install
```

### Building simulation
Please refer to the main project readme for details. These are required:
1. CMake 3.13
2. GNU Makefile
3. GCC

Quick build commands reference:
```bash
mkdir cmake-build-debug
cd cmake-build-debug
cmake ..
make -j4
```

## Running tests

### Simulation
It is possible to test the implementation using simulation without the actual hardware. The process and tests communicate via UDP.


#### Running
To run simulation:
```bash
cd cmake-build-debug
# to clean state
rm *.bin
./nitrokey-fido2-simulation
```

Running actual tests:
```bash
pipenv run pytest test_comm.py
```

Same, but write live logs to output:
```bash
pipenv run pytest test_comm.py  -svx --log-cli-level=DEBUG
```

### Hardware

Similarly to the simulation case, but with `--hardware` switch.

```bash
pipenv run pytest --hardware test_comm.py
```

## CI
For the CI use a Makefile is prepared, which automatically setups and runs both the simulation and tests.
Execution:
```bash
# while being in the tests directory: nitrokey-fido2-firmware/tests/webcrypt-tests
make -j2
```
