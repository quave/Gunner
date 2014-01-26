# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := libgl2jni
LOCAL_CFLAGS    := -Werror
LOCAL_SRC_FILES :=  gl_code.cpp
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv2
LOCAL_STATIC_LIBRARIES := cpufeatures android_native_app_glue ndk_helper

ifneq ($(filter %armeabi-v7a,$(TARGET_ARCH_ABI)),)
LOCAL_CFLAGS += -mhard-float -D_NDK_MATH_NO_SOFTFP=1
LOCAL_LDLIBS += -lm_hard
ifeq (,$(filter -fuse-ld=mcld,$(APP_LDFLAGS) $(LOCAL_LDFLAGS)))
LOCAL_LDFLAGS += -Wl,--no-warn-mismatch
endif
endif

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/ndk_helper)
$(call import-module,android/native_app_glue)
$(call import-module,android/cpufeatures)
