#pragma once

// Provides optional Windows profiling macros

#include <cmath>
#include <cstdint>
#include <cstring>

#if defined(ENABLE_PROFILE) && defined(_WIN32)
#include <profileapi.h>

namespace bvhview
{
constexpr int ProfileRecordMax = 512;
constexpr int ProfileRecordSampleMax = 128;

struct ProfileRecord
{
    const char* name = nullptr;
    std::uint32_t idx = 0;
    std::uint32_t num = 0;
    struct Sample
    {
        LARGE_INTEGER start{};
        LARGE_INTEGER end{};
    };
    Sample samples[ProfileRecordSampleMax]{};
};

struct ProfileRecordData
{
    std::uint32_t num = 0;
    LARGE_INTEGER freq{};
    ProfileRecord* records[ProfileRecordMax]{};
};

struct ProfileTickers
{
    std::uint64_t unitScale = 1000000;
    double alpha = 0.9;
    std::uint32_t samples[ProfileRecordMax]{};
    std::uint64_t iterations[ProfileRecordMax]{};
    double averages[ProfileRecordMax]{};
    double times[ProfileRecordMax]{};
};

inline ProfileRecordData globalProfileRecords;
inline ProfileTickers globalProfileTickers;

inline void ProfileRecordDataInit()
{
    globalProfileRecords = {};
    QueryPerformanceFrequency(&globalProfileRecords.freq);
}

inline void ProfileRecordBegin(ProfileRecord* record, const char* name)
{
    if (!record->name && globalProfileRecords.num < ProfileRecordMax)
    {
        record->name = name;
        globalProfileRecords.records[globalProfileRecords.num++] = record;
    }
    QueryPerformanceCounter(&record->samples[record->idx].start);
}

inline void ProfileRecordEnd(ProfileRecord* record)
{
    QueryPerformanceCounter(&record->samples[record->idx].end);
    record->idx = (record->idx + 1) % ProfileRecordSampleMax;
    record->num++;
}

inline void ProfileTickersInit()
{
    globalProfileTickers = {};
    globalProfileTickers.unitScale = 1000000;
    globalProfileTickers.alpha = 0.9;
}

inline void ProfileTickersUpdate()
{
    for (std::uint32_t i = 0; i < globalProfileRecords.num; i++)
    {
        ProfileRecord* record = globalProfileRecords.records[i];
        if (!record || !record->name) { continue; }

        globalProfileTickers.samples[i] = record->num;
        const int sampleCount = record->num < ProfileRecordSampleMax ? record->num : ProfileRecordSampleMax;
        for (int j = 0; j < sampleCount; j++)
        {
            const double time = static_cast<double>(
                (record->samples[j].end.QuadPart - record->samples[j].start.QuadPart) *
                globalProfileTickers.unitScale) /
                static_cast<double>(globalProfileRecords.freq.QuadPart);
            globalProfileTickers.iterations[i]++;
            globalProfileTickers.averages[i] =
                globalProfileTickers.alpha * globalProfileTickers.averages[i] +
                (1.0 - globalProfileTickers.alpha) * time;
            globalProfileTickers.times[i] =
                globalProfileTickers.averages[i] /
                (1.0 - std::pow(globalProfileTickers.alpha, globalProfileTickers.iterations[i]));
        }
        record->idx = 0;
        record->num = 0;
    }
}
}

#define PROFILE_INIT() ::bvhview::ProfileRecordDataInit()
#define PROFILE_BEGIN(NAME) static ::bvhview::ProfileRecord profileRecord##NAME; ::bvhview::ProfileRecordBegin(&profileRecord##NAME, #NAME)
#define PROFILE_END(NAME) ::bvhview::ProfileRecordEnd(&profileRecord##NAME)
#define PROFILE_TICKERS_INIT() ::bvhview::ProfileTickersInit()
#define PROFILE_TICKERS_UPDATE() ::bvhview::ProfileTickersUpdate()
#else
#define PROFILE_INIT()
#define PROFILE_BEGIN(NAME)
#define PROFILE_END(NAME)
#define PROFILE_TICKERS_INIT()
#define PROFILE_TICKERS_UPDATE()
#endif
