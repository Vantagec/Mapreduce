#include <format>
#include <iostream>

#include <boost/program_options.hpp>

#include "log.hpp"
#include "mapreduce.hpp"

#include "project.h"

int
main(int argc, char const *argv[]) {
    using namespace otus;

    struct ProjectInfo info = {};
    Log::Get().SetSeverity(Log::INFO);
    Log::Get().Info("{}\t{}", info.nameString, info.versionString);

    namespace po = boost::program_options;
    po::options_description desc(
        "Find prefixes from lines of file using mapreduce technique");
    desc.add_options()("help", "Produce this help message")(
        "input,i", po::value<std::string>(), "Input filename to process")(
        "mappers,m", po::value<int>(), "Amount of the mappers threads")(
        "reducers,r", po::value<int>(), "Amount of the reducers threads")(
        "debug,d", po::value<bool>(), "Enable debug output");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    if (vm.count("debug")) {
        Log::Get().SetSeverity(Log::DEBUG);
    }

    std::string file;
    if (!vm.count("input")) {
        Log::Get().Error("No input was provided!\n");
        std::cout << desc << "\n";
        return 2;
    }
    file = vm["input"].as<std::string>();

    int mp_count = 3, rd_count = 2;
    if (vm.count("mappers") && vm.count("reducers")) {
        mp_count = vm["mappers"].as<int>();
        rd_count = vm["reducers"].as<int>();
    }

    Log::Get().Info("Using {} mappers, {} reducers with \"{}\" file", mp_count,
                    rd_count, file);
    PrefixFindRunner mr(mp_count, rd_count);

    bool is_unique_found = false;
    unsigned result = 1;
    while (!is_unique_found && result < 4) {
        mr.set_mapper([result](const std::string &line) {
            PrefixFindRunner::mapper_out out;
            std::string sub_string = line.substr(0, result);

            // std::string sub_string;
            // for (size_t i = 0; i < line.size() && i < result; ++i) {
            //     sub_string += line[i];
            //     out.push_back({sub_string, 1});
            // }

            out.push_back({sub_string, 1});
            return out;
        });
        mr.set_reducer([](const PrefixFindRunner::mapper_chunk &chunk) {
            static bool is_inited = false;
            static PrefixFindRunner::mapper_chunk previous;
            if (!is_inited) {
                previous = chunk;
                is_inited = true;
                return true;
            }

            bool result = true;
            if (!chunk.first.compare(previous.first) || chunk.second > 1) {
                result = false;
            }

            previous = chunk;
            return result;
        });

        if (mr.run(file, std::format("out/iter{}", result))) {
            break;
        }
        result++;
    }

    Log::Get().Info("Result = {}", result);
    return 0;
}
