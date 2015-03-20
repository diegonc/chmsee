#!/usr/bin/env bash

CWD=`pwd`

set -e
trap 'STATUS=$?; cd "$CWD"; exit $STATUS' ERR

XULRUNNER_SDK=`cd .. && pwd`/chmsee-deps/xulrunner/xulrunner-sdk

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
