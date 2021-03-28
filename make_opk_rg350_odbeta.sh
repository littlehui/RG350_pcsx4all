#!/bin/sh

TOOLCHAIN_DIR="/opt/gcw0-toolchain/usr/bin"
OPK_NAME=pcsx4all_rg350_odbeta.opk

echo ${OPK_NAME}

# create default.gcw0.desktop
cat > default.gcw0.desktop <<EOF
[Desktop Entry]
Name=Pcsx4all
Comment=Sony PSX emulator
Exec=pcsx4all -iso %f
Terminal=false
Type=Application
StartupNotify=true
Icon=pcsxr-icon-small
Categories=emulators;
X-OD-NeedsDownscaling=true
EOF

# create opk
FLIST="pcsx4all"
FLIST="${FLIST} default.gcw0.desktop"
FLIST="${FLIST} src/port/sdl/pcsxr-icon-small.png"

rm -f ${OPK_NAME}
${TOOLCHAIN_DIR}/mksquashfs ${FLIST} ${OPK_NAME} -all-root -no-xattrs -noappend -no-exports

cat default.gcw0.desktop
rm -f default.gcw0.desktop
