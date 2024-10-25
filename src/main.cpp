#include <getopt.h>

#include <fmt/core.h>

#define JARGS_IMPLEMENTATION
#include "jargs.hpp"

#include "file.hpp"
#include "font.hpp"
#include "config.hpp"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fmt::print(stderr, "Please specify a file.\n");
        return 1;
    }

    bool pdf = false;
    int body_font_size = ACCHORDING_BODY_FONT_SIZE;
    std::string body_font = ACCHORDING_BODY_FONT;
    std::string header_font = ACCHORDING_HEADER_FONT;
    bool use_utf8 = false;

    jargs::Parser parser;
    parser.add({'p', "pdf", "Generate PDF", [&pdf]() {
        pdf = true;
    }});
    parser.add({'s', "size", "Specify font size", [&body_font_size](auto optarg) {
        body_font_size = std::stoi(optarg.data());
    }});
    parser.add({'b', "body-font", "Specify font \"name[:style]\" for PDF body", [&body_font](auto optarg) {
        body_font = optarg;
    }});
    parser.add({'t', "title-font",
            "Specify font \"name\" for PDF header; should have 'Regular' and 'Bold' styles",
            [&header_font](auto optarg) {
        header_font = optarg;
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

        // These shouldn't fail, as fontconfig will just default
        // back to another font if it can't find a match
        std::string body_font_file = fm.match_name(body_font);
        std::string header_font_file = fm.match_name(fmt::format("{}:Regular", header_font));
        std::string header_bold_font_file = fm.match_name(fmt::format("{}:Bold", header_font));

        fmt::print("Body font: {}\n", body_font_file);
        fmt::print("Header font: {}\n", header_font_file);
        fmt::print("Header font (bold): {}\n", header_bold_font_file);

        std::string_view fn_base(fn);
        fn_base = fn_base.substr(0, fn_base.rfind('.'));

        ff.print_formatted_pdf(fmt::format("{}.pdf", fn_base), body_font_size,
                body_font_file, header_font_file, header_bold_font_file, use_utf8);
    }
}
