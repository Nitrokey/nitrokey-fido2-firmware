FROM ubuntu:20.10
MAINTAINER Nitrokey <info@nitrokey.com>

# Install necessary packages
RUN apt-get update  \
  && apt-get install -y --no-install-recommends \
    ca-certificates \
    make \
    wget \
    bzip2 \
    git \
    python3 \
    python3-pip \
  && rm -rf /var/lib/apt/lists/*

ENV GCC_URL="https://developer.arm.com/-/media/Files/downloads/gnu-rm/8-2019q3/RC1.1/gcc-arm-none-eabi-8-2019-q3-update-linux.tar.bz2?revision=c34d758a-be0c-476e-a2de-af8c6e16a8a2?product=GNU%20Arm%20Embedded%20Toolchain,64-bit,,Linux,8-2019-q3-update"
ENV GCC_NAME="gcc-arm-none-eabi-8-2019-q3-update"
ENV GCC_SHA256="b50b02b0a16e5aad8620e9d7c31110ef285c1dde28980b1a9448b764d77d8f92 gcc.tar.bz2"
ENV GCC_MD5="6341f11972dac8de185646d0fbd73bfc gcc.tar.bz2"
ENV PATH="/opt/${GCC_NAME}/bin/:${PATH}"

# Install ARM compiler
RUN set -eux; \
    wget -O gcc.tar.bz2 ${GCC_URL} || echo "Wget returned error"; \
    echo ${GCC_MD5} | md5sum -c -; \
	echo ${GCC_SHA256} | sha256sum -c -; \
	tar -C /opt -xf gcc.tar.bz2; \
	rm gcc.tar.bz2;

ENV PATH="/opt/${GCC_NAME}/bin/:${PATH}"

RUN pip install -U pynitrokey
RUN pip install -U fido2==0.8.1
RUN nitropy version && arm-none-eabi-gcc --version | head -1
