#!/usr/bin/env sh

CWD=`pwd`

set -e
trap 'STATUS=$?; cd "$CWD"; exit $STATUS' ERR

WORKING_DIR="`cd .. && pwd`/chmsee-deps/sqlite"
VERSION=3080803
URL="http://www.sqlite.org/2015/sqlite-autoconf-$VERSION.tar.gz"
DEB_HOST_MULTIARCH=`dpkg-architecture -qDEB_HOST_MULTIARCH`

mkdir -p "$WORKING_DIR"
cd "$WORKING_DIR"

wget -c "$URL"

mkdir src
tar -xzf "sqlite-autoconf-$VERSION.tar.gz" -C src

cd "src/sqlite-autoconf-$VERSION"

./configure --prefix=/usr --disable-static \
       --libdir=\${prefix}/lib/$DEB_HOST_MULTIARCH \
       --libexecdir=\${libdir}/sqlite3
make
sudo make install

cd "$CWD"
