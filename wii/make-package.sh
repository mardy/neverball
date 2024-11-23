#! /bin/sh

set -e

NAME=neverball
TMPDIR="$(mktemp -d)"
OUTDIR="$TMPDIR/$NAME"
SRCDIR="$(pwd)"

mkdir -p "$OUTDIR"
cp -a \
    wii/meta.xml \
    wii/icon.png \
    "$OUTDIR"
mkdir -p "$OUTDIR/data"
cd "$OUTDIR/data"
# assumes Debian or Ubuntu neverball-data=1.6.0+git20180603-3build2
cp -a \
    /usr/share/games/neverball/* \
    .
rm -r \
    "ttf/"*
cp -a \
    "$SRCDIR/data/ttf/" \
    .
cd -
mkdir -p "$OUTDIR/.neverball-dev"
cp wii/neverballrc "$OUTDIR/.neverball-dev/"
elf2dol "$NAME.elf" "$OUTDIR/boot.dol"

cd $TMPDIR
zip -r "$NAME.zip" "$NAME"
cd -
mv "$TMPDIR/$NAME.zip" .
rm -rf "$TMPDIR"
