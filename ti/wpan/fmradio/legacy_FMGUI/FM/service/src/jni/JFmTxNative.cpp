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
#define LOG_TAG "JFmTxNative"
#include <cutils/properties.h>

using namespace android;

extern "C" {
#include "fmc_common.h"
#include "fm_tx.h"
#include "mcp_hal_log.h"


    typedef FmTxEvent fm_tx_event;
    void nativeJFmTx_Callback(const fm_tx_event *event);
    extern void MCP_HAL_LOG_EnableLogToAndroid(const char *app_name);

} //extern "C"

static jclass _sJClass;
static JavaVM *g_jVM = NULL;

static jmethodID _sMethodId_nativeCb_fmTxCmdEnable;
static jmethodID _sMethodId_nativeCb_fmTxCmdDisable;
static jmethodID _sMethodId_nativeCb_fmTxCmdDestroy;
static jmethodID _sMethodId_nativeCb_fmTxCmdTune;
static jmethodID _sMethodId_nativeCb_fmTxCmdGetTunedFrequency;
static jmethodID _sMethodId_nativeCb_fmTxCmdStartTransmission;
static jmethodID _sMethodId_nativeCb_fmTxCmdStopTransmission;
static jmethodID _sMethodId_nativeCb_fmTxCmdEnableRds;
static jmethodID _sMethodId_nativeCb_fmTxCmdDisableRds;
static jmethodID _sMethodId_nativeCb_fmTxCmdSetRdsTransmissionMode;
//  static jmethodID _sMethodId_nativeCb_fmTxCmdGetRdsTransmissionMode;
static jmethodID _sMethodId_nativeCb_fmTxCmdSetRdsTrafficCodes;
static jmethodID _sMethodId_nativeCb_fmTxCmdGetRdsTrafficCodes;
static jmethodID _sMethodId_nativeCb_fmTxCmdSetRdsTextPsMsg;
static jmethodID _sMethodId_nativeCb_fmTxCmdGetRdsTextPsMsg;
static jmethodID _sMethodId_nativeCb_fmTxCmdSetRdsTextRtMsg;
static jmethodID _sMethodId_nativeCb_fmTxCmdGetRdsTextRtMsg;
static jmethodID _sMethodId_nativeCb_fmTxCmdWriteRdsRawData;
static jmethodID _sMethodId_nativeCb_fmTxCmdReadRdsRawData;
static jmethodID _sMethodId_nativeCb_fmTxCmdChangeAudioSource;
static jmethodID _sMethodId_nativeCb_fmTxCmdSetInterruptMask;
static jmethodID _sMethodId_nativeCb_fmTxCmdSetMonoStereoMode;
static jmethodID _sMethodId_nativeCb_fmTxCmdGetMonoStereoMode;
static jmethodID _sMethodId_nativeCb_fmTxCmdSetPowerLevel;
static jmethodID _sMethodId_nativeCb_fmTxCmdGetPowerLevel;
static jmethodID _sMethodId_nativeCb_fmTxCmdSetMuteMode;
static jmethodID _sMethodId_nativeCb_fmTxCmdGetMuteMode;
static jmethodID _sMethodId_nativeCb_fmTxCmdSetRdsAfCode;
static jmethodID _sMethodId_nativeCb_fmTxCmdGetRdsAfCode;
static jmethodID _sMethodId_nativeCb_fmTxCmdSetRdsPiCode;
static jmethodID _sMethodId_nativeCb_fmTxCmdGetRdsPiCode;
static jmethodID _sMethodId_nativeCb_fmTxCmdSetRdsPtyCode;
static jmethodID _sMethodId_nativeCb_fmTxCmdGetRdsPtyCode;
static jmethodID _sMethodId_nativeCb_fmTxCmdSetRdsTextRepertoire;
static jmethodID _sMethodId_nativeCb_fmTxCmdGetRdsTextRepertoire;
static jmethodID _sMethodId_nativeCb_fmTxCmdSetRdsPsDispalyMode;
static jmethodID _sMethodId_nativeCb_fmTxCmdGetRdsPsDispalyMode;
static jmethodID _sMethodId_nativeCb_fmTxCmdSetRdsPsDisplaySpeed;
static jmethodID _sMethodId_nativeCb_fmTxCmdGetRdsPsDisplaySpeed;
static jmethodID _sMethodId_nativeCb_fmTxCmdSetRdsTransmittedMask;
static jmethodID _sMethodId_nativeCb_fmTxCmdGetRdsTransmittedMask;
static jmethodID _sMethodId_nativeCb_fmTxCmdSetRdsMusicSpeechFlag  ;
static jmethodID _sMethodId_nativeCb_fmTxCmdGetRdsMusicSpeechFlag  ;
static jmethodID _sMethodId_nativeCb_fmTxCmdSetPreEmphasisFilter;
static jmethodID _sMethodId_nativeCb_fmTxCmdGetPreEmphasisFilter;
static jmethodID _sMethodId_nativeCb_fmTxCmdSetRdsExtendedCountryCode;
static jmethodID _sMethodId_nativeCb_fmTxCmdGetRdsExtendedCountryCode;
static jmethodID _sMethodId_nativeCb_fmTxCmdChangeDigitalAudioConfiguration;




static int nativeJFmTx_Create(JNIEnv *env,jobject obj,jobject jContextValue)
{
    FmTxStatus fmStatus;
    FmTxContext *fmTxContext =NULL;
    jclass contextCls = NULL;
    jmethodID setValueMethodId = NULL;
    void * notValid;

    /* Initialize logging module */
    MCP_HAL_LOG_Init();
    MCP_HAL_LOG_EnableLogToAndroid("FM Process");

    MCP_JBTL_LOGD("nativeJFmTx_Create(): Entered");

    fmStatus = FM_TX_Init(notValid);

    if (fmStatus != FM_TX_STATUS_SUCCESS)
    {
        MCP_JBTL_LOGE("nativeJFmTx_Create: FM_TX_Init Failed ");
        return FMC_STATUS_FAILED;
    }

    MCP_JBTL_LOGD("nativeJFmTx_Create: FM_TX_Init returned %d", (int)fmStatus);

    fmStatus = FM_TX_Create(NULL,nativeJFmTx_Callback,&fmTxContext);

    if (fmStatus != FM_TX_STATUS_SUCCESS)
    {
        MCP_JBTL_LOGE("nativeJFmTx_Create: FM_TX_Create Failed ");
        return FMC_STATUS_FAILED;
    }

    MCP_JBTL_LOGD("nativeJFmTx_Create: FM_TX_Create returned %d, context: %x",
                  (int)fmStatus,
                  (unsigned int)fmTxContext);

    MCP_JBTL_LOGD("nativeJFmTx_Create: Setting context value in jContext out parm");

    /* Pass received fmContext via the jContextValue object */
    contextCls = env->GetObjectClass(jContextValue);
    if (contextCls == NULL)
    {
        MCP_JBTL_LOGD("nativeJFmTx_Create: Failed obtaining class for JBtlProfileContext");
        return FMC_STATUS_FAILED;
    }

    setValueMethodId = env->GetMethodID(contextCls, "setValue", "(I)V");
    if (setValueMethodId == NULL)
    {
        MCP_JBTL_LOGD("nativeJFmTx_Create: Failed getting setValue method id");
        return FMC_STATUS_FAILED;
    }
    MCP_JBTL_LOGD("nativeJFmTx_Create: Calling Java setValue(ox%x) in context's class", (unsigned int)fmTxContext);

    LOGD("nativeJFmTx_create: Calling Java setValue(%d) in context's class", (signed int)fmTxContext);

    env->CallVoidMethod(jContextValue, setValueMethodId, (unsigned int)fmTxContext);
    if (env->ExceptionOccurred())
    {
        MCP_JBTL_LOGE("nativeJFmTx_Create: Calling CallVoidMethod(setValue) failed");
        env->ExceptionDescribe();
        return FMC_STATUS_FAILED;
    }


    MCP_JBTL_LOGD("nativeJFmTx_Create:Exiting Successfully");

    return fmStatus;


CLEANUP:

    LOGD("nativeJFmTx_create(): Exiting With a Failure indication");
    return FMC_STATUS_FAILED;

}

static jint nativeJFmTx_Destroy(JNIEnv *env, jobject obj,jlong jContextValue)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    FmTxStatus  status ;
    MCP_JBTL_LOGD("nativeJFmTx_Destroy(): Entered");

    status = FM_TX_Destroy(&fmTxContext);
    MCP_JBTL_LOGD("nativeJFmTx_Destroy: FM_TX_Destroy() returned %d",(int)status);

    status = FM_TX_Deinit();
    MCP_JBTL_LOGD("nativeJFmTx_Destroy: FM_TX_Deinit() returned %d",(int)status);

    /* De-initialize logging module */
    MCP_HAL_LOG_Deinit();

    MCP_JBTL_LOGD("nativeJFmTx_Destroy(): Exit");
    return status;

}



static int nativeJFmTx_Enable(JNIEnv *env, jobject obj, jlong jContextValue)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    MCP_JBTL_LOGD("nativeJFmTx_enable(): Entered");

    FmTxStatus  status =FM_TX_Enable(fmTxContext);
    MCP_JBTL_LOGD("nativeJFmTx_enable: FM_TX_Enable() returned %d",(int)status);

    MCP_JBTL_LOGD("nativeJFmTx_enable(): Exit");
    return status;
}


static int nativeJFmTx_Disable(JNIEnv *env, jobject obj, jlong jContextValue)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    MCP_JBTL_LOGD("nativeJFmTx_Disable(): Entered");

    FmTxStatus  status =FM_TX_Disable(fmTxContext);
    MCP_JBTL_LOGD("nativeJFmTx_Disable: FM_TX_Disable() returned %d",(int)status);

    MCP_JBTL_LOGD("nativeJFmTx_Disable(): Exit");
    return status;
}


static int nativeJFmTx_Tune(JNIEnv *env, jobject obj,jlong jContextValue,jlong jFmFreq)
{
    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;

    LOGD("nativeJFmTx_tune(): Entered");

    FmTxStatus  status =FM_TX_Tune(fmTxContext, (FmcFreq)jFmFreq);
    LOGD("nativeJFmTx_tune: FM_TX_Tune() returned %d",(int)status);


    LOGD("nativeJFmTx_Tune(): Exit");
    return status;

}

static int nativeJFmTx_GetTunedFrequency(JNIEnv *env, jobject obj,jlong jContextValue)
{
    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;

    LOGD("nativeJFmTx_GetTunedFrequency(): Entered");

    FmTxStatus  status =FM_TX_GetTunedFrequency(fmTxContext);
    LOGD("nativeJFmTx_GetTunedFrequency: FM_TX_GetTunedFrequency() returned %d",(int)status);


    LOGD("nativeJFmTx_GetTunedFrequency(): Exit");
    return status;

}
static int nativeJFmTx_StartTransmission(JNIEnv *env, jobject obj, jlong jContextValue)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_StartTransmission(): Entered");

    FmTxStatus  status =FM_TX_StartTransmission(fmTxContext);
    LOGD("nativeJFmTx_StartTransmission: FM_TX_StartTransmission() returned %d",(int)status);

    LOGD("nativeJFmTx_StartTransmission(): Exit");
    return status;
}

static int nativeJFmTx_StopTransmission(JNIEnv *env, jobject obj, jlong jContextValue)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_StopTransmission(): Entered");

    FmTxStatus  status =FM_TX_StopTransmission(fmTxContext);
    LOGD("nativeJFmTx_StopTransmission: FM_TX_StopTransmission() returned %d",(int)status);

    LOGD("nativeJFmTx_StopTransmission(): Exit");
    return status;
}


static int nativeJFmTx_EnableRds(JNIEnv *env, jobject obj, jlong jContextValue)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_EnableRds(): Entered");

    FmTxStatus  status =FM_TX_EnableRds(fmTxContext);
    LOGD("nativeJFmTx_EnableRds: FM_TX_EnableRds() returned %d",(int)status);

    LOGD("nativeJFmTx_EnableRds(): Exit");
    return status;
}


static int nativeJFmTx_DisableRds(JNIEnv *env, jobject obj, jlong jContextValue)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_DisableRds(): Entered");

    FmTxStatus  status =FM_TX_DisableRds(fmTxContext);
    LOGD("nativeJFmTx_DisableRds: FM_TX_DisableRds() returned %d",(int)status);

    LOGD("nativeJFmTx_DisableRds(): Exit");
    return status;
}


static int nativeJFmTx_SetRdsTransmissionMode(JNIEnv *env, jobject obj, jlong jContextValue,jint mode)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_SetRdsTransmissionMode(): Entered");

    FmTxStatus  status =FM_TX_SetRdsTransmissionMode(fmTxContext,(FmTxRdsTransmissionMode)mode);
    LOGD("nativeJFmTx_SetRdsTransmissionMode: FM_TX_SetRdsTransmissionMode() returned %d",(int)status);

    LOGD("nativeJFmTx_SetRdsTransmissionMode(): Exit");
    return status;
}


static int nativeJFmTx_GetRdsTransmissionMode(JNIEnv *env, jobject obj, jlong jContextValue)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_GetRdsTransmissionMode(): Entered");

    FmTxStatus  status =FM_TX_GetRdsTransmissionMode(fmTxContext);
    LOGD("nativeJFmTx_GetRdsTransmissionMode: FM_TX_GetRdsTransmissionMode() returned %d",(int)status);

    LOGD("nativeJFmTx_GetRdsTransmissionMode(): Exit");
    return status;
}

static int nativeJFmTx_SetRdsTextPsMsg(JNIEnv *env, jobject obj, jlong jContextValue,jstring psString,jint length)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    jboolean iscopy;
    const char *psStr = (char*) env->GetStringUTFChars(psString, &iscopy);

    LOGD("nativeJFmTx_SetRdsTextPsMsg(): Entered");
    LOGD("nativeJFmTx_SetRdsTextPsMsg():--> psStr= %s",psStr);

    FmTxStatus  status =FM_TX_SetRdsTextPsMsg(fmTxContext,(const FMC_U8 *)psStr,(FMC_UINT)length);
    LOGD("nativeJFmTx_SetRdsTextPsMsg: FM_TX_SetRdsTextPsMsg() returned %d",(int)status);

    LOGD("nativeJFmTx_SetRdsTextPsMsg(): Exit");

    return status;
}

static int nativeJFmTx_GetRdsTextPsMsg(JNIEnv *env, jobject obj, jlong jContextValue)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_GetRdsTextPsMsg(): Entered");

    FmTxStatus  status =FM_TX_GetRdsTextPsMsg(fmTxContext);
    LOGD("nativeJFmTx_GetRdsTextPsMsg: FM_TX_GetRdsTextPsMsg() returned %d",(int)status);

    LOGD("nativeJFmTx_GetRdsTextPsMsg(): Exit");

    return status;
}

static int nativeJFmTx_WriteRdsRawData(JNIEnv *env, jobject obj, jlong jContextValue,jstring msg,jint length)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_WriteRdsRawData(): Entered");

    jboolean iscopy;
    const char *rawData = (char*) env->GetStringUTFChars(msg, &iscopy);


    FmTxStatus  status =FM_TX_WriteRdsRawData(fmTxContext,(const FMC_U8 *)rawData,(FMC_UINT)length);
    LOGD("nativeJFmTx_WriteRdsRawData: FM_TX_WriteRdsRawData() returned %d",(int)status);

    LOGD("nativeJFmTx_WriteRdsRawData(): Exit");
    return status;
}


static int nativeJFmTx_ReadRdsRawData(JNIEnv *env, jobject obj, jlong jContextValue)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_ReadRdsRawData(): Entered");

    FmTxStatus  status =FM_TX_ReadRdsRawData(fmTxContext);
    LOGD("nativeJFmTx_ReadRdsRawData: FM_TX_ReadRdsRawData() returned %d",(int)status);

    LOGD("nativeJFmTx_ReadRdsRawData(): Exit");
    return status;
}
static int nativeJFmTx_SetMuteMode(JNIEnv *env, jobject obj, jlong jContextValue,jint mode)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_SetMuteMode(): Entered");

    FmTxStatus  status =FM_TX_SetMuteMode(fmTxContext,(FmcMuteMode)mode);
    LOGD("nativeJFmTx_SetMuteMode: FM_TX_SetMuteMode() returned %d",(int)status);

    LOGD("nativeJFmTx_SetMuteMode(): Exit");
    return status;
}

static int nativeJFmTx_GetMuteMode(JNIEnv *env, jobject obj, jlong jContextValue)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_GetMuteMode(): Entered");

    FmTxStatus  status =FM_TX_GetMuteMode(fmTxContext);
    LOGD("nativeJFmTx_GetMuteMode: FM_TX_GetMuteMode() returned %d",(int)status);

    LOGD("nativeJFmTx_GetMuteMode(): Exit");
    return status;
}

static int nativeJFmTx_SetRdsPsDisplayMode(JNIEnv *env, jobject obj, jlong jContextValue, jint displayMode)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_SetRdsPsDisplayMode(): Entered");

    FmTxStatus  status =FM_TX_SetRdsPsDisplayMode(fmTxContext,(FmcRdsPsDisplayMode)displayMode);
    LOGD("nativeJFmTx_SetRdsPsDisplayMode: FM_TX_SetRdsPsDisplayMode() returned %d",(int)status);

    LOGD("nativeJFmTx_SetRdsPsDisplayMode(): Exit");
    return status;
}


static int nativeJFmTx_GetRdsPsDisplayMode(JNIEnv *env, jobject obj, jlong jContextValue)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_GetRdsPsDisplayMode(): Entered");

    FmTxStatus  status =FM_TX_GetRdsPsDisplayMode(fmTxContext);
    LOGD("nativeJFmTx_GetRdsPsDisplayMode: FM_TX_GetRdsPsDisplayMode() returned %d",(int)status);

    LOGD("nativeJFmTx_GetRdsPsDisplayMode(): Exit");
    return status;
}

static int nativeJFmTx_SetRdsTextRtMsg(JNIEnv *env, jobject obj, jlong jContextValue, jint msgType,jstring msg,jint length)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_SetRdsTextRtMsg(): Entered");

    jboolean iscopy;
    const char *rtMsg = (char*) env->GetStringUTFChars(msg, &iscopy);

    LOGD("nativeJFmTx_SetRdsTextRtMsg():--> rtMsg = %s",rtMsg);

    FmTxStatus  status =FM_TX_SetRdsTextRtMsg(fmTxContext,(FmcRdsRtMsgType)msgType,(const FMC_U8 *)rtMsg,(FMC_UINT)length);
    LOGD("nativeJFmTx_SetRdsTextRtMsg: FM_TX_SetRdsTextRtMsg() returned %d",(int)status);

    LOGD("nativeJFmTx_SetRdsTextRtMsg(): Exit");
    return status;
}


static int nativeJFmTx_GetRdsTextRtMsg(JNIEnv *env, jobject obj, jlong jContextValue)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_GetRdsTextRtMsg(): Entered");
    FmTxStatus  status =FM_TX_GetRdsTextRtMsg(fmTxContext);
    LOGD("nativeJFmTx_GetRdsTextRtMsg: FM_TX_SetRdsTextRtMsg() returned %d",(int)status);

    LOGD("nativeJFmTx_GetRdsTextRtMsg(): Exit");
    return status;
}

static int nativeJFmTx_SetRdsTransmittedGroupsMask(JNIEnv *env, jobject obj, jlong jContextValue, jlong fieldMask)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_SetRdsTransmittedGroupsMask(): Entered");

    FmTxStatus  status =FM_TX_SetRdsTransmittedGroupsMask(fmTxContext,(FmTxRdsTransmittedGroupsMask)fieldMask);
    LOGD("nativeJFmTx_SetRdsTransmittedGroupsMask: FM_TX_SetRdsTransmittedGroupsMask() returned %d",(int)status);

    LOGD("nativeJFmTx_SetRdsTransmittedGroupsMask(): Exit");
    return status;
}


static int nativeJFmTx_GetRdsTransmittedGroupsMask(JNIEnv *env, jobject obj, jlong jContextValue)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_GetRdsTransmittedGroupsMask(): Entered");

    FmTxStatus  status =FM_TX_GetRdsTransmittedGroupsMask(fmTxContext);
    LOGD("nativeJFmTx_GetRdsTransmittedGroupsMask: FM_TX_GetRdsTransmittedGroupsMask() returned %d",(int)status);

    LOGD("nativeJFmTx_GetRdsTransmittedGroupsMask(): Exit");
    return status;
}

static int nativeJFmTx_SetRdsTrafficCodes(JNIEnv *env, jobject obj, jlong jContextValue, jint taCode,jint tpCode)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_SetRdsTrafficCodes(): Entered");

    FmTxStatus  status =FM_TX_SetRdsTrafficCodes(fmTxContext,(FmcRdsTaCode)taCode,(FmcRdsTpCode)tpCode);
    LOGD("nativeJFmTx_SetRdsTrafficCodes: FM_TX_SetRdsTrafficCodes() returned %d",(int)status);

    LOGD("nativeJFmTx_SetRdsTrafficCodes(): Exit");
    return status;
}

static int nativeJFmTx_GetRdsTrafficCodes(JNIEnv *env, jobject obj, jlong jContextValue)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_GetRdsTrafficCodes(): Entered");

    FmTxStatus  status =FM_TX_GetRdsTrafficCodes(fmTxContext);
    LOGD("nativeJFmTx_GetRdsTrafficCodes: FM_TX_GetRdsTrafficCodes() returned %d",(int)status);

    LOGD("nativeJFmTx_GetRdsTrafficCodes(): Exit");
    return status;
}

static int nativeJFmTx_SetRdsMusicSpeechFlag(JNIEnv *env, jobject obj, jlong jContextValue, jint musicSpeechFlag)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_SetRdsMusicSpeechFlag(): Entered");

    FmTxStatus  status =FM_TX_SetRdsMusicSpeechFlag(fmTxContext,(FmcRdsMusicSpeechFlag)musicSpeechFlag);
    LOGD("nativeJFmTx_SetRdsMusicSpeechFlag: FM_TX_SetRdsMusicSpeechFlag() returned %d",(int)status);

    LOGD("nativeJFmTx_SetRdsMusicSpeechFlag(): Exit");
    return status;
}


static int nativeJFmTx_GetRdsMusicSpeechFlag(JNIEnv *env, jobject obj, jlong jContextValue)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_GetRdsMusicSpeechFlag(): Entered");

    FmTxStatus  status =FM_TX_GetRdsMusicSpeechFlag(fmTxContext);
    LOGD("nativeJFmTx_GetRdsMusicSpeechFlag: FM_TX_GetRdsMusicSpeechFlag() returned %d",(int)status);

    LOGD("nativeJFmTx_GetRdsMusicSpeechFlag(): Exit");
    return status;
}

static int nativeJFmTx_SetRdsExtendedCountryCode(JNIEnv *env, jobject obj, jlong jContextValue, jint ecc)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_SetRdsExtendedCountryCode(): Entered");

    FmTxStatus  status =FM_TX_SetRdsECC(fmTxContext,(FmcRdsExtendedCountryCode)ecc);
    LOGD("nativeJFmTx_SetRdsExtendedCountryCode: FM_TX_SetRdsECC() returned %d",(int)status);

    LOGD("nativeJFmTx_SetRdsExtendedCountryCode(): Exit");
    return status;
}

static int nativeJFmTx_GetRdsExtendedCountryCode(JNIEnv *env, jobject obj, jlong jContextValue)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_GetRdsExtendedCountryCode(): Entered");

    FmTxStatus  status =FM_TX_GetRdsECC(fmTxContext);
    LOGD("nativeJFmTx_GetRdsExtendedCountryCode: FM_TX_GetRdsECC() returned %d",(int)status);

    LOGD("nativeJFmTx_GetRdsExtendedCountryCode(): Exit");
    return status;
}

static int nativeJFmTx_ChangeAudioSource(JNIEnv *env, jobject obj, jlong jContextValue,jint txSource,jint eSampleFreq)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_ChangeAudioSource(): Entered");

    LOGD(" txSource = %d , Sampling frequency = %d ",(int) txSource, (int) eSampleFreq);
    FmTxStatus  status =FM_TX_ChangeAudioSource(fmTxContext,(FmTxAudioSource)txSource,(ECAL_SampleFrequency)eSampleFreq);
    LOGD("nativeJFmTx_ChangeAudioSource: FM_TX_ChangeAudioSource() returned %d",(int)status);

    LOGD("nativeJFmTx_ChangeAudioSource(): Exit");
    return status;
}


static int nativeJFmTx_ChangeDigitalSourceConfiguration(JNIEnv *env, jobject obj, jlong jContextValue,jint eSampleFreq)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_ChangeDigitalSourceConfiguration(): Entered");

    FmTxStatus  status =FM_TX_ChangeDigitalSourceConfiguration(fmTxContext,(ECAL_SampleFrequency)eSampleFreq);
    LOGD("nativeJFmTx_ChangeDigitalSourceConfiguration: FM_TX_ChangeDigitalSourceConfiguration() returned %d",(int)status);

    LOGD("nativeJFmTx_ChangeDigitalSourceConfiguration(): Exit");
    return status;
}


static int nativeJFmTx_SetRdsTextRepertoire(JNIEnv *env, jobject obj, jlong jContextValue,jint repertoire)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_SetRdsTextRepertoire(): Entered");

    FmTxStatus  status =FM_TX_SetRdsTextRepertoire(fmTxContext,(FmcRdsRepertoire)repertoire);
    LOGD("nativeJFmTx_SetRdsTextRepertoire: FM_TX_SetRdsTextRepertoire() returned %d",(int)status);

    LOGD("nativeJFmTx_SetRdsTextRepertoire(): Exit");
    return status;
}


static int nativeJFmTx_GetRdsTextRepertoire(JNIEnv *env, jobject obj, jlong jContextValue,jint repertoire)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_GetRdsTextRepertoire(): Entered");

    FmTxStatus  status =FM_TX_GetRdsTextRepertoire(fmTxContext);
    LOGD("nativeJFmTx_GetRdsTextRepertoire: FM_TX_GetRdsTextRepertoire() returned %d",(int)status);

    LOGD("nativeJFmTx_GetRdsTextRepertoire(): Exit");
    return status;
}

static int nativeJFmTx_SetRdsPtyCode(JNIEnv *env, jobject obj, jlong jContextValue,jint ptyCode)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_SetRdsPtyCode(): Entered");

    FmTxStatus  status =FM_TX_SetRdsPtyCode(fmTxContext,(FmcRdsPtyCode)ptyCode);
    LOGD("nativeJFmTx_SetRdsPtyCode: FM_TX_SetRdsPtyCode() returned %d",(int)status);

    LOGD("nativeJFmTx_SetRdsPtyCode(): Exit");
    return status;
}

static int nativeJFmTx_GetRdsPtyCode(JNIEnv *env, jobject obj, jlong jContextValue,jint ptyCode)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_GetRdsPtyCode(): Entered");

    FmTxStatus  status =FM_TX_GetRdsPtyCode(fmTxContext);
    LOGD("nativeJFmTx_GetRdsPtyCode: FM_TX_GetRdsPtyCode() returned %d",(int)status);

    LOGD("nativeJFmTx_GetRdsPtyCode(): Exit");
    return status;
}


static int nativeJFmTx_SetRdsPiCode(JNIEnv *env, jobject obj, jlong jContextValue,jint piCode)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_SetRdsPiCode(): Entered");

    FmTxStatus  status =FM_TX_SetRdsPiCode(fmTxContext,(FmcRdsPiCode)piCode);
    LOGD("nativeJFmTx_SetRdsPiCode: FM_TX_SetRdsPiCode() returned %d",(int)status);

    LOGD("nativeJFmTx_SetRdsPiCode(): Exit");
    return status;
}

static int nativeJFmTx_GetRdsPiCode(JNIEnv *env, jobject obj, jlong jContextValue)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_GetRdsPiCode(): Entered");

    FmTxStatus  status =FM_TX_GetRdsPiCode(fmTxContext);
    LOGD("nativeJFmTx_GetRdsPiCode: FM_TX_GetRdsPiCode() returned %d",(int)status);

    LOGD("nativeJFmTx_GetRdsPiCode(): Exit");
    return status;
}
static int nativeJFmTx_SetRdsAfCode(JNIEnv *env, jobject obj, jlong jContextValue,jint afCode)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_SetRdsAfCode(): Entered");

    FmTxStatus  status =FM_TX_SetRdsAfCode(fmTxContext,(FmcAfCode)afCode);
    LOGD("nativeJFmTx_SetRdsAfCode: FM_TX_SetRdsAfCode() returned %d",(int)status);

    LOGD("nativeJFmTx_SetRdsAfCode(): Exit");
    return status;
}

static int nativeJFmTx_GetRdsAfCode(JNIEnv *env, jobject obj, jlong jContextValue)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_GetRdsAfCode(): Entered");

    FmTxStatus  status =FM_TX_GetRdsAfCode(fmTxContext);
    LOGD("nativeJFmTx_GetRdsAfCode: FM_TX_GetRdsAfCode() returned %d",(int)status);

    LOGD("nativeJFmTx_GetRdsAfCode(): Exit");
    return status;
}

static int nativeJFmTx_SetMonoStereoMode(JNIEnv *env, jobject obj, jlong jContextValue,jint monoStereoMode)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_SetMonoStereoMode(): Entered");

    FmTxStatus  status =FM_TX_SetMonoStereoMode(fmTxContext,(FmTxMonoStereoMode)monoStereoMode);
    LOGD("nativeJFmTx_SetMonoStereoMode: FM_TX_SetMonoStereoMode() returned %d",(int)status);

    LOGD("nativeJFmTx_SetMonoStereoMode(): Exit");
    return status;
}

static int nativeJFmTx_GetMonoStereoMode(JNIEnv *env, jobject obj, jlong jContextValue)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_GetMonoStereoMode(): Entered");

    FmTxStatus  status =FM_TX_GetMonoStereoMode(fmTxContext);
    LOGD("nativeJFmTx_GetMonoStereoMode: FM_TX_GetMonoStereoMode() returned %d",(int)status);

    LOGD("nativeJFmTx_GetMonoStereoMode(): Exit");
    return status;
}

static int nativeJFmTx_SetPowerLevel(JNIEnv *env, jobject obj, jlong jContextValue,jint powerLevel)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_SetPowerLevel(): Entered");

    FmTxStatus  status =FM_TX_SetPowerLevel(fmTxContext,(FmTxPowerLevel)powerLevel);
    LOGD("nativeJFmTx_SetPowerLevel: FM_TX_SetPowerLevel() returned %d",(int)status);

    LOGD("nativeJFmTx_SetPowerLevel(): Exit");
    return status;
}

static int nativeJFmTx_GetPowerLevel(JNIEnv *env, jobject obj, jlong jContextValue)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_GetPowerLevel(): Entered");

    FmTxStatus  status =FM_TX_GetPowerLevel(fmTxContext);
    LOGD("nativeJFmTx_GetPowerLevel: FM_TX_GetPowerLevel() returned %d",(int)status);

    LOGD("nativeJFmTx_GetPowerLevel(): Exit");
    return status;
}

static int nativeJFmTx_SetPreEmphasisFilter(JNIEnv *env, jobject obj, jlong jContextValue,jint preEmpFilter)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_SetPreEmphasisFilter(): Entered");

    FmTxStatus  status =FM_TX_SetPreEmphasisFilter(fmTxContext,(FmcEmphasisFilter)preEmpFilter);
    LOGD("nativeJFmTx_SetPreEmphasisFilter: FM_TX_SetPreEmphasisFilter() returned %d",(int)status);

    LOGD("nativeJFmTx_SetPreEmphasisFilter(): Exit");
    return status;
}


static int nativeJFmTx_GetPreEmphasisFilter(JNIEnv *env, jobject obj, jlong jContextValue)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_GetPreEmphasisFilter(): Entered");

    FmTxStatus  status =FM_TX_GetPreEmphasisFilter(fmTxContext);
    LOGD("nativeJFmTx_GetPreEmphasisFilter: FM_TX_GSetPreEmphasisFilter() returned %d",(int)status);

    LOGD("nativeJFmTx_GetPreEmphasisFilter(): Exit");
    return status;
}

static int nativeJFmTx_SetRdsPsScrollSpeed(JNIEnv *env, jobject obj, jlong jContextValue,jint scrollSpeed)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_SetRdsPsScrollSpeed(): Entered");

    FmTxStatus  status =FM_TX_SetRdsPsScrollSpeed(fmTxContext,(FmcRdsPsScrollSpeed)scrollSpeed);
    LOGD("nativeJFmTx_SetRdsPsScrollSpeed: FM_TX_SetRdsPsScrollSpeed() returned %d",(int)status);

    LOGD("nativeJFmTx_SetRdsPsScrollSpeed(): Exit");
    return status;
}

static int nativeJFmTx_GetRdsPsScrollSpeed(JNIEnv *env, jobject obj, jlong jContextValue)
{

    FmTxContext * fmTxContext = (FmTxContext *)jContextValue;
    LOGD("nativeJFmTx_GetRdsPsScrollSpeed(): Entered");

    FmTxStatus  status =FM_TX_GetRdsPsScrollSpeed(fmTxContext);
    LOGD("nativeJFmTx_GetRdsPsScrollSpeed: FM_TX_GetRdsPsScrollSpeed() returned %d",(int)status);

    LOGD("nativeJFmTx_GetRdsPsScrollSpeed(): Exit");
    return status;
}
//################################################################################

//								 SIGNALS

//###############################################################################

extern "C"
{

    void nativeJFmTx_Callback(const fm_tx_event *event)
    {

        MCP_JBTL_LOGI("nativeJFmTx_Callback: Entered, ");
        MCP_JBTL_LOGD( "got event %d", event->eventType);

        JNIEnv* env = NULL;
        jbyteArray jRadioTextMsg= NULL;
        jclass* lptUnavailResources = NULL;

        g_jVM->AttachCurrentThread((&env), NULL);


        if (env == NULL)
        {
            MCP_JBTL_LOGE("nativeJFmTx_Callback: Entered, env is null");
        }


        switch (event->p.cmdDone.cmdType) {

        case FM_TX_CMD_ENABLE:
            MCP_JBTL_LOGI("FM_TX_CMD_ENABLE:Status: %d ",event->status);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdEnable,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_DISABLE:
            MCP_JBTL_LOGI("FM_TX_CMD_DISABLE:Status: %d ",event->status);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdDisable,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_SET_INTERRUPT_MASK:
            MCP_JBTL_LOGI("FM_TX_CMD_SET_INTERRUPT_MASK:Status: %d ",event->status);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdSetInterruptMask,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

            /*
        case FM_TX_CMD_GET_INTERRUPT_STATUS:
            MCP_JBTL_LOGI("FM_TX_CMD_DISABLE:Status: %d ",event->status);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdGetInterruptMask,
                                     (jlong)event->context,
                                     (jint)event->status,
                                     (jint)event->p.cmdDone.v.value);
            break;
            */

        case FM_TX_CMD_START_TRANSMISSION:
            MCP_JBTL_LOGI("FM_TX_CMD_START_TRANSMISSION:Status: %d ",event->status);
            lptUnavailResources = (jclass *)event->p.cmdDone.v.audioOperation.ptUnavailResources;
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdStartTransmission,
                                      (jlong)event->context,
                                      (jint)event->status);
            break;

        case FM_TX_CMD_STOP_TRANSMISSION:
            MCP_JBTL_LOGI("FM_TX_CMD_STOP_TRANSMISSION:Status: %d ",event->status);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdStopTransmission,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_TUNE:
            MCP_JBTL_LOGI("FM_TX_CMD_TUNE:Status: %d ",event->status);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdTune,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_GET_TUNED_FREQUENCY:
            MCP_JBTL_LOGI("FM_TX_CMD_GET_TUNED_FREQUENCY:Status: %d,Value: %d ",event->status,event->p.cmdDone.v.value);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdGetTunedFrequency,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;


        case FM_TX_CMD_SET_MONO_STEREO_MODE:
            MCP_JBTL_LOGI("FM_TX_CMD_SET_MONO_STEREO_MODE:Status: %d ",event->status);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdSetMonoStereoMode,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_GET_MONO_STEREO_MODE:
            MCP_JBTL_LOGI("FM_TX_CMD_GET_MONO_STEREO_MODE:Status: %d,Value: %d ",event->status,event->p.cmdDone.v.value);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdGetMonoStereoMode,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;


        case FM_TX_CMD_SET_POWER_LEVEL:
            MCP_JBTL_LOGI("FM_TX_CMD_SET_POWER_LEVEL:Status: %d ",event->status);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdSetPowerLevel,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_GET_POWER_LEVEL:
            MCP_JBTL_LOGI("FM_TX_CMD_GET_POWER_LEVEL:Status: %d,Value: %d ",event->status,event->p.cmdDone.v.value);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdGetPowerLevel,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_SET_MUTE_MODE:
            MCP_JBTL_LOGI("FM_TX_CMD_SET_MUTE_MODE:Status: %d ",event->status);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdSetMuteMode,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_GET_MUTE_MODE:
            MCP_JBTL_LOGI("FM_TX_CMD_SET_MUTE_MODE:Status: %d ",event->status);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdGetMuteMode,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_ENABLE_RDS:
            MCP_JBTL_LOGI("FM_TX_CMD_ENABLE_RDS:Status: %d ",event->status);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdEnableRds,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_DISABLE_RDS:
            MCP_JBTL_LOGI("FM_TX_CMD_DISABLE_RDS:Status: %d ",event->status);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdDisableRds,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_SET_RDS_TRANSMISSION_MODE:
            MCP_JBTL_LOGI("FM_TX_CMD_SET_RDS_TRANSMISSION_MODE:Status: %d ",event->status);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdSetRdsTransmissionMode,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

            /*
        case FM_TX_CMD_GET_RDS_TRANSMISSION_MODE
            MCP_JBTL_LOGI("FM_TX_CMD_GET_POWER_LEVEL:Status: %d,Value: %d ",event->status,event->p.cmdDone.v.value);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdGetRdsTransmissionMode,
                                     (jlong)event->context,
                                     (jint)event->status,
                                     (jint)event->p.cmdDone.v.value);
            break;
            */

        case FM_TX_CMD_SET_RDS_AF_CODE:
            MCP_JBTL_LOGI("FM_TX_CMD_SET_RDS_AF_CODE:Status: %d ",event->status);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdSetRdsAfCode,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_GET_RDS_AF_CODE:
            MCP_JBTL_LOGI("FM_TX_CMD_GET_POWER_LEVEL:Status: %d,Value: %d ",event->status,event->p.cmdDone.v.value);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdGetRdsAfCode,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_SET_RDS_PI_CODE:
            MCP_JBTL_LOGI("FM_TX_CMD_SET_RDS_PI_CODE:Status: %d ",event->status);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdSetRdsPiCode,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_GET_RDS_PI_CODE:
            MCP_JBTL_LOGI("FM_TX_CMD_GET_RDS_PI_CODE:Status: %d,Value: %d ",event->status,event->p.cmdDone.v.value);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdGetRdsPiCode,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_SET_RDS_PTY_CODE:
            MCP_JBTL_LOGI("FM_TX_CMD_SET_RDS_PI_CODE:Status: %d ",event->status);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdSetRdsPtyCode,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_GET_RDS_PTY_CODE:
            MCP_JBTL_LOGI("FM_TX_CMD_GET_RDS_PTY_CODE:Status: %d,Value: %d ",event->status,event->p.cmdDone.v.value);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdGetRdsPtyCode,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_SET_RDS_TEXT_REPERTOIRE:
            MCP_JBTL_LOGI("FM_TX_CMD_SET_RDS_TEXT_REPERTOIRE:Status: %d ",event->status);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdSetRdsTextRepertoire,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_GET_RDS_TEXT_REPERTOIRE:
            MCP_JBTL_LOGI("FM_TX_CMD_GET_RDS_TEXT_REPERTOIRE:Status: %d,Value: %d ",event->status,event->p.cmdDone.v.value);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdGetRdsTextRepertoire,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_SET_RDS_PS_DISPLAY_MODE:
            MCP_JBTL_LOGI("FM_TX_CMD_SET_RDS_PS_DISPLAY_MODE:Status: %d ",event->status);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdSetRdsPsDispalyMode,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_GET_RDS_PS_DISPLAY_MODE:
            MCP_JBTL_LOGI("FM_TX_CMD_GET_RDS_PS_DISPLAY_MODE:Status: %d,Value: %d ",event->status,event->p.cmdDone.v.value);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdGetRdsPsDispalyMode,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_SET_RDS_PS_DISPLAY_SPEED:
            MCP_JBTL_LOGI("FM_TX_CMD_SET_RDS_PS_DISPLAY_SPEED:Status: %d ",event->status);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdSetRdsPsDisplaySpeed,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_GET_RDS_PS_DISPLAY_SPEED:
            MCP_JBTL_LOGI("FM_TX_CMD_GET_RDS_PS_DISPLAY_SPEED:Status: %d,Value: %d ",event->status,event->p.cmdDone.v.value);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdGetRdsPsDisplaySpeed,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_SET_RDS_TEXT_PS_MSG:
            MCP_JBTL_LOGI("FM_TX_CMD_SET_RDS_TEXT_PS_MSG:Status: %d ",event->status);

            jRadioTextMsg = env->NewByteArray(event->p.cmdDone.v.psMsg.len);

            if (jRadioTextMsg == NULL)
            {
                MCP_JBTL_LOGE("FM_TX_CMD_SET_RDS_TEXT_PS_MSG: Failed converting elements");
                goto EXCEPTION;
            }

            env->SetByteArrayRegion(jRadioTextMsg,0,event->p.cmdDone.v.psMsg.len,(jbyte*)event->p.cmdDone.v.psMsg.msg);

            if (env->ExceptionOccurred())    {
                MCP_JBTL_LOGE("FM_TX_CMD_SET_RDS_TEXT_PS_MSG: env->SetByteArrayRegion failed ");
                goto EXCEPTION;
            }

            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdSetRdsTextPsMsg,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.psMsg.len,
                                      jRadioTextMsg);
            break;

        case FM_TX_CMD_GET_RDS_TEXT_PS_MSG:
            MCP_JBTL_LOGI("FM_TX_CMD_GET_RDS_TEXT_PS_MSG:Status: %d ",event->status);

            jRadioTextMsg = env->NewByteArray(event->p.cmdDone.v.psMsg.len);

            if (jRadioTextMsg == NULL)
            {
                MCP_JBTL_LOGE("FM_TX_CMD_GET_RDS_TEXT_PS_MSG: Failed converting elements");
                goto EXCEPTION;
            }

            env->SetByteArrayRegion(jRadioTextMsg,0,event->p.cmdDone.v.psMsg.len,(jbyte*)event->p.cmdDone.v.psMsg.msg);

            if (env->ExceptionOccurred())    {
                MCP_JBTL_LOGE("FM_TX_CMD_GET_RDS_TEXT_PS_MSG: env->SetByteArrayRegion failed");
                goto EXCEPTION;
            }

            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdGetRdsTextPsMsg,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.psMsg.len,
                                      jRadioTextMsg);
            break;

        case FM_TX_CMD_SET_RDS_TEXT_RT_MSG:
            MCP_JBTL_LOGI("FM_TX_CMD_SET_RDS_TEXT_RT_MSG:Status: %d ",event->status);

            jRadioTextMsg = env->NewByteArray(event->p.cmdDone.v.rtMsg.len);

            if (jRadioTextMsg == NULL)
            {
                MCP_JBTL_LOGE("FM_TX_CMD_SET_RDS_TEXT_RT_MSG: Failed converting elements");
                goto EXCEPTION;
            }

            env->SetByteArrayRegion(jRadioTextMsg,0,event->p.cmdDone.v.rtMsg.len,(jbyte*)event->p.cmdDone.v.rtMsg.msg);

            if (env->ExceptionOccurred())    {
                MCP_JBTL_LOGE("FM_TX_CMD_GET_RDS_TEXT_PS_MSG: env->SetByteArrayRegion failed");
                goto EXCEPTION;
            }

            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdSetRdsTextRtMsg,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.rtMsg.rtMsgType,
                                      (jint)event->p.cmdDone.v.rtMsg.len,
                                      jRadioTextMsg);
            break;

        case FM_TX_CMD_GET_RDS_TEXT_RT_MSG:
            MCP_JBTL_LOGI("FM_TX_CMD_GET_RDS_TEXT_RT_MSG:Status: %d ",event->status);

            jRadioTextMsg = env->NewByteArray(event->p.cmdDone.v.rtMsg.len);

            if (jRadioTextMsg == NULL)
            {
                MCP_JBTL_LOGE("FM_TX_CMD_GET_RDS_TEXT_RT_MSG: Failed converting elements");
                goto EXCEPTION;
            }

            env->SetByteArrayRegion(jRadioTextMsg,0,event->p.cmdDone.v.rtMsg.len,(jbyte*)event->p.cmdDone.v.rtMsg.msg);

            if (env->ExceptionOccurred())    {
                MCP_JBTL_LOGE("FM_TX_CMD_GET_RDS_TEXT_RT_MSG: env->SetByteArrayRegion failed");
                goto EXCEPTION;
            }

            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdGetRdsTextRtMsg,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.rtMsg.rtMsgType,
                                      (jint)event->p.cmdDone.v.rtMsg.len,
                                      jRadioTextMsg);
            break;

        case FM_TX_CMD_SET_RDS_TRANSMITTED_MASK:
            MCP_JBTL_LOGI("FM_TX_CMD_SET_RDS_TRANSMITTED_MASK:Status: %d ",event->status);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdSetRdsTransmittedMask,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_GET_RDS_TRANSMITTED_MASK:
            MCP_JBTL_LOGI("FM_TX_CMD_GET_RDS_TRANSMITTED_MASK:Status: %d ",event->status);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdGetRdsTransmittedMask,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_SET_RDS_TRAFFIC_CODES:
            MCP_JBTL_LOGI("FM_TX_CMD_SET_RDS_TRAFFIC_CODES:Status: %d ",event->status);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdSetRdsTrafficCodes,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.trafficCodes.taCode,
                                      (jint)event->p.cmdDone.v.trafficCodes.tpCode);
            break;

        case FM_TX_CMD_GET_RDS_TRAFFIC_CODES:
            MCP_JBTL_LOGI("FM_TX_CMD_GET_RDS_TRAFFIC_CODES:Status: %d , taCode: %d ,tpCode: %d ",event->status,event->p.cmdDone.v.trafficCodes.taCode,event->p.cmdDone.v.trafficCodes.tpCode);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdGetRdsTrafficCodes,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.trafficCodes.taCode,
                                      (jint)event->p.cmdDone.v.trafficCodes.tpCode);
            break;

        case FM_TX_CMD_SET_RDS_MUSIC_SPEECH_FLAG:
            MCP_JBTL_LOGI("FM_TX_CMD_SET_RDS_MUSIC_SPEECH_FLAG:Status: %d ",event->status);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdSetRdsMusicSpeechFlag,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_GET_RDS_MUSIC_SPEECH_FLAG:
            MCP_JBTL_LOGI("FM_TX_CMD_GET_RDS_MUSIC_SPEECH_FLAG:Status: %d,Value: %d ",event->status,event->p.cmdDone.v.value);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdGetRdsMusicSpeechFlag,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_SET_PRE_EMPHASIS_FILTER:
            MCP_JBTL_LOGI("FM_TX_CMD_SET_PRE_EMPHASIS_FILTER:Status: %d ",event->status);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdSetPreEmphasisFilter,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_GET_PRE_EMPHASIS_FILTER:
            MCP_JBTL_LOGI("FM_TX_CMD_GET_PRE_EMPHASIS_FILTER:Status: %d,Value: %d ",event->status,event->p.cmdDone.v.value);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdGetPreEmphasisFilter,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_SET_RDS_EXTENDED_COUNTRY_CODE:
            MCP_JBTL_LOGI("FM_TX_CMD_SET_RDS_EXTENDED_COUNTRY_CODE:Status: %d ",event->status);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdSetRdsExtendedCountryCode,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_GET_RDS_EXTENDED_COUNTRY_CODE:
            MCP_JBTL_LOGI("FM_TX_CMD_GET_PRE_EMPHASIS_FILTER:Status: %d,Value: %d ",event->status,event->p.cmdDone.v.value);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdGetRdsExtendedCountryCode,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        case FM_TX_CMD_WRITE_RDS_RAW_DATA:
            MCP_JBTL_LOGI("FM_TX_CMD_WRITE_RDS_RAW_DATA:Status: %d ",event->status);
            jRadioTextMsg = env->NewByteArray(event->p.cmdDone.v.rawRds.len);

            if (jRadioTextMsg == NULL)
            {
                MCP_JBTL_LOGE("FM_TX_CMD_WRITE_RDS_RAW_DATA: Failed converting elements");
                goto EXCEPTION;
            }

            env->SetByteArrayRegion(jRadioTextMsg,0,event->p.cmdDone.v.rawRds.len,(jbyte*)event->p.cmdDone.v.rawRds.data);

            if (env->ExceptionOccurred())    {
                MCP_JBTL_LOGE("FM_TX_CMD_WRITE_RDS_RAW_DATA: env->SetByteArrayRegion failed");
                goto EXCEPTION;
            }

            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdWriteRdsRawData,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.rawRds.len,
                                      jRadioTextMsg);
            break;

        case FM_TX_CMD_READ_RDS_RAW_DATA:
            MCP_JBTL_LOGI("FM_TX_CMD_READ_RDS_RAW_DATA:Status: %d ",event->status);
            jRadioTextMsg = env->NewByteArray(event->p.cmdDone.v.rawRds.len);

            if (jRadioTextMsg == NULL)
            {
                MCP_JBTL_LOGE("FM_TX_CMD_READ_RDS_RAW_DATA: Failed converting elements");
                goto EXCEPTION;
            }

            env->SetByteArrayRegion(jRadioTextMsg,0,event->p.cmdDone.v.rawRds.len,(jbyte*)event->p.cmdDone.v.rawRds.data);

            if (env->ExceptionOccurred())    {
                MCP_JBTL_LOGE("FM_TX_CMD_READ_RDS_RAW_DATA: env->SetByteArrayRegion failed");
                goto EXCEPTION;
            }

            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdReadRdsRawData,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.rawRds.len,
                                      jRadioTextMsg);
            break;

        case FM_TX_CMD_CHANGE_AUDIO_SOURCE:
            MCP_JBTL_LOGI("FM_TX_CMD_CHANGE_AUDIO_SOURCE:Status: %d ",event->status);
            lptUnavailResources = (jclass *)event->p.cmdDone.v.audioOperation.ptUnavailResources;
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdChangeAudioSource,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jclass)lptUnavailResources);
            break;

        case FM_TX_CMD_CHANGE_DIGITAL_AUDIO_CONFIGURATION:
            MCP_JBTL_LOGI("FM_TX_CMD_CHANGE_AUDIO_SOURCE:Status: %d ",event->status);
            env->CallStaticVoidMethod(_sJClass,_sMethodId_nativeCb_fmTxCmdChangeDigitalAudioConfiguration,
                                      (jlong)event->context,
                                      (jint)event->status,
                                      (jint)event->p.cmdDone.v.value);
            break;

        default:
            MCP_JBTL_LOGE("nativeJFmTx_Callback():EVENT cmdType-------------->default");
            MCP_JBTL_LOGE("unhandled fm cmdType %d", event->p.cmdDone.cmdType);
            break;
        } //end switch

        if (env->ExceptionOccurred())    {
            MCP_JBTL_LOGE("nativeJFmTx_Callback:  ExceptionOccurred");
            goto EXCEPTION;
        }

//Delete the local references
        if (jRadioTextMsg!= NULL)
            env->DeleteLocalRef(jRadioTextMsg);

        MCP_JBTL_LOGD("nativeJFmTx_Callback: Exiting, Calling DetachCurrentThread at the END");

        g_jVM->DetachCurrentThread();

        return;

EXCEPTION:
        MCP_JBTL_LOGE("nativeJFmTx_Callback: Exiting due to failure");
        if (jRadioTextMsg!= NULL)
            env->DeleteLocalRef(jRadioTextMsg);

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



void nativeJFmTx_ClassInitNative(JNIEnv* env, jclass clazz){
    MCP_JBTL_LOGD("nativeJFmTx_ClassInitNative: Entered");


    if (NULL == env)
    {
        MCP_JBTL_LOGE("nativeJFmRx_ClassInitNative: NULL == env");
    }

    env->GetJavaVM(&g_jVM);

    /* Save class information in global reference in order to prevent class unloading */
    _sJClass = (jclass)env->NewGlobalRef(clazz);

    /* Initialize structure of RBTL callbacks */
    MCP_JBTL_LOGD("nativeJFmTx_ClassInitNative: Initializing FMTX callback structure");


    MCP_JBTL_LOGD("nativeJFmTx_ClassInitNative: Obtaining method IDs");


    _sMethodId_nativeCb_fmTxCmdEnable = env->GetStaticMethodID(clazz,
                                        "nativeCb_fmTxCmdEnable",
                                        "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdEnable);


    _sMethodId_nativeCb_fmTxCmdDisable = env->GetStaticMethodID(clazz,
                                         "nativeCb_fmTxCmdDisable",
                                         "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdDisable);

    _sMethodId_nativeCb_fmTxCmdDestroy = env->GetStaticMethodID(clazz,
                                         "nativeCb_fmTxCmdDestroy",
                                         "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdDestroy);


    _sMethodId_nativeCb_fmTxCmdTune = env->GetStaticMethodID(clazz,
                                      "nativeCb_fmTxCmdTune",
                                      "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdTune);


    _sMethodId_nativeCb_fmTxCmdGetTunedFrequency= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdGetTunedFrequency",
            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdGetTunedFrequency);



    _sMethodId_nativeCb_fmTxCmdStartTransmission = env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdStartTransmission",
            "(JI)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdStartTransmission);


    _sMethodId_nativeCb_fmTxCmdStopTransmission = env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdStopTransmission",
            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdStopTransmission);


    _sMethodId_nativeCb_fmTxCmdEnableRds = env->GetStaticMethodID(clazz,
                                           "nativeCb_fmTxCmdEnableRds",
                                           "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdEnableRds);


    _sMethodId_nativeCb_fmTxCmdDisableRds = env->GetStaticMethodID(clazz,
                                            "nativeCb_fmTxCmdDisableRds",
                                            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdDisableRds);


    _sMethodId_nativeCb_fmTxCmdSetRdsTransmissionMode = env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdSetRdsTransmissionMode",
            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdSetRdsTransmissionMode);
    /*
    _sMethodId_nativeCb_fmTxCmdGetRdsTransmissionMode = env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdGetRdsTransmissionMode",
            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdGetRdsTransmissionMode);
    */

    _sMethodId_nativeCb_fmTxCmdGetRdsTrafficCodes= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdGetRdsTrafficCodes",
            "(JIII)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdGetRdsTrafficCodes);


    _sMethodId_nativeCb_fmTxCmdSetRdsTrafficCodes= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdSetRdsTrafficCodes",
            "(JIII)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdSetRdsTrafficCodes);



    _sMethodId_nativeCb_fmTxCmdSetRdsTextPsMsg= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdSetRdsTextPsMsg",
            "(JII[B)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdSetRdsTextPsMsg);


    _sMethodId_nativeCb_fmTxCmdGetRdsTextPsMsg= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdGetRdsTextPsMsg",
            "(JII[B)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdGetRdsTextPsMsg);


    _sMethodId_nativeCb_fmTxCmdSetRdsTextRtMsg= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdSetRdsTextRtMsg",
            "(JIII[B)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdSetRdsTextRtMsg);


    _sMethodId_nativeCb_fmTxCmdGetRdsTextRtMsg= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdGetRdsTextRtMsg",
            "(JIII[B)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdGetRdsTextRtMsg);


    _sMethodId_nativeCb_fmTxCmdWriteRdsRawData= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdWriteRdsRawData",
            "(JII[B)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdWriteRdsRawData);


    _sMethodId_nativeCb_fmTxCmdReadRdsRawData= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdReadRdsRawData",
            "(JII[B)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdReadRdsRawData);


    _sMethodId_nativeCb_fmTxCmdSetInterruptMask= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdSetInterruptMask",
            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdSetInterruptMask);

    /*
    _sMethodId_nativeCb_fmTxCmdGetInterruptMask= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdGetInterruptMask",
            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdGetInterruptMask);
    */


    _sMethodId_nativeCb_fmTxCmdSetMonoStereoMode= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdSetMonoStereoMode",
            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdSetMonoStereoMode);


    _sMethodId_nativeCb_fmTxCmdGetMonoStereoMode= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdGetMonoStereoMode",
            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdGetMonoStereoMode);



    _sMethodId_nativeCb_fmTxCmdSetPowerLevel= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdSetPowerLevel",
            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdSetPowerLevel);



    _sMethodId_nativeCb_fmTxCmdGetPowerLevel= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdGetPowerLevel",
            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdGetPowerLevel);


    _sMethodId_nativeCb_fmTxCmdSetMuteMode= env->GetStaticMethodID(clazz,
                                            "nativeCb_fmTxCmdSetMuteMode",
                                            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdSetMuteMode);


    _sMethodId_nativeCb_fmTxCmdGetMuteMode= env->GetStaticMethodID(clazz,
                                            "nativeCb_fmTxCmdGetMuteMode",
                                            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdGetMuteMode);


    _sMethodId_nativeCb_fmTxCmdSetRdsAfCode= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdSetRdsAfCode",
            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdSetRdsAfCode);



    _sMethodId_nativeCb_fmTxCmdGetRdsAfCode= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdGetRdsAfCode",
            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdGetRdsAfCode);



    _sMethodId_nativeCb_fmTxCmdSetRdsPiCode= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdSetRdsPiCode",
            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdSetRdsPiCode);


    _sMethodId_nativeCb_fmTxCmdGetRdsPiCode= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdGetRdsPiCode",
            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdGetRdsPiCode);


    _sMethodId_nativeCb_fmTxCmdSetRdsPtyCode= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdSetRdsPtyCode",
            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdSetRdsPtyCode);


    _sMethodId_nativeCb_fmTxCmdGetRdsPtyCode= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdGetRdsPtyCode",
            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdGetRdsPtyCode);



    _sMethodId_nativeCb_fmTxCmdSetRdsPsDispalyMode= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdSetRdsPsDispalyMode",
            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdSetRdsPsDispalyMode);



    _sMethodId_nativeCb_fmTxCmdGetRdsPsDispalyMode= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdGetRdsPsDispalyMode",
            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdGetRdsPsDispalyMode);



    _sMethodId_nativeCb_fmTxCmdSetRdsPsDisplaySpeed= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdSetRdsPsDisplaySpeed",
            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdSetRdsPsDisplaySpeed);


    _sMethodId_nativeCb_fmTxCmdGetRdsPsDisplaySpeed= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdGetRdsPsDisplaySpeed",
            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdGetRdsPsDisplaySpeed);


    _sMethodId_nativeCb_fmTxCmdSetRdsTransmittedMask= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdSetRdsTransmittedMask",
            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdSetRdsTransmittedMask);


    _sMethodId_nativeCb_fmTxCmdGetRdsTransmittedMask= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdGetRdsTransmittedMask",
            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdGetRdsTransmittedMask);


    _sMethodId_nativeCb_fmTxCmdSetRdsMusicSpeechFlag = env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdSetRdsMusicSpeechFlag",
            "(JIJ)V") ;
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdSetRdsMusicSpeechFlag);


    _sMethodId_nativeCb_fmTxCmdGetRdsMusicSpeechFlag = env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdGetRdsMusicSpeechFlag",
            "(JIJ)V") ;
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdGetRdsMusicSpeechFlag);


    _sMethodId_nativeCb_fmTxCmdSetPreEmphasisFilter= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdSetPreEmphasisFilter",
            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdSetPreEmphasisFilter);


    _sMethodId_nativeCb_fmTxCmdGetPreEmphasisFilter= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdGetPreEmphasisFilter",
            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdGetPreEmphasisFilter);



    _sMethodId_nativeCb_fmTxCmdSetRdsExtendedCountryCode= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdSetRdsExtendedCountryCode",
            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdSetRdsExtendedCountryCode);


    _sMethodId_nativeCb_fmTxCmdGetRdsExtendedCountryCode= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdGetRdsExtendedCountryCode",
            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdGetRdsExtendedCountryCode);


    _sMethodId_nativeCb_fmTxCmdChangeDigitalAudioConfiguration= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdChangeDigitalAudioConfiguration",
            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdChangeDigitalAudioConfiguration);


    _sMethodId_nativeCb_fmTxCmdChangeAudioSource= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdChangeAudioSource",
            "(JILcom/ti/jfm/core/JFmCcmVacUnavailResourceList;)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdChangeAudioSource);

    _sMethodId_nativeCb_fmTxCmdSetRdsTextRepertoire= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdSetRdsTextRepertoire",
            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdSetRdsTextRepertoire);


    _sMethodId_nativeCb_fmTxCmdGetRdsTextRepertoire= env->GetStaticMethodID(clazz,
            "nativeCb_fmTxCmdGetRdsTextRepertoire",
            "(JIJ)V");
    VERIFY_METHOD_ID(_sMethodId_nativeCb_fmTxCmdGetRdsTextRepertoire);

    MCP_JBTL_LOGD("nativeJFmTx_ClassInitNative:Exiting");
}

  JNINativeMethod JFmTxNative_sMethods[] = {
    /* name, signature, funcPtr */
    {"nativeJFmTx_ClassInitNative", "()V", (void*)nativeJFmTx_ClassInitNative},
    {"nativeJFmTx_Create","(Lcom/ti/jfm/core/JFmContext;)I", (void*)nativeJFmTx_Create},
    {"nativeJFmTx_Destroy","(J)I", (void*)nativeJFmTx_Destroy},
    {"nativeJFmTx_Enable","(J)I", (void*)nativeJFmTx_Enable},
    {"nativeJFmTx_Disable","(J)I", (void*)nativeJFmTx_Disable},
    {"nativeJFmTx_Tune","(JJ)I", (void*)nativeJFmTx_Tune},
    {"nativeJFmTx_StopTransmission","(J)I", (void*)nativeJFmTx_StopTransmission},
    {"nativeJFmTx_StartTransmission","(J)I", (void*)nativeJFmTx_StartTransmission},
    {"nativeJFmTx_EnableRds","(J)I", (void*)nativeJFmTx_EnableRds},
    {"nativeJFmTx_DisableRds","(J)I", (void*)nativeJFmTx_DisableRds},
    {"nativeJFmTx_SetRdsTransmissionMode","(JI)I", (void*)nativeJFmTx_SetRdsTransmissionMode},
    {"nativeJFmTx_SetRdsTextPsMsg","(JLjava/lang/String;I)I", (void*)nativeJFmTx_SetRdsTextPsMsg},
    {"nativeJFmTx_GetRdsTextPsMsg","(J)I", (void*)nativeJFmTx_GetRdsTextPsMsg},
    {"nativeJFmTx_WriteRdsRawData","(JLjava/lang/String;I)I", (void*)nativeJFmTx_WriteRdsRawData},
    {"nativeJFmTx_SetMuteMode","(JI)I", (void*)nativeJFmTx_SetMuteMode},
    {"nativeJFmTx_GetMuteMode","(J)I", (void*)nativeJFmTx_GetMuteMode},
    {"nativeJFmTx_SetRdsPsDisplayMode","(JI)I", (void*)nativeJFmTx_SetRdsPsDisplayMode},
    {"nativeJFmTx_GetRdsPsDisplayMode","(J)I", (void*)nativeJFmTx_GetRdsPsDisplayMode},
    {"nativeJFmTx_SetRdsTextRtMsg","(JILjava/lang/String;I)I", (void*)nativeJFmTx_SetRdsTextRtMsg},
    {"nativeJFmTx_GetRdsTextRtMsg","(J)I", (void*)nativeJFmTx_GetRdsTextRtMsg},
    {"nativeJFmTx_SetRdsTransmittedGroupsMask","(JJ)I", (void*)nativeJFmTx_SetRdsTransmittedGroupsMask},
    {"nativeJFmTx_GetRdsTransmittedGroupsMask","(J)I", (void*)nativeJFmTx_GetRdsTransmittedGroupsMask},
    {"nativeJFmTx_SetRdsTrafficCodes","(JII)I", (void*)nativeJFmTx_SetRdsTrafficCodes},
    {"nativeJFmTx_GetRdsTrafficCodes","(J)I", (void*)nativeJFmTx_GetRdsTrafficCodes},
    {"nativeJFmTx_SetRdsMusicSpeechFlag","(JI)I", (void*)nativeJFmTx_SetRdsMusicSpeechFlag},
    {"nativeJFmTx_GetRdsMusicSpeechFlag","(J)I", (void*)nativeJFmTx_GetRdsMusicSpeechFlag},
    {"nativeJFmTx_SetRdsExtendedCountryCode","(JI)I", (void*)nativeJFmTx_SetRdsExtendedCountryCode},
    {"nativeJFmTx_GetRdsExtendedCountryCode","(J)I", (void*)nativeJFmTx_GetRdsExtendedCountryCode},
    {"nativeJFmTx_ReadRdsRawData","(J)I", (void*)nativeJFmTx_ReadRdsRawData},
    {"nativeJFmTx_ChangeAudioSource","(JII)I", (void*)nativeJFmTx_ChangeAudioSource},
    {"nativeJFmTx_ChangeDigitalSourceConfiguration","(JI)I", (void*)nativeJFmTx_ChangeDigitalSourceConfiguration},
    {"nativeJFmTx_SetRdsTextRepertoire","(JI)I", (void*)nativeJFmTx_SetRdsTextRepertoire},
    {"nativeJFmTx_GetRdsTextRepertoire","(J)I", (void*)nativeJFmTx_GetRdsTextRepertoire},
    {"nativeJFmTx_SetRdsPtyCode","(JI)I", (void*)nativeJFmTx_SetRdsPtyCode},
    {"nativeJFmTx_GetRdsPtyCode","(J)I", (void*)nativeJFmTx_GetRdsPtyCode},
    {"nativeJFmTx_SetRdsPiCode","(JI)I", (void*)nativeJFmTx_SetRdsPiCode},
    {"nativeJFmTx_GetRdsPiCode","(J)I", (void*)nativeJFmTx_GetRdsPiCode},
    {"nativeJFmTx_SetRdsAfCode","(JI)I", (void*)nativeJFmTx_SetRdsAfCode},
    {"nativeJFmTx_GetRdsAfCode","(J)I", (void*)nativeJFmTx_GetRdsAfCode},
    {"nativeJFmTx_SetMonoStereoMode","(JI)I", (void*)nativeJFmTx_SetMonoStereoMode},
    {"nativeJFmTx_GetMonoStereoMode","(J)I", (void*)nativeJFmTx_GetMonoStereoMode},
    {"nativeJFmTx_SetPowerLevel","(JI)I", (void*)nativeJFmTx_SetPowerLevel},
    {"nativeJFmTx_GetPowerLevel","(J)I", (void*)nativeJFmTx_GetPowerLevel},
    {"nativeJFmTx_SetPreEmphasisFilter","(JI)I", (void*)nativeJFmTx_SetPreEmphasisFilter},
    {"nativeJFmTx_GetPreEmphasisFilter","(J)I", (void*)nativeJFmTx_GetPreEmphasisFilter},
    {"nativeJFmTx_SetRdsPsScrollSpeed","(JI)I", (void*)nativeJFmTx_SetRdsPsScrollSpeed},
    {"nativeJFmTx_GetRdsPsScrollSpeed","(J)I", (void*)nativeJFmTx_GetRdsPsScrollSpeed}

};

/*
 * Register several native methods for one class.
 */

int getTxNativeSize()
{
	return NELEM(JFmTxNative_sMethods);
}

