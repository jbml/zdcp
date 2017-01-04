CXX=g++ -O2 -Wall -fPIC -Isrc $(shell pcre-config --cflags) 
CC=gcc -O2 -Wall -fPIC -Isrc $(shell pcre-config --cflags) 

all: bin/trie bin/config bin/zdcp bin/featurize lib/libzdcp.so.1.0

bin/trie: src/trie/TrieBase.o src/trie/TailTrie.o src/trie/TrieTool.o
	$(CXX) $(CFLAGS) -o $@ $^

bin/config: src/config/ConfigTool.cpp
	$(CXX) $(CFLAGS) -o $@ $^

bin/zdcp: src/document/DcpParser.o src/document/DocParser.o src/document/DocParserFactory.o src/config/SimpleConfig.o src/tools/zdcptool.o src/feature/FeatureSelector.o src/feature/ChiSquareSelector.o src/feature/MISelector.o src/feature/WETSelector.o src/feature/FeatureSelectorFactory.o  src/trie/TrieBase.o src/trie/TailTrie.o src/calculator/WeightCalculatorFactory.o src/calculator/TFWeightCalculator.o src/calculator/TFIDFWeightCalculator.o src/calculator/TWWeightCalculator.o src/learner/SvmLearner.o src/learner/LearnerFactory.o src/libsvm/svm.o src/liblbfgs/LBFGS.o src/learner/LogLinearLearner.o src/classifier/Classifier.o
	$(CXX) $(CFLAGS) -o $@ $^ -lbamboo -lm -ldl $(shell pcre-config --libs)

bin/featurize: src/tools/featurize.cpp src/trie/TrieBase.o src/trie/TailTrie.o
	$(CXX) $(CFLAGS) -o $@ $^ -lbamboo -ldl 

lib/libzdcp.so.1.0: src/lib/zdcp.o src/classifier/Classifier.o src/document/DcpParser.o src/document/DocParser.o src/document/DocParserFactory.o src/config/SimpleConfig.o src/feature/FeatureSelector.o src/feature/ChiSquareSelector.o src/feature/MISelector.o src/feature/WETSelector.o src/feature/FeatureSelectorFactory.o  src/trie/TrieBase.o src/trie/TailTrie.o src/calculator/WeightCalculatorFactory.o src/calculator/TFWeightCalculator.o src/calculator/TFIDFWeightCalculator.o src/calculator/TWWeightCalculator.o src/learner/SvmLearner.o src/learner/LearnerFactory.o src/libsvm/svm.o src/liblbfgs/LBFGS.o src/learner/LogLinearLearner.o
	$(CXX) $(CFLAGS) -shared -Wl,-soname,libzdcp.so.1 -o $@ $^ -lbamboo -lm -ldl $(shell pcre-config --libs)

clean:
	find -name "*.o" -type f -exec rm -rf {} \;
	rm -rf bin/*
