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

    FcObjectSet *os = FcObjectSetBuild(FC_FILE, (char *) 0); // What we want in the list
    FcFontSet *fs = FcFontList(config, pat, os);

    // No font matched pat
    if (!fs->nfont) {
        return res;
    }

    FcPattern *font = fs->fonts[0];
    FcChar8 *file;
    if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch) {
        res = (char*)file;
    } else {
        fmt::print(stderr, "fontconfig: Failed to retrieve filename\n");
        return res;
    }

    FcFontSetDestroy(fs);
    FcObjectSetDestroy(os);

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
