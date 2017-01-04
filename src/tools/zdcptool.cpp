#include <iostream>
#include <getopt.h>

#include "classifier/Classifier.h"


#define FL_VECTOR 1

using namespace zdcp;

static void printHelpMessage()
{
	std::cout << "Usage: dcp [OPTIONS] FILE\n"
				 "OPTIONS:\n"
				 "        -c|--config            configuration file\n"
                 "        -s|--set PARAMTERS     override configuration\n"
                 "        -l|--learn             learning\n"
                 "        -h|--help              this message\n" 
				 "\n"
				 "Report bugs to detrox@gmail.com\n"
			  << std::endl;
}

int main(int argc, char *argv[])
{
    static struct option long_options[] =
    {
        {"config",      required_argument,  0, 'c'},
        {"set",         required_argument,  0, 's'},
        {"load",        no_argument,        0, 'l'},
        {"vector",      no_argument,        0, 'v'},
        {"help",        no_argument,        0, 'h'},

        {0, 0, 0, 0}
    };

    const char *            configFile = "config.cfg";
    const char *            file;
    int                     option;
    int                     option_index;
    int                     flag = 0;

    enum {RM_LEARN = 0, RM_PREDICT} mode = RM_PREDICT;

    while (true) {
        option = getopt_long(argc, argv, "c:s:lhv",
                             long_options, &option_index);
        if (option == -1)
            break;

		switch(option) {
			case 'c':
                configFile = optarg;
				break;
            case 'h':
                printHelpMessage();
                return 0;
            case 'l':
                mode = RM_LEARN;
                break;
            case 'v':
                flag |= FL_VECTOR;
                break;
        }
    }

    if (optind < argc) {
        std::string option;
        file = argv[optind];
        if (flag & FL_VECTOR)
            option.append("generic.showvector = 1");
        Classifier *classifier = new Classifier(configFile, option.c_str());
        if (mode == RM_LEARN) {
            classifier->train(file);
        } else {
            ifstream fin(file);
            if (fin.is_open()) {
                std::string line;
                const double *score;
                int i, num;
                bool first = true;
                std::cout.precision(3);
                while (!fin.eof()) {
                    std::getline(fin, line);
                    if (line.empty()) continue;
                    num = classifier->classify(line.c_str(), &score);
                    if (first) {
                        std::cout << "class\t";
                        for (i = 1; i <= num; i++) 
                            std::cout << classifier->getClassNameById(i) << "\t";
                        first = false;
                        std::cout << std::endl;
                    }
                    std::cout << classifier->getClassNameById((int)score[0]) << "\t";
                    for (i = 1; i <= num; i++) {
                        std::cout << score[i] * 100 << "%\t";
                    }
                    std::cout << std::endl;
                }    
            } else {
                std::cerr << "can not open file " << file << std::endl;
            }
        }
        delete classifier;
    } else {
        printHelpMessage();
    }

}


// vim: ts=4 sw=4 ai cindent et
