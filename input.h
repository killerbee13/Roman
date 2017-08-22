#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

#include <vector>
#include <array>
#include <functional>
#include <algorithm>
#include <memory>
#include <cstdint>

//#include "FRC.h"

//Polymorphic input processing, yay!

class input_indexed_trie;

//Function data
class functdata {
public:
	typedef std::function<std::vector<std::string>(std::vector<std::string>)> inputProcessor;
	enum Mode {Batch, Single, FSingle, Composite, } mode;
	//How does the function handle arguments:
		//Batch: (I) Loops over arbitrarily long input (range is enforced)
		//Single: (E) Takes only [argN, argX] inputs at a time
		//FSingle: (N) Force only [argN, argX] inputs total
		//Composite: Inherit
	//Argument Looping Contracts:
	//(I): Internal looping (Handle "extra" arguments internally)
	//(E): External looping (Function will be executed once for each argument group)
	//(N): Never loop, ignore extra arguments (Function will get exactly as many arguments as it asks for)
	functdata()
		: mode(Mode::Batch), argC(0), argN(0), argX(-1) {;}
	functdata(inputProcessor c, int a, std::string h)
		: mode(Mode::Batch), argC(a), argN(a), argX(-1), call(c), help(h) {;}
	functdata(inputProcessor c, unsigned int n, unsigned int x, std::string h, Mode m)
		: mode(m), argC(n), argN(n), argX(x), call(c), help(h) {;}
	functdata(std::string c)
		: mode(Mode::Composite), argC(0), argN(0), argX(0), 
			call([](std::vector<std::string> inList){return inList;}),
			help(c) {;}
	std::vector<std::string> operator()(std::vector<std::string> in)
		{return call(in);}
	static std::string modeS(Mode);
	int argC;
	//Args: miN, maX
	unsigned int argN, argX;
	inputProcessor call;
	std::string help;
};

//indexed trie for input lookup
//All data is ASCII (char & 127)
class input_indexed_trie {
public:
	//Search trie, also get match length
	functdata* longestPrefix(std::string key, int* len);
	//Find all (valid) terminal nodes downstream from key
	std::vector<std::string> allWithPrefix(std::string key);
	//Add key to index, adding extra rows to the end
	void insert(std::string key, functdata* data);
	//Efficient (?, sorted) insert
	void insert(std::vector<std::pair<std::string, functdata*>> data);
	//Clear data for key, leave extra paths
	///@UNTESTED
	void fast_remove(std::string key);
	//Clear superfluous entries and possibly rebuild index
	///@TODO [0]-MEM reduce() is NYI
	void reduce();
	//Clear data for key, possibly rebuilding index
	void remove(std::string key)
		{ fast_remove(key);
			static uint_fast8_t del_count = 0;
			//reduce() every (reduce_every) remove()s
			if (!isGraph && (((++del_count)%reduce_every)==0)) {reduce();} }
	//Default constructor
	input_indexed_trie() : index(1), isGraph(0) {;}
	//insert constructor
	input_indexed_trie(std::string key, functdata* data)
		: index(1), isGraph(0) {insert(key,data);}
	//Vector constructor
	input_indexed_trie(std::vector<std::pair<std::string, functdata*>> data)
		: index(1), isGraph(0) {insert(data);}
	
	//Run command in trie, returning outputs
	std::vector<std::string> execute(std::vector<std::string> command, std::size_t* consumed=nullptr);
	
	//debug access, returns value-copy of internal index
	std::vector<std::array<std::pair<int, functdata*>, 128>> getIndex()
		{return index;}
private:
	//Get descendents of node
	std::vector<std::string> i_getNodes(unsigned int p, std::string prefix);
	
	constexpr static std::array<std::pair<int, functdata*>, 128> mapNull()
		{return std::array<std::pair<int, functdata*>, 128>();};
	std::vector<std::array<std::pair<int, functdata*>, 128>> index;
	//If the index is "packed" and/or has (directed or un-) cycles
		//Set for any configuration of the tree that may impact reduce()
	bool isGraph;
	const static uint_fast8_t reduce_every = 10;
};

typedef input_indexed_trie input_trie;

input_trie initTrie();

std::vector<std::string> tokenizeInput(std::string);
std::string deTokenize(std::vector<std::string>);

//Sanitize to alphabeticals
std::string sanitizeAlpha(std::string);

//Sanitize to alphanumerics
std::string sanitizeAN(std::string);

bool truthy(std::string);

#endif //INPUT_H_INCLUDED