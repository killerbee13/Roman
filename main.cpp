#include <iostream>

#include <vector>
#include <array>
#include <string>
#include <tuple>
#include <map>
#include <locale>

#include <cstdio>
//#include <libtecla.h>
#include "tecla_oo.h"

#include "roman.h"
#include "FRC.h"
#include "input.h"

//HelpStr, iHelpStr
#include "strings.h"

using std::string;
using std::vector;
using std::cin;
using std::cout;
using std::endl;

bool Continue = true;
std::map<std::string, std::string> vars;
std::map<std::string, bool> settings;

void createInitTrie(input_trie& commands);
std::vector<std::pair<std::string, functdata*>> initTrie1(input_trie& commands);
std::vector<std::pair<std::string, functdata*>> initTrie2(input_trie& commands);

void runCommand(input_trie commands, string in);

//Since every lambda has the same signature, and it's quite verbose
#define INPUT_TRIE_DECL(X) std::pair<std::string, functdata*>(std::string(X), new functdata(\
	[&commands](std::vector<std::string> inList) -> std::vector<std::string>

//This one is pretty much useless, but otherwise the parentheses end up unmatched
#define CPARENS ))

int main(int argc, char** argv)
{
	input_trie commands;
	
	//Add built-in functions to trie
	createInitTrie(commands);
	
	//Aliases and saved commands:
	commands.execute(std::vector<std::string>({"save", "esc", "u"}));
	commands.execute(std::vector<std::string>({"save", ":", "= ec"}));
	commands.execute(std::vector<std::string>({"save", "ls", "= p p-"}));
	
	
	commands.execute(std::vector<std::string>({"..", "long", "off"}));
	commands.execute(std::vector<std::string>({"..", "prompt", "on"}));
	commands.execute(std::vector<std::string>({"..", "s_prompt", "off"}));
	
	commands.execute(std::vector<std::string>({".", "prompt", "$>"}));
	
	// settings["long"] = false;
	// settings["prompt"] = true;
	// settings["s_prompt"] = false;
	
	// vars["prompt"] = "$>";
	
	std::vector<string> scriptedCommands;
	
	bool cont_after = false;
	
	for (int i = 1; i < argc; i++) {
		//cout<<"["<<i<<"] "<<argv[i]<<endl;
		if (argv[i][0] == '-' && argv[i][1] != '\0' && argv[i][1] != ' ') {
			for (int j = 1; argv[i][j] != '\0'; ++j) {
				if (argv[i][j] == 'q') { //quiet (Don't display prompts)
					settings["prompt"] = false;
				} else if (argv[i][j] == 'h') { //help
					//from strings.h
					cout<<HelpStr;
					Continue = false;
				//verbose (Simulate interactive display for scripted commands)
				} else if (argv[i][j] == 'v') {
					settings["s_prompt"] = true;
				} else if (argv[i][j] == 'c') { //continue after scripted commands
					cont_after = true;
				} else if (argv[i][j] == 'l') { //Long form output
					settings["long"] = true;
				}
			}
		} else { //interpret other strings as commands to be run
			scriptedCommands.emplace_back(argv[i]);
		}
	}
	
	Continue = Continue & !scriptedCommands.size();
	
	for (auto i : scriptedCommands) {
		//q overrides v
		if (settings["s_prompt"] && settings["prompt"])
			cout<<vars["prompt"]<<" "<<i<<endl;
		runCommand(commands, i);
		if (!Continue) {
			break;
		}
	}
	
	//Since both are simple variables short-circuiting doesn't really help, and
		//bools are guaranteed to only be 1 or 0
	Continue = Continue | cont_after;
	if (Continue) {
		//Tecla (input lib) setup
		//Tecla handles input buffers, never delete this pointer
		char *line = nullptr;
		//setlocale(LC_TYPE, "");
		Tecla_gl gl(1024, 2048, ".Roman_hist");
		//No memory?
		if (!gl) return 1;
		string in;
		
	input_retry:
		//Actual input loop, large-scope try because all exceptions in main are outside my code
		while (Continue) {
			try {
				//Yes, I know, I'm mixing STDIN/OUT and cin/out, but I'm not going to
					//rewrite this entire section in C just because I'm using a C input
					//library. It rarely matters anyway.
				if (settings["prompt"])
					line=gl.getline((vars["prompt"]+" ").c_str(), nullptr, -1);
				else
					line=gl.getline(nullptr, nullptr, -1);
				
//Turn line into std::string
				if (line)
					in = line;
				else
					break;
				//Strip newline from end
				in.pop_back();
				
				
				if (in.empty()) {
					continue;
				}
				runCommand(commands, in);
			} catch (const std::runtime_error& err) {
				cout<<"Failed to read input. "<<err.what()<<endl;
			} catch (const std::exception& err) {
				cout<<"Unknown error: "<<err.what()<<endl;
				throw err;
			} catch (...) {
				cout<<"Unknown error"<<endl;
				throw;
			}
		}
		switch (gl.status()) {
		//Non-exit (but unexpected) conditions
		case GLR_BLOCKED: //Blocking mode is default?
		case GLR_SIGNAL: //If Tecla didn't already terminate the program, then continue
		case GLR_TIMEOUT: //No inactivity timer is set, so this shouldn't happen
			goto input_retry;
		
		//Normal exit conditions:
		case GLR_NEWLINE: //Nothing went wrong, so exit must have been requested
		case GLR_EOF: //EOF reached, exiting
			break;
		
		//Exceptional exit conditions
		case GLR_ERROR: //An error occurred, just exit
			std::cerr<<"Error reading input:\n"<<gl.error_message()<<std::endl;
		case GLR_FDABORT: //This should never happen
			// status = 1;
		//Anything else:
		default:
			break; //Exit normally
		}
		return 0;
	} else {
		return 0;
	}
}

void runCommand(input_trie commands, string in)
{
	if (in.empty()) {
		return;
	}
	
	//Split input on word boundaries
	vector<string> inList = tokenizeInput(in);
	vector<string> out = commands.execute(inList);
	for (auto i : out) {
		cout<<i<<endl;
	}
}

inline void createInitTrie(input_trie& commands)
{
	auto a = initTrie1(commands);
	commands.insert(a);
	//Commands that require the trie to already be initialized/reference it
	auto b = initTrie2(commands);
	commands.insert(b);
}

inline std::vector<std::pair<std::string, functdata*>> initTrie1(input_trie& commands)
{
	return std::vector<std::pair<std::string, functdata*>>({
		INPUT_TRIE_DECL("h") {
			if (inList.size() > 1) {
				int len = 0;
				functdata *found = commands.longestPrefix(toLower(inList[1]), &len);
				if (found) {
					if (found->mode != functdata::Composite)
						return {found->help};
					else
						return {"Alias of \"" + found->help + "\""};
				} else {
					return {string("Nothing found for \"")+toLower(inList[1])+"\""};
				}
			} else {
				//from strings.h
				return iHelpStr;
			}
		}, 0, 1, std::string("Get help"), functdata::Single CPARENS,
		INPUT_TRIE_DECL("q") {
			Continue = false;
			return {};
		}, 0, std::string("Quit") CPARENS,
		INPUT_TRIE_DECL("..") {
			inList[1].resize(inList[1].size());
			if (inList.size() > 2)
				settings[toLower(inList[1])] = truthy(inList[2]);
			else
				settings[toLower(inList[1])] = !settings[toLower(inList[1])];
			return {};
		}, 1, 2, std::string("Change setting (toggle if no argument)"), functdata::Single CPARENS,
		INPUT_TRIE_DECL("..-") {
			inList[1].resize(inList[1].size());
			settings.erase(toLower(inList[1]));
			return {};
		}, 1, 1, std::string("Clear setting"), functdata::Single CPARENS,
		INPUT_TRIE_DECL("??") {
			if (inList.size() == 2) {
				inList[1].resize(inList[1].size());
				return {settings[toLower(inList[1])]?std::string("on"):std::string("off")};
			} else {
				std::vector<std::string> ret;
				for (auto i : settings) {
					ret.push_back(std::string("\"")+i.first+"\": "+(i.second?std::string("on"):std::string("off")));
				}
				return ret;
			}
		}, 0, 1, std::string("Query setting"), functdata::Single CPARENS,
		INPUT_TRIE_DECL(".") {
			inList[1].resize(inList[1].size());
			if (inList.size() == 2)
				vars.erase(toLower(inList[1]));
			else
				vars[toLower(inList[1])] = inList[2];
			return {};
		}, 1, 2, std::string("Save/Clear variable"), functdata::Single CPARENS,
		INPUT_TRIE_DECL("?") {
			if (inList.size() == 2) {
				inList[1].resize(inList[1].size());
				return {vars[toLower(inList[1])]};
			} else {
				std::vector<std::string> ret;
				for (auto i : vars) {
					ret.push_back(std::string("\"")+i.first+"\": \""+i.second+"\"");
				}
				return ret;
			}
		}, 0, 1, std::string("Query variable"), functdata::Single CPARENS,
		INPUT_TRIE_DECL("e") {
			std::vector<std::string> ret;
			string tmp = "[";
			for (auto i : inList) {
				(tmp+=i)+=':';
			}
			tmp+=']';
			ret.push_back(tmp);
			return ret;
		}, 0, std::string("Echo inputs") CPARENS,
		INPUT_TRIE_DECL("el") {
			//std::vector<std::string> ret;
			inList.erase(inList.begin());
			return inList;
			//for (auto i : inList) {
			//  ret.push_back(i);
			//}
			//return ret;
		}, 0, std::string("Echo inputs (1/line)" CPARENS),
		INPUT_TRIE_DECL("es") {
			return {inList[1]};
		}, 1, 1, std::string("Echo input (Single)"), functdata::Single CPARENS,
		INPUT_TRIE_DECL("ec") {
			std::string ret {inList[1]};
			inList.erase(inList.begin());
			inList.erase(inList.begin());
			for (auto i : inList) {
				ret += ' ' + i;
			}
			return {ret};
		}, 1, -1, std::string("Echo inputs (Collating)"), functdata::Batch CPARENS,
		INPUT_TRIE_DECL("#") {
			return {};
		}, 0, std::string("Echo inputs (1/line)" CPARENS),
		INPUT_TRIE_DECL("d") {
			return {inList[1], inList[1]};
		}, 1, 1, std::string("Duplicate"), functdata::Single CPARENS,
		INPUT_TRIE_DECL("r") {
			return {inList[2], inList[1]};
		}, 2, 2, std::string("Reverse"), functdata::Single CPARENS,
		INPUT_TRIE_DECL("k") {
			std::vector<std::string> ret;
			inList.erase(inList.begin());
			std::vector<std::string> a;
			for (auto i : inList) {
				a = tokenizeInput(i);
				ret.insert(ret.end(), a.begin(), a.end());
			}
			return ret;
		}, 0, std::string("Tokenize input (again)") CPARENS,
		INPUT_TRIE_DECL("u") {
			inList.erase(inList.begin());
			return std::vector<std::string>{deTokenize(inList)};
		}, 0, std::string("Detokenize/escape input") CPARENS,
		INPUT_TRIE_DECL("s") {
			std::vector<std::string> ret;
			inList.erase(inList.begin());
			for (auto i : inList) {
				ret.push_back(sanitizeAN(toLower(i)));
			}
			return ret;
		}, 0, std::string("Sanitize inputs") CPARENS,
		INPUT_TRIE_DECL("sa") {
			std::vector<std::string> ret;
			inList.erase(inList.begin());
			for (auto i : inList) {
				ret.push_back(sanitizeAlpha(toLower(i)));
			}
			return ret;
		}, 0, std::string("Sanitize inputs to only alphabeticals") CPARENS,
		INPUT_TRIE_DECL("sn") {
			std::vector<std::string> ret;
			inList.erase(inList.begin());
			for (auto i : inList) {
				ret.push_back(sanitizeNumeric(i));
			}
			return ret;
		}, 0, std::string("Sanitize input number") CPARENS,
		INPUT_TRIE_DECL("sr") {
			std::vector<std::string> ret;
			inList.erase(inList.begin());
			for (auto i : inList) {
				ret.push_back(Roman::E.sanitizeNumeral(i));
			}
			return ret;
		}, 0, std::string("Sanitize input ERN") CPARENS,
		INPUT_TRIE_DECL("srl") {
			std::vector<std::string> ret;
			inList.erase(inList.begin());
			for (auto i : inList) {
				ret.push_back(Roman::L.sanitizeNumeral(i));
			}
			return ret;
		}, 0, std::string("Sanitize input LRN") CPARENS,
		INPUT_TRIE_DECL("srh") {
			std::vector<std::string> ret;
			inList.erase(inList.begin());
			for (auto i : inList) {
				ret.push_back(Roman::H.sanitizeNumeral(i));
			}
			return ret;
		}, 0, std::string("Sanitize input HRN") CPARENS,
		INPUT_TRIE_DECL("l") {
			std::size_t c = 2, cons = 0;
			std::vector<std::string> ret;
			while ( c < inList.size() ) {
				auto b = std::vector<std::string>(inList.begin()+c, inList.end());
				b.insert(b.begin(), inList[1]);
				auto a = commands.execute(
					b, &cons
				);
				ret.insert(ret.end(), a.begin(), a.end());
				if (cons)
					c += cons;
				else 
					return ret;
			}
			return ret;
		}, 2, -1, std::string("Loop Command"), functdata::Batch CPARENS,
		INPUT_TRIE_DECL("la") {
			std::size_t c = 3, cons = 0;
			std::string r = inList[2];
			while ( c < inList.size() ) {
				r = commands.execute(
					{inList[1], r, inList[c]}, &cons
				)[0];
				if (cons)
					c += cons - 1;
				else 
					return {r};
			}
			return {r};
		}, 2, -1, std::string("Loop-accumulate left"), functdata::Batch CPARENS,
		INPUT_TRIE_DECL("lar") {
			std::size_t c = 3, cons = 0;
			std::reverse(inList.begin()+2, inList.end());
			std::string r = inList[2];
			while ( c < inList.size() ) {
				r = commands.execute(
					{inList[1], inList[c], r}, &cons
				)[0];
				if (cons)
					c += cons - 1;
				else 
					return {r};
			}
			return {r};
		}, 2, -1, std::string("Loop-accumulate right"), functdata::Batch CPARENS,
		INPUT_TRIE_DECL("=") {
			auto a = commands.execute(
				std::vector<std::string>(inList.begin()+2, inList.end())
			);
			return commands.execute(arraycat({inList[1]}, a));
		}, 2, -1, std::string("Apply function"), functdata::Batch CPARENS,
		INPUT_TRIE_DECL("|") {
			std::string f(inList[0], 1); //f is all of inList[0] except '|'
			if (f.empty()) {
				return {"Please specify a funtion for \"|\""};
			}
			auto a = commands.execute(
				std::vector<std::string>(inList.begin()+1, inList.end())
			);
			return commands.execute(arraycat({f}, a));
		}, 1, -1, std::string("Apply (fixed) function"), functdata::Batch CPARENS,
		INPUT_TRIE_DECL("=p") {
			auto a = commands.execute(
				std::vector<std::string>(inList.begin()+2, inList.end())
			);
			return arraycat({inList[1]}, a);
		}, 2, -1, std::string("Pretend to apply function"), functdata::Batch CPARENS,
		INPUT_TRIE_DECL("p") {
			if (inList.size() == 1) {
				auto dump = commands.getIndex();
				std::vector<std::string> ret;
				string tmp;
				for (unsigned i = 0; i != dump.size(); i++) {
					(tmp+=std::to_string(i))+=": ";
					for (unsigned char j = 0; j != 128; j++) {
						if (dump[i][j].first || dump[i][j].second) {
							tmp+=(char)(j?j:'/')+std::to_string(dump[i][j].first)
								+(dump[i][j].second?'*':' ')+' ';
						}
					}
					ret.push_back(tmp);
					tmp.clear();
					if (inList[0][1] == 'l') {
						for (unsigned char j = 0; j != 128; j++) {
							tmp += (
								dump[i][j].first
									? std::to_string(dump[i][j].first)
									: string(" ")
								) + (
								dump[i][j].second
									? '*'
									: ' '
								) + ' ';
							//print 16 to a line
							if (j%16 == 15) {
								ret.push_back(tmp);
								tmp.clear();
							}
						}
					ret.push_back(tmp);
					tmp.clear();
					}
				}
				return ret;
			} else {
				int len = 0;
				functdata *found = commands.longestPrefix(toLower(inList[1]), &len);
				if (found) {
					if (found->mode == functdata::Composite)
						return {string("\"") + inList[1].substr(0,len) + "\": Aliases \""
							+ found->help + "\""
							+ (found->call?"   ":"[!]")};
					else
						return {string("\"") + inList[1].substr(0,len) + "\": "
							+ found->help+" [takes " + std::to_string(found->argN) 
							+ ((found->argX != unsigned(-1))
								? '-' + std::to_string(found->argX)
								: std::string("+"))
							+ "; "
							+ functdata::modeS(found->mode) + ']'
							+ (found->call?' ':'!')};
				} else
					return {std::to_string(len) + "; Nothing found for \""
						+ toLower(inList[1])+"\""};
			}
		}, 0, 1, std::string("Print/Probe input trie (Long)"), functdata::Single CPARENS,
		INPUT_TRIE_DECL("p-") {
			return commands.allWithPrefix(inList.size()>1?inList[1]:"");
		}, 0, 1, std::string("List input trie"), functdata::Single CPARENS,

		INPUT_TRIE_DECL("+") {
			return {toStr(fromStr<mpz_class>(sanitizeNumeric(inList[1]))
				+ fromStr<mpz_class>(sanitizeNumeric(inList[2])))};
		}, 2, 2, std::string("Add numbers"), functdata::FSingle CPARENS,
		INPUT_TRIE_DECL("-") {
			return {toStr(fromStr<mpz_class>(sanitizeNumeric(inList[1]))
				- fromStr<mpz_class>(sanitizeNumeric(inList[2])))};
		}, 2, 2, std::string("Subtract numbers"), functdata::FSingle CPARENS,
		INPUT_TRIE_DECL("*") {
			return {toStr(fromStr<mpz_class>(sanitizeNumeric(inList[1]))
				* fromStr<mpz_class>(sanitizeNumeric(inList[2])))};
		}, 2, 2, std::string("Multiply numbers"), functdata::FSingle CPARENS,
		INPUT_TRIE_DECL("/") {
			return {toStr(fromStr<mpz_class>(sanitizeNumeric(inList[1]))
				/ fromStr<mpz_class>(sanitizeNumeric(inList[2])))};
		}, 2, 2, std::string("Divide numbers"), functdata::FSingle CPARENS,
		INPUT_TRIE_DECL("%") {
			return {toStr(fromStr<mpz_class>(sanitizeNumeric(inList[1]))
				% fromStr<mpz_class>(sanitizeNumeric(inList[2])))};
		}, 2, 2, std::string("Modulo numbers"), functdata::FSingle CPARENS,

		INPUT_TRIE_DECL("t") {
			return {(settings["long"]?(sanitizeNumeric(inList[1]) + " = "):std::string("")) + 
				Roman::E(fromStr<mpz_class>(sanitizeNumeric(inList[1])))};
		}, 1, 1, std::string("Convert decimal to extended roman"), functdata::Single CPARENS,
		INPUT_TRIE_DECL("tc") {
			return {(settings["long"]?(sanitizeNumeric(inList[1]) + " = "):std::string("")) + 
				Roman::C(fromStr<mpz_class>(sanitizeNumeric(inList[1])))};
		}, 1, 1, std::string("Convert decimal to \"compatible\" roman"), functdata::Single CPARENS,
		INPUT_TRIE_DECL("tl") {
			return {(settings["long"]?(sanitizeNumeric(inList[1]) + " = "):std::string("")) + 
				Roman::L(fromStr<mpz_class>(sanitizeNumeric(inList[1])))};
		}, 1, 1, std::string("Convert decimal to lowercase-extended roman"), functdata::Single CPARENS,
		INPUT_TRIE_DECL("ta") {
			return {(settings["long"]?(sanitizeNumeric(inList[1]) + " = "):std::string("")) + 
				Roman::S.toR(fromStr<mpz_class>(sanitizeNumeric(inList[1])))};
		}, 1, 1, std::string("Convert decimal to absolutely standard roman"), functdata::Single CPARENS,
		INPUT_TRIE_DECL("th") {
			return {(settings["long"]?(sanitizeNumeric(inList[1]) + " = "):std::string("")) + 
				Roman::H.toR(fromStr<mpz_class>(sanitizeNumeric(inList[1])))};
		}, 1, 1, std::string("Convert decimal to hyper-extended roman"), functdata::Single CPARENS,

		INPUT_TRIE_DECL("f") {
			return {(settings["long"]?(Roman::E.sanitizeNumeral(inList[1]) + " = "):std::string("")) +
				toStr(Roman::E(Roman::E.sanitizeNumeral(inList[1])))};
		}, 1, 1, std::string("Convert extended RN to decimal"), functdata::Single CPARENS,
		INPUT_TRIE_DECL("fc") {
			return {(settings["long"]?(Roman::C.sanitizeNumeral(inList[1]) + " = "):std::string("")) +
				toStr(Roman::C(Roman::C.sanitizeNumeral(inList[1])))};
		}, 1, 1, std::string("Convert \"compatible\" RN to decimal"), functdata::Single CPARENS,
		INPUT_TRIE_DECL("fl") {
			return {(settings["long"]?(Roman::L.sanitizeNumeral(inList[1]) + " = "):std::string("")) +
				toStr(Roman::L(Roman::L.sanitizeNumeral(inList[1])))};
		}, 1, 1, std::string("Convert lowercase-extended RN to decimal"), functdata::Single CPARENS,
		INPUT_TRIE_DECL("fa") {
			return {(settings["long"]?(Roman::S.sanitizeNumeral(inList[1]) + " = "):std::string("")) +
				toStr(Roman::S(Roman::S.sanitizeNumeral(inList[1])))};
		}, 1, 1, std::string("Convert absolutely standard RN to decimal"), functdata::Single CPARENS,
		INPUT_TRIE_DECL("fh") {
			return {(settings["long"]?(Roman::H.sanitizeNumeral(inList[1]) + " = "):std::string("")) +
				toStr(Roman::H(Roman::H.sanitizeNumeral(inList[1])))};
		}, 1, 1, std::string("Convert hyper-extended RN to decimal"), functdata::Single CPARENS,

		INPUT_TRIE_DECL("c") {
			return {(settings["long"]?(Roman::E.sanitizeNumeral(inList[1]) + " = "):std::string("")) +
				toStr(Roman::E.reduce(Roman::E.sanitizeNumeral(inList[1])))};
		}, 1, 1, std::string("Canonicize extended RN"), functdata::Single CPARENS,
		INPUT_TRIE_DECL("cc") {
			return {(settings["long"]?(Roman::C.sanitizeNumeral(inList[1]) + " = "):std::string("")) +
				toStr(Roman::C.reduce(Roman::C.sanitizeNumeral(inList[1])))};
		}, 1, 1, std::string("Canonicize \"compatible\" RN"), functdata::Single CPARENS,
		INPUT_TRIE_DECL("cl") {
			return {(settings["long"]?(Roman::L.sanitizeNumeral(inList[1]) + " = "):std::string("")) +
				toStr(Roman::L.reduce(Roman::L.sanitizeNumeral(inList[1])))};
		}, 1, 1, std::string("Canonicize lowercase-extended RN"), functdata::Single CPARENS,
		INPUT_TRIE_DECL("ca") {
			return {(settings["long"]?(Roman::S.sanitizeNumeral(inList[1]) + " = "):std::string("")) +
				toStr(Roman::S.reduce(Roman::S.sanitizeNumeral(inList[1])))};
		}, 1, 1, std::string("Canonicize absolutely standard RN"), functdata::Single CPARENS,
		INPUT_TRIE_DECL("ch") {
			return {(settings["long"]?(Roman::H.sanitizeNumeral(inList[1]) + " = "):std::string("")) +
				toStr(Roman::H.reduce(Roman::H.sanitizeNumeral(inList[1])))};
		}, 1, 1, std::string("Canonicize hyper-extended standard RN"), functdata::Single CPARENS,

	});
}

inline std::vector<std::pair<std::string, functdata*>> initTrie2(input_trie& commands)
{
	return std::vector<std::pair<std::string, functdata*>>({
		INPUT_TRIE_DECL("x") {
			return commands.execute(
				std::vector<std::string>(inList.begin()+1, inList.end())
			);
		}, 1, std::string("Execute Command") CPARENS,
		INPUT_TRIE_DECL("save") {
			int len = 0;
			commands.longestPrefix(inList[1], &len);
			if (static_cast<unsigned>(len) == inList[1].length()) {
				return {std::string("\"") + inList[1] + "\" is already a command."};
			} else {
				commands.insert(
					std::string(inList[1]), new functdata(
						[inList, &commands](std::vector<std::string> inList2) -> std::vector<std::string> {
						return commands.execute(tokenizeInput(inList[2] + " "
						//+ commands.execute(arraycat(std::vector<std::string>(1,"ec"), std::vector<std::string>(inList2.begin()+1, inList2.end())))[0]
							+ deTokenize(std::vector<std::string>(inList2.begin()+1, inList2.end()))
							));
					}, 0, -1, inList[2], functdata::Composite));
				return {"Warning: This feature is highly experimental."};
			}
		}, 2, 2, std::string("Save Command (Simple, EXP)"), functdata::Single CPARENS,
		INPUT_TRIE_DECL("del") {
			auto f = commands.longestPrefix(inList[1], nullptr);
			if (f != nullptr && f->mode == functdata::Composite) {
				commands.remove(inList[1]);
				return {"Deleted " + inList[1]};
			} else {
				return {"Did not delete built-in command " + inList[1]};
			}
		}, 1, 1, std::string("Delete saved Command"), functdata::Single CPARENS,
		INPUT_TRIE_DECL("x-") {
			auto f = commands.longestPrefix(inList[1], nullptr);
			if (f != nullptr) {
				if (f->mode == functdata::Composite) {
					//return tokenizeInput(f->help);
					return arraycat(tokenizeInput(f->help), std::vector<std::string>(inList.begin()+2, inList.end()));
				} else {
					return {inList[1]};
				}
			} else {
				return {"Nothing found for \"" + inList[1] + "\""};
			}
		}, 1, 1, std::string("Expand Alias"), functdata::Single CPARENS,
		INPUT_TRIE_DECL("x=") {
			auto f = commands.longestPrefix(inList[1], nullptr);
			//auto allnames = commands.allWithPrefix(inList.size()>1?inList[1]:"");
			if (f != nullptr) {
				if (f->mode == functdata::Composite) {
					return arraycat(tokenizeInput(f->help), std::vector<std::string>(inList.begin()+2, inList.end()));
				} else {
					//return {inList[1]};
					if (inList[1] == "=" && inList.size() > 3) {
						/*std::vector<std::string> a, b;
						a = arraycat(std::vector<std::string>({inList[1]}), commands.execute({"x=", inList[2]}));
						b = commands.execute(arraycat(std::vector<std::string>({"x="}), std::vector<std::string>(inList.begin()+3, inList.end())));
						return arraycat(a,b);//*/
						return arraycat(arraycat(std::vector<std::string>({inList[1]}),
							commands.execute({"x=", inList[2]})),
							commands.execute(arraycat(std::vector<std::string>({"x="}),
							std::vector<std::string>(inList.begin()+3, inList.end()))));
					}
					return std::vector<std::string>(inList.begin()+1, inList.end());
				}
			} else {
				return {"Nothing found for \"" + inList[1] + "\""};
			}
		}, 1, std::string("Expand Alias (smart)") CPARENS,
		INPUT_TRIE_DECL("pa") {
			auto allnames = commands.allWithPrefix(inList.size()>1?inList[1]:"");
			//std::vector<std::pair<std::string,std::string>> aliases;
			std::vector<std::string> ret;
			for (auto i : allnames) {
				auto j = commands.longestPrefix(i, nullptr);
				//No need to test for null because all results are guaranteed to exist
				if (j->mode == functdata::Composite) {
					//aliases.push_back(std::make_pair(i,j->help);
					if (inList[0] == "pa") {
						ret.push_back(std::string("\"")+i+"\": "+j->help);
					} else {
						ret.push_back(i);
					}
				}
			}
			return ret;
		}, 0, 1, std::string("Print all aliases"), functdata::Single CPARENS,
		INPUT_TRIE_DECL("") {
			return inList;
		}, 1, std::string("") CPARENS,
	});
}
