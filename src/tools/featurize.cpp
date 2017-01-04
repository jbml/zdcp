#include <iostream>
#include <fstream>
#include <string>
#include <bamboo/bamboo.hxx>
#include <trie/Trie.h>
#include <getopt.h>

static void printHelpMessage()
{
	std::cout << "Usage: featurize [OPTIONS] FILE\n"
				 "OPTIONS:\n"
				 "        -f|--feature           feature index\n"
                 "        -h|--help              this message\n" 
				 "        -a|--showall           show all words\n"
				 "\n"
				 "Report bugs to detrox@gmail.com\n"
			  << std::endl;
}

void featurize(bamboo::Parser *parser, Trie *trie, const char *text, int showall)
{
    std::vector<bamboo::Token *> tokenizedTerms;
    std::vector<bamboo::Token *>::const_iterator it;

	parser->setopt(BAMBOO_OPTION_TEXT, text);
	parser->parse(tokenizedTerms);

	for (it = tokenizedTerms.begin(); it != tokenizedTerms.end(); it++) {
		const char *token = (*it)->get_orig_token();
		if (trie->search(token)) {
			std::cout << "[" << token << "] ";
		} else {
			if (showall)
				std::cout <<  token << " ";
		}
	}

	std::cout << std::endl;
	
}

int main(int argc, char *argv[])
{
    static struct option long_options[] =
    {
        {"feature",      required_argument,  0, 'f'},
        {"showall",     no_argument,        0, 'a'},
        {"help",        no_argument,        0, 'h'},
        {0, 0, 0, 0}
    };

    const char *            featureFile = "feature.idx";
	int					    option, option_index = 0;
	int						showall = 0;

    while (true) {
        option = getopt_long(argc, argv, "af:h",
                             long_options, &option_index);
        if (option == -1)
            break;

		switch(option) {
			case 'f':
                featureFile = optarg;
				break;
			case 'a':
				showall = 1;
				break;
            case 'h':
                printHelpMessage();
                return 0;
        }
    }

	std::ifstream ifs;
    bamboo::ParserFactory *factory;
	Trie *trie;

    factory = bamboo::ParserFactory::get_instance();
	trie = new Trie(featureFile);

	if (optind < argc) {
		ifs.open(argv[optind]);
		if (ifs.is_open()) {
			std::string line;
			while(!ifs.eof()) {
				std::getline(ifs, line);	
				featurize(factory->create("crf_seg"), trie, line.c_str(), showall);
			}
		}
	} else {
		printHelpMessage();
	}

	delete trie;
}
