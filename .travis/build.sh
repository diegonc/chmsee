#!/usr/bin/env bash

CWD=`pwd`

set -e
trap 'STATUS=$?; cd "$CWD"; exit $STATUS' ERR

# Native
cd src
make -f Makefile.debian
cd "$CWD"
exit 0

# chroot
DEPS_DIR="`cd .. && pwd`/chmsee-deps"
ARCH_DIR="$DEPS_DIR/arch-root"
SRC_DIR="$ARCH_DIR/chmsee"

mkdir "$SRC_DIR"
sudo mount --bind "$CWD" "$SRC_DIR"

sudo chroot "$ARCH_DIR" /usr/bin/bash -c 'cd /chmsee/src && make -f Makefile.arch'
