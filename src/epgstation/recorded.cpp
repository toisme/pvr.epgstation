/*
 * Copyright (C) 2015-2020 Yuki MIZUNO
 * https://github.com/Harekaze/pvr.epgstation/
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include "recorded.h"
#include "api.h"
#include "epgstation/types.h"
#include "kodi/libXBMC_addon.h"
#include "schedule.h"
#include "json/json.hpp"

extern ADDON::CHelper_libXBMC_addon* XBMC;

namespace epgstation {
bool Recorded::refresh()
{
    nlohmann::json response;

    if (api::getRecorded(response) == api::REQUEST_FAILED) {
        return false;
    }

    programs.clear();

    for (const auto& p : response["recorded"]) {
        auto r = p.get<program>();
        programs.push_back(r);
    }

    XBMC->Log(ADDON::LOG_NOTICE, "Updated recorded program: ammount = %d", programs.size());

    return true;
}

bool Recorded::remove(std::string id)
{
    if (api::deleteRecordedProgram(id) != api::REQUEST_FAILED) {
        XBMC->Log(ADDON::LOG_NOTICE, "Deleted recording: #%s", id.c_str());
        return true;
    }
    XBMC->Log(ADDON::LOG_ERROR, "Failed to delete recording: #%s", id.c_str());
    return false;
}

}
