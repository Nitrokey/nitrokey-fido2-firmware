# On modern systems, udev has a TAG uaccess, which is used in 73-seat-late.rules
# On older systems, we use GROUP plugdev with MODE
# --> Try `make setup` first, if it doesn't work, try `make legacy-setup`.


setup: | install activate
	@echo "*** Setup finished. You can now use the Nitrokey FIDO2 device."

legacy-setup: | install-legacy activate
	@echo "*** Setup for the legacy systems finished. You can now use the Nitrokey FIDO2 device."

RULES_PATH=/etc/udev/rules.d

activate:
	@echo "*** This action will reload the udev deamon"
	sudo udevadm control --reload-rules
	sudo udevadm trigger

install:
	@echo "*** This action will copy the rules file to the $(RULES_PATH) directory"
	sudo cp $(PWD)/70-nitrokey-fido2-access.rules ${RULES_PATH}/70-nitrokey-fido2-access.rules

install-legacy:
	@echo "*** This action will copy the legacy rules file to the $(RULES_PATH) directory"
	sudo cp $(PWD)/70-nitrokey-fido2-legacy-access.rules ${RULES_PATH}/70-nitrokey-fido2-access.rules
