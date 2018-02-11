#!/bin/bash -ex
# latest version of this file can be found at
# https://android.googlesource.com/platform/external/lldb-utils
#
# Download & build glog on the local machine
# works on Linux, OS X, and Windows (Cygwin)
# leaves output in /tmp/prebuilts/libglog/$OS-x86

PROJ=libglog
VER=0.3.5
MSVS=2015

# source "$(dirname "${BASH_SOURCE[0]}")/build-common.sh" "$@"
BASE=${PROJ#lib}-$VER
TGZ=v${VER}.tar.gz

# curl -L https://github.com/google/glog/archive/$TGZ -o $TGZ

tar xzf $TGZ || cat $TGZ # if this fails, we're probably getting an http error
cd $BASE
# patch -p3 <"$SCRIPT_DIR/glog.patches"
case "$OS" in
	windows)
		devenv google-glog.sln /Upgrade
		devenv google-glog.sln /Build Debug
		devenv google-glog.sln /Build Release
		cp -a Debug $INSTALL/
		cp -a Release $INSTALL/
		mkdir -p $INSTALL/include
		cp -a src/windows/glog $INSTALL/include/
		;;
	linux|darwin)
		mkdir $RD/build
		cd $RD/build
		$RD/$BASE/configure --prefix=$INSTALL
		make -j$CORES
		make install
		;;
esac

case "$OS" in
	darwin)
		LIB=lib/libglog.0.dylib
		install_name_tool -id @executable_path/../$LIB $INSTALL/$LIB
		;;
esac
