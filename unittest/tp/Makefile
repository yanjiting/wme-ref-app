
USER_DEPTH = .

include $(USER_DEPTH)/bld/linux/env.make
include $(USER_DEPTH)/bld/linux/platform.make

USER_BIN = unittest_tp

USER_SRCS = \
	main.cpp	\
	CmConnectionInterfaceTest.cpp	\
	CmCsLenPkgHeadLen16BitTest.cpp	\
	CmReactorBaseTest.cpp	\
	CmWSHandleRFC6455Test.cpp	\


USER_INCLDIRS +=	\
	-I./	\
	-I../../vendor/gtest/gtest-1.7.0/include	\
	-I../../vendor/gtest/gtest-1.7.0	\
	-I../../vendor/gtest/gmock-1.7.0/include	\
	-I../../vendor/gtest/gmock-1.7.0/	\
	-I../../mediaengine/util/include	\
	-I../../mediaengine/util/include/linux	\
	-I../../mediaengine/util/include/client	\
	-I../../mediaengine/tp/src	\
	-I../../mediaengine/tp/include	\
	-I../../api	\

USER_DEFFLAGS = -DCM_DISABLE_EVENT_REPORT -DSUPPORT_DNS -DSUPPORT_REACTOR -DCM_ENABLE_EPOLL -DSUPPORT_HTTP -DCM_SUPPORT_OPENSSL_VERIFY
USER_DEFFLAGS += -DNON_ROOT_SUPPORT

USER_LINK_LIBS =	\
	../../vendor/gtest/bld/linux/gtest/libgtest.a	\
	../../vendor/gtest/bld/linux/gmock/libgmock.a	\
	../../distribution/linux/libtp.a	\
	../../distribution/linux/libutil.a	\
	-lpthread -lssl -lcrypto -lpthread -ldl -lresolv -lexpat

USER_OBJ_DIR = $(USER_DEPTH)/bld/linux/objs/common/util

USER_BIN_DIR = $(USER_DEPTH)/bld/linux/bin

include $(USER_DEPTH)/bld/linux/all_in_one.make
