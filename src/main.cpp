#include <getopt.h>

#include <fmt/core.h>

#define JARGS_IMPLEMENTATION
#include "jargs.hpp"

#include "file.hpp"

int main(int argc, char **argv)
{
    if (argc < 2) {
        fmt::print(stderr, "Please specify a file.\n");
        return 1;
    }

    bool pdf = false;

    FileFormatter ff;

    jargs::Parser parser;
    parser.add({'p', "pdf", "Generate PDF", [&pdf]() {
        pdf = true;
    }});
    parser.add({'s', "size", "Specify font size", [&ff](auto optarg) {
        ff.put_metadata(FF_SIZE, optarg);
    }});
    parser.add({'b', "body-font", "Specify font \"name[:style]\" for PDF body", [&ff](auto optarg) {
        ff.put_metadata(FF_BODY_FONT, optarg);
    }});
    parser.add({'t', "title-font",
            "Specify font \"name\" for PDF header; should have 'Regular' and 'Bold' styles",
            [&ff](auto optarg) {
        ff.put_metadata(FF_TITLE_FONT, optarg);
    }});
    parser.add({'u', "utf8", "Use UTF-8 in PDF generation", [&ff]() {
        ff.put_metadata(FF_UTF8, "true");
    }});
    parser.add_help("acchording [args] file");

    parser.parse(argc, argv);

    const char *fn = argv[argc-1];
    ff.init(fn);

    if (!pdf) {
        ff.print_formatted_txt();
    } else {
        std::string_view fn_base(fn);
        fn_base = fn_base.substr(0, fn_base.rfind('.'));

        ff.print_formatted_pdf(fmt::format("{}.pdf", fn_base));
    }
}
