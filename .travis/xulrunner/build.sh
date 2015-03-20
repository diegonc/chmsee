#!/usr/bin/env sh

CWD=`pwd`

set -e
trap 'STATUS=$?; cd "$CWD"; exit $STATUS' ERR

WORKING_DIR="`cd .. && pwd`/chmsee-deps/xulrunner"
VERSION=31.0
URL="ftp://ftp.mozilla.org/pub/mozilla.org/xulrunner/releases/$VERSION/source/xulrunner-$VERSION.source.tar.bz2"

sudo apt-get install libgtk2.0-dev libnss3-dev libxt-dev libhunspell-dev libstartup-notification0-dev libdbus-glib-1-dev libpulse-dev libevent-dev libvpx-dev libicu-dev libffi-dev libasound2-dev libicu-dev zip unzip pkg-config diffutils yasm libegl1-mesa-dev libgstreamer-plugins-base0.10-dev wget

mkdir -p "$WORKING_DIR"
cd "$WORKING_DIR"

wget -c "$URL"

mkdir src
tar -xjf "xulrunner-$VERSION.source.tar.bz2" -C src

cd src/mozilla-release
cp "$CWD/.travis/xulrunner/mozconfig" .mozconfig

heartbeat() {
  local i=0
  while true; do
    echo -n ".";
    let "i += 1"
    if [ "$i" -eq 79 ]; then
      echo
      let i=0
    fi
    sleep 1;
  done
}
heartbeat &
HEARTBEAT_PID=$!

build() {
     make -f client.mk build \
  || make -f client.mk build \
  || make -f client.mk build
}

build 2>&1 | gzip -c > "$WORKING_DIR/build.log.gz" || {
	STATUS=$?
	kill $HEARTBEAT_PID || true
	echo
	echo Recent log:
	zcat "$WORKING_DIR/build.log.gz" | tail -n500
	exit $STATUS
}
kill $HEARTBEAT_PID || true

sudo make -f client.mk DESTDIR="/usr" install

cd "$CWD"
