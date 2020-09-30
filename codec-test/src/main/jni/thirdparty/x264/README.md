Get source from https://code.videolan.org/videolan/x264.git, build with comand:
>
> https://github.com/imalimin/HardwareVideoCodec
> NDK=~/opt/android-ndk-r14b
> PLATFORM=$NDK/platforms/android-21/arch-arm64/
> TOOLCHAIN=$NDK/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64
> ARM_INC=$PLATFORM/usr/include/
> ARM_LIB=$PLATFORM/usr/lib/
> PREFIX=./android/arm64-v8a
>
> ./configure \
>     --prefix=$PREFIX \
>     --enable-pic \
>     --enable-static \
>     --enable-shared \
>     --host=aarch64-linux \
>     --cross-prefix=$TOOLCHAIN/bin/aarch64-linux-android- \
>     --sysroot=$PLATFORM \
>     --extra-cflags="-fPIC -DX264_VERSION -DANDROID -DHAVE_PTHREAD -DNDEBUG -static -D__ARM_ARCH_8__ -D__ARM_ARCH_8A__ -O3 -march=armv8-a -mtune=cortex-a57.cortex-a53 -ftree-vectorize -ffast-math"
>
> make clean
> make -j4
> make install

