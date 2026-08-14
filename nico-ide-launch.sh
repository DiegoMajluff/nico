#!/bin/sh
DIR="$(cd "$(dirname "$0")" && pwd)"

export QT_AUTO_SCREEN_SCALE_FACTOR="${QT_AUTO_SCREEN_SCALE_FACTOR:-0}"
export QT_SCALE_FACTOR="${QT_SCALE_FACTOR:-1}"
export XCURSOR_SIZE="${XCURSOR_SIZE:-2}"

exec "$DIR/nico-ide/build/nico-ide" "$@"
