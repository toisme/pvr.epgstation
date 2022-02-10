/*
 * Copyright (C) 2015-2020 Yuki MIZUNO
 * https://github.com/Harekaze/pvr.epgstation/
 * SPDX-License-Identifier: GPL-3.0-only
 */
#ifndef EPGSTATION_TYPES_H
#define EPGSTATION_TYPES_H
#include "json/json.hpp"
#include <string>

#define OPTIONAL_JSON_FROM(v)                                             \
    if (nlohmann_json_j.contains(#v) && !nlohmann_json_j[#v].is_null()) { \
        NLOHMANN_JSON_FROM(v);                                            \
    }

namespace epgstation {
class storage {
public:
    long long total;
    long long used;

    friend void from_json(const nlohmann::json& nlohmann_json_j, storage& nlohmann_json_t)
    {
        if (nlohmann_json_j.contains("items") && nlohmann_json_j["items"].is_array()) {
            nlohmann_json_t.total = nlohmann_json_j["items"].at(0)["total"];
            nlohmann_json_t.used = nlohmann_json_j["items"].at(0)["used"];
        }
    }
};

enum ReservedState {
    STATE_RESERVED,
    STATE_CONFLICT,
    STATE_SKIPPED,
};

class program {
public:
    uint64_t id;
    uint64_t programId;
    uint64_t channelId;
    uint64_t eventId;
    std::string channelType;
    std::string name;
    std::string description;
    std::string extended;
    time_t startAt;
    time_t endAt;
    uint8_t genre1 = 0xff;
    uint8_t genre2 = 0xff;
    bool recording;
    bool hasThumbnail;
    bool original;
    std::vector<std::pair<uint64_t, std::string>> encoded;
    int16_t ruleId = 0; // Optional for reserved program
    ReservedState state; // Optional for reserved program

    uint8_t subGenre1 = 0xff;
    uint64_t thumbnailId;
    uint64_t originalId;

    friend void from_json(const nlohmann::json& nlohmann_json_j, program& nlohmann_json_t)
    {
        NLOHMANN_JSON_FROM(id);
        OPTIONAL_JSON_FROM(programId);
        OPTIONAL_JSON_FROM(eventId);
        NLOHMANN_JSON_FROM(channelId);
        OPTIONAL_JSON_FROM(channelType);
        NLOHMANN_JSON_FROM(name);
        OPTIONAL_JSON_FROM(description);
        OPTIONAL_JSON_FROM(extended);
        nlohmann_json_t.startAt = static_cast<time_t>(nlohmann_json_j["startAt"].get<long long>() / 1000ll);
        nlohmann_json_t.endAt = static_cast<time_t>(nlohmann_json_j["endAt"].get<long long>() / 1000ll);
        OPTIONAL_JSON_FROM(genre1);
        OPTIONAL_JSON_FROM(genre2);
        OPTIONAL_JSON_FROM(recording);
        OPTIONAL_JSON_FROM(hasThumbnail);
        OPTIONAL_JSON_FROM(original);
        if (nlohmann_json_j.contains("encoded") && nlohmann_json_j["encoded"].is_array()) {
            std::transform(nlohmann_json_j["encoded"].begin(), nlohmann_json_j["encoded"].end(), std::back_inserter(nlohmann_json_t.encoded), [](const nlohmann::json& e) {
                return std::make_pair(e["encodedId"], e["name"]);
            });
        }

        OPTIONAL_JSON_FROM(subGenre1);
        nlohmann_json_t.genre2 = nlohmann_json_t.subGenre1;
        if (nlohmann_json_j.contains("thumbnails") && nlohmann_json_j["thumbnails"].is_array() && nlohmann_json_j["thumbnails"].size()) {
            nlohmann_json_j["thumbnails"].at(0).get_to(nlohmann_json_t.thumbnailId);
            nlohmann_json_t.hasThumbnail = true;
        }
        if (nlohmann_json_j.contains("videoFiles") && nlohmann_json_j["videoFiles"].is_array()) {
            nlohmann_json_t.encoded.clear();
            for (const auto& e : nlohmann_json_j["videoFiles"]) {
                if (e["type"] == "ts")
                {
                    nlohmann_json_t.originalId = e["id"];
                    nlohmann_json_t.original = true;
                }
                else if (e["type"] == "encoded")
                {
                    nlohmann_json_t.encoded.push_back(std::make_pair(e["id"], e["filename"]));
                }
            }
        }
    }
};

class channel {
public:
    uint64_t id;
    uint32_t serviceId;
    uint32_t networkId;
    std::string name;
    bool hasLogoData;
    std::string channelType;
    uint8_t channelTypeId;
    uint8_t type;
    uint8_t remoteControlKeyId;
    std::string channelStr;
    int internalId; // for internal use

    friend void from_json(const nlohmann::json& nlohmann_json_j, epgstation::channel& nlohmann_json_t)
    {
        NLOHMANN_JSON_FROM(id);
        NLOHMANN_JSON_FROM(serviceId);
        NLOHMANN_JSON_FROM(networkId);
        NLOHMANN_JSON_FROM(name);
        NLOHMANN_JSON_FROM(hasLogoData);
        NLOHMANN_JSON_FROM(channelType);
        OPTIONAL_JSON_FROM(channelTypeId);
        OPTIONAL_JSON_FROM(type);
        OPTIONAL_JSON_FROM(remoteControlKeyId);
        if (nlohmann_json_j.contains("channel") && nlohmann_json_j["channel"].is_string()) {
            nlohmann_json_t.channelStr = nlohmann_json_j["channel"];
        }
    }
};

class rule {
public:
    uint32_t id;
    std::string keyword = "";
    bool title = true;
    bool description = false;
    bool enable = true;
    uint16_t week = 0;
    uint64_t station = 0;
    uint16_t startTime = 0;
    uint16_t timeRange = 0;
    std::string directory = "";

    friend void from_json(const nlohmann::json& nlohmann_json_j, rule& nlohmann_json_t)
    {
        NLOHMANN_JSON_FROM(id);
        if (nlohmann_json_j.contains("searchOption") && nlohmann_json_j["searchOption"].is_object()) {
            if (nlohmann_json_j["searchOption"].contains("keyword") && nlohmann_json_j["searchOption"]["keyword"].is_string()) {
                nlohmann_json_t.keyword = nlohmann_json_j["searchOption"]["keyword"];
            }
            if (nlohmann_json_j["searchOption"].contains("name") && nlohmann_json_j["searchOption"]["name"].is_boolean()) {
                nlohmann_json_t.title = nlohmann_json_j["searchOption"]["name"];
            }
            if (nlohmann_json_j["searchOption"].contains("description") && nlohmann_json_j["searchOption"]["description"].is_boolean()) {
                nlohmann_json_t.description = nlohmann_json_j["searchOption"]["description"];
            }
            if (nlohmann_json_j["searchOption"].contains("channelIds") && nlohmann_json_j["searchOption"]["channelIds"].is_array() && nlohmann_json_j["searchOption"]["channelIds"].size()) {
                nlohmann_json_t.station = nlohmann_json_j["searchOption"]["channelIds"].at(0).get<uint64_t>();
            }
            if (nlohmann_json_j["searchOption"].contains("times") && nlohmann_json_j["searchOption"]["times"].is_array() && nlohmann_json_j["searchOption"]["times"].size()) {
                if (nlohmann_json_j["searchOption"]["times"].at(0).contains("week") && nlohmann_json_j["searchOption"]["times"].at(0)["week"].is_number()) {
                    nlohmann_json_t.week = nlohmann_json_j["searchOption"]["times"].at(0)["week"].get<uint16_t>();
                    nlohmann_json_t.week = ((nlohmann_json_t.week & 0b00000001) << 6) | ((nlohmann_json_t.week & 0b01111110) >> 1);
                }
                if (nlohmann_json_j["searchOption"]["times"].at(0).contains("start") && nlohmann_json_j["searchOption"]["times"].at(0)["start"].is_number()) {
                    nlohmann_json_t.startTime = nlohmann_json_j["searchOption"]["times"].at(0)["start"].get<uint16_t>();
                }
                if (nlohmann_json_j["searchOption"]["times"].at(0).contains("range") && nlohmann_json_j["searchOption"]["times"].at(0)["range"].is_number()) {
                    nlohmann_json_t.timeRange = nlohmann_json_j["searchOption"]["times"].at(0)["range"].get<uint16_t>();
                }
            }
        }
        if (nlohmann_json_j.contains("reserveOption") && nlohmann_json_j["reserveOption"].is_object()) {
            if (nlohmann_json_j["reserveOption"].contains("enable") && nlohmann_json_j["reserveOption"]["enable"].is_boolean()) {
                nlohmann_json_t.enable = nlohmann_json_j["reserveOption"]["enable"];
            }
        }
        if (nlohmann_json_j.contains("saveOption") && nlohmann_json_j["saveOption"].is_object()) {
            if (nlohmann_json_j["saveOption"].contains("directory") && nlohmann_json_j["saveOption"]["directory"].is_string()) {
                nlohmann_json_t.directory = nlohmann_json_j["saveOption"]["directory"];
            }
        }
        OPTIONAL_JSON_FROM(station);
    }
};

} // namespace epgstation

#endif /* end of include guard */
