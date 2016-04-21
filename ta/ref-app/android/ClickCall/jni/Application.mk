APP_ABI := armeabi-v7a
APP_STL := c++_shared
APP_PLATFORM := android-9
LIBCXX_USE_GABIXX := true

#BUILD_ALL_WME_IN_REFAPP := 1
ifeq ($(BUILD_ALL_WME_IN_REFAPP), 1)
APP_CPPFLAGS := -frtti -std=c++11 -fexceptions -g
NDK_TOOLCHAIN_VERSION := 4.8
endif
