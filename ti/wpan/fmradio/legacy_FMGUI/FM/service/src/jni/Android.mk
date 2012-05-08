LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
BTIPS_TARGET_PLATFORM?=zoom2
FM_MCP_STK?=1


FM_STACK_ROOT = hardware/ti/wpan/fmradio/fm_stack

include $(CLEAR_VARS)

#ifeq ($(OMAP_ENHANCEMENT),true)
LOCAL_SRC_FILES +=	JFmRxNative.cpp \
			JFmTxNative.cpp
#endif

LOCAL_C_INCLUDES += \
        $(JNI_H_INCLUDE) \
        $(FM_STACK_ROOT)/MCP_Common/Platform/fmhal/inc \
        $(FM_STACK_ROOT)/MCP_Common/Platform/fmhal/inc/int \
        $(FM_STACK_ROOT)/MCP_Common/Platform/fmhal/LINUX/android_zoom2/inc \
        $(FM_STACK_ROOT)/MCP_Common/Platform/os/LINUX/android_zoom2/inc \
        $(FM_STACK_ROOT)/MCP_Common/Platform/fmhal/LINUX/common/inc \
        $(FM_STACK_ROOT)/MCP_Common/Platform/os/LINUX/common/inc \
        $(FM_STACK_ROOT)/MCP_Common/Platform/inc \
        $(FM_STACK_ROOT)/MCP_Common/tran \
        $(FM_STACK_ROOT)/MCP_Common/inc \
        $(FM_STACK_ROOT)/MCP_Common/Platform/inc/2.2_zoom2 \
        $(FM_STACK_ROOT)/HSW_FMStack/stack/inc \
        $(FM_STACK_ROOT)/HSW_FMStack/stack/inc/int \

LOCAL_SHARED_LIBRARIES := \
	libnativehelper \
	libcutils \
	libutils \
	liblog 


ifeq ($(BOARD_HAVE_BLUETOOTH),true)
LOCAL_C_INCLUDES += \
	external/dbus \
	system/bluetooth/bluez-clean-headers
LOCAL_CFLAGS += -DHAVE_BLUETOOTH
LOCAL_SHARED_LIBRARIES += libbluedroid libdbus
endif

ifeq ($(FM_MCP_STK),1)
LOCAL_SHARED_LIBRARIES += libfmstack
LOCAL_SHARED_LIBRARIES += libmcphal
endif
LOCAL_MODULE_TAGS := eng

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE := libfmradio

include $(BUILD_SHARED_LIBRARY)

