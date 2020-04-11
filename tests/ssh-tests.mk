IMAGE=nkfido2-tests-ssh-ubuntu 
KPATH='/root/.ssh/id_ecdsa_sk' 
CMD=/usr/bin/ssh-keygen -t ecdsa-sk -f $(KPATH) -v
CMD2=$(CMD) -O resident
.PHONY: ubuntu-ssh
ubuntu-ssh:
	sudo docker build  -t $(IMAGE) -f ssh-ubuntu.dockerfile .
	sudo docker run -it --rm --privileged  $(IMAGE) $(CMD)
	sudo docker run -it --rm --privileged  $(IMAGE) $(CMD2)
