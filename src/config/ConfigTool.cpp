#include "SimpleConfig.h"
#include <vector>
#include <iostream>

int main(int argc, char *argv[])
{
    std::string     value;
    const char      *key;

    if (argc < 3) {
        std::cerr << "format: " << argv[0] << " FILE KEY" << std::endl;
        return 0;
    }

    SimpleConfig cfg(argv[1]);

    key = argv[2];
    cfg.getValue(key, value);
    std::cout << key << " = " << value << std::endl;

    return 0;
}

// vim: ts=4 sw=4 ai cindent et
