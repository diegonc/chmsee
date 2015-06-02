#!/usr/bin/env sh

CWD=`pwd`

set -e
trap 'STATUS=$?; cd "$CWD"; exit $STATUS' ERR

WORKING_DIR="`cd .. && pwd`/chmsee-deps/xulrunner"
VERSION=38.0.1
TAR_FILE=xulrunner-$VERSION.en-US.linux-x86_64.sdk.tar.bz2
URL="ftp://ftp.mozilla.org/pub/mozilla.org/xulrunner/releases/$VERSION/sdk/$TAR_FILE"

sudo update-alternatives --query gcc || true
sudo update-alternatives --query g++ || true

sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
sudo apt-get update -qq
sudo apt-get install -qq g++-4.8
sudo update-alternatives --set gcc /usr/bin/gcc-4.8

mkdir -p "$WORKING_DIR"
cd "$WORKING_DIR"

wget -c "$URL"

tar -xjf "$TAR_FILE"

cd "$CWD"
