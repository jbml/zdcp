#include <limits.h>
#include <cstdio>
#include <cassert>
#include <stdexcept>
#include <getopt.h>

#include <map>
#include <string>

#include "TailTrie.h"

#define TrieType TailTrie
//#define TrieType TrieBase

static void 
searchTrie(const char *index, const char *query, bool prefix)
{
    TrieType                              trie(index);
    std::map<std::string, int>            map;
    std::map<std::string, int>::iterator  it;
  

    if (prefix) {
	    trie.prefixSearch(query, map);
        for (it = map.begin(); it != map.end(); it++) {
	        std::cout << it->first << " = " << it->second << std::endl;
        }
    } else {
	    std::cout << query << " = " << trie.search(query) << std::endl;
    }
}

static int
buildTrie(const char *index, const char *source, bool verbose=false)
{
    FILE        *fp;
    int      i;
    int         val;
    char        str[LINE_MAX];
    TrieType    trie;

    if ((fp = fopen(source, "r")) == NULL)
        THROW_RUNTIME_ERROR(std::string("can not open source ") + source);

#define __X(x) #x    
#define _X(x) __X(x)    
    for(i = 0; ; i++) {
        if (fscanf(fp, "%d %" _X(LINE_MAX) "[^\r\n]", &val, str) == EOF) 
            break;
        trie.insert(str, val);
        
        if (verbose && i % 1000 == 0) 
            std::clog << i << " items processed.\n";
    }
#undef _X
#undef __X
    if (verbose)
        std::clog << i << " items processed.\n";

    trie.save(index);

    return i;
}

static void trieExport(const char *s, int val, void *puser) 
{
    fprintf((FILE *)puser, "%d %s\n", val, s);
}

static void dumpTrie(const char *index, const char *target)
{
    FILE        *fp;
    TrieType    trie(index);

    if (strcmp(target, "-") == 0)
        fp = stdout;
    else if ((fp = fopen(target, "w+")) == NULL)
        THROW_RUNTIME_ERROR(std::string("can not open target ") + target);
    
	trie.explore(trieExport, fp);

    fclose(fp);
}


static void printHelpMessage()
{
	std::cout << "Usage: trie [OPTIONS]\n"
				 "OPTIONS:\n"
				 "        -b|--build            build index, needs -i and -s\n"
				 "        -d|--dump             dump index, needs -i\n"
				 "        -h|--help             help message\n"
				 "        -i|--index            index file\n"
				 "        -s|--source           source file\n"
                 "        -p|--prefix           enable prefix match\n"
				 "        -q|--query QUERY      query index, needs -i\n"
				 "        -v|--verbose          verbose\n"
				 "\n"
				 "Report bugs to detrox@gmail.com\n"
			  << std::endl;
}



int main(int argc, char *argv[])
{
    static struct option long_options[] =
    {
        {"build",       no_argument,        0, 'b'},
        {"dump",        required_argument,  0, 'd'},
        {"help",        no_argument,        0, 'h'},
        {"prefix",      no_argument,        0, 'p'},
        {"query",       required_argument,  0, 'q'},
        {"index",       required_argument,  0, 'i'},
        {"source",      required_argument,  0, 's'},
        {"verbose",     no_argument,        0, 'v'},

        {0, 0, 0, 0}
    };

	enum Action {
		ACTION_NO = 0,
		ACTION_BUILD,
		ACTION_DUMP,
		ACTION_QUERY,
        ACTION_QUERY_PREFIX,
		ACTION_INFO 
	} action = ACTION_NO;

	int                 option_index;
    int                 option;

    bool                verbose    = false;
    bool                prefix     = false;
	const char          *index     = NULL;
    const char          *source    = NULL;
    const char          *query     = NULL;
    const char          *dump      = NULL;

    while (true) {
        option = getopt_long(argc, argv, "bd:hpq:i:s:v",
                             long_options, &option_index);
        if (option == -1)
            break;

		switch(option) {
			case 'b':
				action = ACTION_BUILD;
				break;
			case 'd':
				action = ACTION_DUMP;
				dump = optarg;
				break;
			case 'h':
				printHelpMessage();
                return 0;
			case 'p':
                prefix = true;
                break;
			case 'q':
				action = ACTION_QUERY;
				query = optarg;
				break;
			case 'i':
				index = optarg;
				break;
			case 's':
				source = optarg;
				break;
			case 'v':
				verbose = true;
				break;
		}
    }

	if (action == ACTION_BUILD && index && source) {
		buildTrie(index, source, verbose);
	} else if (action == ACTION_QUERY && index && query) {
		searchTrie(index, query, prefix);
	} else if (action == ACTION_DUMP && index && dump) {
		dumpTrie(index, dump);
	} else if (action == ACTION_INFO && index) {
		//_info(index);
	} else {
		printHelpMessage();
	}

    return 0;
}


// vim: ts=4 sw=4 cindent et
