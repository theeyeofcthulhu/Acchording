#pragma once
#include <optional>
#include <queue>
#include <string>
#include <vector>

struct Section {
    std::string name;
    std::optional<std::queue<std::string>> chords;
    std::string text;
};

class FileFormatter {
public:
    FileFormatter(const char *fn);

    void print_info();
    void print_formatted_txt();
    void print_formatted_pdf(const std::string &fn, int body_font_size, const std::string &body_font);
private:
    std::string author;
    std::string title;

    std::optional<std::string> capo, key, tuning;

    std::vector<Section> secs;

    static void print_section(Section &sec, std::ostream &out);
};
