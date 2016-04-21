
# for native libs
ifeq ($(ENABLE_GCOV),1)
LOCAL_SRC_FILES += $(ROOT)/build/android/gcov_flush.cpp
LOCAL_CFLAGS += -fprofile-arcs -ftest-coverage
LOCAL_LDLIBS += -lgcov -lgcc
endif

# for unittest apps
ifeq ($(HAVE_GCOV_FLAG),1)
LOCAL_CFLAGS += -DENABLED_GCOV_FLAG -DRUN_ON_DEVICE
endif

