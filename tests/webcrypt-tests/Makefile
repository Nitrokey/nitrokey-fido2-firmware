SIMTIME=30
SIMNAME=nitrokey-fido2-simulation
SIMDIR=../../cmake-build-debug
SIM=$(SIMDIR)/$(SIMNAME)

all: CI

$(SIM):
	mkdir -p $(SIMDIR)
	cd $(SIMDIR) &&	cmake .. && $(MAKE) -j2

.PHONY: pipenv
pipenv: Pipfile.lock

Pipfile.lock:
	pipenv install

.PHONY: CI
CI: $(SIM) pipenv
	# cleanup
	-killall $(SIMNAME)
	-rm -v *.bin
	# start time-limited simulation
	( $(SIM) > simulation.log && echo "Simulation started") &
	(sleep $(SIMTIME) && killall $(SIMNAME) && echo "Simulation stopped") &
	sleep 1
	# start tests
	pipenv run pytest test_comm.py  -svx # --log-cli-level=DEBUG

.PHONY: clean clean-full
clean:
	-rm -v *.bin *.log $(SIM)

clean-full: clean
	pipenv uninstall --all
	-rm Pipfile.lock
