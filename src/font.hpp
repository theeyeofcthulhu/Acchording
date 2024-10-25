#pragma once

#include <string>

#include <fontconfig/fontconfig.h>

class FontMatcher {
public:
    FontMatcher();
    ~FontMatcher();

    std::string match_name(std::string name);
private:
    std::string get_matching_font(FcPattern *pat);

    FcConfig *config;
};
