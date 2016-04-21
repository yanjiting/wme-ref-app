APP_MODULES := dolphin-unittest
APP_ABI := armeabi-v7a
APP_PLATFORM := android-11
APP_STL      := c++_shared
APP_CPPFLAGS += -fexceptions -frtti
include $(call all-subset-makefiles)
