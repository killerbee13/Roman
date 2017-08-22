#ifndef STRINGS_H_INCLUDED
#define STRINGS_H_INCLUDED

//Command-line help
char HelpStr[] =
"USAGE: sequences [(options|scripted-command) [...]]                           \n"
"OPTIONS:                                                                      \n"
"        -q      Quiet mode; do not print prompts (overrides -v)               \n"
"    Equivalent to 'set prompt off'                                            \n"
"        -v      Verbose mode; simulate user input for scripted commands       \n"
"    Equivalent to 'set s_prompt on'                                           \n"
"        -c      Continue; switch to interactive mode after evaluating scripted\n"
"    commands or printing this text                                            \n"
"        -h      Help; print this message and exit                             \n"
"    See \"h\" command for help with command syntax                            \n"
"Each argument may be either a command or a series of options. All             \n"
"options are guaranteed to be processed before scripted commands are           \n"
"interpreted, and all commands are run in order.                               \n";

//Interactive main help
std::vector<std::string> iHelpStr = {
{"Commands include the following: (Abbreviate to capital letters)"},
{"Echo (string...), eXecute (command (args...)), Probe trie (command), Help"},
{"(command), Quit, To (extended/Compatible/Lowercase-extended/Absolutely"},
{" standard) roman (number), From (extended/Compatible/Lowercase-extended/"},
{"Absolutely standard) roman (numeral), Canonicize (e/C/L/A) (numeral)"},
{"Input is accepted as \"e test test2\""},
{"For a complete list, run \"= p p-\". "},};

#endif //STRINGS_H_INCLUDED