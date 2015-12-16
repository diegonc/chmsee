#!/usr/bin/env sh

CWD=`pwd`

set -e
trap 'STATUS=$?; cd "$CWD"; exit $STATUS' ERR

WORKING_DIR="`cd .. && pwd`/chmsee-deps/xulrunner"
VERSION=41.0.2
TAR_FILE=xulrunner-$VERSION.en-US.linux-x86_64.sdk.tar.bz2
URL="http://archive.mozilla.org/pub/xulrunner/releases/$VERSION/sdk/$TAR_FILE"

sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
sudo apt-get update -qq
sudo apt-get install -qq g++-4.8 libnss3-dev
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.8 90
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 90

mkdir -p "$WORKING_DIR"
cd "$WORKING_DIR"

wget -c "$URL"

tar -xjf "$TAR_FILE"

cd "$CWD"
