#include <cassert>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>

#include <fmt/core.h>
#include <fmt/ostream.h>

#include <hpdf.h>

#include "file.hpp"

#include <iostream>
#include "dbg.h"

Section scan_section(std::string_view sec)
{
    Section res;

    assert(sec.contains('[') && sec.contains(']'));
    assert(sec.starts_with('['));

    res.name = sec.substr(1, sec.find(']')-1);

    size_t remainder_beg = sec.find(']') + 1;
    for (; std::isspace(sec[remainder_beg]) && remainder_beg < sec.size(); remainder_beg++)
        ;

    // No content
    if (remainder_beg == sec.size()) {
        res.text = "";
        return res;
    }

    std::string_view remainder = sec.substr(remainder_beg);

    if (remainder.starts_with("chords:")) {
        // Have to copy to make stringstream
        // (C++26 should have stringstream with string_view,
        // in gcc yet?)
        std::string chords_s(remainder.substr(remainder.find(':') + 1, remainder.find('\n') - (remainder.find(':')+1)));

        res.chords.emplace(); // Initializes the optional queue

        std::stringstream ss(chords_s);
        std::string buf;

        while (std::getline(ss, buf, ' ')) {
            if (!buf.empty())
                res.chords->push(buf);
        }

        if (res.chords->empty()) {
            fmt::print(stderr, "Warning: chords are empty\n");
        }

        remainder = remainder.substr(remainder.find('\n') + 1);
    }

    while (isspace(remainder[remainder.size()-1]))
        remainder.remove_suffix(1);

    res.text = remainder;
    res.text.append("\n");

    return res;
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

            std::string_view prop(buf.begin(), buf.begin() + sep);

            // Skip spaces
            for (sep += 1; std::isspace(buf[sep]) && sep < buf.size(); sep++)
                ;
            std::string_view value(buf.begin() + sep, buf.end());

            if (prop == "author")
                author = value;
            else if (prop == "title")
                title = value;
            else if (prop == "capo")
                capo.emplace(value);
            else if (prop == "key")
                key.emplace(value);
            else if (prop == "tuning")
                tuning.emplace(value);
            else
                fmt::print(stderr, "Warning: Property \"{}\" not recognized\n", prop);
        }
    }

    if (!f) {
        fmt::print(stderr, "Warning: File ended before any [Tags]\n");
        return;
    }

    // Read sections

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

        secs.push_back(scan_section(buf));
    } while (std::getline(f, buf));
}

void FileFormatter::print_info()
{
    fmt::print("\"{}\" by {}\nCapo on {}, key of {}, tuning: {}\n", title, author,
                                                                    capo.value_or("?"),
                                                                    key.value_or("?"),
                                                                    tuning.value_or("?"));

    for (const auto &sec : secs) {
        fmt::print("Section \"{}\", {} chords\n", sec.name, sec.chords.has_value() ? "with" : "without");
    }
}

void FileFormatter::print_section(Section &sec, std::ostream &out)
{
    fmt::print(out, "[{}]\n\n", sec.name);

    if (sec.chords.has_value()) {
        std::stringstream ss(sec.text);
        std::string buf;

        while (std::getline(ss, buf)) {
            std::string chord_line(buf.size(), ' ');
            while (buf.contains('>')) {
                size_t pos = buf.find('>');

                buf.erase(pos, 1);
                
                if (!sec.chords->empty()) {
                    chord_line.insert(pos, sec.chords->front());
                    sec.chords->pop();
                } else {
                    chord_line.insert(pos, "?");
                }
            }

            fmt::print(out, "{}\n{}\n", chord_line, buf);
        }
    } else {
        fmt::print(out, "{}", sec.text);
    }
    fmt::print(out, "\n");
}

void FileFormatter::print_formatted_txt()
{
    fmt::print("{} - {}\n", author, title);
    fmt::print("Capo {} - Key {} - Tuning: {}\n\n", capo.value_or("-"), key.value_or("?"), tuning.value_or("Standard"));

    for (auto &sec : secs) {
        print_section(sec, std::cout);
    }
}

// https://github.com/libharu/libharu/wiki/Error-handling
void error_handler (HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data)
{
    fmt::print(stderr, "hpdf: error_no={:x}, detail_no={}\n",
      (unsigned int) error_no, (int) detail_no);
    throw std::exception (); /* throw exception on error */
}


void FileFormatter::print_formatted_pdf(const std::string &fn, int body_font_size, const std::string &body_font)
{
    HPDF_Doc pdf = HPDF_New(error_handler, NULL);

    if (!pdf) {
        fmt::print(stderr, "hpdf: cannot create document\n");
        return;
    }

    try {
        HPDF_Page page = HPDF_AddPage(pdf);
        
        HPDF_REAL width, height;
        height = HPDF_Page_GetHeight (page);
        width = HPDF_Page_GetWidth (page);

        int pos = height - 50;
        const int left_margin = 50;

        // Title
        HPDF_Font def_font = HPDF_GetFont(pdf, "Helvetica-Bold", NULL);
        HPDF_Page_SetFontAndSize(page, def_font, 18);

        std::string header = fmt::format("{} - {}", author, title);
        HPDF_Page_BeginText(page);
        HPDF_Page_TextOut(page, left_margin, pos, header.c_str());
        HPDF_Page_EndText(page);

        // Sub header
        def_font = HPDF_GetFont(pdf, "Helvetica", NULL);
        HPDF_Page_SetFontAndSize(page, def_font, 12);

        std::string sub_header = fmt::format("Capo {} - Key {} - Tuning: {}", capo.value_or("-"), key.value_or("?"), tuning.value_or("Standard"));
        HPDF_Page_BeginText(page);
        HPDF_Page_TextOut(page, left_margin, (pos -= 20), sub_header.c_str());
        HPDF_Page_EndText(page);

        // Sections
        HPDF_Page_BeginText(page);
        HPDF_Page_MoveTextPos(page, left_margin, (pos -= 20));

        const char *font_name = HPDF_LoadTTFontFromFile(pdf, body_font.c_str(), HPDF_TRUE);
        def_font = HPDF_GetFont(pdf, font_name, NULL);

        HPDF_Page_SetFontAndSize(page, def_font, body_font_size);
        for (auto &sec : secs) {
            std::stringstream ss;
            print_section(sec, ss);

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

                HPDF_Page_MoveTextPos(page, 0, -(body_font_size+2));
                HPDF_Page_ShowText(page, buf.c_str());
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
