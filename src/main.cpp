#include <getopt.h>

#include <fmt/core.h>

#include "file.hpp"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fmt::print(stderr, "Please specify a file.\n");
        return 1;
    }

    bool pdf = false;
    int body_font_size = 11;
    std::string body_font = "fonts/Inconsolata-Regular.ttf";
    bool use_utf8 = false;

    int flag;
    while ((flag = getopt(argc, argv, "ps:f:u")) != -1) {
        switch (flag) {
        case 'p':
            pdf = true;
            break;
        case 's':
            body_font_size = std::stoi(optarg);
            break;
        case 'f':
            body_font = optarg;
            break;
        case 'u':
            use_utf8 = true;
            break;
        case '?':
        default:
            return 1;
        }
    }

    const char *fn = argv[argc-1];

    FileFormatter ff(fn);

    if (!pdf) {
        ff.print_formatted_txt();
    } else {
        std::string_view fn_base(fn);
        fn_base = fn_base.substr(0, fn_base.rfind('.'));

        ff.print_formatted_pdf(fmt::format("{}.pdf", fn_base), body_font_size, body_font, use_utf8);
    }
}
