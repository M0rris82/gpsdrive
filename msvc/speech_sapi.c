

#define CINTERFACE
#define COBJMACROS

#include <windows.h>
#include <sapi.h>
#include <speech.h>

#include <stdio.h>
#include <glib.h>


GUID  sapi4_clsid_ttsenum = {0xd67c0280, 0xc743, 0x11cd, 0x80, 0xe5, 0x00, 0xaa, 0x00, 0x3e, 0x4b, 0x50};


void * speech_sapi4_modeget(void)
{
    HRESULT hr;
    ITTSCentral * sapi4_ttscentral = NULL;
    
    GUID sapi4_iid_ttscentral = {0x28016060L, 0x4A47, 0x101B, 0x93, 0x1A, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F};


    hr = CoInitialize(NULL);
    if(FAILED(hr))
        return NULL;

    hr = CoCreateInstance(&sapi4_clsid_ttsenum, NULL, CLSCTX_ALL, &sapi4_iid_ttscentral, (void **)&sapi4_ttscentral);
    if( FAILED(hr) )
    {
        CoUninitialize();
        return NULL;
    }

    return NULL;
}


void * speech_sapi4_enum(void)
{
    HRESULT hr;
    ITTSEnum * sapi4_ttsenum = NULL;
    TTSMODEINFO tts_info[100];
    int ulFound = 0;

    GUID    sapi4_iid_ttsenum = {0x6B837B20, 0x4A47, 0x101B, 0x93, 0x1A, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F};

    hr = CoInitialize(NULL);
    if(FAILED(hr))
        return NULL;

    hr = CoCreateInstance(&sapi4_clsid_ttsenum, NULL, CLSCTX_ALL, &sapi4_iid_ttsenum, (void **)&sapi4_ttsenum);
    if( FAILED(hr) )
    {
        CoUninitialize();
        return NULL;
    }

    sapi4_ttsenum->lpVtbl->Reset(sapi4_ttsenum);

    hr = sapi4_ttsenum->lpVtbl->Next(sapi4_ttsenum, 100, tts_info, &ulFound);
    if(SUCCEEDED(hr)) {
        while (ulFound--) {
        wprintf(L"lang:%u speaker:%s mode:%s\n", 
            /*tts_info[ulFound].szProductName,*/ tts_info[ulFound].language.LanguageID, 
            tts_info[ulFound].szSpeaker, tts_info[ulFound].szModeName);
        }
    }
    
    return NULL;
}

void *
speech_sapi4_open(void)
{
    HRESULT hr;
    IVoiceText * sapi4_pVoice = NULL;
    
    GUID     sapi4_clsid_vtxt = {0xf1dc95a0, 0xba7, 0x11ce, 0xa1, 0x66, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c};
    GUID sapi4_iid_ivoicetext = {0xc4fe8740, 0xe093, 0x11cd, 0xa1, 0x66, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c};


    hr = CoInitialize(NULL);
    if(FAILED(hr))
        return NULL;

    hr = CoCreateInstance(&sapi4_clsid_vtxt, NULL, CLSCTX_ALL, &sapi4_iid_ivoicetext, (void **)&sapi4_pVoice);
    if( FAILED(hr) )
    {
        CoUninitialize();
        return NULL;
    }

    /* XXX - find a way to get current application name - instead of this hardcoded value! */
    sapi4_pVoice->lpVtbl->Register(sapi4_pVoice, NULL, L"gpsdrive.exe" , NULL, GUID_NULL, 0, NULL);

    return sapi4_pVoice;
}


void
speech_sapi4_close(IVoiceText *ifp)
{
    CoUninitialize();
}


int
speech_sapi4_out(IVoiceText * ifp, char* text)
{
    gunichar2* utf16_text;
    IVoiceText * sapi4_pVoice = ifp;
    HRESULT hr;

    printf("speech_sapi4_out: %s\n", text);

    /* XXX - problem with german umlaut (ä,ö,ü,ß) and probably other such non standard chars
     * currently, g_utf8_to_utf16() refuses to convert such a text.
     * if using g_locale_to_utf8() before g_utf8_to_utf16(), SAPI4 simply crashes! */
    utf16_text = g_utf8_to_utf16(text, -1 /* zero terminated */,
                    NULL, NULL, NULL);
    if(utf16_text != NULL) {
        hr = sapi4_pVoice->lpVtbl->Speak(sapi4_pVoice, utf16_text, 0, NULL);
        g_free(utf16_text);
        return SUCCEEDED(hr);
    }
    printf("speech_sapi4_out: failed to convert %s to utf16!\n", text);
    return FALSE;
}


void *
speech_sapi5_open(void)
{
    HRESULT hr;
    ISpVoice * sapi5_pVoice = NULL;

    GUID clsid_spvoice = {0x96749377, 0x3391, 0x11D2, 0x9E, 0xE3, 0x00, 0xC0, 0x4F, 0x79, 0x73, 0x96};
    GUID  iid_ispvoice = {0x6C44DF74, 0x72B9, 0x4992, 0xA1, 0xEC, 0xEF, 0x99, 0x6E, 0x04, 0x22, 0xD4};


    hr = CoInitialize(NULL);
    if (FAILED(hr))
        return NULL;

    hr = CoCreateInstance(&clsid_spvoice, NULL, CLSCTX_ALL, &iid_ispvoice, (void **)&sapi5_pVoice);
    if( FAILED( hr ) )
        return NULL;

    return sapi5_pVoice;
}


void
speech_sapi5_close(void * ifp)
{
    ISpVoice * sapi5_pVoice = ifp;


    ISpVoice_Release(sapi5_pVoice);
    CoUninitialize();
}


int
speech_sapi5_out(void *ifp, char* text)
{
    HRESULT hr;
    ISpVoice * pVoice = ifp;
    gunichar2* utf16_text;


    printf("speech_out_sapi5: %s\n", text);

    utf16_text = g_utf8_to_utf16(text, -1 /* zero terminated */,
                    NULL, NULL, NULL);
    if(utf16_text != NULL) {
        hr = ISpVoice_Speak(pVoice,utf16_text, 0, NULL);
        g_free(utf16_text);
        return SUCCEEDED(hr);
    }

    printf("speech_sapi5_out: failed to convert %s to utf16!\n", text);
    return FALSE;
}


int speech_sapi_out(char *text)
{
    void * sapi4_ifp;
//    void * sapi5_ifp;

    sapi4_ifp = speech_sapi4_open();
    if(sapi4_ifp != NULL) {
        speech_sapi4_out(sapi4_ifp, text);
        speech_sapi4_close(sapi4_ifp);
    }

#if 0
    sapi5_ifp = speech_sapi5_open();
    if(sapi5_ifp != NULL) {
        speech_sapi5_out(sapi5_ifp, text);
        speech_sapi5_close(sapi5_ifp);
    }
#endif

    //speech_sapi4_enum();

//    speech_sapi4_modeget();

    return TRUE;
}


#define CINTERFACE
#define COBJMACROS

#include <windows.h>
#include <sapi.h>
#include <speech.h>

#include <stdio.h>
#include <glib.h>


GUID  sapi4_clsid_ttsenum = {0xd67c0280, 0xc743, 0x11cd, 0x80, 0xe5, 0x00, 0xaa, 0x00, 0x3e, 0x4b, 0x50};


void * speech_sapi4_modeget(void)
{
    HRESULT hr;
    ITTSCentral * sapi4_ttscentral = NULL;
    
    GUID sapi4_iid_ttscentral = {0x28016060L, 0x4A47, 0x101B, 0x93, 0x1A, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F};


    hr = CoInitialize(NULL);
    if(FAILED(hr))
        return NULL;

    hr = CoCreateInstance(&sapi4_clsid_ttsenum, NULL, CLSCTX_ALL, &sapi4_iid_ttscentral, (void **)&sapi4_ttscentral);
    if( FAILED(hr) )
    {
        CoUninitialize();
        return NULL;
    }

    return NULL;
}


void * speech_sapi4_enum(void)
{
    HRESULT hr;
    ITTSEnum * sapi4_ttsenum = NULL;
    TTSMODEINFO tts_info[100];
    int ulFound = 0;

    GUID    sapi4_iid_ttsenum = {0x6B837B20, 0x4A47, 0x101B, 0x93, 0x1A, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F};

    hr = CoInitialize(NULL);
    if(FAILED(hr))
        return NULL;

    hr = CoCreateInstance(&sapi4_clsid_ttsenum, NULL, CLSCTX_ALL, &sapi4_iid_ttsenum, (void **)&sapi4_ttsenum);
    if( FAILED(hr) )
    {
        CoUninitialize();
        return NULL;
    }

    sapi4_ttsenum->lpVtbl->Reset(sapi4_ttsenum);

    hr = sapi4_ttsenum->lpVtbl->Next(sapi4_ttsenum, 100, tts_info, &ulFound);
    if(SUCCEEDED(hr)) {
        while (ulFound--) {
        wprintf(L"lang:%u speaker:%s mode:%s\n", 
            /*tts_info[ulFound].szProductName,*/ tts_info[ulFound].language.LanguageID, 
            tts_info[ulFound].szSpeaker, tts_info[ulFound].szModeName);
        }
    }
    
    return NULL;
}

void *
speech_sapi4_open(void)
{
    HRESULT hr;
    IVoiceText * sapi4_pVoice = NULL;
    
    GUID     sapi4_clsid_vtxt = {0xf1dc95a0, 0xba7, 0x11ce, 0xa1, 0x66, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c};
    GUID sapi4_iid_ivoicetext = {0xc4fe8740, 0xe093, 0x11cd, 0xa1, 0x66, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c};


    hr = CoInitialize(NULL);
    if(FAILED(hr))
        return NULL;

    hr = CoCreateInstance(&sapi4_clsid_vtxt, NULL, CLSCTX_ALL, &sapi4_iid_ivoicetext, (void **)&sapi4_pVoice);
    if( FAILED(hr) )
    {
        CoUninitialize();
        return NULL;
    }

    /* XXX - find a way to get current application name - instead of this hardcoded value! */
    sapi4_pVoice->lpVtbl->Register(sapi4_pVoice, NULL, L"gpsdrive.exe" , NULL, GUID_NULL, 0, NULL);

    return sapi4_pVoice;
}


void
speech_sapi4_close(IVoiceText *ifp)
{
    CoUninitialize();
}


int
speech_sapi4_out(IVoiceText * ifp, char* text)
{
    gunichar2* utf16_text;
    IVoiceText * sapi4_pVoice = ifp;
    HRESULT hr;

    printf("speech_sapi4_out: %s\n", text);

    /* XXX - problem with german umlaut (ä,ö,ü,ß) and probably other such non standard chars
     * currently, g_utf8_to_utf16() refuses to convert such a text.
     * if using g_locale_to_utf8() before g_utf8_to_utf16(), SAPI4 simply crashes! */
    utf16_text = g_utf8_to_utf16(text, -1 /* zero terminated */,
                    NULL, NULL, NULL);
    if(utf16_text != NULL) {
        hr = sapi4_pVoice->lpVtbl->Speak(sapi4_pVoice, utf16_text, 0, NULL);
        g_free(utf16_text);
        return SUCCEEDED(hr);
    }
    printf("speech_sapi4_out: failed to convert %s to utf16!\n", text);
    return FALSE;
}


void *
speech_sapi5_open(void)
{
    HRESULT hr;
    ISpVoice * sapi5_pVoice = NULL;

    GUID clsid_spvoice = {0x96749377, 0x3391, 0x11D2, 0x9E, 0xE3, 0x00, 0xC0, 0x4F, 0x79, 0x73, 0x96};
    GUID  iid_ispvoice = {0x6C44DF74, 0x72B9, 0x4992, 0xA1, 0xEC, 0xEF, 0x99, 0x6E, 0x04, 0x22, 0xD4};


    hr = CoInitialize(NULL);
    if (FAILED(hr))
        return NULL;

    hr = CoCreateInstance(&clsid_spvoice, NULL, CLSCTX_ALL, &iid_ispvoice, (void **)&sapi5_pVoice);
    if( FAILED( hr ) )
        return NULL;

    return sapi5_pVoice;
}


void
speech_sapi5_close(void * ifp)
{
    ISpVoice * sapi5_pVoice = ifp;


    ISpVoice_Release(sapi5_pVoice);
    CoUninitialize();
}


int
speech_sapi5_out(void *ifp, char* text)
{
    HRESULT hr;
    ISpVoice * pVoice = ifp;
    gunichar2* utf16_text;


    printf("speech_out_sapi5: %s\n", text);

    utf16_text = g_utf8_to_utf16(text, -1 /* zero terminated */,
                    NULL, NULL, NULL);
    if(utf16_text != NULL) {
        hr = ISpVoice_Speak(pVoice,utf16_text, 0, NULL);
        g_free(utf16_text);
        return SUCCEEDED(hr);
    }

    printf("speech_sapi5_out: failed to convert %s to utf16!\n", text);
    return FALSE;
}


int speech_sapi_out(char *text)
{
    void * sapi4_ifp;
//    void * sapi5_ifp;

    sapi4_ifp = speech_sapi4_open();
    if(sapi4_ifp != NULL) {
        speech_sapi4_out(sapi4_ifp, text);
        speech_sapi4_close(sapi4_ifp);
    }

#if 0
    sapi5_ifp = speech_sapi5_open();
    if(sapi5_ifp != NULL) {
        speech_sapi5_out(sapi5_ifp, text);
        speech_sapi5_close(sapi5_ifp);
    }
#endif

    //speech_sapi4_enum();

//    speech_sapi4_modeget();

    return TRUE;
}


#define CINTERFACE
#define COBJMACROS

#include <windows.h>
#include <sapi.h>
#include <speech.h>

#include <stdio.h>
#include <glib.h>


GUID  sapi4_clsid_ttsenum = {0xd67c0280, 0xc743, 0x11cd, 0x80, 0xe5, 0x00, 0xaa, 0x00, 0x3e, 0x4b, 0x50};


void * speech_sapi4_modeget(void)
{
    HRESULT hr;
    ITTSCentral * sapi4_ttscentral = NULL;
    
    GUID sapi4_iid_ttscentral = {0x28016060L, 0x4A47, 0x101B, 0x93, 0x1A, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F};


    hr = CoInitialize(NULL);
    if(FAILED(hr))
        return NULL;

    hr = CoCreateInstance(&sapi4_clsid_ttsenum, NULL, CLSCTX_ALL, &sapi4_iid_ttscentral, (void **)&sapi4_ttscentral);
    if( FAILED(hr) )
    {
        CoUninitialize();
        return NULL;
    }

    return NULL;
}


void * speech_sapi4_enum(void)
{
    HRESULT hr;
    ITTSEnum * sapi4_ttsenum = NULL;
    TTSMODEINFO tts_info[100];
    int ulFound = 0;

    GUID    sapi4_iid_ttsenum = {0x6B837B20, 0x4A47, 0x101B, 0x93, 0x1A, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F};

    hr = CoInitialize(NULL);
    if(FAILED(hr))
        return NULL;

    hr = CoCreateInstance(&sapi4_clsid_ttsenum, NULL, CLSCTX_ALL, &sapi4_iid_ttsenum, (void **)&sapi4_ttsenum);
    if( FAILED(hr) )
    {
        CoUninitialize();
        return NULL;
    }

    sapi4_ttsenum->lpVtbl->Reset(sapi4_ttsenum);

    hr = sapi4_ttsenum->lpVtbl->Next(sapi4_ttsenum, 100, tts_info, &ulFound);
    if(SUCCEEDED(hr)) {
        while (ulFound--) {
        wprintf(L"lang:%u speaker:%s mode:%s\n", 
            /*tts_info[ulFound].szProductName,*/ tts_info[ulFound].language.LanguageID, 
            tts_info[ulFound].szSpeaker, tts_info[ulFound].szModeName);
        }
    }
    
    return NULL;
}

void *
speech_sapi4_open(void)
{
    HRESULT hr;
    IVoiceText * sapi4_pVoice = NULL;
    
    GUID     sapi4_clsid_vtxt = {0xf1dc95a0, 0xba7, 0x11ce, 0xa1, 0x66, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c};
    GUID sapi4_iid_ivoicetext = {0xc4fe8740, 0xe093, 0x11cd, 0xa1, 0x66, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c};


    hr = CoInitialize(NULL);
    if(FAILED(hr))
        return NULL;

    hr = CoCreateInstance(&sapi4_clsid_vtxt, NULL, CLSCTX_ALL, &sapi4_iid_ivoicetext, (void **)&sapi4_pVoice);
    if( FAILED(hr) )
    {
        CoUninitialize();
        return NULL;
    }

    /* XXX - find a way to get current application name - instead of this hardcoded value! */
    sapi4_pVoice->lpVtbl->Register(sapi4_pVoice, NULL, L"gpsdrive.exe" , NULL, GUID_NULL, 0, NULL);

    return sapi4_pVoice;
}


void
speech_sapi4_close(IVoiceText *ifp)
{
    CoUninitialize();
}


int
speech_sapi4_out(IVoiceText * ifp, char* text)
{
    gunichar2* utf16_text;
    IVoiceText * sapi4_pVoice = ifp;
    HRESULT hr;

    printf("speech_sapi4_out: %s\n", text);

    /* XXX - problem with german umlaut (ä,ö,ü,ß) and probably other such non standard chars
     * currently, g_utf8_to_utf16() refuses to convert such a text.
     * if using g_locale_to_utf8() before g_utf8_to_utf16(), SAPI4 simply crashes! */
    utf16_text = g_utf8_to_utf16(text, -1 /* zero terminated */,
                    NULL, NULL, NULL);
    if(utf16_text != NULL) {
        hr = sapi4_pVoice->lpVtbl->Speak(sapi4_pVoice, utf16_text, 0, NULL);
        g_free(utf16_text);
        return SUCCEEDED(hr);
    }
    printf("speech_sapi4_out: failed to convert %s to utf16!\n", text);
    return FALSE;
}


void *
speech_sapi5_open(void)
{
    HRESULT hr;
    ISpVoice * sapi5_pVoice = NULL;

    GUID clsid_spvoice = {0x96749377, 0x3391, 0x11D2, 0x9E, 0xE3, 0x00, 0xC0, 0x4F, 0x79, 0x73, 0x96};
    GUID  iid_ispvoice = {0x6C44DF74, 0x72B9, 0x4992, 0xA1, 0xEC, 0xEF, 0x99, 0x6E, 0x04, 0x22, 0xD4};


    hr = CoInitialize(NULL);
    if (FAILED(hr))
        return NULL;

    hr = CoCreateInstance(&clsid_spvoice, NULL, CLSCTX_ALL, &iid_ispvoice, (void **)&sapi5_pVoice);
    if( FAILED( hr ) )
        return NULL;

    return sapi5_pVoice;
}


void
speech_sapi5_close(void * ifp)
{
    ISpVoice * sapi5_pVoice = ifp;


    ISpVoice_Release(sapi5_pVoice);
    CoUninitialize();
}


int
speech_sapi5_out(void *ifp, char* text)
{
    HRESULT hr;
    ISpVoice * pVoice = ifp;
    gunichar2* utf16_text;


    printf("speech_out_sapi5: %s\n", text);

    utf16_text = g_utf8_to_utf16(text, -1 /* zero terminated */,
                    NULL, NULL, NULL);
    if(utf16_text != NULL) {
        hr = ISpVoice_Speak(pVoice,utf16_text, 0, NULL);
        g_free(utf16_text);
        return SUCCEEDED(hr);
    }

    printf("speech_sapi5_out: failed to convert %s to utf16!\n", text);
    return FALSE;
}


int speech_sapi_out(char *text)
{
    void * sapi4_ifp;
//    void * sapi5_ifp;

    sapi4_ifp = speech_sapi4_open();
    if(sapi4_ifp != NULL) {
        speech_sapi4_out(sapi4_ifp, text);
        speech_sapi4_close(sapi4_ifp);
    }

#if 0
    sapi5_ifp = speech_sapi5_open();
    if(sapi5_ifp != NULL) {
        speech_sapi5_out(sapi5_ifp, text);
        speech_sapi5_close(sapi5_ifp);
    }
#endif

    //speech_sapi4_enum();

//    speech_sapi4_modeget();

    return TRUE;
}


#define CINTERFACE
#define COBJMACROS

#include <windows.h>
#include <sapi.h>
#include <speech.h>

#include <stdio.h>
#include <glib.h>


GUID  sapi4_clsid_ttsenum = {0xd67c0280, 0xc743, 0x11cd, 0x80, 0xe5, 0x00, 0xaa, 0x00, 0x3e, 0x4b, 0x50};


void * speech_sapi4_modeget(void)
{
    HRESULT hr;
    ITTSCentral * sapi4_ttscentral = NULL;
    
    GUID sapi4_iid_ttscentral = {0x28016060L, 0x4A47, 0x101B, 0x93, 0x1A, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F};


    hr = CoInitialize(NULL);
    if(FAILED(hr))
        return NULL;

    hr = CoCreateInstance(&sapi4_clsid_ttsenum, NULL, CLSCTX_ALL, &sapi4_iid_ttscentral, (void **)&sapi4_ttscentral);
    if( FAILED(hr) )
    {
        CoUninitialize();
        return NULL;
    }

    return NULL;
}


void * speech_sapi4_enum(void)
{
    HRESULT hr;
    ITTSEnum * sapi4_ttsenum = NULL;
    TTSMODEINFO tts_info[100];
    int ulFound = 0;

    GUID    sapi4_iid_ttsenum = {0x6B837B20, 0x4A47, 0x101B, 0x93, 0x1A, 0x00, 0xAA, 0x00, 0x47, 0xBA, 0x4F};

    hr = CoInitialize(NULL);
    if(FAILED(hr))
        return NULL;

    hr = CoCreateInstance(&sapi4_clsid_ttsenum, NULL, CLSCTX_ALL, &sapi4_iid_ttsenum, (void **)&sapi4_ttsenum);
    if( FAILED(hr) )
    {
        CoUninitialize();
        return NULL;
    }

    sapi4_ttsenum->lpVtbl->Reset(sapi4_ttsenum);

    hr = sapi4_ttsenum->lpVtbl->Next(sapi4_ttsenum, 100, tts_info, &ulFound);
    if(SUCCEEDED(hr)) {
        while (ulFound--) {
        wprintf(L"lang:%u speaker:%s mode:%s\n", 
            /*tts_info[ulFound].szProductName,*/ tts_info[ulFound].language.LanguageID, 
            tts_info[ulFound].szSpeaker, tts_info[ulFound].szModeName);
        }
    }
    
    return NULL;
}

void *
speech_sapi4_open(void)
{
    HRESULT hr;
    IVoiceText * sapi4_pVoice = NULL;
    
    GUID     sapi4_clsid_vtxt = {0xf1dc95a0, 0xba7, 0x11ce, 0xa1, 0x66, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c};
    GUID sapi4_iid_ivoicetext = {0xc4fe8740, 0xe093, 0x11cd, 0xa1, 0x66, 0x0, 0xaa, 0x0, 0x4c, 0xd6, 0x5c};


    hr = CoInitialize(NULL);
    if(FAILED(hr))
        return NULL;

    hr = CoCreateInstance(&sapi4_clsid_vtxt, NULL, CLSCTX_ALL, &sapi4_iid_ivoicetext, (void **)&sapi4_pVoice);
    if( FAILED(hr) )
    {
        CoUninitialize();
        return NULL;
    }

    /* XXX - find a way to get current application name - instead of this hardcoded value! */
    sapi4_pVoice->lpVtbl->Register(sapi4_pVoice, NULL, L"gpsdrive.exe" , NULL, GUID_NULL, 0, NULL);

    return sapi4_pVoice;
}


void
speech_sapi4_close(IVoiceText *ifp)
{
    CoUninitialize();
}


int
speech_sapi4_out(IVoiceText * ifp, char* text)
{
    gunichar2* utf16_text;
    IVoiceText * sapi4_pVoice = ifp;
    HRESULT hr;

    printf("speech_sapi4_out: %s\n", text);

    /* XXX - problem with german umlaut (ä,ö,ü,ß) and probably other such non standard chars
     * currently, g_utf8_to_utf16() refuses to convert such a text.
     * if using g_locale_to_utf8() before g_utf8_to_utf16(), SAPI4 simply crashes! */
    utf16_text = g_utf8_to_utf16(text, -1 /* zero terminated */,
                    NULL, NULL, NULL);
    if(utf16_text != NULL) {
        hr = sapi4_pVoice->lpVtbl->Speak(sapi4_pVoice, utf16_text, 0, NULL);
        g_free(utf16_text);
        return SUCCEEDED(hr);
    }
    printf("speech_sapi4_out: failed to convert %s to utf16!\n", text);
    return FALSE;
}


void *
speech_sapi5_open(void)
{
    HRESULT hr;
    ISpVoice * sapi5_pVoice = NULL;

    GUID clsid_spvoice = {0x96749377, 0x3391, 0x11D2, 0x9E, 0xE3, 0x00, 0xC0, 0x4F, 0x79, 0x73, 0x96};
    GUID  iid_ispvoice = {0x6C44DF74, 0x72B9, 0x4992, 0xA1, 0xEC, 0xEF, 0x99, 0x6E, 0x04, 0x22, 0xD4};


    hr = CoInitialize(NULL);
    if (FAILED(hr))
        return NULL;

    hr = CoCreateInstance(&clsid_spvoice, NULL, CLSCTX_ALL, &iid_ispvoice, (void **)&sapi5_pVoice);
    if( FAILED( hr ) )
        return NULL;

    return sapi5_pVoice;
}


void
speech_sapi5_close(void * ifp)
{
    ISpVoice * sapi5_pVoice = ifp;


    ISpVoice_Release(sapi5_pVoice);
    CoUninitialize();
}


int
speech_sapi5_out(void *ifp, char* text)
{
    HRESULT hr;
    ISpVoice * pVoice = ifp;
    gunichar2* utf16_text;


    printf("speech_out_sapi5: %s\n", text);

    utf16_text = g_utf8_to_utf16(text, -1 /* zero terminated */,
                    NULL, NULL, NULL);
    if(utf16_text != NULL) {
        hr = ISpVoice_Speak(pVoice,utf16_text, 0, NULL);
        g_free(utf16_text);
        return SUCCEEDED(hr);
    }

    printf("speech_sapi5_out: failed to convert %s to utf16!\n", text);
    return FALSE;
}


int speech_sapi_out(char *text)
{
    void * sapi4_ifp;
//    void * sapi5_ifp;

    sapi4_ifp = speech_sapi4_open();
    if(sapi4_ifp != NULL) {
        speech_sapi4_out(sapi4_ifp, text);
        speech_sapi4_close(sapi4_ifp);
    }

#if 0
    sapi5_ifp = speech_sapi5_open();
    if(sapi5_ifp != NULL) {
        speech_sapi5_out(sapi5_ifp, text);
        speech_sapi5_close(sapi5_ifp);
    }
#endif

    //speech_sapi4_enum();

//    speech_sapi4_modeget();

    return TRUE;
}