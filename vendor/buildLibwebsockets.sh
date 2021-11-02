mkdir libwebsockets/out/
cd libwebsockets/out/
/Users/shiruiwei/Library/Android/sdk/cmake/3.6.4111459/bin/cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=~/extsoft/android-ndk-r16b/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI="arm64-v8a" \
    -DANDROID_NDK=~/extsoft/android-ndk-r16b \
    -DANDROID_NATIVE_API_LEVEL=21 \
    -DANDROID_ARM_NEON=ON \
    -DANDROID_TOOLCHAIN=clang \
    -DLWS_WITHOUT_TESTAPPS=1 \
    -DLWS_HAVE_EVP_MD_CTX_free=1 \
    -DLWS_HAVE_HMAC_CTX_new=1 \
    -DLWS_OPENSSL_INCLUDE_DIRS=../../openssl/include -DLWS_OPENSSL_LIBRARIES="../../openssl/libssl.a;../../openssl/libcrypto.a"

make -j4
