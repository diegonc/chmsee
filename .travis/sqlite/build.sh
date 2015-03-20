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

export CFLAGS="-O2 -fno-strict-aliasing \
	-DSQLITE_SECURE_DELETE -DSQLITE_ENABLE_COLUMN_METADATA \
	-DSQLITE_ENABLE_FTS3 -DSQLITE_ENABLE_RTREE=1 -DSQLITE_SOUNDEX=1 \
	-DSQLITE_ENABLE_UNLOCK_NOTIFY \
	-DSQLITE_OMIT_LOOKASIDE=1"

./configure --prefix=/usr --disable-static \
       --libdir=\${prefix}/lib/$DEB_HOST_MULTIARCH \
       --libexecdir=\${libdir}/sqlite3
make
sudo make install

cd "$CWD"
