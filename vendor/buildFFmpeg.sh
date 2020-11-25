#!/bin/bash

set -e
cd FFmpeg

# Set your own NDK here
NDK=~/extsoft/android-ndk-r14b

ARM_PLATFORM=$NDK/platforms/android-21/arch-arm/
ARM_PREBUILT=$NDK/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64

ARM64_PLATFORM=$NDK/platforms/android-21/arch-arm64/
ARM64_PREBUILT=$NDK/toolchains/aarch64-linux-android-4.9/prebuilt/darwin-x86_64

X86_PLATFORM=$NDK/platforms/android-21/arch-x86/
X86_PREBUILT=$NDK/toolchains/x86-4.9/prebuilt/darwin-x86_64

X86_64_PLATFORM=$NDK/platforms/android-21/arch-x86_64/
X86_64_PREBUILT=$NDK/toolchains/x86_64-4.9/prebuilt/darwin-x86_64

BUILD_DIR=`pwd`/android
X264_DIR=../x264/android
#HEVC_DIR=/home/shiruiwei/thirdparty/x265/build/android
#    --enable-libx265

function build_one
{
if [ $ARCH == "arm" ]
then
    PLATFORM=$ARM_PLATFORM
    PREBUILT=$ARM_PREBUILT
    HOST=arm-linux-androideabi
elif [ $ARCH == "arm64" ]
then
    PLATFORM=$ARM64_PLATFORM
    PREBUILT=$ARM64_PREBUILT
    HOST=aarch64-linux-android
elif [ $ARCH == "x86_64" ]
then
    PLATFORM=$X86_64_PLATFORM
    PREBUILT=$X86_64_PREBUILT
    HOST=x86_64-linux-android
fi

#    --prefix=$PREFIX \

# TODO Adding aac decoder brings "libnative.so has text relocations. This is wasting memory and prevents security hardening. Please fix." message in Android.
pushd ./
./configure --target-os=android \
    --incdir=$BUILD_DIR/include \
    --libdir=$BUILD_DIR/lib/$CPUDIR \
    --enable-cross-compile \
    --extra-libs="-lgcc" \
    --arch=$ARCH \
    --cc=$PREBUILT/bin/$HOST-gcc \
    --ranlib=$PREBUILT/bin/$HOST-ranlib \
    --cross-prefix=$PREBUILT/bin/$HOST- \
    --nm=$PREBUILT/bin/$HOST-nm \
    --sysroot=$PLATFORM \
    --extra-cflags="-fvisibility=hidden -fdata-sections -ffunction-sections -Os -fPIC -DANDROID \
        -DCONFIG_MPEG4_ENCODER=0 -DHAVE_SYS_UIO_H=1 -Dipv6mr_interface=ipv6mr_ifindex \
        -I$X264_DIR/$CPUDIR/include -I$HEVC_DIR/$CPUDIR/include \
        -fasm -Wno-psabi -fno-short-enums -fno-strict-aliasing -finline-limit=300 $OPTIMIZE_CFLAGS " \
    --enable-static \
    --disable-shared \
    --enable-small \
    --extra-ldflags="-Wl,-rpath-link=$PLATFORM/usr/lib -L$X264_DIR/$CPUDIR/lib -L$PLATFORM/usr/lib -nostdlib -lc -lm -ldl -llog" \
    --disable-everything \
    --disable-ffplay \
    --disable-ffmpeg \
    --disable-ffprobe \
    --disable-avfilter \
    --disable-avdevice \
    --disable-debug \
	--disable-nonfree \
	--enable-swscale \
	--disable-avfilter \
    --enable-gpl                \
	--enable-encoder=aac        \
	--enable-encoder=h264       \
	--enable-decoder=h264       \
    --enable-libx264            \
    --enable-encoder=libx264    \
    --enable-encoder=libx265    \
    --enable-decoder=hevc       \
    --enable-encoder=hevc       \
   	--enable-decoder=mpeg4      \
	--enable-decoder=aac        \
    --enable-decoder=mp1        \
    --enable-decoder=mp2        \
    --enable-decoder=mp3        \
	--enable-muxer=mpegts       \
	--enable-demuxer=h264       \
    --enable-demuxer=hevc       \
	--enable-demuxer=m4v        \
	--enable-demuxer=mlv        \
	--enable-demuxer=mov        \
	--enable-demuxer=mp3        \
    --enable-demuxer=mp12       \
    --enable-demuxer=aac        \
	--enable-demuxer=mpegvideo  \
    --enable-parser=aac         \
    --enable-parser=aac_latm    \
    --enable-parser=mpegaudio   \
    --enable-encoder=mjpeg      \
    --enable-muxer=mp4          \
    $ADDITIONAL_CONFIGURE_FLAG

make clean
# fix bug for build ffmpeg3.0+
#rm compat/strtod.o
make -j4 install
$PREBUILT/bin/$HOST-ar d libavcodec/libavcodec.a inverse.o
#$PREBUILT/bin/$HOST-ld -rpath-link=$PLATFORM/usr/lib -L$PLATFORM/usr/lib  -soname libffmpeg.so -shared -nostdlib  -z,noexecstack -Bsymbolic --whole-archive --no-undefined -o $PREFIX/libffmpeg.so libavcodec/libavcodec.a libavformat/libavformat.a libavutil/libavutil.a libswscale/libswscale.a -lc -lm -lz -ldl -llog  --warn-once  --dynamic-linker=/system/bin/linker $PREBUILT/lib/gcc/$HOST/4.9/libgcc.a
popd
}

#arm64-v8a
CPU=arm64-v8a
CPUDIR=arm64-v8a
ARCH=arm64
OPTIMIZE_CFLAGS=
PREFIX=$BUILD_DIR/$CPUDIR
ADDITIONAL_CONFIGURE_FLAG=
build_one

#arm v7n
#CPU=armv7-a
#CPUDIR=armeabi-v7a
#ARCH=arm
#OPTIMIZE_CFLAGS="-mfloat-abi=softfp -mfpu=neon -marm -march=$CPU -mtune=cortex-a8"
#PREFIX=$BUILD_DIR/$CPUDIR
#ADDITIONAL_CONFIGURE_FLAG=--enable-neon
#build_one

#arm v6
#CPU=armv6
#CPUDIR=armeabi
#ARCH=arm
#OPTIMIZE_CFLAGS="-marm -march=$CPU"
#PREFIX=$BUILD_DIR/$CPUDIR 
#ADDITIONAL_CONFIGURE_FLAG=--disable-neon
#build_one

#x86
#CPU=x86
#CPUDIR=x86
#ARCH=x86
#OPTIMIZE_CFLAGS="-fomit-frame-pointer"
#PREFIX=$BUILD_DIR/$CPUDIR
#ADDITIONAL_CONFIGURE_FLAG=--disable-asm
#build_one
