# On modern systems, udev has a TAG uaccess, which is used in 73-seat-late.rules
# On older systems, we use GROUP plugdev with MODE
# --> Try `make setup` first, if it doesn't work, try `make legacy-setup`.


setup: install activate
legacy-setup: install-legacy activate

RULES_PATH=/etc/udev/rules.d

activate:
	sudo udevadm control --reload-rules
	sudo udevadm trigger

install:
	sudo cp $(PWD)/70-nitrokey-fido2-access.rules ${RULES_PATH}/70-nitrokey-fido2-access.rules

install-legacy:
	sudo cp $(PWD)/70-nitrokey-fido2-legacy-access.rules ${RULES_PATH}/70-nitrokey-fido2-access.rules
