#!/usr/bin/env sh

CWD=`pwd`

set -e
trap 'STATUS=$?; cd "$CWD"; exit $STATUS' ERR

WORKING_DIR="`cd .. && pwd`/chmsee-deps/xulrunner"
VERSION=31.0
URL="ftp://ftp.mozilla.org/pub/mozilla.org/xulrunner/releases/$VERSION/sdk/xulrunner-$VERSION.en_US.linux-x86_64.sdk.tar.bz2"

sudo apt-get install libgtk2.0-dev libnss3-dev libxt-dev libhunspell-dev libstartup-notification0-dev libdbus-glib-1-dev libpulse-dev libevent-dev libvpx-dev libicu-dev libffi-dev libasound2-dev libicu-dev zip unzip pkg-config diffutils yasm libegl1-mesa-dev libgstreamer-plugins-base0.10-dev wget

mkdir -p "$WORKING_DIR"
cd "$WORKING_DIR"

wget -c "$URL"

tar -xjf "xulrunner-$VERSION.source.tar.bz2"

cd "$CWD"
