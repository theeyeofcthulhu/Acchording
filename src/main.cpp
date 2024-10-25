#include <getopt.h>

#include <fmt/core.h>

#define JARGS_IMPLEMENTATION
#include "jargs.hpp"

#include "file.hpp"
#include "font.hpp"

#define DEFAULT_FONT "Ubuntu Mono:Regular"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fmt::print(stderr, "Please specify a file.\n");
        return 1;
    }

    bool pdf = false;
    int body_font_size = 11;
    std::string body_font = DEFAULT_FONT;
    bool use_utf8 = false;

    jargs::Parser parser;
    parser.add({'p', "pdf", "Generate PDF", [&pdf]() {
        pdf = true;
    }});
    parser.add({'s', "size", "Specify font size", [&body_font_size](auto optarg) {
        body_font_size = std::stoi(optarg.data());
    }});
    parser.add({'f', "font", "Specify font \"name[:style]\"", [&body_font](auto optarg) {
        body_font = optarg;
    }});
    parser.add({'u', "utf8", "Use UTF-8 in PDF generation", [&use_utf8]() {
        use_utf8 = true;
    }});
    parser.add_help("acchording [args] file");

    parser.parse(argc, argv);

    const char *fn = argv[argc-1];

    FileFormatter ff(fn);

    if (!pdf) {
        ff.print_formatted_txt();
    } else {
        FontMatcher fm;
        std::string font_file = fm.match_name(body_font);
        if (font_file.empty()) {
            fmt::print(stderr, "Error: Font \"{}\" not found. Aborting.\n", body_font);
            return 1;
        }

        fmt::print("Found font: {}\n", font_file);

        std::string_view fn_base(fn);
        fn_base = fn_base.substr(0, fn_base.rfind('.'));

        ff.print_formatted_pdf(fmt::format("{}.pdf", fn_base), body_font_size, font_file, use_utf8);
    }
}
