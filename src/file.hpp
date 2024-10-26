#pragma once

#include <optional>
#include <map>
#include <queue>
#include <string>
#include <vector>

class Section {
public:
    enum class Type {
        Normal,
        Reproducible,
        Reproducing
    };

    Section(std::string_view sec);
    void print(std::ostream &out);

    // This is for accessing the other sections
    // when reaccessing a prior defined one
    static std::vector<Section> *global_array;

private:
    Type type = Type::Normal;

    std::string name;
    std::optional<std::queue<std::string>> chords;
    std::string text;

    bool hide_name = false; // Hide tag name
    std::optional<std::string> output; // For reproducing later
};

class FileFormatter {
public:
    FileFormatter(const char *fn);

    void print_formatted_txt();
    void print_formatted_pdf(const std::string &fn,
        int body_font_size, const std::string &body_font,
        const std::string &header_font, const std::string &header_font_bold,
        bool use_utf8);
private:
    std::map<std::string, std::string> metadata;

    std::vector<Section> secs;

    static void print_section(std::vector<Section> &secs, int index, std::ostream &out);

    std::string title();
    std::string subtitle();
};
