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

ENV GCC_URL="https://developer.arm.com/-/media/Files/downloads/gnu-rm/8-2018q4/gcc-arm-none-eabi-8-2018-q4-major-linux.tar.bz2?revision=d830f9dd-cd4f-406d-8672-cca9210dd220?product=GNU%20Arm%20Embedded%20Toolchain,64-bit,,Linux,8-2018-q4-major"
ENV GCC_NAME="gcc-arm-none-eabi-8-2018-q4-major"
ENV GCC_SHA256="fb31fbdfe08406ece43eef5df623c0b2deb8b53e405e2c878300f7a1f303ee52 gcc.tar.bz2"
ENV GCC_MD5="f55f90d483ddb3bcf4dae5882c2094cd gcc.tar.bz2"

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
