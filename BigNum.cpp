#include "BigNum.h"

#include <cmath>
#include <limits>
#include <deque>
#include <algorithm>

#include <cassert>

//from roman.h:
//Sanitize decimal string
std::string sanitizeNumeric(std::string);

//Handle signed integers and default construction
BigNum::BigNum(long long x) {
  sign = static_cast<BigNum::Sign>(-(x < 0));
  //This might not be necessary
  x = abs(x);
  //Start in LSP
  std::size_t i = 0;
  do {
    this->set(i,static_cast<unsigned char>(x%10));
    ++i;
  } while (x/=10);
}
//Mostly for UDL's and particularly large integral values
BigNum::BigNum(unsigned long long x) {
  sign = pos;
  //Start in LSP
  std::size_t i = 0;
  do {
    this->set(i,static_cast<unsigned char>(x%10));
    ++i;
  } while (x/=10);
}
//Minimize garbage digits at end
BigNum::BigNum(long double x) {
  sign = static_cast<BigNum::Sign>(signbit(x));
  //Only handle integers
  x = round(x);
  std::deque<unsigned char> digits;
  //Loop down the decimal value of x only as many times as there are sig figs
  for (unsigned int i = 0; i != std::numeric_limits<long double>::digits10
        //BigNum can only store integers
        && x >= 1.-.001; //floats aren't exact
        //Increment place counter and divide x by ten
        ++i, x /= 10) {
    digits.push_front(fmod(x,10));
  }
  //digits now contains digits of x in ascending order
  //Place now contains the "offset" of x
  std::size_t place = (x>1)?static_cast<std::size_t>(log10(x)) + 1:0;
  for (auto i : digits) {
    this->set(place,i);
  }
}
//Don't assume string is already cleaned
BigNum::BigNum(const std::string& v) {
  //Simple-minded sign check:
  //If there is a '-' in string, then it's negative
  sign = static_cast<BigNum::Sign>(v.find_first_of('-') != std::string::npos);
  std::string x = sanitizeNumeric(v);
  m_vals.resize(x.length()/max_power+static_cast<std::size_t>(static_cast<bool>(x.length()%max_power)));
  /*for (auto it = x.rbegin(); it != x.rend(); ++it) {
    //Slightly hackish but we know that it's only digits
    //this->set((it - x.rbegin()), (*it) - '0');
    //A chunking solution might be faster but I doubt it's worth the effort
    
    //Addition-and-multiplication-only solution (requires resize)
    m_vals[(it - x.rbegin())/max_power] 
      += (((*it) - '0') * ((it - x.rbegin())%max_power));
  }//*/
  std::reverse(x.begin(), x.end());
  for (std::size_t i = 0; i != x.length(); ++i) {
    /*m_vals[i/max_power] 
      += ((x[i] - '0') * (i%max_power));*/
    this->set(i,(x[i] - '0'));
  }
}

//Comparisons:
bool BigNum::operator==(const BigNum& rhs) const {
  if (m_vals.size() != rhs.m_vals.size() || sign != rhs.sign)
    return false;
  for (std::size_t i = m_vals.size(); i != std::numeric_limits<std::size_t>::max(); --i) {
    if (m_vals[i] != rhs.m_vals[i])
      return false;
  }
  return true;
}

bool BigNum::operator< (const BigNum& rhs) const {
  //Every negative number is less than every positive number
  if (sign == Sign::neg && rhs.sign == Sign::pos) {
    return true;
  //Every positive number is greater than every negative number
  } else if (sign == Sign::pos && rhs.sign == Sign::neg) {
    return false;
  }
  //Otherwise signs are the same
  if (m_vals.size() > rhs.m_vals.size())
    return false;
  for (std::size_t i = m_vals.size(); i != std::numeric_limits<std::size_t>::max(); --i) {
    if (m_vals[i] > rhs.m_vals[i])
      return false;
  }
  //Equality is not <
  return !(m_vals[0] == rhs.m_vals[0]);
}

//Compound assignment:
BigNum& BigNum::operator+=(const BigNum& rhs) {
  //Addition
  if (sign == rhs.sign) {
    //Resize to be the larger of the two vectors
    m_vals.resize((m_vals.size() < rhs.m_vals.size()) ? m_vals.size() : rhs.m_vals.size());
    //Loop over the smaller of them
    std::size_t small = (m_vals.size() > rhs.m_vals.size()) ? m_vals.size() : rhs.m_vals.size();
    for (std::size_t i = 0; i != small; ++i) {
      m_vals[i] += rhs.m_vals[i];
      if (m_vals[i] >= max_word) {
        if (i == m_vals.size() - 1)
          m_vals.push_back(0);
        //This will only carry 1s in normal circumstances
        //Up to 9999+9999+1
        m_vals[i+1] += m_vals[i]/max_word;
        m_vals[i] %= max_word;
      }
    }
    if (m_vals.size() < rhs.m_vals.size()) {
      //Copy end of rhs.m_vals into m_vals
      m_vals.insert(m_vals.end(),
        rhs.m_vals.begin()+(rhs.m_vals.size()-m_vals.size()), rhs.m_vals.end());
    }
  //Subtraction
  } else {
    //Return 0
    if (abs(*this) == abs(rhs)) {
      m_vals.clear();
      m_vals.push_back(0);
      m_vals.shrink_to_fit();
      sign = Sign::pos;
    //Keep sign, perform subtraction
    } else if (abs(*this) > abs(rhs)) {
      
    //Change sign, perform subtraction
    } else {
      sign = static_cast<Sign>(!sign);
      
    }
  }
  return *this;
}

std::size_t BigNum::length() {
  //Note that the last word cannot be 0 in a valid BigNum
  return m_vals.size() + static_cast<std::size_t>(log10(m_vals.back())) + 1;
}

//Digital access (returns 0-9, 0 for OOB)
unsigned char BigNum::operator[](std::size_t) {
  std::size_t word = pos/max_power;
  //I decided it was slightly less ugly than using pow()
  static unsigned short p_table[] = {1, 10, 100, 1000};
  unsigned short m = p_table[pos%max_power];
  if (word < m_vals.size())
    return static_cast<unsigned char>((m_vals[word]%(10*m))/m);
  else return 0;
}

BigNum& BigNum::set(std::size_t pos, unsigned char v) {
  std::size_t word = pos/max_power;
  //I decided it was slightly less ugly than using pow()
  static unsigned short p_table[] = {1, 10, 100, 1000};
  unsigned short m = p_table[pos%max_power];
  if (word+1 >= m_vals.size()) {
    m_vals.resize(word+1);
  }
  m_vals[word] = m_vals[word] - (m_vals[word]%(10*m))/m + (v%10)*m;
  return *this;
}

//For simplicity of implementation:
//rhs MUST BE < *(this)
/*BigNum& BigNum::smallSub(const BigNum& rhs) {
  assert(abs(rhs) < abs(*this));
  return rhs;
}//*/
//Equivalent to multiplication by 10^(by) but simpler
BigNum shiftTens(BigNum x, short by)
{
  unsigned short words = abs(by)/BigNum::max_power, digits = abs(by)%BigNum::max_power;
  //Positive (multiply)
  if (by > 0) {
    for (unsigned short i = words; i; --i) {
      //Add a 0 in the LSP
      x.m_vals.insert(x.m_vals.begin(), 0);
    }
    if (digits) {
      //Needed because next operation invalidates x
      std::size_t xl = x.length();
      //Make space for more digits
      x.m_vals.push_back(0);
      //Loop down
      for (std::size_t i = xl; i != 0; --i) {
        x.set(i+digits, x[i]);
      }
      //The last word won't be overwritten so we need to set it if it matters
      if (words == 0) {
        for (unsigned short i = digits; i!= 0; --i) {
          x.set(i,0);
        }
      }
    }
  //Negative (divide)
  } else if (by < 0) {
    for (unsigned short i = words; i != 0; --i) {
      //Delete the LSP
      x.m_vals.erase(x.m_vals.begin());
    }
    if (digits) {
      //Loop up
      for (unsigned short i = 0; i != x.length(); ++i) {
        x.set(i, x[i+digits]);
      }
      //Clear top
      for (unsigned short i = digits; i!= 0; --i) {
        x.set(x.length() - i, 0);
      }
    }
  } //Note: 0 is no-op
  return x;
}