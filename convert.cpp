#include "roman.h"

#include <iostream>

//Sanitize decimal string (Strips "-" as well)
std::string sanitizeNumeric(std::string in)
{
	std::string tmp;
	for (auto i : in) {
		if (isdigit(i)) {
			tmp += i;
		}
	}
	return tmp;
}

namespace Roman {

const std::map<ConversionContext::Numerals, const std::vector<char>> ConversionContext::m_symbols
	{
		{Numerals::Extended, {'I','V','X','L','C','D','M','A','B','E','F','G',
			'H','J','K','N','O','P','Q','R','S','T','U','W','Y','Z'}},
		{Numerals::Compatible, {'I','V','X','L','C','D'}},
		{Numerals::Lowercase, {'I','V','X','L','C','D','i','v','x','l','c','d'}},
		{Numerals::HyperExtended, {'I','V','X','L','C','D','M','A','B','E','F','G',
			'H','J','K','N','O','P','Q','R','S','T','U','W','Y','Z',
			'i','v','x','l','c','d','m','a','b','e','f','g',
			'h','j','k','n','o','p','q','r','s','t','u','w','y','z'}},
		//the second 'M' will never be used, but the length must be even
		{Numerals::Standard, {'I','V','X','L','C','D', 'M', 'M'}},
	};

std::string glyph(const unsigned char& digit, const std::size_t& place,
									const std::vector<char>& symbols, bool clock = false)
{
	std::size_t s = (symbols.size()/2), 
		p = place%s,
		d = place/s;
	//std::cout<<static_cast<int>(digit)<<';'<<s<<';'<<p<<';'<<d<<';'<<std::endl;
	switch (digit%10) {
	case 9: //IX or C(I)
		if (!clock) {
			return repeat('(',d) + symbols[2*p] + repeat(')',d)
				+ glyph(1,place+1,symbols);
		} else {//VIIII
			return repeat('(',d) + symbols[2*p+1] + repeat(')',d)
				+ repeat('(',d) + symbols[2*p] + repeat(')',d)
				+ repeat('(',d) + symbols[2*p] + repeat(')',d)
				+ repeat('(',d) + symbols[2*p] + repeat(')',d)
				+ repeat('(',d) + symbols[2*p] + repeat(')',d);
		}
		break;
	case 8: //VIII
		return repeat('(',d) + symbols[2*p+1] + repeat(')',d)
			+ repeat('(',d) + symbols[2*p] + repeat(')',d)
			+ repeat('(',d) + symbols[2*p] + repeat(')',d)
			+ repeat('(',d) + symbols[2*p] + repeat(')',d);
		break;
	case 7: //VII
		return repeat('(',d) + symbols[2*p+1] + repeat(')',d)
			+ repeat('(',d) + symbols[2*p] + repeat(')',d)
			+ repeat('(',d) + symbols[2*p] + repeat(')',d);
		break;
	case 6: //VI
		return repeat('(',d) + symbols[2*p+1] + repeat(')',d)
			+ repeat('(',d) + symbols[2*p] + repeat(')',d);
		break;
	case 5: //V
		return repeat('(',d) + symbols[2*p+1] + repeat(')',d);
		break;
	case 4: //IV
		if (!clock) {
			return repeat('(',d) + symbols[2*p] + repeat(')',d)
				+ repeat('(',d) + symbols[2*p+1] + repeat(')',d);
		} else { //IIII
			return repeat('(',d) + symbols[2*p] + repeat(')',d)
				+ repeat('(',d) + symbols[2*p] + repeat(')',d)
				+ repeat('(',d) + symbols[2*p] + repeat(')',d)
				+ repeat('(',d) + symbols[2*p] + repeat(')',d);
		}
		break;
	case 3: //III
		return repeat('(',d) + symbols[2*p] + repeat(')',d)
			+ repeat('(',d) + symbols[2*p] + repeat(')',d)
			+ repeat('(',d) + symbols[2*p] + repeat(')',d);
		break;
	case 2: //II
		return repeat('(',d) + symbols[2*p] + repeat(')',d)
			+ repeat('(',d) + symbols[2*p] + repeat(')',d);
		break;
	case 1: //I
		return repeat('(',d) + symbols[2*p] + repeat(')',d);
		break;
	default:
		return std::string();
	}
}

//Sanitize numeral string
std::string ConversionContext::sanitizeNumeral(std::string in) const
{
	std::string tmp;
	for (auto i : in) {
		if (std::find(m_symbols.at(m_context).begin(), m_symbols.at(m_context).end(), i)
				!= m_symbols.at(m_context).end()
				|| i == 'M' || i == 'm' || (m_context != Numerals::Standard && (i == '(' || i == ')'))) {
			tmp += i;
		}
	}
	return tmp;
}

std::string ConversionContext::toR(mpz_class val) const
{
	std::string ret;
	if (val == 0) {
		return std::string("");
	}
	val = abs(val);
	
	if (m_context == Numerals::Standard) {
		mpz_class thousands = val/1000;
		//Only properly format numbers up to 99,999 (What are you thinking with
			//more than that in Standard notation? (Most programs handle only 3,999)
										//Most M's to write at once
		if (thousands < 100) {
			ret += repeat('M', thousands.get_ui());
		} else {
			ret+= std::string("M[*")+toStr(thousands)+"]";
		}
		//Get part of val less than 1000
		val %= 1000_mpz;
	}
	if (val == 0) {
		return ret;
	}
	//static_cast<std::size_t>(log10(m_vals.back())) + 1
	for (std::size_t s = static_cast<std::size_t>(log10(val.get_d())) + 1; s != 0 ; s--) {
		mpz_class pow;
		//pow(10, s)
		mpz_ui_pow_ui(pow.get_mpz_t(), 10, s);
		mpz_class digit = (val%pow*10)/pow;
		ret += glyph(static_cast<unsigned char>(digit.get_ui()),
										s-1, m_symbols.at(m_context));
	}
	return ret;
}

mpz_class ConversionContext::toD(std::string val) const
{
	size_t run_length = 0;
	mpz_class prev = 0, out = 0, curr = 0, sub = 0, mult = 1, multBy;
	//value of (I) in current context
	mpz_ui_pow_ui(multBy.get_mpz_t(), 10, m_symbols.at(m_context).size()/2);
	//Compatible/Lowercase doesn't output M but can read it
	if (m_context == Numerals::Compatible) {
		auto loc = std::string::npos;
		while ((loc = val.find('M')) != std::string::npos) {
			//Erase 1 character at loc and then insert "(I)" into that place
			val.erase(loc,1).insert(loc,"(I)");
		}
	}
	//Similar to above:
	if (m_context == Numerals::Lowercase) {
		auto loc = std::string::npos;
		while ((loc = val.find('M')) != std::string::npos) {
			//Erase 1 character at loc and then insert "i" into that place
			val.erase(loc,1).insert(loc,"i");
		}
		loc = std::string::npos;
		while ((loc = val.find('m')) != std::string::npos) {
			//Erase 1 character at loc and then insert "(I)" into that place
			val.erase(loc,1).insert(loc,"(I)");
		}
	}
	for (auto i : val) {
		auto p = find(m_symbols.at(m_context).begin(), m_symbols.at(m_context).end(), i);
		if (p != m_symbols.at(m_context).end()) {
			mpz_class pow;
			mpz_ui_pow_ui(pow.get_mpz_t(), 10, (p - m_symbols.at(m_context).begin()) / 2);
			//If at odd location
			if ((p - m_symbols.at(m_context).begin()) % 2)
				pow *= 5;
				curr = pow * mult;
				if (curr < prev) {
					out += prev * run_length;
				run_length = 0;
			} else if (curr > prev) {
				sub += prev * run_length; //This prevents underflow
				run_length = 0;
			}//If equal, do nothing special
			prev = curr;
			++run_length;
		} else if (i == '(') {
			mult *= multBy;
		} else if (i == ')') {
			mult /= multBy;
		}
	}
	out += prev * run_length;
	out -= sub; //sub is (by definition) less than out here
	return out;
}

}