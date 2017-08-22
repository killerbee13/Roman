#ifndef ROMAN_H_INCLUDED
#define ROMAN_H_INCLUDED
#include <string>
#include <vector>
#include <map>

//mpz_class
#include <gmpxx.h>

#include "FRC.h"

//Sanitize decimal string
std::string sanitizeNumeric(std::string);

namespace Roman {

class ConversionContext {
public:
	//Sanitize roman numeral string
	std::string sanitizeNumeral(std::string) const;
	
	enum class Numerals : int
		{Extended, Compatible, Lowercase, Standard, HyperExtended};
	
	constexpr ConversionContext(Numerals type=Numerals::Extended, bool cl = false)
		: m_context(type), clock(cl) {;}
	
	mpz_class toD(std::string val) const;
	
	std::string toR(mpz_class val) const;
	
	//"Canonicize" RN input
	std::string reduce(std::string val) const {return toR(toD(val));}

	//string is RN and mpz_class is decimal
	mpz_class operator()(const std::string& val) const {return toD(val);}
	std::string operator()(const mpz_class& val) const {return toR(val);}
		
	const static std::map<Numerals, const std::vector<char>> m_symbols;
private:
	Numerals m_context;
	bool clock;
};

//Standard converters which double as function objects
constexpr ConversionContext
	E(ConversionContext::Numerals::Extended),
	H(ConversionContext::Numerals::HyperExtended),
	C(ConversionContext::Numerals::Compatible),
	L(ConversionContext::Numerals::Lowercase),
	S(ConversionContext::Numerals::Standard),
	Cl(ConversionContext::Numerals::Compatible, true);

}
#endif //ROMAN_H_INCLUDED