#!/usr/bin/env bash

CWD=`pwd`

set -e
trap 'STATUS=$?; cd "$CWD"; exit $STATUS' ERR

XULRUNNER_SDK=`cd .. && pwd`/chmsee-deps/xulrunner/xulrunner-sdk

sudo apt-get install libchm-dev

# Native
cd src

cat > Makefile << EOF
LIBXUL_SDK = $XULRUNNER_SDK

NSPR_INCLUDES = -I/usr/include/nspr
NSPR_LIBS = -lplds4 -lplc4 -lnspr4 -lpthread -ldl

CHMLIB_INCLUDES = -I/usr/include
CHMLIB_LIBS = -lchm

PYTHON2 = /usr/bin/python
XPCOMCHM = libxpcomchm.so

DEFINES = -pthread
LDFLAGS = -Wl,-soname,\${TARGET} -L\${LIBXUL_SDK}/bin -lpthread

include ./common.mk
EOF

make

cd "$CWD"
exit 0

# chroot
DEPS_DIR="`cd .. && pwd`/chmsee-deps"
ARCH_DIR="$DEPS_DIR/arch-root"
SRC_DIR="$ARCH_DIR/chmsee"

mkdir "$SRC_DIR"
sudo mount --bind "$CWD" "$SRC_DIR"

sudo chroot "$ARCH_DIR" /usr/bin/bash -c 'cd /chmsee/src && make -f Makefile.arch'
