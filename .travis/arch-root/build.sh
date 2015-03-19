#!/usr/bin/env bash

CWD=`pwd`

set -e
trap 'STATUS=$?; cd "$CWD"; exit $STATUS' ERR

DEPS_DIR="`cd .. && pwd`/chmsee-deps"
PAC_CONF="$DEPS_DIR/pacman.conf"
ARCH_DIR="$DEPS_DIR/arch-root"
DB_DIR="$ARCH_DIR/var/lib/pacman"
CACHE_DIR="$ARCH_DIR/var/cache/pacman/pkg"
LOG_DIR="$ARCH_DIR/var/log"
PACKAGES="bash bzip2 coreutils diffutils file findutils gawk gcc-libs \
          gettext glibc grep gzip less perl sed tar base-devel xulrunner"

cat > "$PAC_CONF" << EOF
[options]
RootDir = $ARCH_DIR
DBPath = $DB_DIR
CacheDir = $CACHE_DIR
LogFile = $LOG_DIR/pacman.log
SigLevel = Never
Architecture = auto

[core]
Server = http://mirror.rackspace.com/archlinux/\$repo/os/\$arch

[extra]
Server = http://mirror.rackspace.com/archlinux/\$repo/os/\$arch

[community]
Server = http://mirror.rackspace.com/archlinux/\$repo/os/\$arch

EOF

mkdir -m755 -p "$ARCH_DIR"
mkdir -m755 -p "$DB_DIR"
mkdir -m755 -p "$CACHE_DIR"
mkdir -m755 -p "$LOG_DIR"

sudo pacman --config "$PAC_CONF" --noconfirm --noprogressbar -Sy
sudo pacman --config "$PAC_CONF" --noconfirm --noscriptlet --noprogressbar -S $PACKAGES

cd "$CWD"
