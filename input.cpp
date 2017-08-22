#include "input.h"

#include <exception>
#include <cctype>

#include <iostream>
#include <deque>
#include <utility>
#include <tuple>

//Search trie, also get match length
functdata* input_indexed_trie::longestPrefix(std::string key, int* len)
{
	if (key.empty())
		return nullptr;
	unsigned int p = 0, i = 0;
	functdata *r = nullptr;
	if (len)
		*len = 0;
	for (; i != key.length() - 1; ++i) {
		//std::cout<<"key["<<i<<"]:"<<key[i]<<";p:"<<p;
		//Limit to ASCII, NUL becomes DEL
		(key[i] &= 127) ? false : key[i] = 127;
		//Keep track of last good return value
		if (index[p][key[i]].second) {
			r = index[p][key[i]].second;
			if (len)
				(*len) = i+1;
		}
		if (index[p][key[i]].first == 0) {
			//std::cout<<";no child"<<std::endl;
			return r;
		}
		p = index[p][key[i]].first;
		//std::cout<<";new p:"<<p<<std::endl;
	}
	//std::cout<<"key["<<i<<"]:"<<key[i]<<";p:"<<p<<std::endl;
	if (index[p][key[i]].second) {
		r = index[p][key[i]].second;
		if (len)
			(*len) = i+1;
	}
	return r;
}

//Add key to index, adding extra rows to the end
void input_indexed_trie::insert(std::string key, functdata* data)
{
	if (key.empty())
		return;
	unsigned int p = 0, i = 0;
	for (; i != key.length() - 1; ++i) {
		//Limit to ASCII, NUL becomes DEL
		(key[i] &= 127) ? false : key[i] = 127;
		if (index[p][key[i]].first == 0) {
			index[p][key[i]].first = index.size();
			index.push_back(mapNull());
		}
		p = index[p][key[i]].first;
	}
	index[p][key[i]].second = data;
}

//Efficient (sorted) insert (Does this matter?)
void input_indexed_trie::insert(std::vector<std::pair<std::string, functdata*>> data)
{
	//Sort input by strings
	std::sort(data.begin(), data.end(), 
		[](std::pair<std::string, functdata*> a, std::pair<std::string, functdata*> b)
		{return a.first>b.first;});
	for (auto i : data) {
		insert(i.first, i.second);
	}
}

//Find all (valid) terminal nodes downstream from key
std::vector<std::string> input_indexed_trie::allWithPrefix(std::string key)
{
	if (key.empty())
		return i_getNodes(0, key);
	unsigned int p = 0, i = 0;
	//Stop one char early and finish outside loop
	for (; i != key.length() - 1; ++i) {
		(key[i] &= 127) ? false : key[i] = 127;
		if (index[p][key[i]].first == 0) {
			return std::vector<std::string>(); //key is not a prefix in trie
		}
		p = index[p][key[i]].first;
	}
	std::vector<std::string> tmp;
	//If key is a valid node, add it
	if (index[p][key[i]].second) 
		tmp.push_back(key);
	//If key has no children, stop
	if (!index[p][key[i]].first) {
		return tmp;
	}
	p = index[p][key[i]].first;
	auto tmp2 = i_getNodes(p, key);
	tmp.insert(tmp.end(), tmp2.begin(), tmp2.end());
	return tmp;
}

//Get descendents of node
std::vector<std::string> input_indexed_trie::i_getNodes(unsigned int p, std::string prefix)
{
	//Depth-first (recursive) search
	std::vector<std::string> retArray;
	for (unsigned char j = 0; j != 128; ++j) {
		if (index[p][j].first || index[p][j].second) {
		}
		if (index[p][j].second) { //Terminal node, add to output
			retArray.push_back(prefix + static_cast<char>(j));
		}
		if (index[p][j].first) { //Node has children, recurse
			auto tmp = i_getNodes(index[p][j].first, prefix + static_cast<char>(j));
			retArray.insert(retArray.end(), tmp.begin(), tmp.end());
		}
	}
	return retArray;
}

//Clear data for key, leave extra paths
void input_indexed_trie::fast_remove(std::string key)
{
	if (key.empty())
		return;
	unsigned int p = 0, i = 0;
	for (; i != key.length() - 1; ++i) {
		(key[i] &= 127) ? false : key[i] = 127;
		if (index[p][key[i]].first == 0) {
			return; //key is not in trie
		}
		p = index[p][key[i]].first;
	}
	//key (or a string with prefix key) is in trie
	delete (index[p][key[i]].second);
	index[p][key[i]].second = nullptr;
}

//Clear superfluous entries and possibly rebuild index
void input_indexed_trie::reduce()
{
	auto tmp = i_getNodes(0, std::string());
	std::vector<std::pair<std::string, functdata*>> data;
	for (auto i : tmp) {
		data.emplace_back(i, longestPrefix(i, nullptr));
	}
	//Clear index
	index = {mapNull()};
	for (auto i : data) {
		insert(i.first, i.second);
	}
	return;
}


//FSM for input
std::vector<std::string> tokenizeInput(std::string in)
{
	size_t word = 0;
	std::vector<std::string> ret(1);
	int_fast16_t state = 1;
	int_fast16_t states[7][5] = {
	// N, ', ", \, S,
		{1, 2, 3, 4, 0}, //Space
		{1, 2, 3, 4, 0}, //Normal
		{2, 1, 2, 5, 2,}, //'-quote
		{3, 3, 1, 6, 3,}, //"-quote
		{1, 1, 1, 1, 1,}, //Escape (normal)
		{2, 2, 2, 2, 2,}, //Escape ('-quote)
		{3, 3, 3, 3, 3,}, //Escape ("-quote)
	};
	
	//Iterate characters and add to vector
	for (auto i : in) {
		//If we're at the end of a space-group then increase size of vector
		if ((state == 0) && (i != ' ')) {
			word++;
			ret.emplace_back("");
		}
		switch (i) {
		case '\'':
			//if in double-quote or any escape
			if (state == 3 || state > 3) {
				ret[word] += i;
			}
			state = states[state][1];
			break;
		case '"':
			//if in single-quote or any escape
			if (state == 2 || state > 3) {
				ret[word] += i;
			}
			state = states[state][2];
			break;
		case '\\':
			//If in an escape
			if (state > 3) {
				ret[word] += i;
			}
			state = states[state][3];
			break;
		case ' ':
			//If in any kind of quote or escape
			if (state > 1) {
				ret[word] += i;
			}
			state = states[state][4];
			break;
		default:
			//Always add normal characters
			ret[word] += i;
			state = states[state][0];
			break;
		}
	}
	return ret;
}

//Turn array back to string and escape as needed
std::string deTokenize(std::vector<std::string> in)
{
	std::string ret;
	for (auto i : in) {
		for (size_t j = 0; j < i.length(); j++) {
			if (i[j] == ' ' || i[j] == '\'' || i[j] == '"' || i[j] == '\\') {
				i.insert(j++, 1, '\\');
			}
		}
		//trailing space doesn't matter
		ret+=i+' ';
	}
	return ret;
}

//Sanitize to alphabetics
std::string sanitizeAlpha(std::string in)
{
	std::string tmp;
	for (auto i : in) {
		if (isalpha(i)) {
			tmp += i;
		}
	}
	return tmp;
}

//Sanitize to alphanumerics
std::string sanitizeAN(std::string in)
{
	std::string tmp;
	for (auto i : in) {
		if (isalnum(i)) {
			tmp += i;
		}
	}
	return tmp;
}

bool truthy(std::string in)
{
	//".. promt on" or ".. promt t"
	if (in[0] == 't' || in[0] == '1' || in[0] == 'y' || in == "on") {
		return true;
	}
	return false;
}

//Run command in trie, returning outputs
std::vector<std::string> input_indexed_trie::execute(std::vector<std::string> command,
				std::size_t* consumed)
{
	//std::transform(command[0].begin(), command[0].end(), command[0].begin(), ::tolower);
	int len = 0;
	functdata *found = longestPrefix(command[0], &len);
	if (found) {
		std::vector<std::string> args {command[0]}, out;
		//Add semicolon to allow 0-argument functions to work
		if (command.size() == 1)
			command.push_back(";");
		/*if (found->mode == functdata::Composite) {
			//Tokenize saved command (saved in help)
			auto saved = execute({"k", found->help});
			saved.insert(saved.end(), command.begin()+1, command.end());
			//Execute new command with same arguments
			return execute(saved, consumed);
		}//*/
		//Check if caller requested a count
		std::size_t *cons;
		if (!consumed) 
			cons = new std::size_t;
		else
			cons = consumed;
		//Loop over arguments and accumulate found->argX aruments, or until ';',
			//and call (*found) with that part of command
		
		//enum Mode {Batch, Single, FSingle, Composite, } mode;
		
		for (*cons = 1; (*cons) != command.size(); ++(*cons)) {
			if (command[*cons] != ";") {
				//Allow esaped semicolons through
				if (command[*cons] == "\\;")
					args.push_back(";");
				else
					args.push_back(command[*cons]);
			}
			if (args.size() > found->argX || command[*cons] == ";"
							|| (*cons) == command.size()-1) {
				if (args.size() > found->argN) {
					auto a = (*found)(args);
					out.insert(out.end(), a.begin(), a.end());
					//Clear consumed arguments
					args = {command[0]};
					//FSingle functions can only be used once per call
					if (found->mode == functdata::FSingle)
						break;
				} else {
					std::vector<std::string> ret;
					ret.push_back(std::string("Please specify between ")
						+std::to_string(found->argN)+" and "
						+std::to_string(found->argX)+" arguments for "+command[0].substr(0,len));
					std::string tmp = "arguments recieved: ("
						+std::to_string(args.size()-1)+") [";
					for (auto i : args) {
						(tmp+=i)+=':';
					}
					tmp+=']';
					ret.push_back(tmp);
					out.insert(out.end(), ret.begin(), ret.end());
					args = {command[0]};
				}
			}
		}
		
		//If the caller didn't request a count then don't leak memory
		if (!consumed) 
			delete cons;
		
		return out;
	} else {
		return std::vector<std::string>({"Nothing found for \""+command[0]+"\""});
	}
}

std::string functdata::modeS(Mode m)
{
	//{Batch, Single, FSingle, Bounded, Grouped, }
	switch (m) {
	case Mode::Batch:
		return "Batch";
	case Mode::Single:
		return "Single";
	case Mode::FSingle:
		return "FSingle";
	case Mode::Composite:
		return "Composite";
	default:
		return "Undefined";
	}
}
