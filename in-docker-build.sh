#!/bin/bash -xe
version=$1

cp -rf /solo-base/ /solo/

cd /solo/targets/stm32l432
pwd

out_dir="/builds"
rm -rf ${out_dir}/*

function build() {
    part=${1}
    output=${2}
    release=${3}
    what="${part}"

    rm -rf release/*
    make ${what} VERSION_FULL=${version} RELEASE=${release}
    mkdir -p ${out_dir}/${output}/
    cp release/* ${out_dir}/${output}/
}

build debug-release-buildv debug 0
build release-buildv release 1

arm-none-eabi-gcc --version | head -1
find ${out_dir} | xargs sha256sum || true
echo "done"
