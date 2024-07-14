#ifndef COMMAND_LINE_PARSER_H
#define COMMAND_LINE_PARSER_H

#include <boost/program_options.hpp>

namespace po = boost::program_options;

class CommandLineParser {
public:
    CommandLineParser();

    void parse(int argc, char* argv[]);

    double getFrequency() const;
    int getOctave() const;
    double getAmplitude() const;
    bool isVerbose() const;

private:
    po::options_description desc;
    po::variables_map vm;

    double frequency;
    int octave;
    double amplitude;
    bool verboseMode;
};

#endif // COMMAND_LINE_PARSER_H

