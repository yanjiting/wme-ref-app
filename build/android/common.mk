#
# usage: $(call fgetkey,key:val), $(call fgetval,key:val,default)
fgetkey = $(firstword $(subst :, ,$1))
fgetval = $(or $(word 2,$(subst :, ,$1)),$(value 2))


#
# usage: 
#  $(eval $(call prebuilt_shared_module,"mod_name","path_of_lib"))
# 		 
#  MODULES := mod1_name:lib1_path [mod2_name:lib2_path ...]
#  $(foreach mod,$(MODULES),$(eval $(call prebuilt_shared_module,$(call fgetkey,$(mod)),$(call fgetval,$(mod),"default"))))
define prebuilt_shared_module
ifneq ($(BUILD_ALL_IN_ONE_MK), 1)
ifneq ($($(1)_shared), 1)
ifeq ($(2), $(wildcard $(2)))
include $(CLEAR_VARS)
LOCAL_MODULE := $(1)
LOCAL_SRC_FILES := $(2)
$(1)_shared := 1
include $(PREBUILT_SHARED_LIBRARY)
endif
endif
endif
endef


#
# usage: 
#  $(eval $(call prebuilt_static_module,"mod_name","path_of_lib"))
#  
#  MODULES := mod1_name:lib1_path [mod2_name:lib2_path ...]
#  $(foreach mod,$(MODULES),$(eval $(call prebuilt_static_module,$(call fgetkey,$(mod)),$(call fgetval,$(mod),"default"))))
define prebuilt_static_module
ifneq ($(BUILD_ALL_IN_ONE_MK), 1)
ifneq ($($(1)_static), 1)
ifeq ($(2), $(wildcard $(2)))
include $(CLEAR_VARS)
LOCAL_MODULE := $(1)
LOCAL_SRC_FILES := $(2)
$(1)_static := 1
include $(PREBUILT_STATIC_LIBRARY)
endif
endif
endif
endef


#
# usage: $(eval $(call include_module,"path_of_Android.mk"))
# 		 MODULES := mod1_Android.mk [mod2_Android.mk ...]
#        $(foreach mod,$(MODULES),$(eval $(call include_module,$(mod))))
define include_module
include $(CLEAR_VARS)
include $(1)
endef

