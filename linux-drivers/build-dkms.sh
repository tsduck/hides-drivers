#!/bin/bash
# Build script for the Linux DKMS for HiDes device drivers.

SCRIPT=$(basename $BASH_SOURCE)
ROOTDIR=$(cd $(dirname $BASH_SOURCE); pwd)
export TMPDIR="$ROOTDIR/tmp"

# Basic functions (cleanexit does automatic cleanup).
cleanexit() { rm -rf "$TMPDIR"; exit $1; }
verbose() { echo >&2 "$SCRIPT: $*"; }
error() { echo >&2 "$SCRIPT: $*"; cleanexit 1; }

# Cleanup if interrupted.
trap cleanexit SIGINT

# Package version from a text file here.
VERSION=$(grep -m1 DRIVER_RELEASE_VERSION "$ROOTDIR/it950x_driver/it950x-core.h" | sed -e 's/^[^"]*"[^0-9]*//' -e 's/".*$//')
verbose "building version $VERSION"

# Target installation directory for DKMS.
DKMS_INSTALL="/usr/src/hides-$VERSION"

# Build a replica in a temporary directory.
SRCDIR="$TMPDIR/hides-dkms-$VERSION"
DKMS_DIR="$SRCDIR/hides-$VERSION"

# We build the packages here.
PKGDIR="$ROOTDIR/packages"

# Build DKMS directory.
verbose "building DKMS structure in $DKMS_DIR"
rm -rf "$TMPDIR"
mkdir -p "$DKMS_DIR" || cleanexit 1
sed -e "s/{{VERSION}}/$VERSION/g" "$ROOTDIR/config/dkms.conf.template" >"$DKMS_DIR/dkms.conf"
sed -e "s/{{VERSION}}/$VERSION/g" "$ROOTDIR/config/Makefile.template" >"$DKMS_DIR/Makefile"
cp -r "$ROOTDIR/it950x_driver" "$DKMS_DIR"
make -C "$DKMS_DIR" --no-print-directory --silent clean
cp "$ROOTDIR/60-hides.rules" "$ROOTDIR/60-hides.perms" "$SRCDIR"
find "$SRCDIR" -type f -print0 | xargs -0 chmod 644
find "$SRCDIR" -type d -print0 | xargs -0 chmod 755

# Build a source tarball for the hides-dkms package.
mkdir -p "$PKGDIR" || cleanexit 1
SRCTARBALL="$PKGDIR/hides-dkms-$VERSION.tgz"
verbose "creating $SRCTARBALL"
tar czp -f "$SRCTARBALL" -C "$TMPDIR" --owner=0 --group=0 "hides-dkms-$VERSION"

# Determine operating system type and version.
FC_DISTRO=$(grep " release " /etc/fedora-release 2>/dev/null | sed -e 's/^.* release \([0-9\.]*\) .*$/\1/')
RH_DISTRO=$(grep " release " /etc/redhat-release 2>/dev/null | sed -e 's/^.* release \([0-9\.]*\) .*$/\1/')
UB_DISTRO=$([[ -f /etc/lsb-release ]] && source /etc/lsb-release 2>/dev/null && [[ "`tr A-Z a-z <<<$DISTRIB_ID`" = ubuntu ]] && echo $DISTRIB_RELEASE)
DB_DISTRO=$(head -1 /etc/debian_version 2>/dev/null)
DISTRO_PREFIX=
DISTRO_VERSION=
RPM_DISTRO=false
DEB_DISTRO=false
if [[ -n "$FC_DISTRO" ]]; then
    DISTRO_PREFIX=".fc"
    DISTRO_VERSION="$FC_DISTRO"
    RPM_DISTRO=true
elif [[ -n "$RH_DISTRO" ]]; then
    DISTRO_PREFIX=".el"
    DISTRO_VERSION="$RH_DISTRO"
    RPM_DISTRO=true
elif [[ -n "$UB_DISTRO" ]]; then
    DISTRO_VERSION="$UB_DISTRO"
    DEB_DISTRO=true
elif [[ -n "$DB_DISTRO" ]]; then
    DISTRO_VERSION="$DEB_DISTRO"
    DEB_DISTRO=true
fi
DISTRO="$DISTRO_PREFIX"$(sed <<<"$DISTRO_VERSION" -e 's/\..*//')
verbose "Linux distro: $DISTRO, full version: $DISTRO_VERSION"
! $RPM_DISTRO && ! $DEB_DISTRO && error "unsupported Linux distro"

# Build RPM distro.
if $RPM_DISTRO; then
    verbose "building RPM package"

    # User's rpmbuild environment:
    RPMBUILD="$HOME/rpmbuild"
    if [[ ! -d "$RPMBUILD" ]]; then
        [[ -z "$(which rpmdev-setuptree 2>/dev/null)" ]] && error "rpmdev-setuptree not found, install package rpmdevtools"
        rpmdev-setuptree
        [[ ! -d "$RPMBUILD" ]] && error "$RPMBUILD not found"
    fi

    # Build RPM package.
    RELEASE=0
    cp -f "$SRCTARBALL" $RPMBUILD/SOURCES/
    rpmbuild -ba -D "version $VERSION" -D "release $RELEASE$DISTRO" "$ROOTDIR/config/hides-dkms.spec"

    # Collect package files.
    cp -f \
        "$RPMBUILD/RPMS/noarch/hides-dkms-$VERSION-$RELEASE$DISTRO.noarch.rpm" \
        "$RPMBUILD/SRPMS/hides-dkms-$VERSION-$RELEASE$DISTRO.src.rpm" \
        "$PKGDIR"
fi

# Build DEB distro.
if $DEB_DISTRO; then
    verbose "building DEB package"

    # Root of .deb package build.
    DEBDIR="$TMPDIR/deb"
    rm -rf "$DEBDIR"

    # Build file structure of the package.
    mkdir -p -m 0755 "$DEBDIR/DEBIAN" "$DEBDIR/usr/src" "$DEBDIR/etc/udev/rules.d" "$DEBDIR/etc/security/console.perms.d"
    for f in control postinst prerm; do
        sed -e "s/{{VERSION}}/$VERSION/g" "$ROOTDIR/config/$f.template" >"$DEBDIR/DEBIAN/$f"
    done
    chmod 755 "$DEBDIR/DEBIAN/postinst" "$DEBDIR/DEBIAN/prerm"
    cp -rp "$SRCDIR/hides-$VERSION" "$DEBDIR/usr/src"
    install -m 644 "$SRCDIR/60-hides.rules" "$DEBDIR/etc/udev/rules.d"
    install -m 644 "$SRCDIR/60-hides.perms" "$DEBDIR/etc/security/console.perms.d"

    # Build the binary package
    dpkg --build "$DEBDIR" "$PKGDIR"
fi

# Final cleanup.
cleanexit 0
