
#pragma once


struct CommandLineOptions {
    float windowWidth;  // unsigned casted to a float
    float windowHeight; // unsigned casted to a float
    float imageScale;
    bool verbose;

    CommandLineOptions(int argc, const char* argv[]);
};
