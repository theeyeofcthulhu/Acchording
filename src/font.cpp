#include <cstdlib>
#include <iostream>
#include <string>

#include <fontconfig/fontconfig.h>

#include <fmt/core.h>

#include "font.hpp"

FontMatcher::FontMatcher()
{
    if (!FcInit()) {
        fmt::print("fontconfig: FcInit() failed, aborting!\n");
        std::exit(1);
    }

    config = FcConfigGetCurrent();
    FcConfigSetRescanInterval(config, 0);
}

FontMatcher::~FontMatcher()
{
    FcFini();
}

std::string FontMatcher::get_matching_font(FcPattern *pat)
{
    std::string res;

    FcConfigSubstitute(config, pat, FcMatchPattern);
    FcDefaultSubstitute(pat);

    FcResult match_result;
    FcPattern *match = FcFontMatch(config, pat, &match_result);

    // No font matched pat
    if (match_result != FcResultMatch) {
        fmt::print(stderr, "fontconfig: Failed to match font\n");
        return res;
    }

    FcChar8* file;
    if (FcPatternGetString(match, FC_FILE, 0, &file) == FcResultMatch) {
        res = (char*)file;
    } else {
        fmt::print(stderr, "fontconfig: Failed to retrieve filename\n");
        return res;
    }

    FcPatternDestroy(match);

    return res;
}

std::string FontMatcher::match_name(std::string name)
{
    std::string res;

    FcChar8* family = (FcChar8*)name.c_str();

    // Split the string into Family and Style
    // if they appear separated by ':'
    FcChar8* style = NULL;
    if (size_t pos = name.find(':'); pos != std::string::npos) {
        name[pos] = '\0';
        style = (FcChar8*)(name.c_str() + pos + 1);
    } else {
        style = (FcChar8*)"Regular";
    }

    // Matches .ttf fonts with the given name and style
    FcPattern *pat = FcPatternBuild (0,
                                     FC_FAMILY, FcTypeString, family,
                                     FC_STYLE, FcTypeString, style,
                                     FC_FONTFORMAT, FcTypeString, "TrueType",
                                     (char *) 0);


    res = get_matching_font(pat);
    FcPatternDestroy(pat);

    return res;
}
