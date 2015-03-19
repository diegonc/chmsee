#!/usr/bin/env sh

CWD=`pwd`

set -e
trap 'STATUS=$?; cd "$CWD"; exit $STATUS' ERR

WORKING_DIR="`cd .. && pwd`/chmsee-deps/pacman"
VERSION="4.2.1"
URL="https://sources.archlinux.org/other/pacman/pacman-$VERSION.tar.gz"

sudo apt-get install -qq libarchive-dev

mkdir -p "$WORKING_DIR"
cd "$WORKING_DIR"

wget -c "$URL"

mkdir src
tar -xzf "pacman-$VERSION.tar.gz" -C src

cd "src/pacman-$VERSION"

./configure --prefix=/usr --sysconfdir=/etc \
    --localstatedir=/var --disable-doc \
    --with-scriptlet-shell=/usr/bin/bash \
    --with-ldconfig=/usr/bin/ldconfig
make
make -C contrib
sudo make install
sudo make -C contrib install

cd "$CWD"
