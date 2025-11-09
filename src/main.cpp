#include "aurora/assembler.hpp"
#include "aurora/emulator.hpp"
#include "aurora/utils.hpp"

#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace {

void printUsage() {
    std::cout << "Aurora-16 Software CPU\n"
              << "Usage:\n"
              << "  softcpu assemble <source.asm> -o <program.bin> [--origin 0x0000]\n"
              << "  softcpu run <program.bin> [--origin 0x0000] [--entry 0x0000] [--cycles N] [--trace]\n"
              << "  softcpu dump <program.bin> --start 0x0000 --length 64 [--origin 0x0000]\n";
}

std::optional<std::uint16_t> parseWord(const std::string& text) {
    if (auto value = aurora::util::parseNumber(text)) {
        return static_cast<std::uint16_t>(*value & 0xFFFF);
    }
    return std::nullopt;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    const std::string command = argv[1];
    if (command == "assemble") {
        std::string input;
        std::string output = "a.bin";
        std::uint16_t origin = aurora::kResetVector;
        for (int i = 2; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "-o" || arg == "--output") {
                if (i + 1 >= argc) {
                    std::cerr << "missing output path\n";
                    return 1;
                }
                output = argv[++i];
            } else if (arg == "--origin") {
                if (i + 1 >= argc) {
                    std::cerr << "missing origin value\n";
                    return 1;
                }
                auto value = parseWord(argv[++i]);
                if (!value) {
                    std::cerr << "invalid origin\n";
                    return 1;
                }
                origin = *value;
            } else if (arg == "--help") {
                printUsage();
                return 0;
            } else if (!arg.empty() && arg[0] == '-') {
                std::cerr << "unknown option: " << arg << '\n';
                return 1;
            } else {
                input = arg;
            }
        }
        if (input.empty()) {
            std::cerr << "assemble requires an input file\n";
            return 1;
        }
        aurora::Assembler assembler;
        aurora::AssemblerOptions options;
        options.origin = origin;
        const auto result = assembler.assembleFile(input, options);
        for (const auto& message : result.messages) {
            std::cerr << message << '\n';
        }
        if (!result.ok) {
            return 1;
        }
        if (!aurora::util::writeBinaryFile(output, result.bytes)) {
            std::cerr << "failed to write " << output << '\n';
            return 1;
        }
        std::cout << "Wrote " << result.bytes.size() << " bytes to " << output << '\n';
        return 0;
    }

    if (command == "run") {
        std::string program_path;
        std::uint16_t origin = aurora::kResetVector;
        std::uint16_t entry = aurora::kResetVector;
        std::uint64_t cycles = 0;
        bool trace = false;
        for (int i = 2; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--origin") {
                if (i + 1 >= argc) {
                    std::cerr << "missing origin value\n";
                    return 1;
                }
                auto value = parseWord(argv[++i]);
                if (!value) {
                    std::cerr << "invalid origin\n";
                    return 1;
                }
                origin = *value;
                if (entry == aurora::kResetVector) {
                    entry = origin;
                }
            } else if (arg == "--entry") {
                if (i + 1 >= argc) {
                    std::cerr << "missing entry value\n";
                    return 1;
                }
                auto value = parseWord(argv[++i]);
                if (!value) {
                    std::cerr << "invalid entry\n";
                    return 1;
                }
                entry = *value;
            } else if (arg == "--cycles") {
                if (i + 1 >= argc) {
                    std::cerr << "missing cycle limit\n";
                    return 1;
                }
                cycles = std::strtoull(argv[++i], nullptr, 0);
            } else if (arg == "--trace") {
                trace = true;
            } else if (arg == "--help") {
                printUsage();
                return 0;
            } else if (!arg.empty() && arg[0] == '-') {
                std::cerr << "unknown option: " << arg << '\n';
                return 1;
            } else {
                program_path = arg;
            }
        }
        if (program_path.empty()) {
            std::cerr << "run requires a binary image\n";
            return 1;
        }
        aurora::Emulator emulator;
        emulator.reset();
        if (!emulator.loadBinaryFile(program_path, origin)) {
            std::cerr << "unable to load " << program_path << '\n';
            return 1;
        }
        emulator.registers().pc = entry;
        aurora::RunOptions run_options;
        run_options.cycle_limit = cycles;
        run_options.trace = trace;
        if (!emulator.run(run_options)) {
            std::cerr << "execution stopped due to fault\n";
            return 1;
        }
        return 0;
    }

    if (command == "dump") {
        std::string program_path;
        std::uint16_t origin = aurora::kResetVector;
        std::uint16_t start = 0;
        std::size_t length = 64;
        bool have_start = false;
        bool have_length = false;
        for (int i = 2; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--origin") {
                if (i + 1 >= argc) {
                    std::cerr << "missing origin value\n";
                    return 1;
                }
                auto value = parseWord(argv[++i]);
                if (!value) {
                    std::cerr << "invalid origin\n";
                    return 1;
                }
                origin = *value;
            } else if (arg == "--start") {
                if (i + 1 >= argc) {
                    std::cerr << "missing start value\n";
                    return 1;
                }
                auto value = parseWord(argv[++i]);
                if (!value) {
                    std::cerr << "invalid start address\n";
                    return 1;
                }
                start = *value;
                have_start = true;
            } else if (arg == "--length") {
                if (i + 1 >= argc) {
                    std::cerr << "missing length value\n";
                    return 1;
                }
                length = std::strtoul(argv[++i], nullptr, 0);
                have_length = true;
            } else if (!arg.empty() && arg[0] == '-') {
                std::cerr << "unknown option: " << arg << '\n';
                return 1;
            } else {
                program_path = arg;
            }
        }
        if (program_path.empty() || !have_start || !have_length) {
            std::cerr << "dump requires binary file, --start and --length\n";
            return 1;
        }
        aurora::Emulator emulator;
        emulator.reset();
        if (!emulator.loadBinaryFile(program_path, origin)) {
            std::cerr << "unable to load " << program_path << '\n';
            return 1;
        }
        if (!emulator.dumpToStdout(start, length)) {
            std::cerr << "dump request outside memory bounds\n";
            return 1;
        }
        return 0;
    }

    printUsage();
    return 1;
}
