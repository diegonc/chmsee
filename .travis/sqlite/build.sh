#!/usr/bin/env sh

CWD=`pwd`

set -e
trap 'STATUS=$?; cd "$CWD"; exit $STATUS' ERR

WORKING_DIR="`cd .. && pwd`/chmsee-deps/sqlite"
VERSION=3080803
URL="http://www.sqlite.org/2015/sqlite-autoconf-$VERSION.tar.gz"
DEB_HOST_MULTIARCH=`dpkg-arichitecture -qDEB_HOST_MULTIARCH`

#sudo apt-get install libgtk2.0-dev libnss3-dev libxt-dev libhunspell-dev libstartup-notification0-dev libdbus-glib-1-dev libpulse-dev libevent-dev libvpx-dev libicu-dev libffi-dev libasound2-dev libsqlite3-dev zip unzip pkg-config diffutils yasm libegl1-mesa-dev libgstreamer-plugins-base0.10-dev wget

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
