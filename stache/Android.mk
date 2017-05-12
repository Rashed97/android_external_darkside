LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    container.c \
	keys.c \
    stache.c

LOCAL_C_INCLUDES := \
    external/keyutils \
	external/libsodium/src/libsodium/include

LOCAL_SHARED_LIBRARIES := \
    libkeyutils \
	liblog \
	libsodium

LOCAL_MODULE := stached
LOCAL_MODULE_TAGS := optional
LOCAL_INIT_RC := stached.rc

include $(BUILD_EXECUTABLE)
