#!/bin/sh
export PACPUS_ROOT=$PACPUS_ROOT_IGEP

BUILD_DIR=./build_arm
if [ -z $1 ]; then J="1"; else J="$1"; fi

# get the correct version of cmake for ARM architecture
if [ -f  ${OECORE_NATIVE_SYSROOT}/usr/bin/cmake ]; then
	CMAKE_NATIVE=${OECORE_NATIVE_SYSROOT}/usr/bin/cmake
else
	echo "cross toolchain is not installed, please read https://devel.hds.utc.fr/projects/igep/wiki/toolchain/install"
exit 1
fi

if [ -d "$BUILD_DIR" ]; then
    rm -rf $BUILD_DIR
fi

mkdir $BUILD_DIR
cd $BUILD_DIR

$CMAKE_NATIVE .. -DPACPUS_INSTALL_DIR=$PACPUS_ROOT  \
        -DCMAKE_TOOLCHAIN_FILE=${OECORE_CMAKE_CROSS_TOOLCHAIN} ../..

make -j$J
sudo make install


