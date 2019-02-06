
env3_sim:
	python3 -m venv env3_sim
	cd python-fido2 && ../env3_sim/bin/python3 setup.py install

.PHONY: test_only
test_only: env3_sim
	./env3_sim/bin/python3 -u tools/ctap_test.py

.PHONY: test_simulation
test_simulation: env3_sim $(name)
	$(CC) --version
	./env3_sim/bin/python3 --version
	-rm -v authenticator*
	-killall $(name)
	./$(name) &
	./env3_sim/bin/python3 -u python-fido2/examples/credential.py
	./env3_sim/bin/python3 -u python-fido2/examples/get_info.py
	./env3_sim/bin/python3 -u python-fido2/examples/multi_device.py
	./env3_sim/bin/python3 -u tools/ctap_test.py
	@echo "!!! All tests returned non-error code"