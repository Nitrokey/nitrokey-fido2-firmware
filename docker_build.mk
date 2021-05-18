DOCKER_TOOLCHAIN_IMAGE := nitrokey/nitrokey-fido2-firmware-build

docker-build-toolchain:
	docker build -t $(DOCKER_TOOLCHAIN_IMAGE) .
	docker tag $(DOCKER_TOOLCHAIN_IMAGE):latest $(DOCKER_TOOLCHAIN_IMAGE):${SOLO_VERSION}
	docker tag $(DOCKER_TOOLCHAIN_IMAGE):latest $(DOCKER_TOOLCHAIN_IMAGE):${SOLO_VERSION_MAJ}
	docker tag $(DOCKER_TOOLCHAIN_IMAGE):latest $(DOCKER_TOOLCHAIN_IMAGE):${SOLO_VERSION_MAJ}.${SOLO_VERSION_MIN}

uncached-docker-build-toolchain:
	docker build --no-cache -t $(DOCKER_TOOLCHAIN_IMAGE) .
	docker tag $(DOCKER_TOOLCHAIN_IMAGE):latest $(DOCKER_TOOLCHAIN_IMAGE):${SOLO_VERSION}
	docker tag $(DOCKER_TOOLCHAIN_IMAGE):latest $(DOCKER_TOOLCHAIN_IMAGE):${SOLO_VERSION_MAJ}
	docker tag $(DOCKER_TOOLCHAIN_IMAGE):latest $(DOCKER_TOOLCHAIN_IMAGE):${SOLO_VERSION_MAJ}.${SOLO_VERSION_MIN}

docker-build-all:
	docker run --rm -v "$(CURDIR)/builds:/builds" \
					-v "$(CURDIR):/solo-base:ro" \
				    $(DOCKER_TOOLCHAIN_IMAGE) "solo-base/in-docker-build.sh" ${SOLO_VERSION_FULL}
				    # -u $(shell id -u ${USER}):$(shell id -g ${USER}) \ # this was in line 18 but it breaks the ci

test-docker:
	rm -rf builds/*
	$(MAKE) uncached-docker-build-toolchain
	NTAGS=$$(docker images | grep -c "$(DOCKER_TOOLCHAIN_IMAGE)") && [ $$NTAGS -eq 4 ]
	$(MAKE) docker-build-all
