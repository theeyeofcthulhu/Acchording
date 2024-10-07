/**
  https://github.com/theeyeofcthulhu/jargs.hpp

  jargs.hpp
  Copyright (C) 2024  theeyeofcthulhu

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  JARGS.HPP -- C++ command line argument parser

  Usage example:

    #include <string_view>

    #define JARGS_IMPLEMENTATION
    #include "jargs.hpp"

    int main(int argc, char **argv) {
        bool flag = false;
        std::string_view filename;

        jargs::Parser parser;
        // -f, --flag
        parser.add({'f', "flag", "Set flag", [&flag]() {
            flag = true;
        }});
        // --filename a.out, --filename=a.out
        parser.add({"filename", "Specify filename", [&filename](auto optarg) {
            filename = optarg;
        }});
        // -psomething, -p something
        parser.add({'p', "Print something", [](auto optarg) {
            std::cout << optarg << '\n';
        }});
        parser.add_help("example [args]");

        parser.parse(argc, argv);

        std::cout << "flag: " << flag << '\n';
        std::cout << "filename: " << filename << '\n';
    }

  Output:
    
    $ example -f --filename=a.out
      flag: 1
      filename: a.out
    $ example -fp something
      something
      flag: 1
      filename:
    $ example --help
      Usage: example [args]
        -f, --flag                     Set flag
        --filename ARG                 Specify filename
        -p ARG                         Print something
        -h, --help                     Print help
 */

#ifndef JARGS_HPP
#define JARGS_HPP

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace jargs
{

struct Flag {
    char short_name;
    std::string_view long_name;
    std::string_view description;
    bool expects_value;
    // Functions are copied by value lest we segfault
    std::function<void(std::string_view optarg)> action;

    // With arg

    // short, long
    Flag(char c, std::string_view s, std::string_view desc,
         std::function<void(std::string_view optarg)> f)
                 : short_name(c), long_name(s), description(desc)
                 , expects_value(true), action(f)
    {}
    // long
    Flag(std::string_view s, std::string_view desc,
         std::function<void(std::string_view optarg)> f)
                : Flag('\0', s, desc, f)
    {}
    // short
    Flag(char c, std::string_view desc,
         std::function<void(std::string_view optarg)> f)
                : Flag(c, std::string_view(), desc, f)
    {}

    // Without arg

    // short, long
    Flag(char c, std::string_view s, std::string_view desc, std::function<void()> f)
                 : short_name(c), long_name(s), description(desc)
                 , expects_value(false), action([f](std::string_view optarg){ (void)optarg; f(); })
    {}
    // long
    Flag(std::string_view s, std::string_view desc, std::function<void()> f)
                 : Flag('\0', s, desc, f)
    {}
    // short
    Flag(char c, std::string_view desc, std::function<void()> f)
                 : Flag(c, std::string_view(), desc, f)
    {}
};

class Parser {
public:
    void add(Flag f);
    void add_help(std::string_view usage);
    void parse(int argc, const char *const *argv);
private:
    std::vector<Flag> flags;

    void print_help_page(std::string_view usage);
};

} /* namespace jargs */

#ifdef JARGS_IMPLEMENTATION

#include <algorithm>
#include <cassert>
#include <iostream>
#include <sstream>

namespace jargs
{

void Parser::add(Flag f)
{
    flags.push_back(std::move(f));
}

void Parser::add_help(std::string_view usage)
{
    add({'h', "help", "Print help", [usage, this](){
        print_help_page(usage);
        std::exit(1);
    }});
}

void Parser::parse(int argc, const char *const *argv)
{
    for (int i = 1; i < argc; i++) {
        std::string_view arg = argv[i];

        if (arg.size() >= 3 && arg.starts_with("--")) {
            auto flag = arg.substr(2, arg.find('=')-2);

            auto spec = std::find_if(flags.begin(), flags.end(), [flag](const auto &f){
                    return f.long_name == flag;
                    });

            if (spec == flags.end()) {
                std::cerr << argv[0] << ": unknown option: '--" << flag << "'\n";
                std::exit(1);
            }

            if (spec->expects_value) {
                // --opt=arg
                if (arg.contains('=')) {
                    auto optarg = arg.substr(arg.find('=')+1);
                    if (optarg.size() == 0) {
                        std::cerr << argv[0] << ": option '--" << flag << "' requires an argument\n";
                        std::exit(1);
                    } else {
                        spec->action(optarg);
                    }
                // --opt arg
                } else {
                    if (i == argc-1) {
                        std::cerr << argv[0] << ": option '--" << flag << "' requires an argument\n";
                        std::exit(1);
                    } else {
                        spec->action(argv[++i]);
                    }
                }
            // --opt
            } else {
                spec->action(std::string_view());
            }
        } else if (arg.size() >= 2 && arg.starts_with('-')) {
            auto flag = arg.substr(1);

            for (size_t j = 0; j < flag.size(); j++) {
                char c = flag[j];

                auto spec = std::find_if(flags.begin(), flags.end(), [c](const auto &f){
                        return f.short_name == c;
                        });

                if (spec == flags.end()) {
                    std::cerr << argv[0] << ": unknown option: '-" << c << "'\n";
                    std::exit(1);
                }

                if (spec->expects_value) {
                    // -oarg
                    if (j != flag.size()-1) {
                        spec->action(flag.substr(j+1));
                        break;
                    // -o arg
                    } else {
                        if (i == argc-1) {
                            std::cerr << argv[0] << ": option '-" << c << "' requires an argument\n";
                            std::exit(1);
                        } else {
                            spec->action(argv[++i]);
                        }
                    }
                // -o
                } else {
                    spec->action(std::string_view());
                }
            }
        }
    }
}

void Parser::print_help_page(std::string_view usage)
{
    const size_t lhs_max = 32;

    std::cout << "Usage: " << usage << '\n';
    for (const auto &f : flags) {
        std::stringstream lhs;
        lhs << "  ";
        if (f.short_name)
            lhs << "-" << f.short_name;
        if (f.short_name != '\0' && !f.long_name.empty())
            lhs << ", ";
        if (f.long_name.size())
            lhs << "--" << f.long_name;

        if (f.expects_value)
            lhs << " ARG";
    
        if (lhs.str().size() <= lhs_max) {
            std::cout << lhs.str() << std::string(lhs_max - lhs.str().size(), ' ') << " " << f.description << '\n';
        } else {
            std::cout << lhs.str() << '\n' << std::string(lhs_max, ' ') << " " << f.description << '\n';
        }
    }
}

}

#endif /* JARGS_IMPLEMENTATION */
#endif /* JARGS_HPP */
