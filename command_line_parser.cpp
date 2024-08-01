#include "command_line_parser.hpp"
#include <iostream>
#include <stdexcept>

CommandLineParser::CommandLineParser()
    : desc("Allowed options"),
      frequency(10.0),
      octave(10),
      amplitude(10.0),
      persistence(1.2),
      lacunarity(4.0),
      verboseMode(false) {
    desc.add_options()
        ("help,h", "produce help message")
        ("frequency,f", po::value<double>(&frequency)->default_value(3.0), "set frequency")
        ("octave,o", po::value<int>(&octave)->default_value(10), "set octave")
        ("amplitude,a", po::value<double>(&amplitude)->default_value(1.0), "set amplitude")
        ("persistence,p", po::value<double>(&persistence)->default_value(0.5), "set persistence")
        ("lacunarity,l", po::value<double>(&lacunarity)->default_value(2.0), "set lacunarity")
        ("verbose,v", po::bool_switch(&verboseMode), "enable verbose mode");
}

void CommandLineParser::parse(int argc, char* argv[]) {
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            throw std::runtime_error("Help requested");
        }
    } catch (const po::error& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        throw;
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

double CommandLineParser::getPersistence() const {
    return persistence;
}

double CommandLineParser::getLacunarity() const {
    return lacunarity;
}

bool CommandLineParser::isVerbose() const {
    return verboseMode;
}
