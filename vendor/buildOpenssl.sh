# https://github.com/openssl/openssl/blob/master/NOTES-ANDROID.md
export ANDROID_NDK_ROOT=~/Library/Android/sdk/ndk/23.1.7779620
PATH=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/darwin-x86_64/bin:$PATH

./Configure android-arm64 -D__ANDROID_API__=21
make -j4
