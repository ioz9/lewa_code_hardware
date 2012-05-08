#
# Copyright (C) 2008 The Android Open Source Project
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
ifeq ($(BUILD_FM_RADIO),true)
ifeq ($(BOARD_HAVE_BLUETOOTH),true)
ifeq ($(FM_CHR_DEV_ST),true)
include hardware/ti/wpan/fmradio/fm_chrlib/Android.mk
endif
include hardware/ti/wpan/fmradio/fm_stack/Android.mk
include hardware/ti/wpan/fmradio/fm_app/Android.mk
endif
ifeq ($(BUILD_TI_FM_APPS),true)
include hardware/ti/wpan/fmradio/legacy_FMGUI/FM/Android.mk
endif
endif
