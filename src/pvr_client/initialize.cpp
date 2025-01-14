/*
 * Copyright (C) 2015-2020 Yuki MIZUNO
 * https://github.com/Harekaze/pvr.epgstation/
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include "epgstation/api.h"
#include "epgstation/epgstation.h"
#include "kodi/libKODI_guilib.h"
#include "kodi/libXBMC_addon.h"
#include "kodi/libXBMC_pvr.h"
#include "kodi/xbmc_pvr_dll.h"
#include <climits>
#include <cstdio>
#include <iostream>

#define MENUHOOK_FORCE_REFRESH_RECORDING 0x01
#define MENUHOOK_FORCE_REFRESH_TIMER 0x02
#define MENUHOOK_FORCE_EXECUTE_SCHEDULER 0x04

#define MSG_FORCE_REFRESH_RECORDING 30800
#define MSG_FORCE_REFRESH_TIMER 30801
#define MSG_FORCE_EXECUTE_SCHEDULER 30802

epgstation::Channels g_channels;
epgstation::Schedule g_schedule;
epgstation::Recorded g_recorded;
epgstation::Rule g_rule;
epgstation::Reserve g_reserve;
ADDON::CHelper_libXBMC_addon* XBMC = nullptr;
CHelper_libXBMC_pvr* PVR = nullptr;
CHelper_libKODI_guilib* GUI = nullptr;
time_t lastStartTime;

ADDON_STATUS currentStatus = ADDON_STATUS_UNKNOWN;

extern "C" {

ADDON_STATUS ADDON_Create(void* callbacks, void* props)
{
    if (!callbacks || !props) {
        return ADDON_STATUS_UNKNOWN;
    }

    XBMC = new ADDON::CHelper_libXBMC_addon;
    PVR = new CHelper_libXBMC_pvr;
    GUI = new CHelper_libKODI_guilib;

    if (
        !XBMC->RegisterMe(callbacks)
        || !PVR->RegisterMe(callbacks)
        || !GUI->RegisterMe(callbacks)) {
        delete PVR;
        delete XBMC;
        delete GUI;
        PVR = nullptr;
        XBMC = nullptr;
        GUI = nullptr;
        return ADDON_STATUS_PERMANENT_FAILURE;
    }

    time(&lastStartTime);

    const char* strUserPath = static_cast<PVR_PROPERTIES*>(props)->strUserPath;

    if (!XBMC->DirectoryExists(strUserPath)) {
        XBMC->CreateDirectory(strUserPath);
    }

    char serverUrl[1024];
    if (XBMC->GetSetting("server_url", &serverUrl)) {
        epgstation::api::baseURL = serverUrl;
        constexpr char httpPrefix[] = "http://";
        constexpr char httpsPrefix[] = "https://";
        if (epgstation::api::baseURL.rfind(httpPrefix, 0) != 0 && epgstation::api::baseURL.rfind(httpsPrefix, 0) != 0) {
            if (currentStatus == ADDON_STATUS_UNKNOWN) {
                XBMC->QueueNotification(ADDON::QUEUE_WARNING, XBMC->GetLocalizedString(30600));
                currentStatus = ADDON_STATUS_NEED_SETTINGS;
            }
            return currentStatus;
        }
        if (*(epgstation::api::baseURL.end() - 1) != '/') {
            epgstation::api::baseURL += "/";
        }
        epgstation::api::baseURL += "api/";
    }

    g_channels.liveStreamingPath = epgstation::api::baseURL + "streams/live/%llu/m2ts";
    g_channels.channelLogoPath = epgstation::api::baseURL + "channels/%llu/logo";
    g_recorded.recordedStreamingPath = epgstation::api::baseURL + "videos/%llu";
    g_recorded.recordedThumbnailPath = epgstation::api::baseURL + "thumbnails/%llu";

    unsigned int mode;
    XBMC->GetSetting("live_transcode", &mode);
    std::string transcodeParams = "?mode=" + std::to_string(mode);

    XBMC->Log(ADDON::LOG_NOTICE, "Transcoding parameter: %s", transcodeParams.c_str());

    g_channels.liveStreamingPath += transcodeParams;

    PVR_MENUHOOK menuHookRec = {
        .iHookId = MENUHOOK_FORCE_REFRESH_RECORDING,
        .iLocalizedStringId = MSG_FORCE_REFRESH_RECORDING,
        .category = PVR_MENUHOOK_RECORDING,
    };
    PVR->AddMenuHook(&menuHookRec);

    PVR_MENUHOOK menuHookTimer = {
        .iHookId = MENUHOOK_FORCE_REFRESH_TIMER,
        .iLocalizedStringId = MSG_FORCE_REFRESH_TIMER,
        .category = PVR_MENUHOOK_TIMER,
    };
    PVR->AddMenuHook(&menuHookTimer);

    PVR_MENUHOOK menuHookScheduler = {
        .iHookId = MENUHOOK_FORCE_EXECUTE_SCHEDULER,
        .iLocalizedStringId = MSG_FORCE_EXECUTE_SCHEDULER,
        .category = PVR_MENUHOOK_EPG,
    };
    PVR->AddMenuHook(&menuHookScheduler);

    currentStatus = ADDON_STATUS_OK;

    return currentStatus;
}

ADDON_STATUS ADDON_GetStatus(void)
{
    return currentStatus;
}

void ADDON_Destroy(void)
{
    currentStatus = ADDON_STATUS_UNKNOWN;
}

// Settings configuration

ADDON_STATUS ADDON_SetSetting(const char* settingName, const void* settingValue)
{
    time_t now;
    time(&now);
    if (now - 2 < lastStartTime) {
        return currentStatus;
    }
    if (currentStatus == ADDON_STATUS_OK) {
        return ADDON_STATUS_NEED_RESTART;
    }
    return ADDON_STATUS_PERMANENT_FAILURE;
}

PVR_ERROR CallMenuHook(const PVR_MENUHOOK& menuhook, const PVR_MENUHOOK_DATA& item)
{
    switch (menuhook.iHookId) {
    case MENUHOOK_FORCE_REFRESH_RECORDING: {
        PVR->TriggerRecordingUpdate();
        return PVR_ERROR_NO_ERROR;
    }
    case MENUHOOK_FORCE_REFRESH_TIMER: {
        PVR->TriggerTimerUpdate();
        return PVR_ERROR_NO_ERROR;
    }
    case MENUHOOK_FORCE_EXECUTE_SCHEDULER: {
        if (g_schedule.update()) {
            PVR->TriggerChannelUpdate();
            return PVR_ERROR_NO_ERROR;
        }
        return PVR_ERROR_SERVER_ERROR;
    }
    }
    return PVR_ERROR_FAILED;
}

void OnSystemWake()
{
    PVR->TriggerRecordingUpdate();
    PVR->TriggerTimerUpdate();
    PVR->TriggerChannelUpdate();
}

void OnPowerSavingDeactivated()
{
    PVR->TriggerRecordingUpdate();
    PVR->TriggerTimerUpdate();
    PVR->TriggerChannelUpdate();
}

/* not implemented */
void OnSystemSleep() {}
void OnPowerSavingActivated() {}
PVR_ERROR GetStreamTimes(PVR_STREAM_TIMES* times) { return PVR_ERROR_NOT_IMPLEMENTED; }
}
