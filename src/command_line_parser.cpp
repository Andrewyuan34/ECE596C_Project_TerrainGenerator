#include "command_line_parser.hpp"
#include <iostream>
#include <stdexcept>

CommandLineParser::CommandLineParser()
    : desc("Allowed options"),
      frequency(3.0),
      octave(10),
      amplitude(0.8),
      persistence(0.5),
      lacunarity(2.0) {
    desc.add_options()
        ("help,h", "produce help message")
        ("frequency,f", po::value<double>(&frequency)->default_value(3.0), "set frequency   Range: 1~5       Step: 1") // around 3 looks good
        ("octave,o", po::value<int>(&octave)->default_value(10), "set octave      Range: 2~20      Step: 1") // around 10 looks good
        ("amplitude,a", po::value<double>(&amplitude)->default_value(0.5), "set amplitude   Range: 0.4~0.8   Step: 0.01") // around 0.5 looks good
        ("persistence,p", po::value<double>(&persistence)->default_value(0.5), "set persistence Range: 0.4~0.6   Step: 0.01") // around 0.5 looks good
        ("lacunarity,l", po::value<double>(&lacunarity)->default_value(2.0), "set lacunarity  Range: 1~3       Step: 0.1") // around 2 looks good
        ("width,w", po::value<int>(&width)->default_value(6), "set width Range: 1~13   Step: 1" ) // The larger the width, the more detailed the terrain
        ("step,t", po::value<int>(&step)->default_value(1), "set step Range: 0~5   Step:1" )// The larger the step, the less detailed the terrain
        ("seed,s", po::value<int>(&seed)->default_value(42), "set seed");
}

void CommandLineParser::parse(int argc, char* argv[]) {
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
        if (vm.count("help")) {
            std::cout << desc << "\n";
        }

        // Check if the values are within the specified range
        if (frequency < 1.0 || frequency > 5.0) {
            throw std::out_of_range("Frequency must be between 1 and 5.");
        }
        if (octave < 2 || octave > 20) {
            throw std::out_of_range("Octave must be between 2 and 20.");
        }
        if (amplitude < 0.4 || amplitude > 0.8) {
            throw std::out_of_range("Amplitude must be between 0.4 and 0.8.");
        }
        if (persistence < 0.4 || persistence > 0.6) {
            throw std::out_of_range("Persistence must be between 0.4 and 0.6.");
        }
        if (lacunarity < 1.0 || lacunarity > 3.0) {
            throw std::out_of_range("Lacunarity must be between 1 and 3.");
        }
        if (width < 1 || width > 13) {
            throw std::out_of_range("Width must be between 1 and 13.");
        }
        if (step < 0 || step > 5) {
            throw std::out_of_range("Step must be between 0 and 5.");
        }
    } catch (const po::error& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        throw;
    } catch (const std::out_of_range& ex) {
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

int CommandLineParser::getSeed() const {
    return seed;
}

int CommandLineParser::getWidth() const {
    return width;
}

int CommandLineParser::getStep() const {
    return step;
}


