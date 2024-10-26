#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

#include <fmt/core.h>
#include <fmt/ostream.h>

#include <hpdf.h>

#include "file.hpp"

std::vector<Section> *Section::global_array = nullptr;

Section::Section(std::string_view sec)
{
    assert(sec.contains('[') && sec.contains(']'));
    assert(sec.starts_with('['));

    name = sec.substr(1, sec.find(']')-1);

    size_t remainder_beg = sec.find(']') + 1;
    for (; std::isspace(sec[remainder_beg]) && remainder_beg < sec.size(); remainder_beg++)
        ;

    // Parse special commands
    while (true) {
        if (name.starts_with('!')) {
            hide_name = true;
            name.erase(name.begin());
        } else if (name.starts_with('>')) {
            type = Section::Type::Reproducible;
            name.erase(name.begin());
        } else if (name.starts_with('<')) {
            type = Section::Type::Reproducing;
            name.erase(name.begin());
        } else {
            break;
        }
    }
    if (type == Section::Type::Reproducing)
        return;

    // No content
    if (remainder_beg == sec.size()) {
        text = "";
        return;
    }

    std::string_view remainder = sec.substr(remainder_beg);

    if (remainder.starts_with("chords:")) {
        // Have to copy to make stringstream
        // (C++26 should have stringstream with string_view,
        // in gcc yet?)
        std::string chords_s(remainder.substr(remainder.find(':') + 1, remainder.find('\n') - (remainder.find(':')+1)));

        chords.emplace(); // Initializes the optional queue

        std::stringstream ss(chords_s);
        std::string buf;

        while (std::getline(ss, buf, ' ')) {
            if (!buf.empty())
                chords->push(buf);
        }

        if (chords->empty()) {
            fmt::print(stderr, "Warning: chords are empty\n");
        }

        remainder = remainder.substr(remainder.find('\n') + 1);
    }

    while (isspace(remainder[remainder.size()-1]))
        remainder.remove_suffix(1);

    text = remainder;
    text.append("\n");
}

void Section::print(std::ostream &out)
{
    assert(global_array);

    if (type == Section::Type::Reproducing) {
        for (size_t j = 0; j < global_array->size(); j++) {
            const Section &other = (*global_array)[j];
            if (other.type == Section::Type::Reproducible && other.name == name) {
                if (!other.output.has_value()) {
                    fmt::print(stderr, "Warning: Attempting to reproduce [{}], which is undefined at this point\n", name);
                    return;
                }
                out << other.output.value();
                return;
            }
        }
        fmt::print(stderr, "Warning: Trying to reproduce [{}], which was never defined\n", name);
        return;
    }

    std::stringstream outs;

    fmt::print(outs, "\n");

    if (!hide_name)
        fmt::print(outs, "[{}]\n\n", name);

    if (chords.has_value()) {
        std::stringstream ss(text);
        std::string buf;

        while (std::getline(ss, buf)) {
            std::string chord_line(buf.size(), ' ');
            while (buf.contains('>')) {
                // TODO: check if file is UTF-8
                // Skips UTF-8 continuation bytes (starting with 0b10)
                auto utf8_pos = [](const std::string &s, char c) {
                    std::pair<size_t, size_t> poss; // 1. real pos, 2. effective pos
                    for (poss.first = 0, poss.second = 0; s[poss.first] != c && poss.first < s.size(); poss.first++) {
                        if ((s[poss.first] & 0b11000000) != 0b10000000) { // UTF-8 continuation char
                            poss.second += 1;
                        }
                    }
                    return poss;
                };

                auto poss = utf8_pos(buf, '>');

                buf.erase(poss.first, 1);
                
                if (!chords->empty()) {
                    chord_line.insert(poss.second, chords->front());
                    chords->pop();
                } else {
                    chord_line.insert(poss.second, "?");
                }
            }

            fmt::print(outs, "{}\n{}\n", chord_line, buf);
        }
    } else {
        fmt::print(outs, "{}", text);
    }

    if (type == Section::Type::Reproducible) {
        output.emplace(outs.str());
    }
    out << outs.str();
}

FileFormatter::FileFormatter(const char *fn)
{
    // Read file
    std::ifstream f(fn);
    if (!f.is_open()) {
        std::perror(fn);
        std::exit(1);
    }

    std::string buf;

    // Read meta info
    while (std::getline(f, buf)) {
        if (buf.empty())
            continue;

        // Song text begins
        if (buf.starts_with('['))
            break;

        if (!buf.contains(':')) {
            fmt::print(stderr, "Warning: Line, \"{}\", does not provide a property and a value\n", buf);
        } else {
            size_t sep = buf.find(':');

            std::string prop(buf.begin(), buf.begin() + sep);

            // Skip spaces
            for (sep += 1; std::isspace(buf[sep]) && sep < buf.size(); sep++)
                ;
            std::string_view value(buf.begin() + sep, buf.end());

            metadata[prop] = value;
        }
    }

    if (!metadata.contains("title")) {
        fmt::print(stderr, "Warning: No title provided\n");
        metadata["title"] = "Untitled";
    }

    if (!f) {
        fmt::print(stderr, "Warning: File ended before any [Tags]\n");
        return;
    }

    // Read sections
    
    Section::global_array = &secs;

    // We still have the first line in buf because
    // last loop ended because of it
    do {
        if (buf.empty())
            continue;
        if (!buf.starts_with('[') || !buf.ends_with(']')) {
            fmt::print(stderr, "Warning: Line, \"{}\", does not provide a correct [Tag]\n", buf);
            continue;
        }
        if (buf.length() <= 2) {
            fmt::print(stderr, "Warning: Empty tag disregarded\n", buf);
            continue;
        }

        auto pos = f.tellg();

        std::stringbuf section_content;
        f.get(section_content, '['); // getline() would extract the '['

        // Happens if section is empty
        if (f.fail()) {
            // A failed ifstream::get() puts the stream in a bad state
            f.clear();
            f.seekg(pos);
        }

        buf.push_back('\n'); // Was removed
        buf.append(section_content.str());

        secs.push_back(Section(buf));
    } while (std::getline(f, buf));
}

std::string FileFormatter::title()
{
    assert(metadata.contains("title"));
    if (metadata.contains("author"))
        return fmt::format("{} - {}", metadata.at("author"), metadata.at("title"));
    else
        return fmt::format("{}", metadata.at("title"));
}

std::string FileFormatter::subtitle()
{
    std::stringstream ss;
    bool previous = false;

    if (metadata.contains("capo")) {
        ss << fmt::format("Capo {}", metadata["capo"]);
        previous = true;
    }
    if (metadata.contains("key")) {
        if (previous)
            ss << " - ";
        ss << fmt::format("Key {}", metadata["key"]);
        previous = true;
    }
    if (metadata.contains("tuning")) {
        if (previous)
            ss << " - ";
        ss << fmt::format("Tuning: {}", metadata["tuning"]);
    }
    return ss.str();
}

void FileFormatter::print_formatted_txt()
{
    fmt::print("{}\n", title());
    auto sub = subtitle();
    if (!sub.empty())
        fmt::print("{}\n", sub);

    for (auto &sec : secs) {
        sec.print(std::cout);
    }
}

// https://github.com/libharu/libharu/wiki/Error-handling
void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data)
{
    (void)user_data;
    fmt::print(stderr, "hpdf: error_no={:x}, detail_no={}\n",
      (unsigned int) error_no, (int) detail_no);
    throw std::exception (); /* throw exception on error */
}

void FileFormatter::print_formatted_pdf(const std::string &fn,
        int body_font_size, const std::string &body_font,
        const std::string &header_font, const std::string &header_font_bold,
        bool use_utf8)
{
    HPDF_Doc pdf = HPDF_New(error_handler, NULL);

    if (!pdf) {
        fmt::print(stderr, "hpdf: cannot create document\n");
        return;
    }

    try {
        if (use_utf8) {
            HPDF_UseUTFEncodings(pdf);
            HPDF_SetCurrentEncoder(pdf, "UTF-8");
        }

        HPDF_Page page = HPDF_AddPage(pdf);
        
        HPDF_REAL height;
        height = HPDF_Page_GetHeight(page);

        int pos = height - 50;
        const int left_margin = 50;

        const char *font_name;
        HPDF_Font def_font;

        // Title
        std::string header = title();

        font_name = HPDF_LoadTTFontFromFile(pdf, header_font_bold.c_str(), HPDF_TRUE);
        def_font = HPDF_GetFont(pdf, font_name, use_utf8 ? "UTF-8" : NULL);
        HPDF_Page_SetFontAndSize(page, def_font, 18);

        HPDF_Page_BeginText(page);
        HPDF_Page_TextOut(page, left_margin, pos, header.c_str());
        HPDF_Page_EndText(page);

        // Sub header
        std::string sub_header = subtitle();

        font_name = HPDF_LoadTTFontFromFile(pdf, header_font.c_str(), HPDF_TRUE);
        def_font = HPDF_GetFont(pdf, font_name, use_utf8 ? "UTF-8" : NULL);
        HPDF_Page_SetFontAndSize(page, def_font, 12);

        HPDF_Page_BeginText(page);
        HPDF_Page_TextOut(page, left_margin, (pos -= 20), sub_header.c_str());
        HPDF_Page_EndText(page);

        // Sections
        HPDF_Page_BeginText(page);
        HPDF_Page_MoveTextPos(page, left_margin, (pos -= 10));

        font_name = HPDF_LoadTTFontFromFile(pdf, body_font.c_str(), HPDF_TRUE);
        def_font = HPDF_GetFont(pdf, font_name, use_utf8 ? "UTF-8" : NULL);

        HPDF_Page_SetFontAndSize(page, def_font, body_font_size);
        for (auto &sec : secs) {
            std::stringstream ss;
            sec.print(ss);

            std::string buf;
            while (std::getline(ss, buf)) {
                // Page is full, go to next
                if (HPDF_Point p = HPDF_Page_GetCurrentTextPos(page); p.y < 50) {
                    HPDF_Page_EndText(page);
                    page = HPDF_AddPage(pdf);

                    HPDF_Page_BeginText(page);

                    pos = height - 30;
                    HPDF_Page_MoveTextPos(page, left_margin, pos);
                    HPDF_Page_SetFontAndSize(page, def_font, body_font_size);
                }

                HPDF_Page_ShowText(page, buf.c_str());
                HPDF_Page_MoveTextPos(page, 0, -(body_font_size+2));
            }
        }
        HPDF_Page_EndText(page);

        HPDF_SaveToFile(pdf, fn.c_str());
    } catch (...) {
        HPDF_Free(pdf);
        return;
    }

    HPDF_Free (pdf);
}
