# setup the xmake config to build for android target
set-android:
    xmake f -p android -a arm64-v8a --ndk=~/Android/Sdk/ndk/28.0.12433566/ --android_sdk=~/Android/Sdk/ --ndk_sdkver=24 -c

set-jetson:
    xmake f -p linux -a arm64 -c

set-default:
    xmake f -p linux -a x86_64 -c
