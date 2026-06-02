// Parses and loads BVH animations

#include "bvh/bvh_data.hpp"

#include <cctype>
#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <utility>
#include <vector>

namespace bvhview
{
constexpr int PARSER_ERR_MAX = 512;

struct Parser
{
    const char* filename;
    int offset;
    const char* data;
    int row;
    int col;
    char err[PARSER_ERR_MAX];
};

void ParserInit(Parser* par, const char* filename, const char* data)
{
    par->filename = filename;
    par->offset = 0;
    par->data = data;
    par->row = 0;
    par->col = 0;
    par->err[0] = '\0';
}

char ParserPeek(const Parser* par)
{
    return par->data[par->offset];
}

char ParserPeekForward(const Parser* par, int steps)
{
    return par->data[par->offset + steps];
}

bool ParserMatch(const Parser* par, char match)
{
    return match == par->data[par->offset];
}

bool ParserOneOf(const Parser* par, const char* matches)
{
    return std::strchr(matches, par->data[par->offset]);
}

bool ParserStartsWithCaseless(const Parser* par, const char* prefix)
{
    const char* start = par->data + par->offset;
    while (*prefix)
    {
        if (std::tolower(static_cast<unsigned char>(*prefix)) != std::tolower(static_cast<unsigned char>(*start)))
        {
            return false;
        }
        prefix++;
        start++;
    }
    return true;
}

void ParserInc(Parser* par)
{
    if (par->data[par->offset] == '\n')
    {
        par->row++;
        par->col = 0;
    }
    else
    {
        par->col++;
    }
    par->offset++;
}

void ParserAdvance(Parser* par, int num)
{
    for (int i = 0; i < num; i++)
    {
        ParserInc(par);
    }
}

const char* ParserCharName(char c)
{
    static char parserCharName[2];
    switch (c)
    {
        case '\0':
            return "end of file";
        case '\r':
            return "new line";
        case '\n':
            return "new line";
        case '\t':
            return "tab";
        case '\v':
            return "vertical tab";
        case '\b':
            return "backspace";
        case '\f':
            return "form feed";
        default:
            parserCharName[0] = c;
            parserCharName[1] = '\0';
            return parserCharName;
    }
}

void ParserError(Parser* par, const char* format, ...)
{
    char message[PARSER_ERR_MAX];
    va_list args;
    va_start(args, format);
    std::vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    std::snprintf(par->err, PARSER_ERR_MAX, "%s:%i:%i: error: %s", par->filename, par->row, par->col, message);
}

void BVHDataInit(BVHData* bvh)
{
    *bvh = {};
}

void BVHDataFree(BVHData* bvh)
{
    *bvh = {};
}

int BVHDataAddJoint(BVHData* bvh)
{
    bvh->joints.emplace_back();
    bvh->jointCount = static_cast<int>(bvh->joints.size());
    return bvh->jointCount - 1;
}

void BVHParseWhitespace(Parser* par)
{
    while (ParserOneOf(par, " \r\t\v"))
    {
        ParserInc(par);
    }
}

bool BVHParseString(Parser* par, const char* string)
{
    if (ParserStartsWithCaseless(par, string))
    {
        ParserAdvance(par, static_cast<int>(std::strlen(string)));
        return true;
    }

    ParserError(par, "expected '%s' at '%s'", string, ParserCharName(ParserPeek(par)));
    return false;
}

bool BVHParseNewline(Parser* par)
{
    BVHParseWhitespace(par);
    if (ParserMatch(par, '\n'))
    {
        ParserInc(par);
        BVHParseWhitespace(par);
        return true;
    }

    ParserError(par, "expected newline at '%s'", ParserCharName(ParserPeek(par)));
    return false;
}

bool BVHParseJointName(BVHJointData* jnt, Parser* par)
{
    BVHParseWhitespace(par);

    char buffer[256];
    int chrnum = 0;
    while (chrnum < 255 && ParserOneOf(par, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_:-."))
    {
        buffer[chrnum++] = ParserPeek(par);
        ParserInc(par);
    }
    buffer[chrnum] = '\0';

    if (chrnum > 0)
    {
        jnt->name = buffer;
        BVHParseWhitespace(par);
        return true;
    }

    ParserError(par, "expected joint name at '%s'", ParserCharName(ParserPeek(par)));
    return false;
}

bool BVHParseFloat(float* out, Parser* par)
{
    BVHParseWhitespace(par);
    char* end;
    errno = 0;
    *out = static_cast<float>(std::strtod(par->data + par->offset, &end));

    if (errno == 0)
    {
        ParserAdvance(par, static_cast<int>(end - (par->data + par->offset)));
        return true;
    }

    ParserError(par, "expected float at '%s'", ParserCharName(ParserPeek(par)));
    return false;
}

bool BVHParseInt(int* out, Parser* par)
{
    BVHParseWhitespace(par);
    char* end;
    errno = 0;
    *out = static_cast<int>(std::strtol(par->data + par->offset, &end, 10));

    if (errno == 0)
    {
        ParserAdvance(par, static_cast<int>(end - (par->data + par->offset)));
        return true;
    }

    ParserError(par, "expected integer at '%s'", ParserCharName(ParserPeek(par)));
    return false;
}

bool BVHParseJointOffset(BVHJointData* jnt, Parser* par)
{
    if (!BVHParseString(par, "OFFSET"))
    {
        return false;
    }
    if (!BVHParseFloat(&jnt->offset.x, par))
    {
        return false;
    }
    if (!BVHParseFloat(&jnt->offset.y, par))
    {
        return false;
    }
    if (!BVHParseFloat(&jnt->offset.z, par))
    {
        return false;
    }
    return BVHParseNewline(par);
}

bool BVHParseChannelEnum(char* channel, Parser* par, const char* channelName, char channelValue)
{
    BVHParseWhitespace(par);
    if (!BVHParseString(par, channelName))
    {
        return false;
    }
    BVHParseWhitespace(par);
    *channel = channelValue;
    return true;
}

bool BVHParseChannel(char* channel, Parser* par)
{
    BVHParseWhitespace(par);
    if (ParserPeek(par) == '\0')
    {
        ParserError(par, "expected channel at end of file");
        return false;
    }

    const char axis = static_cast<char>(std::tolower(static_cast<unsigned char>(ParserPeek(par))));
    const char kind = static_cast<char>(std::tolower(static_cast<unsigned char>(ParserPeekForward(par, 1))));
    if (axis == 'x' && kind == 'p')
    {
        return BVHParseChannelEnum(channel, par, "Xposition", CHANNEL_X_POSITION);
    }
    if (axis == 'y' && kind == 'p')
    {
        return BVHParseChannelEnum(channel, par, "Yposition", CHANNEL_Y_POSITION);
    }
    if (axis == 'z' && kind == 'p')
    {
        return BVHParseChannelEnum(channel, par, "Zposition", CHANNEL_Z_POSITION);
    }
    if (axis == 'x' && kind == 'r')
    {
        return BVHParseChannelEnum(channel, par, "Xrotation", CHANNEL_X_ROTATION);
    }
    if (axis == 'y' && kind == 'r')
    {
        return BVHParseChannelEnum(channel, par, "Yrotation", CHANNEL_Y_ROTATION);
    }
    if (axis == 'z' && kind == 'r')
    {
        return BVHParseChannelEnum(channel, par, "Zrotation", CHANNEL_Z_ROTATION);
    }

    ParserError(par, "expected channel type");
    return false;
}

bool BVHParseJointChannels(BVHJointData* jnt, Parser* par)
{
    if (!BVHParseString(par, "CHANNELS"))
    {
        return false;
    }
    if (!BVHParseInt(&jnt->channelCount, par))
    {
        return false;
    }
    for (int i = 0; i < jnt->channelCount; i++)
    {
        if (!BVHParseChannel(&jnt->channels[i], par))
        {
            return false;
        }
    }
    return BVHParseNewline(par);
}

bool BVHParseJoints(BVHData* bvh, int parent, Parser* par)
{
    while (ParserOneOf(par, "JEje"))
    {
        const int j = BVHDataAddJoint(bvh);
        bvh->joints[j].parent = parent;
        const char keyword = static_cast<char>(std::toupper(static_cast<unsigned char>(ParserPeek(par))));

        if (keyword == 'J')
        {
            if (!BVHParseString(par, "JOINT"))
            {
                return false;
            }
            if (!BVHParseJointName(&bvh->joints[j], par))
            {
                return false;
            }
            if (!BVHParseNewline(par))
            {
                return false;
            }
            if (!BVHParseString(par, "{"))
            {
                return false;
            }
            if (!BVHParseNewline(par))
            {
                return false;
            }
            if (!BVHParseJointOffset(&bvh->joints[j], par))
            {
                return false;
            }
            if (!BVHParseJointChannels(&bvh->joints[j], par))
            {
                return false;
            }
            if (!BVHParseJoints(bvh, j, par))
            {
                return false;
            }
            if (!BVHParseString(par, "}"))
            {
                return false;
            }
            if (!BVHParseNewline(par))
            {
                return false;
            }
        }
        else
        {
            bvh->joints[j].endSite = true;
            if (!BVHParseString(par, "End Site"))
            {
                return false;
            }
            bvh->joints[j].name = "End Site";
            if (!BVHParseNewline(par))
            {
                return false;
            }
            if (!BVHParseString(par, "{"))
            {
                return false;
            }
            if (!BVHParseNewline(par))
            {
                return false;
            }
            if (!BVHParseJointOffset(&bvh->joints[j], par))
            {
                return false;
            }
            if (!BVHParseString(par, "}"))
            {
                return false;
            }
            if (!BVHParseNewline(par))
            {
                return false;
            }
        }
    }
    return true;
}

bool BVHParseMotionData(BVHData* bvh, Parser* par)
{
    int channelCount = 0;
    for (int i = 0; i < bvh->jointCount; i++)
    {
        channelCount += bvh->joints[i].channelCount;
    }

    bvh->channelCount = channelCount;
    bvh->motionData.resize(bvh->frameCount * channelCount);
    for (int i = 0; i < bvh->frameCount; i++)
    {
        for (int j = 0; j < channelCount; j++)
        {
            if (!BVHParseFloat(&bvh->motionData[i * channelCount + j], par))
            {
                return false;
            }
        }
        if (!BVHParseNewline(par))
        {
            return false;
        }
    }
    return true;
}

bool BVHParse(BVHData* bvh, Parser* par)
{
    if (!BVHParseString(par, "HIERARCHY"))
    {
        return false;
    }
    if (!BVHParseNewline(par))
    {
        return false;
    }

    const int j = BVHDataAddJoint(bvh);
    if (!BVHParseString(par, "ROOT"))
    {
        return false;
    }
    if (!BVHParseJointName(&bvh->joints[j], par))
    {
        return false;
    }
    if (!BVHParseNewline(par))
    {
        return false;
    }
    if (!BVHParseString(par, "{"))
    {
        return false;
    }
    if (!BVHParseNewline(par))
    {
        return false;
    }
    if (!BVHParseJointOffset(&bvh->joints[j], par))
    {
        return false;
    }
    if (!BVHParseJointChannels(&bvh->joints[j], par))
    {
        return false;
    }
    if (!BVHParseJoints(bvh, j, par))
    {
        return false;
    }
    if (!BVHParseString(par, "}"))
    {
        return false;
    }
    if (!BVHParseNewline(par))
    {
        return false;
    }
    if (!BVHParseString(par, "MOTION"))
    {
        return false;
    }
    if (!BVHParseNewline(par))
    {
        return false;
    }
    if (!BVHParseString(par, "Frames:"))
    {
        return false;
    }
    if (!BVHParseInt(&bvh->frameCount, par))
    {
        return false;
    }
    if (!BVHParseNewline(par))
    {
        return false;
    }
    if (!BVHParseString(par, "Frame Time:"))
    {
        return false;
    }
    if (!BVHParseFloat(&bvh->frameTime, par))
    {
        return false;
    }
    if (!BVHParseNewline(par))
    {
        return false;
    }
    if (bvh->frameTime == 0.0f)
    {
        bvh->frameTime = 1.0f / 60.0f;
    }
    return BVHParseMotionData(bvh, par);
}

bool BVHDataLoad(BVHData* bvh, const char* filename, char* errMsg, int errMsgSize)
{
    FILE* file = std::fopen(filename, "rb");
    if (!file)
    {
        std::snprintf(errMsg, errMsgSize, "Error: Could not find file '%s'\n", filename);
        return false;
    }

    std::fseek(file, 0, SEEK_END);
    const long int length = std::ftell(file);
    std::fseek(file, 0, SEEK_SET);
    std::vector<char> buffer(static_cast<std::size_t>(length) + 2);
    std::fread(buffer.data(), 1, static_cast<std::size_t>(length), file);
    std::fclose(file);
    buffer[length] = '\n';
    buffer[length + 1] = '\0';

    BVHData parsed;
    Parser par;
    ParserInit(&par, filename, buffer.data());
    const bool result = BVHParse(&parsed, &par);
    if (!result)
    {
        std::snprintf(errMsg, errMsgSize, "Error: Could not parse BVH file:\n    %s", par.err);
        return false;
    }

    *bvh = std::move(parsed);
    errMsg[0] = '\0';
    std::printf("INFO: parsed '%s' successfully\n", filename);
    return true;
}
}
