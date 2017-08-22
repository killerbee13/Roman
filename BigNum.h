#ifndef BIGNUM_H_DEFINED
#define BIGNUM_H_DEFINED

#include <cctype>
#include <cstdint>
#include <cstddef>
#include <string>
#include <sstream>
#include <iomanip>
//Possibly to be replaced with map
#include <vector>

//Use actual decimal math, 4 decimal digits to a "word"
class BigNum {
public:
  BigNum(long long=0);
  BigNum(unsigned long long);
  BigNum(long double);
  BigNum(const std::string&);
  
  //Comparisons: (Others outside class)
  bool operator==(const BigNum& rhs) const;
  bool operator< (const BigNum& rhs) const;
  
  //Arithmetic assignment block: (Non-assignment outside)
  BigNum& operator+=(const BigNum& rhs);
  //These are too complex, I'll add them later
  /*BigNum& operator*=(const BigNum& rhs);
  BigNum& operator/=(const BigNum& rhs);
  BigNum& operator%=(const BigNum& rhs);//*/
  
  //Equivalent to multiplication by 10^(by) but simpler
  friend BigNum shiftTens(BigNum x, short by);
  
  //Explicit sign bit is weird
  friend BigNum operator-(BigNum x);
  friend BigNum abs(BigNum x);
  friend bool signbit(const BigNum& x);
  
  //Get number of digits
  std::size_t length();
  //Increasing significance: [0] = ones, [9] = billions
  //Digital access (returns 0-9, 0 for OOB)
  unsigned char operator[](std::size_t);
  //Set digit in position
  BigNum& set(std::size_t, unsigned char);
  
  //Write to stream
  friend std::string to_string(const BigNum& x);
  
  //For simplicity of implementation:
  //rhs MUST BE < *(this)
  BigNum& smallSub(const BigNum& rhs);
  friend BigNum smallSub(BigNum lhs, const BigNum& rhs);
private:
  //Equivalent descriptions of the largest "word" magnitude
  static constexpr unsigned long long max_word = 10000, max_power = 4;
  std::vector<unsigned short> m_vals;
  enum Sign : bool {pos, neg} sign;
  
  //Get number of words
  std::size_t size();
  //Set entire word
  BigNum& setw(std::size_t, unsigned short);
};

//This should handle numeric literals
//Note: Only use this for decimal integer literals
inline BigNum operator"" _b (const char* x) {
  return BigNum(std::string(x));
}
//Floating-point literals
inline BigNum operator"" _b (long double x) {
  return BigNum(x);
}

//BigNum comparisons
inline bool operator!=(const BigNum& lhs, const BigNum& rhs) {
  return !(lhs == rhs);
}
inline bool operator> (const BigNum& lhs, const BigNum& rhs) {
  return (rhs < lhs);
}
inline bool operator<=(const BigNum& lhs, const BigNum& rhs) {
  return !(rhs < lhs);
}
inline bool operator>=(const BigNum& lhs, const BigNum& rhs) {
  return !(lhs < rhs);
}
//Prevent unnecessary duplication of code
inline BigNum& operator-=(BigNum& lhs, const BigNum& rhs) {
  return lhs+=(-rhs);
}
//Arithmetic: (inline)
inline BigNum operator+(BigNum lhs, const BigNum& rhs) {
  lhs += rhs;
  return lhs;
}
inline BigNum operator-(const BigNum& lhs, const BigNum& rhs) {
  BigNum tmp;
  if (lhs < rhs) {
    tmp = lhs;
    tmp-=rhs;
  } else {
    tmp = rhs;
    tmp-=lhs;
  }
  return tmp;
}
BigNum smallSub(BigNum lhs, const BigNum& rhs);
BigNum shiftTens(BigNum x, short by);
/*inline BigNum operator*(BigNum lhs, const BigNum& rhs) {
  lhs *= rhs;
  return lhs;
}
inline BigNum operator/(BigNum lhs, const BigNum& rhs) {
  lhs /= rhs;
  return lhs;
}
inline BigNum operator%(BigNum lhs, const BigNum& rhs) {
  lhs %= rhs;
  return lhs;
}//*/
//Prefix increment
inline BigNum& operator++(BigNum& x) {
  return x += 1_b;
}
//prefix decrement
inline BigNum& operator--(BigNum& x) {
  return x += -(1_b);
}
//postfix increment
inline BigNum operator++(BigNum x, int) {
  BigNum tmp(x);
  ++x;
  return tmp;
}
//postfix decrement
inline BigNum operator--(BigNum x, int) {
  BigNum tmp(x);
  --x;
  return tmp;
}
//Unary minus operator
inline BigNum operator-(BigNum x) {
  x.sign = static_cast<BigNum::Sign>(!x.sign);
  return x;
}
inline BigNum abs(BigNum x) {
  x.sign = BigNum::pos;
  return x;
}
inline bool signbit(const BigNum& x) {
  return static_cast<bool>(x.sign);
}

inline std::ostream& operator<<(std::ostream& os, const BigNum& x)
{
  os<<to_string(x);
  return os;
}
inline std::string to_string(const BigNum& x)
{
  std::stringstream buf;
  for (auto it = x.m_vals.rbegin(); it != x.m_vals.rend(); ++it) {
    //ret+=std::to_string(*it);
    buf<<*it<<std::setw(0)<<' '<<std::setw(4)<<std::setfill('0');
  }
  return buf.str();
}

#endif //BIGNUM_H_DEFINED