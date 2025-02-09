#include "../include/full_sys.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include "../include/full_sys.h"

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        std::cerr << "Usage: " << argv[0] << " <vmh_file> <ram_accurate (true/false)> <with_decoder (true/false)>\n";
        return 1;
    }

    // Parse command-line arguments
    std::string vmh_file = argv[1];
    bool ram_accurate = (std::string(argv[2]) == "true");
    bool with_decoder = (std::string(argv[3]) == "true");

    // Run the full system
    try
    {
        run_full_system(argv[1], ram_accurate, with_decoder);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
