/*
 *
 * Copyright 2001-2010 Texas Instruments, Inc. - http://www.ti.com/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "android_runtime/AndroidRuntime.h"
#include "jni.h"
#include "JNIHelp.h"
#include "McpJbtlLog.h"

#define LOG_TAG "JFmRxNative"
#include <cutils/properties.h>

using namespace android;

extern "C" {
#include "fm_rx.h"
#include "fmc_common.h"
#include "mcp_hal_log.h"

    typedef FmRxEvent fm_rx_event;
    void nativeJFmRx_Callback(const fm_rx_event *event);
#ifdef DEBUG
    extern void MCP_HAL_LOG_EnableLogToAndroid(const char *app_name);
#endif

} //extern "C"

static JavaVM *g_jVM = NULL;
static jclass _sJClass;

static jmethodID _sMethodId_nativeCb_fmRxRawRDS;
static jmethodID _sMethodId_nativeCb_fmRxRadioText;
static jmethodID _sMethodId_nativeCb_fmRxPiCodeChanged;
static jmethodID _sMethodId_nativeCb_fmRxPtyCodeChanged;
static jmethodID _sMethodId_nativeCb_fmRxPsChanged;
static jmethodID _sMethodId_nativeCb_fmRxMonoStereoModeChanged;
static jmethodID _sMethodId_nativeCb_fmRxAudioPathChanged;
static jmethodID _sMethodId_nativeCb_fmRxAfSwitchFreqFailed;
static jmethodID _sMethodId_nativeCb_fmRxAfSwitchStart;
static jmethodID _sMethodId_nativeCb_fmRxAfSwitchComplete;
static jmethodID _sMethodId_nativeCb_fmRxAfListChanged;
static jmethodID _sMethodId_nativeCb_fmRxCompleteScanDone;
static jmethodID _sMethodId_nativeCb_fmRxCmdEnable;
static jmethodID _sMethodId_nativeCb_fmRxCmdDisable;

static jmethodID _sMethodId_nativeCb_fmRxCmdEnableAudio;
static jmethodID _sMethodId_nativeCb_fmRxCmdChangeAudioTarget;
static jmethodID _sMethodId_nativeCb_fmRxCmdSetBand;
static jmethodID _sMethodId_nativeCb_fmRxCmdGetBand;
static jmethodID _sMethodId_nativeCb_fmRxCmdSetMonoStereoMode;
static jmethodID _sMethodId_nativeCb_fmRxCmdGetMonoStereoMode;
static jmethodID _sMethodId_nativeCb_fmRxCmdGetMuteMode;
static jmethodID _sMethodId_nativeCb_fmRxCmdSetMuteMode;
static jmethodID _sMethodId_nativeCb_fmRxCmdSetRfDependentMuteMode;
static jmethodID _sMethodId_nativeCb_fmRxCmdGetRfDependentMuteMode;
static jmethodID _sMethodId_nativeCb_fmRxCmdSetRssiThreshhold;
static jmethodID _sMethodId_nativeCb_fmRxCmdGetRssiThreshhold;
static jmethodID _sMethodId_nativeCb_fmRxCmdSetDeemphasisFilter;
static jmethodID _sMethodId_nativeCb_fmRxCmdGetDeemphasisFilter;
static jmethodID _sMethodId_nativeCb_fmRxCmdSetVolume;
static jmethodID _sMethodId_nativeCb_fmRxCmdGetVolume;
static jmethodID _sMethodId_nativeCb_fmRxCmdGetChannelSpacing;
static jmethodID _sMethodId_nativeCb_fmRxCmdSetChannelSpacing;
static jmethodID _sMethodId_nativeCb_fmRxCmdTune;
static jmethodID _sMethodId_nativeCb_fmRxCmdGetTunedFrequency;
static jmethodID _sMethodId_nativeCb_fmRxCmdSeek;
static jmethodID _sMethodId_nativeCb_fmRxCmdStopSeek;
static jmethodID _sMethodId_nativeCb_fmRxCmdGetRssi;
static jmethodID _sMethodId_nativeCb_fmRxCmdEnableRds;
static jmethodID _sMethodId_nativeCb_fmRxCmdDisableRds;
static jmethodID _sMethodId_nativeCb_fmRxCmdGetRdsSystem;
static jmethodID _sMethodId_nativeCb_fmRxCmdSetRdsSystem;
static jmethodID _sMethodId_nativeCb_fmRxCmdSetRdsGroupMask;
static jmethodID _sMethodId_nativeCb_fmRxCmdGetRdsGroupMask;
static jmethodID _sMethodId_nativeCb_fmRxCmdSetRdsAfSwitchMode;
static jmethodID _sMethodId_nativeCb_fmRxCmdGetRdsAfSwitchMode;
static jmethodID _sMethodId_nativeCb_fmRxCmdDisableAudio;
static jmethodID _sMethodId_nativeCb_fmRxCmdDestroy;
static jmethodID _sMethodId_nativeCb_fmRxCmdChangeDigitalAudioConfiguration;
static jmethodID _sMethodId_nativeCb_fmRxCmdGetFwVersion;
static jmethodID _sMethodId_nativeCb_fmRxCmdIsValidChannel;
static jmethodID _sMethodId_nativeCb_fmRxCmdGetCompleteScanProgress;
static jmethodID _sMethodId_nativeCb_fmRxCmdStopCompleteScan;


static int nativeJFmRx_Create(JNIEnv *env,jobject obj,jobject jContextValue)
{
    FmRxStatus fmStatus;
    FmRxContext *fmRxContext =NULL;
    jclass contextCls = NULL;
    jmethodID setValueMethodId = NULL;
    void * notValid;

#ifdef DEBUG
    /* Initialize logging module */
    MCP_HAL_LOG_Init();
    MCP_HAL_LOG_EnableLogToAndroid("FM Process");
#endif

    MCP_JBTL_LOGD("Java_JFmRx_nativeJFmRx_Create(): Entered");

    fmStatus = FM_RX_Init(notValid);

    if (fmStatus != FM_RX_STATUS_SUCCESS)
    {
        MCP_JBTL_LOGE("nativeJFmRx_Create: FM_RX_Init Failed ");
        return FMC_STATUS_FAILED;
    }

    MCP_JBTL_LOGD("nativeJFmRx_Create: FM_RX_Init returned %d", (int)fmStatus);

    fmStatus = FM_RX_Create(NULL,nativeJFmRx_Callback,&fmRxContext);

    if (fmStatus != FM_RX_STATUS_SUCCESS)
    {
        MCP_JBTL_LOGE("nativeJFmRx_Create: FM_RX_Create Failed ");
        return FMC_STATUS_FAILED;
    }

    MCP_JBTL_LOGD("nativeJFmRx_Create: FM_RX_Create returned %d, context: %x",
                  (int)fmStatus,
                  (unsigned int)fmRxContext);

    MCP_JBTL_LOGD("nativeJFmRx_create: Setting context value in jContext out parm");

    /* Pass received fmContext via the jContextValue object */
    contextCls = env->GetObjectClass(jContextValue);
    if (contextCls == NULL)
    {
        MCP_JBTL_LOGD("nativeJFmRx_create: Failed obtaining class for JBtlProfileContext");
        return FMC_STATUS_FAILED;
    }


    setValueMethodId = env->GetMethodID(contextCls, "setValue", "(I)V");
    if (setValueMethodId == NULL)
    {
        MCP_JBTL_LOGD("nativeJFmRx_create: Failed getting setValue method id");
        return FMC_STATUS_FAILED;
    }
    MCP_JBTL_LOGD("nativeJFmRx_create: Calling Java setValue(ox%x) in context's class", (unsigned int)fmRxContext);


    env->CallVoidMethod(jContextValue, setValueMethodId, (unsigned int)fmRxContext);
    if (env->ExceptionOccurred())
    {
        MCP_JBTL_LOGE("nativeJFmRx_create: Calling CallVoidMethod(setValue) failed");
        env->ExceptionDescribe();
        return FMC_STATUS_FAILED;
    }


    MCP_JBTL_LOGD("nativeJFmRx_create:Exiting Successfully");

    return fmStatus;


}



static int nativeJFmRx_Destroy(JNIEnv *env, jobject obj,jlong jContextValue)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;
    FmRxStatus  status ;
    MCP_JBTL_LOGD("nativeJFmRx_destroy(): Entered");

    status = FM_RX_Destroy(&fmRxContext);
    MCP_JBTL_LOGD("nativeJFmRx_destroy: FM_RX_Destroy() returned %d",(int)status);

    status = FM_RX_Deinit();
    MCP_JBTL_LOGD("nativeJFmRx_destroy: FM_RX_Deinit() returned %d",(int)status);


#ifdef DEBUG
    /* De-initialize logging module */
    MCP_HAL_LOG_Deinit();
#endif

    MCP_JBTL_LOGD("nativeJFmRx_destroy(): Exit");

    return status;
}



static int nativeJFmRx_Enable(JNIEnv *env, jobject obj, jlong jContextValue)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;
    MCP_JBTL_LOGD("nativeJFmRx_enable(): Entered");

    FmRxStatus  status =FM_RX_Enable(fmRxContext);
    MCP_JBTL_LOGD("nativeJFmRx_enable: FM_RX_Enable() returned %d",(int)status);


    MCP_JBTL_LOGD("nativeJFmRx_enable(): Exit");
    return status;
}



static int nativeJFmRx_Disable(JNIEnv *env, jobject obj, jlong jContextValue)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;
    MCP_JBTL_LOGD("nativeJFmRx_disable(): Entered");

    FmRxStatus  status =FM_RX_Disable(fmRxContext);
    MCP_JBTL_LOGD("nativeJFmRx_disable: FM_RX_Disable() returned %d",(int)status);


    MCP_JBTL_LOGD("nativeJFmRx_disable(): Exit");;
    return status;
}



static int nativeJFmRx_SetBand(JNIEnv *env, jobject obj,jlong jContextValue, jint jFmBand)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;
    FmcBand band = (FmcBand)jFmBand;
    MCP_JBTL_LOGD("nativeJFmRx_setBand(): Entered");

    FmRxStatus  status =FM_RX_SetBand(fmRxContext, (FmcBand)band);
    MCP_JBTL_LOGD("nativeJFmRx_setBand: FM_RX_SetBand() returned %d",(int)status);


    MCP_JBTL_LOGD("nativeJFmRx_setBand(): Exit");
    return status;
}


static int nativeJFmRx_GetBand(JNIEnv *env, jobject obj,jlong jContextValue)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;

    MCP_JBTL_LOGD("nativeJFmRx_getBand(): Entered");

    FmRxStatus  status =FM_RX_GetBand(fmRxContext);
    MCP_JBTL_LOGD("nativeJFmRx_getBand: FM_RX_GetBand() returned %d",(int)status);


    MCP_JBTL_LOGD("nativeJFmRx_getBand(): Exit");
    return status;
}


static int nativeJFmRx_Tune(JNIEnv *env, jobject obj,jlong jContextValue,jint jFmFreq)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;

    MCP_JBTL_LOGD("nativeJFmRx_tune(): Entered");

    FmRxStatus  status =FM_RX_Tune(fmRxContext, (FmcFreq)jFmFreq);
    MCP_JBTL_LOGD("nativeJFmRx_tune: FM_RX_Tune() returned %d",(int)status);


    MCP_JBTL_LOGD("nativeJFmRx_Tune(): Exit");
    return status;

}


static int nativeJFmRx_GetTunedFrequency(JNIEnv *env, jobject obj,jlong jContextValue)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;

    MCP_JBTL_LOGD("nativeJFmRx_getTunedFrequency(): Entered");

    FmRxStatus  status =FM_RX_GetTunedFrequency(fmRxContext);
    MCP_JBTL_LOGD("nativeJFmRx_getTunedFrequency: FM_RX_GetTunedFrequency() returned %d",(int)status);


    MCP_JBTL_LOGD("nativeJFmRx_getTunedFrequency(): Exit");
    return status;
}



static int nativeJFmRx_SetMonoStereoMode(JNIEnv *env, jobject obj,jlong jContextValue,jint jFmMode)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;

    MCP_JBTL_LOGD("nativeJFmRx_SetMonoStereoMode(): Entered");

    FmRxStatus  status =FM_RX_SetMonoStereoMode(fmRxContext,(FmRxMonoStereoMode)jFmMode);
    MCP_JBTL_LOGD("nativeJFmRx_SetMonoStereoMode: FM_RX_SetMonoStereoMode() returned %d",(int)status);


    MCP_JBTL_LOGD("nativeJFmRx_SetMonoStereoMode(): Exit");
    return status	;
}



static int nativeJFmRx_GetMonoStereoMode(JNIEnv *env, jobject obj,jlong jContextValue)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;

    MCP_JBTL_LOGD("nativeJFmRx_GetMonoStereoMode(): Entered");

    FmRxStatus  status =FM_RX_GetMonoStereoMode(fmRxContext);
    MCP_JBTL_LOGD("nativeJFmRx_GetMonoStereoMode: FM_RX_GetMonoStereoMode() returned %d",(int)status);


    MCP_JBTL_LOGD("nativeJFmRx_GetMonoStereoMode(): Exit");
    return status	;
}



static int nativeJFmRx_SetMuteMode(JNIEnv *env, jobject obj,jlong jContextValue,jint jFmMuteMode)
{

    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;

    MCP_JBTL_LOGD("nativeJFmRx_setMuteMode(): Entered");

    FmRxStatus  status =FM_RX_SetMuteMode(fmRxContext,(FmcMuteMode)jFmMuteMode);
    MCP_JBTL_LOGD("nativeJFmRx_setMuteMode: FM_RX_SetMuteMode() returned %d",(int)status);


    MCP_JBTL_LOGD("nativeJFmRx_setMuteMode(): Exit");
    return status;


}


static int nativeJFmRx_GetMuteMode(JNIEnv *env, jobject obj,jlong jContextValue)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;

    MCP_JBTL_LOGD("nativeJFmRx_getMuteMode(): Entered");

    FmRxStatus  status =FM_RX_GetMuteMode(fmRxContext);
    MCP_JBTL_LOGD("nativeJFmRx_getMuteMode: FM_RX_GetMuteMode() returned %d",(int)status);


    MCP_JBTL_LOGD("nativeJFmRx_getMuteMode(): Exit");
    return status;
}


static int nativeJFmRx_SetRssiThreshold(JNIEnv *env, jobject obj,jlong jContextValue,jint jFmRssi)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;

    MCP_JBTL_LOGD("nativeJFmRx_setRssiThreshold(): Entered");

    FmRxStatus  status = FM_RX_SetRssiThreshold(fmRxContext, (FMC_INT)jFmRssi);
    MCP_JBTL_LOGD("nativeJFmRx_setRssiThreshold: FM_RX_SetRssiThreshold() returned %d",(int)status);


    MCP_JBTL_LOGD("nativeJFmRx_setRssiThreshold(): Exit");
    return status;
}

static int nativeJFmRx_GetRssiThreshold(JNIEnv *env, jobject obj,jlong jContextValue)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;

    MCP_JBTL_LOGD("nativeJFmRx_getRssiThreshold(): Entered");

    FmRxStatus status =FM_RX_GetRssiThreshold(fmRxContext);
    MCP_JBTL_LOGD("nativeJFmRx_getRssiThreshold: FM_RX_GetRssiThreshold() returned %d",(int)status);


    MCP_JBTL_LOGD("nativeJFmRx_getRssiThreshold(): Exit");
    return status;
}

static int nativeJFmRx_GetRssi(JNIEnv *env, jobject obj,jlong jContextValue)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;

    MCP_JBTL_LOGD("nativeJFmRx_getRssi(): Entered");

    FmRxStatus  status =FM_RX_GetRssi(fmRxContext);
    MCP_JBTL_LOGD("nativeJFmRx_getRssi: FM_RX_GetRssi() returned %d",(int)status);


    MCP_JBTL_LOGD("nativeJFmRx_getRssi(): Exit");
    return status;
}

static int nativeJFmRx_SetVolume(JNIEnv *env, jobject obj,jlong jContextValue,jint jFmVolume)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;
    MCP_JBTL_LOGD("nativeJFmRx_SetVolume(): Entered");
    FmRxStatus  status = FM_RX_SetVolume(fmRxContext,(FMC_UINT)jFmVolume);
    MCP_JBTL_LOGD("nativeJFmRx_SetVolume: FM_RX_SetVolume() returned %d",(int)status);


    MCP_JBTL_LOGD("nativeJFmRx_SetVolume(): Exit");
    return status;

}

static int nativeJFmRx_GetVolume(JNIEnv *env, jobject obj,jlong jContextValue)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;

    MCP_JBTL_LOGD("nativeJFmRx_getVolume(): Entered");

    FmRxStatus  status =FM_RX_GetVolume(fmRxContext);
    MCP_JBTL_LOGD("nativeJFmRx_getVolume: FM_RX_GetVolume() returned %d",(int)status);


    MCP_JBTL_LOGD("nativeJFmRx_getVolume(): Exit");
    return status;
}

static int nativeJFmRx_SetChannelSpacing(JNIEnv *env, jobject obj,jlong jContextValue,jint jFmChannelSpacing)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;
    LOGD("nativeJFmRx_SetChannelSpacing(): Entered");
    FmRxStatus  status = FM_RX_SetChannelSpacing(fmRxContext,(FMC_UINT)jFmChannelSpacing);
    LOGD("nativeJFmRx_SetChannelSpacing: FM_RX_SetChannelSpacing() returned %d",(int)status);


    LOGD("nativeJFmRx_SetChannelSpacing(): Exit");
    return status;

}

static int nativeJFmRx_GetChannelSpacing(JNIEnv *env, jobject obj,jlong jContextValue)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;

    LOGD("nativeJFmRx_GetChannelSpacing(): Entered");

    FmRxStatus  status =FM_RX_GetChannelSpacing(fmRxContext);
    LOGD("nativeJFmRx_GetChannelSpacing: FM_RX_GetChannelSpacing() returned %d",(int)status);


    LOGD("nativeJFmRx_GetChannelSpacing(): Exit");
    return status;
}

static jint nativeJFmRx_SetDeEmphasisFilter(JNIEnv *env, jobject obj,jlong jContextValue,jint jFmEmphasisFilter)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;
    MCP_JBTL_LOGD("nativeJFmRx_SetDeEmphasisFilter(): Entered");
    FmRxStatus  status =FM_RX_SetDeEmphasisFilter(fmRxContext,(FmcEmphasisFilter) jFmEmphasisFilter);
    MCP_JBTL_LOGD("nativeJFmRx_SetDeEmphasisFilter: FM_RX_SetDeEmphasisFilter() returned %d",(int)status);


    MCP_JBTL_LOGD("nativeJFmRx_SetDeEmphasisFilter(): Exit");
    return status;
}


static int nativeJFmRx_GetDeEmphasisFilter(JNIEnv *env, jobject obj,jlong jContextValue)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;

    MCP_JBTL_LOGD("nativeJFmRx_GetDeEmphasisFilter(): Entered");

    FmRxStatus  status =FM_RX_GetDeEmphasisFilter(fmRxContext);
    MCP_JBTL_LOGD("nativeJFmRx_GetDeEmphasisFilter: FM_RX_GetDeEmphasisFilter() returned %d",(int)status);


    MCP_JBTL_LOGD("nativeJFmRx_GetDeEmphasisFilter(): Exit");
    return status;
}



static int nativeJFmRx_Seek(JNIEnv *env, jobject obj,jlong jContextValue,jint jdirection)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;
    FmRxSeekDirection direction = (FmRxSeekDirection)jdirection;

    MCP_JBTL_LOGD("nativeJFmRx_Seek(): Entered");

    FmRxStatus  status =FM_RX_Seek(fmRxContext, (FmRxSeekDirection)direction);
    MCP_JBTL_LOGD("nativeJFmRx_Seek: FM_RX_Seek() returned %d",(int)status);


    MCP_JBTL_LOGD("nativeJFmRx_Seek(): Exit");
    return status;

}


static int nativeJFmRx_StopSeek(JNIEnv *env, jobject obj,jlong jContextValue)
{
    FmRxContext * fmRxContext =(FmRxContext *)jContextValue;

    MCP_JBTL_LOGD("nativeJFmRx_StopSeek(): Entered");

    FmRxStatus  status =FM_RX_StopSeek(fmRxContext);
    MCP_JBTL_LOGD("nativeJFmRx_StopSeek: FM_RX_StopSeek() returned %d",(int)status);


    MCP_JBTL_LOGD("nativeJFmRx_StopSeek(): Exit");
    return status;
}

static int nativeJFmRx_EnableRDS(JNIEnv *env, jobject obj,jlong jContextValue)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;

    MCP_JBTL_LOGD("nativeJFmRx_enableRDS(): Entered");

    FmRxStatus  status =FM_RX_EnableRds(fmRxContext);
    MCP_JBTL_LOGD("nativeJFmRx_enableRDS: FM_RX_EnableRds() returned %d",(int)status);


    MCP_JBTL_LOGD("nativeJFmRx_enableRDS(): Exit");
    return status;
}

static int nativeJFmRx_DisableRDS(JNIEnv *env, jobject obj,jlong jContextValue)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;

    MCP_JBTL_LOGD("nativeJFmRx_DisableRDS(): Entered");

    FmRxStatus  status =FM_RX_DisableRds(fmRxContext);
    MCP_JBTL_LOGD("nativeJFmRx_DisableRDS: FM_RX_DisableRds() returned %d",(int)status);


    MCP_JBTL_LOGD("nativeJFmRx_DisableRDS(): Exit");
    return status;
}

static int nativeJFmRx_EnableAudioRouting(JNIEnv *env, jobject obj,jlong jContextValue)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;

    MCP_JBTL_LOGD("nativeJFmRx_enableAudioRouting(): Entered");

    FmRxStatus  status =FM_RX_EnableAudioRouting(fmRxContext);
    MCP_JBTL_LOGD("nativeJFmRx_enableAudioRouting: FM_RX_EnableAudioRouting() returned %d",(int)status);


    MCP_JBTL_LOGD("nativeJFmRx_enableAudioRouting(): Exit");
    return status;
}

static int  nativeJFmRx_DisableAudioRouting(JNIEnv *env, jobject obj,jlong jContextValue)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;

    MCP_JBTL_LOGD("nativeJFmRx_disableAudioRouting(): Entered");

    FmRxStatus  status =FM_RX_DisableAudioRouting(fmRxContext);
    MCP_JBTL_LOGD("nativeJFmRx_disableAudioRouting: FM_RX_DisableAudioRouting() returned %d",(int)status);


    MCP_JBTL_LOGD("nativeJFmRx_disableAudioRouting(): Exit");
    return status;
}

static int nativeJFmRx_SetRdsAfSwitchMode(JNIEnv *env, jobject obj,jlong jContextValue,jint jRdsAfSwitchMode)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;
    MCP_JBTL_LOGD("nativeJFmRx_setRdsAfSwitchMode(): Entered");
    FmRxStatus  status =FM_RX_SetRdsAfSwitchMode(fmRxContext, (FmRxRdsAfSwitchMode)jRdsAfSwitchMode);
    MCP_JBTL_LOGD("nativeJFmRx_setRdsAfSwitchMode: FM_RX_SetRdsAfSwitchMode() returned %d",(int)status);


    MCP_JBTL_LOGD("nativeJFmRx_setRdsAfSwitchMode(): Exit");
    return status;

}

static int nativeJFmRx_GetRdsAfSwitchMode(JNIEnv *env, jobject obj,jlong jContextValue)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;

    MCP_JBTL_LOGD("nativeJFmRx_getRdsAfSwitchMode(): Entered");

    FmRxStatus  status =FM_RX_GetRdsAfSwitchMode(fmRxContext);
    MCP_JBTL_LOGD("nativeJFmRx_getRdsAfSwitchMode: FM_RX_GetRdsAfSwitchMode() returned %d",(int)status);


    MCP_JBTL_LOGD("nativeJFmRx_getRdsAfSwitchMode(): Exit");
    return status;
}

static int   nativeJFmRx_ChangeAudioTarget (JNIEnv *env, jobject obj,jlong jContextValue, jint jFmRxAudioTargetMask, jint digitalConfig)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;
    MCP_JBTL_LOGD("nativeJFmRx_ChangeAudioTarget(): Entered");

    FmRxStatus  status =FM_RX_ChangeAudioTarget(fmRxContext,(FmRxAudioTargetMask)jFmRxAudioTargetMask,(ECAL_SampleFrequency)digitalConfig);
    MCP_JBTL_LOGD("nativeJFmRx_ChangeAudioTarget: FM_RX_ChangeAudioTarget() returned %d",(int)status);


    MCP_JBTL_LOGD("nativeJFmRx_ChangeAudioTarget(): Exit");
    return status;

}


static int    nativeJFmRx_ChangeDigitalTargetConfiguration(JNIEnv *env, jobject obj,jlong jContextValue,jint digitalConfig)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;
    MCP_JBTL_LOGD("nativeJFmRx_ChangeDigitalTargetConfiguration(): Entered");

    FmRxStatus  status =FM_RX_ChangeDigitalTargetConfiguration(fmRxContext,(ECAL_SampleFrequency)digitalConfig);
    MCP_JBTL_LOGD("nativeJFmRx_ChangeDigitalTargetConfiguration: FM_RX_ChangeDigitalTargetConfiguration() returned %d",(int)status);


    MCP_JBTL_LOGD("nativeJFmRx_ChangeDigitalTargetConfiguration(): Exit");
    return status;

}


static int   nativeJFmRx_SetRfDependentMuteMode(JNIEnv *env, jobject obj,jlong jContextValue, jint mode)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;
    MCP_JBTL_LOGD("nativeJFmRx_SetRfDependentMuteMode(): Entered");

    FmRxStatus  status =FM_RX_SetRfDependentMuteMode(fmRxContext,(FmRxRfDependentMuteMode)mode);
    MCP_JBTL_LOGD("nativeJFmRx_SetRfDependentMuteMode: FM_RX_SetRfDependentMuteMode() returned %d",(int)status);


    MCP_JBTL_LOGD("nativeJFmRx_SetRfDependentMuteMode(): Exit");
    return status;


}


static int    nativeJFmRx_GetRfDependentMute(JNIEnv *env, jobject obj,jlong jContextValue)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;
    MCP_JBTL_LOGD(" nativeJFmRx_GetRfDependentMute(): Entered");

    FmRxStatus  status =FM_RX_GetRfDependentMute(fmRxContext);
    MCP_JBTL_LOGD("Nati nativeJFmRx_GetRfDependentMute: FM_RX_GetRfDependentMute() returned %d",(int)status);


    MCP_JBTL_LOGD(" nativeJFmRx_GetRfDependentMute(): Exit");
    return status;

}


static int    nativeJFmRx_SetRdsSystem(JNIEnv *env, jobject obj,jlong jContextValue, jint rdsSystem)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;
    MCP_JBTL_LOGD(" nativeJFmRx_SetRdsSystem(): Entered");

    FmRxStatus  status =FM_RX_SetRdsSystem(fmRxContext,(FmcRdsSystem)rdsSystem);
    MCP_JBTL_LOGD(" nativeJFmRx_SetRdsSystem: FM_RX_SetRdsSystem() returned %d",(int)status);


    MCP_JBTL_LOGD(" nativeJFmRx_SetRdsSystem(): Exit");
    return status;

}


static  int   nativeJFmRx_GetRdsSystem(JNIEnv *env, jobject obj,jlong jContextValue)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;
    MCP_JBTL_LOGD("nativeJFmRx_GetRdsSystem(): Entered");

    FmRxStatus  status =FM_RX_GetRdsSystem(fmRxContext);
    MCP_JBTL_LOGD(" nativeJFmRx_GetRdsSystem: FM_RX_GetRdsSystem() returned %d",(int)status);


    MCP_JBTL_LOGD(" nativeJFmRx_GetRdsSystem(): Exit");
    return status;

}


static int   nativeJFmRx_SetRdsGroupMask(JNIEnv *env, jobject obj,jlong jContextValue, jlong groupMask)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;
    MCP_JBTL_LOGD("nativeJFmRx_SetRdsGroupMask(): Entered");

    FmRxStatus  status =FM_RX_SetRdsGroupMask(fmRxContext,(FmcRdsGroupTypeMask)groupMask);
    MCP_JBTL_LOGD(" nativeJFmRx_SetRdsGroupMask: FM_RX_SetRdsGroupMask() returned %d",(int)status);


    MCP_JBTL_LOGD(" nativeJFmRx_SetRdsGroupMask(): Exit");
    return status;

}

static int   nativeJFmRx_GetRdsGroupMask(JNIEnv *env, jobject obj,jlong jContextValue)
{
    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;
    MCP_JBTL_LOGD("nativeJFmRx_GetRdsGroupMask(): Entered");

    FmRxStatus  status =  FM_RX_GetRdsGroupMask(fmRxContext);

    MCP_JBTL_LOGD(" nativeJFmRx_GetRdsGroupMask: FM_RX_GetRdsGroupMask() returned %d",(int)status);


    MCP_JBTL_LOGD(" nativeJFmRx_GetRdsGroupMask(): Exit");
    return status;

}

static int nativeJFmRx_CompleteScan(JNIEnv *env, jobject obj, jlong jContextValue)
{

    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;
    LOGD("nativeJFmRx_CompleteScan(): Entered");

    FmRxStatus  status = FM_RX_CompleteScan(fmRxContext);

    LOGD("nativeJFmRx_CompleteScan: FM_RX_CompleteScan() returned %d",(int)status);

    LOGD("nativeJFmRx_CompleteScan(): Exit");
    return status;
}

static int nativeJFmRx_GetCompleteScanProgress(JNIEnv *env, jobject obj, jlong jContextValue)
{

    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;
    LOGD("nativeJFmRx_GetCompleteScanProgress(): Entered");

    FmRxStatus  status = FM_RX_GetCompleteScanProgress(fmRxContext);

    LOGD("nativeJFmRx_GetCompleteScanProgress: FM_RX_GetCompleteScanProgress() returned %d",(int)status);

    LOGD("nativeJFmRx_GetCompleteScanProgress(): Exit");
    return status;
}

static int nativeJFmRx_StopCompleteScan(JNIEnv *env, jobject obj, jlong jContextValue)
{

    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;
    LOGD("nativeJFmRx_StopCompleteScan(): Entered");

    FmRxStatus  status = FM_RX_StopCompleteScan(fmRxContext);

    LOGD("nativeJFmRx_StopCompleteScan: FM_RX_StopCompleteScan() returned %d",(int)status);

    LOGD("nativeJFmRx_StopCompleteScan(): Exit");
    return status;
}

static int nativeJFmRx_IsValidChannel(JNIEnv *env, jobject obj, jlong jContextValue)
{

    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;
    LOGD("nativeJFmRx_IsValidChannel(): Entered");

    FmRxStatus  status = FM_RX_IsValidChannel(fmRxContext);

    LOGD("nativeJFmRx_IsValidChannel: FM_RX_IsValidChannel() returned %d",(int)status);

    LOGD("nativeJFmRx_IsValidChannel(): Exit");
    return status;
}


static int nativeJFmRx_GetFwVersion(JNIEnv *env, jobject obj, jlong jContextValue)
{

    FmRxContext * fmRxContext = (FmRxContext *)jContextValue;
    LOGD("nativeJFmRx_GetFwVersion(): Entered");

    FmRxStatus  status = FM_RX_GetFwVersion(fmRxContext);

    LOGD("nativeJFmRx_GetFwVersion: FM_RX_GetFwVersion() returned %d",(int)status);

    LOGD("nativeJFmRx_GetFwVersion(): Exit");
    return status;
}


//################################################################################

//								 SIGNALS

//###############################################################################

extern "C"
{

    void nativeJFmRx_Callback(const fm_rx_event *event)
    {

        MCP_JBTL_LOGI("nativeJFmRx_Callback: Entered, ");
        MCP_JBTL_LOGD( "got event %d", event->eventType);

        JNIEnv* env = NULL;
        jintArray jAfListData = NULL;
        jbyteArray jNameString= NULL;
        jbyteArray jRadioTxtMsg = NULL;
        jbyteArray jGroupData = NULL;
        jintArray jChannelsData = NULL ;
        jsize len = NULL ;
        int k = 0;

        g_jVM->AttachCurrentThread((&env), NULL);

        if (env == NULL)
        {
            MCP_JBTL_LOGE("nativeJFmRx_Callback: Entered, env is null");
        }


        switch (event->eventType) {
        case FM_RX_EVENT_CMD_DONE:

            switch (event->p.cmdDone.cmd){

            case FM_RX_CMD_ENABLE:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdEnable,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_DISABLE:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdDisable,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_SET_BAND:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdSetBand,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_GET_BAND:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdGetBand,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_SET_MONO_STEREO_MODE:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdSetMonoStereoMode,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_GET_MONO_STEREO_MODE:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdGetMonoStereoMode,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_SET_MUTE_MODE:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdSetMuteMode,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_GET_MUTE_MODE:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdGetMuteMode,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_SET_RF_DEPENDENT_MUTE_MODE:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdSetRfDependentMuteMode,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_GET_RF_DEPENDENT_MUTE_MODE:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdGetRfDependentMuteMode,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_SET_RSSI_THRESHOLD:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdSetRssiThreshhold,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_GET_RSSI_THRESHOLD:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdGetRssiThreshhold,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_SET_DEEMPHASIS_FILTER:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdSetDeemphasisFilter,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_GET_DEEMPHASIS_FILTER:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdGetDeemphasisFilter,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_SET_VOLUME:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdSetVolume,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_GET_VOLUME:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdGetVolume,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_TUNE:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdTune,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_GET_TUNED_FREQUENCY:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdGetTunedFrequency,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_SEEK:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdSeek,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_STOP_SEEK:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdStopSeek,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_GET_RSSI:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdGetRssi,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_ENABLE_RDS:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdEnableRds,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_DISABLE_RDS:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdDisableRds,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_SET_RDS_SYSTEM:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdSetRdsSystem,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_GET_RDS_SYSTEM:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdGetRdsSystem,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_SET_RDS_GROUP_MASK:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdSetRdsGroupMask,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_GET_RDS_GROUP_MASK:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdGetRdsGroupMask,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_SET_RDS_AF_SWITCH_MODE:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdSetRdsAfSwitchMode,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_GET_RDS_AF_SWITCH_MODE:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdGetRdsAfSwitchMode,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_ENABLE_AUDIO:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdEnableAudio,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_DISABLE_AUDIO:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdDisableAudio, (jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_CHANGE_AUDIO_TARGET:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdChangeAudioTarget,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_CHANGE_DIGITAL_AUDIO_CONFIGURATION:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdChangeDigitalAudioConfiguration,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_SET_CHANNEL_SPACING:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdSetChannelSpacing,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_GET_CHANNEL_SPACING:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdGetChannelSpacing,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_GET_FW_VERSION:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdGetFwVersion,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_IS_CHANNEL_VALID:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdIsValidChannel,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_COMPLETE_SCAN_PROGRESS:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdGetCompleteScanProgress,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            case FM_RX_CMD_STOP_COMPLETE_SCAN:
                env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCmdStopCompleteScan,(jlong)event->context,
                                          (jint)event->status,
                                          (jint)event->p.cmdDone.cmd,
                                          (jlong)event->p.cmdDone.value);
                break;

            default:
                MCP_JBTL_LOGE("nativeJFmRx_Callback:FM_RX_EVENT_CMD_DONE,unhendeld event(%d)",event->p.cmdDone.cmd);
                break;
            }

            break;

        case FM_RX_EVENT_MONO_STEREO_MODE_CHANGED:

            MCP_JBTL_LOGD("nativeJFmRx_Callback():EVENT --------------->FM_RX_EVENT_MONO_STEREO_MODE_CHANGED");
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxMonoStereoModeChanged,(jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.monoStereoMode.mode);


            break;

        case FM_RX_EVENT_PI_CODE_CHANGED:

            MCP_JBTL_LOGD("nativeJFmRx_Callback():EVENT --------------->FM_RX_EVENT_PI_CODE_CHANGED");
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxPiCodeChanged,(jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.piChangedData.pi);

            break;

        case FM_RX_EVENT_AF_SWITCH_START:

            MCP_JBTL_LOGD("nativeJFmRx_Callback():EVENT --------------->FM_RX_EVENT_AF_SWITCH_START");
            MCP_JBTL_LOGD("AF switch process has started...");
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxAfSwitchStart,(jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.afSwitchData.pi,
                                      (jint)event->p.afSwitchData.tunedFreq,
                                      (jint)event->p.afSwitchData.afFreq);

            break;

        case FM_RX_EVENT_AF_SWITCH_TO_FREQ_FAILED:

            MCP_JBTL_LOGD("nativeJFmRx_Callback():EVENT --------------->FM_RX_EVENT_AF_SWITCH_TO_FREQ_FAILED");
            MCP_JBTL_LOGD("AF switch to %d failed", event->p.afSwitchData.afFreq);
            env->CallStaticVoidMethod(_sJClass, _sMethodId_nativeCb_fmRxAfSwitchFreqFailed,(jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.afSwitchData.pi,
                                      (jint)event->p.afSwitchData.tunedFreq,
                                      (jint)event->p.afSwitchData.afFreq);
            break;

        case FM_RX_EVENT_AF_SWITCH_COMPLETE:
            MCP_JBTL_LOGD("nativeJFmRx_Callback():EVENT --------------->FM_RX_EVENT_AF_SWITCH_COMPLETE");
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxAfSwitchComplete,(jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.afSwitchData.pi,
                                      (jint)event->p.afSwitchData.tunedFreq,
                                      (jint)event->p.afSwitchData.afFreq);
            break;

        case FM_RX_EVENT_AF_LIST_CHANGED:

            MCP_JBTL_LOGD("nativeJFmRx_Callback():EVENT --------------->FM_RX_EVENT_AF_LIST_CHANGED");
            MCP_JBTL_LOGD("p.afListData.afListSize is: %d",event->p.afListData.afListSize);
            for (k=0;k<event->p.afListData.afListSize ;k++)
                MCP_JBTL_LOGD("event->p.afListData.afList %d",event->p.afListData.afList[k]);

            jAfListData = env->NewIntArray(event->p.afListData.afListSize);

            if (jAfListData == NULL)
            {
                MCP_JBTL_LOGE("nativeJFmRx_Callback: Failed converting elements");
                goto EXCEPTION;
            }

           env->SetIntArrayRegion(jAfListData,0,event->p.afListData.afListSize,(jint*)event->p.afListData.afList);

            if (env->ExceptionOccurred())    {
                MCP_JBTL_LOGE("nativeJFmRx_Callback: Calling Java nativeCb_fmRxAfListChanged failed");
                goto EXCEPTION;
            }

            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxAfListChanged,(jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.afListData.pi,
				       jAfListData,
                                      (jint)event->p.afListData.afListSize
                                      );

            break;

        case FM_RX_EVENT_PS_CHANGED:
            MCP_JBTL_LOGD("nativeJFmRx_Callback():EVENT --------------->FM_RX_EVENT_PS_CHANGED '%s'", (char*)(event->p.psData.name));
            len =  strlen((char*)(event->p.psData.name));
            MCP_JBTL_LOGD("nativeJFmRx_Callback():EVENT --------------->FM_RX_EVENT_PS_CHANGED len %d",len);


            jNameString = env->NewByteArray(len);

            if (jNameString == NULL)
            {
                MCP_JBTL_LOGE("nativeJFmRx_Callback: Failed converting elements");
                goto EXCEPTION;
            }

            env->SetByteArrayRegion(jNameString,0,len,(jbyte*)event->p.psData.name);

            if (env->ExceptionOccurred())    {
                MCP_JBTL_LOGE("nativeJFmRx_Callback: Calling Java nativeCb_fmRxRadioText failed");
                goto EXCEPTION;
            }

            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxPsChanged,(jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.psData.frequency,
                                      jNameString,
                                      (jint)event->p.psData.repertoire);

            break;

        case FM_RX_EVENT_RADIO_TEXT:
            MCP_JBTL_LOGD("nativeJFmRx_Callback():EVENT --------------->FM_RX_EVENT_RADIO_TEXT");

            jRadioTxtMsg = env->NewByteArray(event->p.radioTextData.len);

            if (jRadioTxtMsg == NULL)
            {
                MCP_JBTL_LOGE("nativeJFmRx_Callback: Failed converting elements");
                goto EXCEPTION;
            }

            env->SetByteArrayRegion(jRadioTxtMsg,0,event->p.radioTextData.len,(jbyte*)event->p.radioTextData.msg);

            if (env->ExceptionOccurred())    {
                MCP_JBTL_LOGE("nativeJFmRx_Callback: Calling Java nativeCb_fmRxRadioText failed");
                goto EXCEPTION;
            }

            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxRadioText,(jlong)event->context,
                                      (jint)event->status,
                                      (jboolean)event->p.radioTextData.resetDisplay,
                                      jRadioTxtMsg,
                                      (jint)event->p.radioTextData.len,
                                      (jint)event->p.radioTextData.startIndex,
                                      (jint)event->p.radioTextData.repertoire);

            break;

        case FM_RX_EVENT_RAW_RDS:
            MCP_JBTL_LOGD("nativeJFmRx_Callback():EVENT --------------->FM_RX_EVENT_RAW_RDS");

            len =  sizeof(event->p.rawRdsGroupData.groupData);

            jGroupData = env->NewByteArray(len);

            if (jGroupData == NULL)
            {
                MCP_JBTL_LOGE("nativeJFmRx_Callback: Failed converting elements");
                goto EXCEPTION;
            }

            env->SetByteArrayRegion(jGroupData,0,len,(jbyte*)event->p.rawRdsGroupData.groupData);

            if (env->ExceptionOccurred())    {
                MCP_JBTL_LOGE("nativeJFmRx_Callback: Calling Java jniCb_fmRxRadioText failed");
                goto EXCEPTION;
            }

            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxRawRDS,(jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.rawRdsGroupData.groupBitInMask,
                                      jGroupData);
            break;

        case FM_RX_EVENT_AUDIO_PATH_CHANGED:
            MCP_JBTL_LOGD("Audio Path Changed Event received");
            break;

        case FM_RX_EVENT_PTY_CODE_CHANGED:
            MCP_JBTL_LOGD("nativeJFmRx_Callback():EVENT --------------->FM_RX_EVENT_PTY_CODE_CHANGED");
            MCP_JBTL_LOGD("RDS PTY Code has changed to %d",event->p.ptyChangedData.pty);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxPtyCodeChanged,(jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.ptyChangedData.pty);
            break;

        case FM_RX_EVENT_COMPLETE_SCAN_DONE:

            len =  sizeof(event->p.completeScanData.channelsData)/ sizeof(int);
            MCP_JBTL_LOGD("nativeJFmRx_Callback():EVENT --------------->FM_RX_EVENT_COMPLETE_SCAN_DONE");
            MCP_JBTL_LOGD("len %d",len);
            jChannelsData = env->NewIntArray(len);

            if (jChannelsData == NULL)
            {
                MCP_JBTL_LOGE("nativeJFmRx_Callback: Failed converting elements");
                goto EXCEPTION;
            }

            env->SetIntArrayRegion(jChannelsData,0,len,(jint*)event->p.completeScanData.channelsData);

            if (env->ExceptionOccurred())    {
                MCP_JBTL_LOGE("nativeJFmRx_Callback: Calling Java jniCb_fmRxRadioText failed");
                goto EXCEPTION;
            }

            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmRxCompleteScanDone,(jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.completeScanData.numOfChannels,
                                      jChannelsData);
            break;

        default:
            MCP_JBTL_LOGE("nativeJFmRx_Callback():EVENT --------------->default");
            MCP_JBTL_LOGE("unhandled fm event %d", event->eventType);
            break;
        } //end switch

        if (env->ExceptionOccurred())    {
            MCP_JBTL_LOGE("nativeJFmRx_Callback:  ExceptionOccurred");
            goto EXCEPTION;
        }

//Delete the local references
        if (jAfListData!= NULL)
            env->DeleteLocalRef(jAfListData);
        if (jRadioTxtMsg!= NULL)
            env->DeleteLocalRef(jRadioTxtMsg);
        if (jNameString!= NULL)
            env->DeleteLocalRef(jNameString);
        if (jGroupData!= NULL)
            env->DeleteLocalRef(jGroupData);

        if (jChannelsData!= NULL)
            env->DeleteLocalRef(jChannelsData);

        MCP_JBTL_LOGD("nativeJFmRx_Callback: Exiting, Calling DetachCurrentThread at the END");

        g_jVM->DetachCurrentThread();

        return;

EXCEPTION:

        /*Delete Jni Local refrencece */
        MCP_JBTL_LOGE("nativeJFmRx_Callback: Exiting due to failure");
        if (jAfListData!= NULL)
            env->DeleteLocalRef(jAfListData);
        if (jRadioTxtMsg!= NULL)
            env->DeleteLocalRef(jRadioTxtMsg);
        if (jNameString!= NULL)
            env->DeleteLocalRef(jNameString);
        if (jGroupData!= NULL)
            env->DeleteLocalRef(jGroupData);
        if (jChannelsData!= NULL)
            env->DeleteLocalRef(jChannelsData);
        if (env->ExceptionOccurred())    {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }

        g_jVM->DetachCurrentThread();

        return;

    }


} //extern c


/**********************************************************************
*				Callback registration

***********************************************************************/
#define VERIFY_METHOD_ID(methodId) \
		if (!_VerifyMethodId(methodId, #methodId)) { \
			MCP_JBTL_LOGE("Error obtaining method id for %s", #methodId);	\
			return; 	\
		}

static bool _VerifyMethodId(jmethodID methodId, const char *name)
{
    bool result = true;

    if (methodId == NULL)
    {
        MCP_JBTL_LOGE("_VerifyMethodId: Failed getting method id of %s", name);
        result = false;
    }

    return result;
}



void nativeJFmRx_ClassInitNative(JNIEnv* env, jclass clazz){
    MCP_JBTL_LOGD("nativeJFmRx_ClassInitNative: Entered");

    if (NULL == env)
    {
        MCP_JBTL_LOGE("nativeJFmRx_ClassInitNative: NULL == env");
    }

    env->GetJavaVM(&g_jVM);

    /* Save class information in global reference in order to prevent class unloading */
    _sJClass = (jclass)env->NewGlobalRef(clazz);


    MCP_JBTL_LOGI("nativeJFmRx_ClassInitNative: Obtaining method IDs");

    _sMethodId_nativeCb_fmRxRawRDS = env->GetStaticMethodID(clazz,
                                     "nativeCb_fmRxRawRDS",
                                     "(JII[B)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxRawRDS);


    _sMethodId_nativeCb_fmRxRadioText  = env->GetStaticMethodID(clazz,
                                         "nativeCb_fmRxRadioText",
                                         "(JIZ[BIII)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxRadioText);


    _sMethodId_nativeCb_fmRxPiCodeChanged  = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxPiCodeChanged",
            "(JII)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxPiCodeChanged);


    _sMethodId_nativeCb_fmRxPtyCodeChanged  = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxPtyCodeChanged",
            "(JII)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxPtyCodeChanged);


    _sMethodId_nativeCb_fmRxPsChanged  = env->GetStaticMethodID(clazz,
                                         "nativeCb_fmRxPsChanged",
                                         "(JII[BI)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxPsChanged);


    _sMethodId_nativeCb_fmRxMonoStereoModeChanged  = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxMonoStereoModeChanged",
            "(JII)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxMonoStereoModeChanged);


    _sMethodId_nativeCb_fmRxAudioPathChanged  = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxAudioPathChanged",
            "(JI)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxAudioPathChanged);


    _sMethodId_nativeCb_fmRxAfSwitchFreqFailed  = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxAfSwitchFreqFailed",
            "(JIIII)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxAfSwitchFreqFailed);


    _sMethodId_nativeCb_fmRxAfSwitchStart  = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxAfSwitchStart",
            "(JIIII)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxAfSwitchStart);


    _sMethodId_nativeCb_fmRxAfSwitchComplete  = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxAfSwitchComplete",
            "(JIIII)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxAfSwitchComplete);


    _sMethodId_nativeCb_fmRxAfListChanged  = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxAfListChanged",
            "(JII[II)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxAfListChanged);


    _sMethodId_nativeCb_fmRxCmdEnable = env->GetStaticMethodID(clazz,
                                        "nativeCb_fmRxCmdEnable",
                                        "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdEnable);


    _sMethodId_nativeCb_fmRxCmdDisable = env->GetStaticMethodID(clazz,
                                         "nativeCb_fmRxCmdDisable",
                                         "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdDisable);

    _sMethodId_nativeCb_fmRxCmdEnableAudio = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxCmdEnableAudio",
            "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdEnableAudio);



    _sMethodId_nativeCb_fmRxCmdChangeAudioTarget = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxCmdChangeAudioTarget",
            "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdChangeAudioTarget);


    _sMethodId_nativeCb_fmRxCmdSetBand = env->GetStaticMethodID(clazz,
                                         "nativeCb_fmRxCmdSetBand",
                                         "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdSetBand);

    _sMethodId_nativeCb_fmRxCmdGetBand = env->GetStaticMethodID(clazz,
                                         "nativeCb_fmRxCmdGetBand",
                                         "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdGetBand);



    _sMethodId_nativeCb_fmRxCmdSetMonoStereoMode = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxCmdSetMonoStereoMode",
            "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdSetMonoStereoMode);



    _sMethodId_nativeCb_fmRxCmdGetMonoStereoMode = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxCmdGetMonoStereoMode",
            "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdGetMonoStereoMode);



    _sMethodId_nativeCb_fmRxCmdGetMuteMode = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxCmdGetMuteMode",
            "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdGetMuteMode);



    _sMethodId_nativeCb_fmRxCmdSetMuteMode = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxCmdSetMuteMode",
            "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdSetMuteMode);



    _sMethodId_nativeCb_fmRxCmdSetRfDependentMuteMode = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxCmdSetRfDependentMuteMode",
            "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdSetRfDependentMuteMode);



    _sMethodId_nativeCb_fmRxCmdGetRfDependentMuteMode = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxCmdGetRfDependentMuteMode",
            "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdGetRfDependentMuteMode);



    _sMethodId_nativeCb_fmRxCmdSetRssiThreshhold = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxCmdSetRssiThreshhold",
            "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdSetRssiThreshhold);



    _sMethodId_nativeCb_fmRxCmdGetRssiThreshhold = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxCmdGetRssiThreshhold",
            "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdGetRssiThreshhold);



    _sMethodId_nativeCb_fmRxCmdSetDeemphasisFilter = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxCmdSetDeemphasisFilter",
            "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdSetDeemphasisFilter);


    _sMethodId_nativeCb_fmRxCmdGetDeemphasisFilter = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxCmdGetDeemphasisFilter",
            "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdGetDeemphasisFilter);



    _sMethodId_nativeCb_fmRxCmdSetVolume = env->GetStaticMethodID(clazz,
                                           "nativeCb_fmRxCmdSetVolume",
                                           "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdSetVolume);



    _sMethodId_nativeCb_fmRxCmdGetVolume = env->GetStaticMethodID(clazz,
                                           "nativeCb_fmRxCmdGetVolume",
                                           "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdGetVolume);

    _sMethodId_nativeCb_fmRxCmdSetChannelSpacing = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxCmdSetChannelSpacing",
            "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdSetChannelSpacing);



    _sMethodId_nativeCb_fmRxCmdGetChannelSpacing = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxCmdGetChannelSpacing",
            "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdGetChannelSpacing);



    _sMethodId_nativeCb_fmRxCmdTune = env->GetStaticMethodID(clazz,
                                      "nativeCb_fmRxCmdTune",
                                      "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdTune);


    _sMethodId_nativeCb_fmRxCmdGetTunedFrequency = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxCmdGetTunedFrequency",
            "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdGetTunedFrequency);


    _sMethodId_nativeCb_fmRxCmdSeek = env->GetStaticMethodID(clazz,
                                      "nativeCb_fmRxCmdSeek",
                                      "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdSeek);



    _sMethodId_nativeCb_fmRxCmdStopSeek = env->GetStaticMethodID(clazz,
                                          "nativeCb_fmRxCmdStopSeek",
                                          "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdStopSeek);


    _sMethodId_nativeCb_fmRxCmdGetRssi = env->GetStaticMethodID(clazz,
                                         "nativeCb_fmRxCmdGetRssi",
                                         "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdGetRssi);


    _sMethodId_nativeCb_fmRxCmdEnableRds = env->GetStaticMethodID(clazz,
                                           "nativeCb_fmRxCmdEnableRds",
                                           "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdEnableRds);


    _sMethodId_nativeCb_fmRxCmdDisableRds = env->GetStaticMethodID(clazz,
                                            "nativeCb_fmRxCmdDisableRds",
                                            "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdDisableRds);


    _sMethodId_nativeCb_fmRxCmdGetRdsSystem = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxCmdGetRdsSystem",
            "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdGetRdsSystem);



    _sMethodId_nativeCb_fmRxCmdSetRdsSystem = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxCmdSetRdsSystem",
            "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdSetRdsSystem);


    _sMethodId_nativeCb_fmRxCmdSetRdsGroupMask = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxCmdSetRdsGroupMask",
            "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdSetRdsGroupMask);


    _sMethodId_nativeCb_fmRxCmdGetRdsGroupMask = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxCmdGetRdsGroupMask",
            "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdGetRdsGroupMask);


    _sMethodId_nativeCb_fmRxCmdSetRdsAfSwitchMode = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxCmdSetRdsAfSwitchMode",
            "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdSetRdsAfSwitchMode);


    _sMethodId_nativeCb_fmRxCmdGetRdsAfSwitchMode = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxCmdGetRdsAfSwitchMode",
            "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdGetRdsAfSwitchMode);


    _sMethodId_nativeCb_fmRxCmdDisableAudio = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxCmdDisableAudio",
            "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdDisableAudio);

    _sMethodId_nativeCb_fmRxCmdDestroy = env->GetStaticMethodID(clazz,
                                         "nativeCb_fmRxCmdDestroy",
                                         "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdDestroy);


    _sMethodId_nativeCb_fmRxCmdChangeDigitalAudioConfiguration= env->GetStaticMethodID(clazz,
            "nativeCb_fmRxCmdChangeDigitalAudioConfiguration",
            "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdChangeDigitalAudioConfiguration);

    _sMethodId_nativeCb_fmRxCompleteScanDone  = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxCompleteScanDone",
            "(JII[I)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCompleteScanDone);



    _sMethodId_nativeCb_fmRxCmdGetFwVersion = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxCmdGetFwVersion",
            "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdGetFwVersion);

    _sMethodId_nativeCb_fmRxCmdIsValidChannel = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxCmdIsValidChannel",
            "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdIsValidChannel);


    _sMethodId_nativeCb_fmRxCmdGetCompleteScanProgress = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxCmdGetCompleteScanProgress",
            "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdGetCompleteScanProgress);

    _sMethodId_nativeCb_fmRxCmdStopCompleteScan = env->GetStaticMethodID(clazz,
            "nativeCb_fmRxCmdStopCompleteScan",
            "(JIIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmRxCmdStopCompleteScan);


    MCP_JBTL_LOGD("nativeJFmRx_ClassInitNative:Exiting");
}

static JNINativeMethod JFmRxNative_sMethods[] = {
    /* name, signature, funcPtr */
    {"nativeJFmRx_ClassInitNative", "()V", (void*)nativeJFmRx_ClassInitNative},
    {"nativeJFmRx_Create", "(Lcom/ti/jfm/core/JFmContext;)I", (void*)nativeJFmRx_Create},
    {"nativeJFmRx_Destroy", "(J)I", (void*)nativeJFmRx_Destroy},
    {"nativeJFmRx_Enable", "(J)I", (void*)nativeJFmRx_Enable},
    {"nativeJFmRx_Disable", "(J)I", (void*)nativeJFmRx_Disable},
    {"nativeJFmRx_SetBand","(JI)I", (void*)nativeJFmRx_SetBand},
    {"nativeJFmRx_GetBand","(J)I", (void*)nativeJFmRx_GetBand},
    {"nativeJFmRx_Tune","(JI)I", (void*)nativeJFmRx_Tune},
    {"nativeJFmRx_GetTunedFrequency","(J)I", (void*)nativeJFmRx_GetTunedFrequency},
    {"nativeJFmRx_SetMonoStereoMode","(JI)I", (void*)nativeJFmRx_SetMonoStereoMode},
    {"nativeJFmRx_GetMonoStereoMode","(J)I", (void*)nativeJFmRx_GetMonoStereoMode},
    {"nativeJFmRx_SetMuteMode","(JI)I", (void*)nativeJFmRx_SetMuteMode},
    {"nativeJFmRx_GetMuteMode","(J)I", (void*)nativeJFmRx_GetMuteMode},
    {"nativeJFmRx_SetRssiThreshold","(JI)I", (void*)nativeJFmRx_SetRssiThreshold},
    {"nativeJFmRx_GetRssiThreshold","(J)I", (void*)nativeJFmRx_GetRssiThreshold},
    {"nativeJFmRx_GetRssi","(J)I", (void*)nativeJFmRx_GetRssi},
    {"nativeJFmRx_SetVolume","(JI)I", (void*)nativeJFmRx_SetVolume},
    {"nativeJFmRx_GetVolume","(J)I", (void*)nativeJFmRx_GetVolume},
    {"nativeJFmRx_SetChannelSpacing","(JI)I", (void*)nativeJFmRx_SetChannelSpacing},
    {"nativeJFmRx_GetChannelSpacing","(J)I", (void*)nativeJFmRx_GetChannelSpacing},
    {"nativeJFmRx_SetDeEmphasisFilter","(JI)I", (void*)nativeJFmRx_SetDeEmphasisFilter},
    {"nativeJFmRx_GetDeEmphasisFilter","(J)I", (void*)nativeJFmRx_GetDeEmphasisFilter},
    {"nativeJFmRx_Seek","(JI)I", (void*)nativeJFmRx_Seek},
    {"nativeJFmRx_StopSeek","(J)I", (void*)nativeJFmRx_StopSeek},
    {"nativeJFmRx_EnableRDS","(J)I", (void*)nativeJFmRx_EnableRDS},
    {"nativeJFmRx_DisableRDS","(J)I", (void*)nativeJFmRx_DisableRDS},
    {"nativeJFmRx_EnableAudioRouting","(J)I", (void*)nativeJFmRx_EnableAudioRouting},
    {"nativeJFmRx_DisableAudioRouting","(J)I", (void*)nativeJFmRx_DisableAudioRouting},
    {"nativeJFmRx_SetRdsAfSwitchMode","(JI)I", (void*)nativeJFmRx_SetRdsAfSwitchMode},
    {"nativeJFmRx_GetRdsAfSwitchMode","(J)I", (void*)nativeJFmRx_GetRdsAfSwitchMode},
    {"nativeJFmRx_ChangeAudioTarget","(JII)I",(void*)nativeJFmRx_ChangeAudioTarget},
    {"nativeJFmRx_ChangeDigitalTargetConfiguration","(JI)I",(void*)nativeJFmRx_ChangeDigitalTargetConfiguration},
    {"nativeJFmRx_SetRfDependentMuteMode","(JI)I",(void*)nativeJFmRx_SetRfDependentMuteMode},
    {"nativeJFmRx_GetRfDependentMute","(J)I",(void*)nativeJFmRx_GetRfDependentMute},
    {"nativeJFmRx_SetRdsSystem","(JI)I",(void*)nativeJFmRx_SetRdsSystem},
    {"nativeJFmRx_GetRdsSystem","(J)I",(void*)nativeJFmRx_GetRdsSystem},
    {"nativeJFmRx_SetRdsGroupMask","(JJ)I",(void*)nativeJFmRx_SetRdsGroupMask},
    {"nativeJFmRx_GetRdsGroupMask","(J)I",(void*)nativeJFmRx_GetRdsGroupMask},
    {"nativeJFmRx_CompleteScan","(J)I",(void*)nativeJFmRx_CompleteScan},
    {"nativeJFmRx_IsValidChannel","(J)I",(void*)nativeJFmRx_IsValidChannel},
    {"nativeJFmRx_GetFwVersion","(J)I",(void*)nativeJFmRx_GetFwVersion},
    {"nativeJFmRx_GetCompleteScanProgress","(J)I",(void*)nativeJFmRx_GetCompleteScanProgress},
    {"nativeJFmRx_StopCompleteScan","(J)I",(void*)nativeJFmRx_StopCompleteScan},

};

/**********************************/


/*
 * Register several native methods for one class.
 */
static int registerNatives(JNIEnv* env, const char* className,
			   JNINativeMethod* gMethods, int numMethods)
{
	jclass clazz;

	clazz = env->FindClass(className);
	if (clazz == NULL) {
		 MCP_JBTL_LOGE("Can not find class %s\n", className);
		return JNI_FALSE;
	}

	if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
		MCP_JBTL_LOGE("Can not RegisterNatives\n");
		return JNI_FALSE;
	}

	return JNI_TRUE;
}

extern JNINativeMethod JFmTxNative_sMethods[];
extern int getTxNativeSize();
jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv* env = NULL;
	jint result = -1;

	MCP_JBTL_LOGE("OnLoad");

	if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) {
		goto bail;
	}

	if (!registerNatives(env,
			     "com/ti/jfm/core/JFmRx",
	                     JFmRxNative_sMethods,
			     NELEM(JFmRxNative_sMethods))) {
		goto bail;
	}

	if (!registerNatives(env,
			     "com/ti/jfm/core/JFmTx",
	                     JFmTxNative_sMethods,
			     getTxNativeSize())) {
		goto bail;
	}

	env->GetJavaVM(&g_jVM);

	/* success -- return valid version number */
	result = JNI_VERSION_1_4;

bail:
	return result;
}


