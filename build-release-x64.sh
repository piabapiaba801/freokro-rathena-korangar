#!/usr/bin/env bash
set -euo pipefail

# FreokRO fast native 64-bit Release build.
# Defaults favor compile speed and low memory use while retaining runtime optimization.
JOBS="${JOBS:-2}"
TARGET="${1:-all}"

export CFLAGS="${CFLAGS:--O2 -DNDEBUG -m64}"
export CXXFLAGS="${CXXFLAGS:--O2 -DNDEBUG -m64}"
export LDFLAGS="${LDFLAGS:--m64}"

# LTO deliberately disabled: it increases build/link time and memory considerably.
# 64-bit is the configure default; --disable-64bit is intentionally never used.
if [[ ! -f Makefile || ! -f config.status ]]; then
  ./configure --disable-debug --disable-lto --enable-packetver=20250716 \
    CFLAGS="$CFLAGS" CXXFLAGS="$CXXFLAGS" LDFLAGS="$LDFLAGS"
fi

case "$TARGET" in
  clean) exec make clean ;;
  login|char|map|web|all) exec make -j"$JOBS" "$TARGET" ;;
  *) echo "Usage: $0 [all|login|char|map|web|clean]" >&2; exit 2 ;;
esac
