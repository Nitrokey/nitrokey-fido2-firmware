FROM ubuntu:xenial

RUN apt update
#RUN apt install -yq --no-install-suggests --no-install-recommends software-properties-common python-software-properties
#RUN apt-add-repository -y "ppa:ubuntu-toolchain-r/test"
#RUN apt update
#RUN apt-get -yq --no-install-suggests --no-install-recommends install gcc-7
RUN apt-get -yq --no-install-suggests --no-install-recommends install build-essential
RUN apt-get -yq --no-install-suggests --no-install-recommends install cppcheck python3 python3-venv make git python3-dev python3-cffi libffi-dev
RUN apt-get -yq --no-install-suggests --no-install-recommends install libc-dev-bin libasan2 libc6-dev libgcc-5-dev libstdc++-5-dev
RUN apt-get -yq --no-install-suggests --no-install-recommends install libasan2-dbg libtsan0-dbg libubsan0-dbg liblsan0-dbg psmisc
ENV CC=gcc
