#pragma once

#include <optional>
#include <map>
#include <queue>
#include <string>
#include <vector>

// INCREASE FOR NEW OPTION
#define FF_NOPTIONS     10
// Data printed out
#define FF_TITLE 		"title"
#define FF_AUTHOR 		"author"
#define FF_CAPO 		"capo"
#define FF_KEY      	"key"
#define FF_TUNING 		"tuning"
// Options for PDF generation
#define FF_SIZE 		"size" // int
#define FF_BODY_FONT 	"body-font"
#define FF_TITLE_FONT	"title-font"
#define FF_UTF8 		"utf8" // Should have value 'true' or '1', everything else is valued as false
#define FF_SPLIT 		"split" // Should have value 'true' or '1', everything else is valued as false

class Section {
public:
    enum class Type {
        Normal,
        Reproducible,
        Reproducing
    };

    Section(std::string_view sec);
    void print(std::ostream &out);

    bool page_break() const { return m_page_break; }

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

    bool m_page_break = false;
};

class FileFormatter {
public:
    void init(const char *fn);

    void put_metadata(std::string_view key, std::string_view value);

    void print_formatted_txt();
    void print_formatted_pdf(const std::string &fn);
private:
    std::map<std::string, std::string> metadata;

    std::vector<Section> secs;

    static bool is_valid_option(std::string_view opt);

    std::string title();
    std::string subtitle();
};
