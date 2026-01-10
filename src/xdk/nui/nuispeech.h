#pragma once
#include "../win_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _NUI_SPEECH_LANGUAGE {
    NUI_SPEECH_LANGUAGE_EN_US = 0x0000,
    NUI_SPEECH_LANGUAGE_FR_CA = 0x0001,
    NUI_SPEECH_LANGUAGE_EN_GB = 0x0002,
    NUI_SPEECH_LANGUAGE_ES_MX = 0x0003,
    NUI_SPEECH_LANGUAGE_JA_JP = 0x0004,
    NUI_SPEECH_LANGUAGE_FR_FR = 0x0005,
    NUI_SPEECH_LANGUAGE_ES_ES = 0x0006,
    NUI_SPEECH_LANGUAGE_DE_DE = 0x0007,
    NUI_SPEECH_LANGUAGE_IT_IT = 0x0008,
} NUI_SPEECH_LANGUAGE;

enum NUI_SPEECH_MICTYPE {
    NUI_SPEECH_KINECT = 0x0000,
    NUI_SPEECH_WAVE = 0x0001,
};

typedef enum _NUI_SPEECH_LOADOPTIONS {
    NUI_SPEECH_LOADOPTIONS_STATIC = 0x0000,
    NUI_SPEECH_LOADOPTIONS_DYNAMIC = 0x0001,
} NUI_SPEECH_LOADOPTIONS;

typedef struct _NUI_SPEECH_INIT_PROPERTIES { /* Size=0x8 */
    /* 0x0000 */ NUI_SPEECH_LANGUAGE Language;
    /* 0x0004 */ NUI_SPEECH_MICTYPE MicrophoneType;
} NUI_SPEECH_INIT_PROPERTIES;

typedef struct _NUI_SPEECH_GRAMMAR { /* Size=0x8 */
    /* 0x0000 */ ULONG ulId;
    /* 0x0004 */ HANDLE hGrammar;
} NUI_SPEECH_GRAMMAR;

HRESULT NuiSpeechCreateGrammar(ULONG ulGrammarId, NUI_SPEECH_GRAMMAR *pGrammar);
HRESULT NuiSpeechLoadGrammar(
    LPCWSTR pcwszGrammarName,
    ULONG ulGrammarId,
    NUI_SPEECH_LOADOPTIONS Options,
    NUI_SPEECH_GRAMMAR *pGrammar
);
HRESULT NuiSpeechUnloadGrammar(NUI_SPEECH_GRAMMAR *pGrammar);
HRESULT
NuiSpeechEnable(NUI_SPEECH_INIT_PROPERTIES *pInitProperties, DWORD dwHardwareThread);
HRESULT NuiSpeechDisable();
HRESULT NuiSpeechSetEventInterest(ULONG ulEvents);
HRESULT NuiSpeechStartRecognition();
HRESULT NuiSpeechStopRecognition();

#ifdef __cplusplus
}
#endif
