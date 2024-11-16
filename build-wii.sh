#! /bin/sh

CONSOLE=wii

. /etc/profile.d/devkit-env.sh
. "$DEVKITPRO/${CONSOLE}vars.sh"
export PKG_CONFIG_PATH=$($DEVKITPRO/portlibs_prefix.sh $CONSOLE)/lib/pkgconfig/:$PORTLIBS_PPC/lib/pkgconfig
make \
    -j16 \
    CPPFLAGS="$CPPFLAGS" \
    CFLAGS="$CFLAGS" \
    LDFLAGS="$LDFLAGS -L$DEVKITPRO/libogc/lib/$CONSOLE" \
    ENABLE_FETCH=0 \
    ENABLE_NLS=0 \
    PLATFORM=$CONSOLE \
    -o sols
