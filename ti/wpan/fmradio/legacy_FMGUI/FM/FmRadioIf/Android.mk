LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := fmradioif
LOCAL_MODULE_TAGS := eng
LOCAL_SRC_FILES := $(call all-java-files-under, src) \
           src/java/com/ti/fm/IFmRadio.aidl

include $(BUILD_JAVA_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := com.ti.fm.fmradioif.xml
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/permissions
LOCAL_SRC_FILES := $(LOCAL_MODULE)

LOCAL_CERTIFICATE := platform
include $(BUILD_PREBUILT)

