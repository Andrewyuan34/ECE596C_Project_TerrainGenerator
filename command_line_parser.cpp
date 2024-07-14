#include "command_line_parser.h"
#include <iostream>

CommandLineParser::CommandLineParser()
    : desc("Allowed options"), frequency(440.0), octave(5), amplitude(1.0), verboseMode(false)
{
    desc.add_options()
        ("help", "produce help message")
        ("frequency,f", po::value<double>(&frequency)->default_value(440.0), "set frequency")
        ("octave,o", po::value<int>(&octave)->default_value(5), "set octave")
        ("amplitude,a", po::value<double>(&amplitude)->default_value(1.0), "set amplitude")
        ("verbose,v", "enable verbose mode")
    ;
}

void CommandLineParser::parse(int argc, char* argv[]) {
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            // Optionally throw an exception or handle help output here.
        }

        if (vm.count("verbose")) {
            verboseMode = true;
        }
    } catch(const std::exception& e) {
        std::cerr << "Error parsing command line options: " << e.what() << "\n";
        // Optionally handle the error here.
    }
}

double CommandLineParser::getFrequency() const {
    return frequency;
}

int CommandLineParser::getOctave() const {
    return octave;
}

double CommandLineParser::getAmplitude() const {
    return amplitude;
}

bool CommandLineParser::isVerbose() const {
    return verboseMode;
}

