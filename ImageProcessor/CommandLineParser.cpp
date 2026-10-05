/**
 * @file CommandLineParser.cpp
 */

#include "CommandLineParser.h"
#include "Exceptions.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

namespace ip {

namespace {
    /// argv 의 다음 인자를 안전하게 가져온다.
    std::string nextArg(int argc, char* argv[], int& i, const std::string& flag) {
        if (i + 1 >= argc) {
            throw ArgumentError(flag + ": missing value");
        }

        return argv[++i];
    }
} // anonymous namespace

ProgramOptions CommandLineParser::parse(int argc, char* argv[]) {
    ProgramOptions options;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg == "--input" || arg == "-i") {
            options.inputPath = nextArg(argc, argv, i, arg);
        }
        else if (arg == "--output" || arg == "-o") {
            options.outputPath = nextArg(argc, argv, i, arg);
        }
        else if (arg == "--filter" || arg == "-f") {
            options.filterName = nextArg(argc, argv, i, arg);
        }
        else if (arg == "--pipeline" || arg == "-p") {
            std::string filterString = nextArg(argc, argv, i, arg);

            std::vector<std::string> filters;
            std::string              filterWord;
            std::stringstream        ss(filterString);
            
            while (std::getline(ss, filterWord, ',')) {
                filterWord.erase(
                    std::remove(filterWord.begin(), filterWord.end(), ' '),
                    filterWord.end()
                );
                filters.push_back(filterWord);
            }

            if (filters.size() == 0) {
                throw ArgumentError(arg + ": Filter Parsing Error");
            }

            options.filterNames = filters;
        }
        else if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            std::exit(0);
        }
        else {
            throw ArgumentError("Unknown option: " + arg);
        }
    }

    // 필수 인자 검증
    if (options.inputPath.empty()) {
        throw ArgumentError("--input is required");
    }
    if (options.outputPath.empty()) {
        throw ArgumentError("--output is required");
    }
    if (options.filterName.empty() && options.filterNames.size() == 0) {
        throw ArgumentError("--filter is required");
    }
    return options;
}

void CommandLineParser::printUsage(const std::string& exeName) {
    std::cout
        << "Usage:\n"
        << "  " << exeName << " --input <path> --output <path> --filter <name>\n\n"
        << "Options:\n"
        << "  -i, --input   <path>   Input BMP file (24-bit, uncompressed)\n"
        << "  -o, --output  <path>   Output BMP file\n"
        << "  -f, --filter  <name>   Filter to apply (e.g. grayscale, threshold:128)\n"
        << "  -h, --help             Show this message\n\n"
        << "  -p, --pipeline  '<name>, <name>...'   Filter to apply (e.g. 'grayscale, blur')\n"
        << "Examples:\n"
        << "  " << exeName << " -i input.bmp -o result.bmp -f grayscale\n"
        << "  " << exeName << " -i input.bmp -o result.bmp -f threshold:128\n";
}

} // namespace ip
