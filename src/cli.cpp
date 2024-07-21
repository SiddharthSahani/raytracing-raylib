
#include "cli.h"
#include <argparse/argparse.hpp>


CommandLineOptions::CommandLineOptions(int argc, const char* argv[]) {
    argparse::ArgumentParser parser("realtime-raytracing", "0.1");

    parser.add_argument("-w", "--windowWidth")
        .help("Window width")
        .default_value(1280u)
        .scan<'u', unsigned>();

    parser.add_argument("-h", "--windowHeight")
        .help("Window height")
        .default_value(720u)
        .scan<'u', unsigned>();

    parser.add_argument("-s", "--scale")
        .help("Window to image ratio")
        .default_value(2.0f)
        .scan<'f', float>();

    parser.add_argument("--verbose")
        .help("Enable verbose logging")
        .default_value(false)
        .implicit_value(true);

    try {
        parser.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << '\n';
        std::cerr << parser << '\n';
        exit(1);
    }

    windowWidth = parser.get<unsigned>("windowWidth");
    windowHeight = parser.get<unsigned>("windowHeight");
    imageScale = parser.get<float>("scale");
    verbose = parser.get<bool>("verbose");
}
