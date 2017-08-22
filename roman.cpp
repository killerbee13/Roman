#include "roman.h"

#include <iostream>
#include <array>
#include <map>

#warning This file has been deprecated

std::map<unsigned long long int, char> values = {
    {1ULL,'I'},             {5ULL,'V'},
    {10ULL,'X'},            {50ULL,'L'},
    {100ULL,'C'},           {500ULL,'D'},
    {1000ULL,'M'},//Simple
                            {5000ULL,'A'},
    {10000ULL,'B'},         {50000ULL,'E'},
    {100000ULL,'F'},        {500000ULL,'G'},
    {1000000ULL,'H'},       {5000000ULL,'J'},
    {10000000ULL,'K'},      {50000000ULL,'N'},
    {100000000ULL,'O'},     {500000000ULL,'P'},
    {1000000000ULL,'Q'},    {5000000000ULL,'R'},
    {10000000000ULL,'S'},   {50000000000ULL,'T'},
    {100000000000ULL,'U'},  {500000000000ULL,'W'},
    {1000000000000ULL,'Y'}, {5000000000000ULL,'Z'},
  };

std::map<char, unsigned long long int> valuesR = {
    {'I',1ULL},             {'V',5ULL},
    {'X',10ULL},            {'L',50ULL},
    {'C',100ULL},           {'D',500ULL},
    {'M',1000ULL},//Simple
                            {'A',5000ULL},
    {'B',10000ULL},         {'E',50000ULL},
    {'F',100000ULL},        {'G',500000ULL},
    {'H',1000000ULL},       {'J',5000000ULL},
    {'K',10000000ULL},      {'N',50000000ULL},
    {'O',100000000ULL},     {'P',500000000ULL},
    {'Q',1000000000ULL},    {'R',5000000000ULL},
    {'S',10000000000ULL},   {'T',50000000000ULL},
    {'U',100000000000ULL},  {'W',500000000000ULL},
    {'Y',1000000000000ULL}, {'Z',5000000000000ULL},
    //Basic extended
    {'i',1000ULL},          {'v',5000ULL},
    {'x',10000ULL},         {'l',50000ULL},
    {'c',100000ULL},        {'d',500000ULL},
    {'m',1000000ULL},
  };

std::array<unsigned long long int, 26> valVals = {
    1ULL,             5ULL,
    10ULL,            50ULL,
    100ULL,           500ULL,
    1000ULL,//Simple
                      5000ULL,
    10000ULL,         50000ULL,
    100000ULL,        500000ULL,
    1000000ULL,       5000000ULL,
    10000000ULL,      50000000ULL,
    100000000ULL,     500000000ULL,
    1000000000ULL,    5000000000ULL,
    10000000000ULL,   50000000000ULL,
    100000000000ULL,  500000000000ULL,
    1000000000000ULL, 5000000000000ULL,
  };

//Exact implementation for 64-bit ints (slightly recursive)
std::string toERoman(unsigned long long val, unsigned int depth, size_t maxVal)
{
  if (maxVal > 24) {
    return std::string("");
  }
  std::string ret;
  //Recurse if overly large)
         if (val >= 10*valVals[maxVal]) {
    ret += toERoman(val/(10*valVals[maxVal]), depth+1, maxVal);
  } else if (val >= 9*valVals[maxVal]) { //Y(I) or (in "c" mode, C(I)
    ret += repeat('(',depth) + values[valVals[maxVal]] + repeat(')',depth)
      + repeat('(',depth+1) + values[valVals[0]] + repeat(')',depth+1);
      val %= valVals[maxVal];
  }
  val %= 10*valVals[maxVal];
  for (unsigned int s = maxVal; ; s-=2) {
    //std::cout<<s<<';'<<(val%(valVals[s]*10))/valVals[s]<<std::endl;
    //if val is greater than current char then get last digit
    switch ((val%(valVals[s]*10))/valVals[s]) {
    case 9: //IX
      ret += repeat('(',depth) + values[valVals[s]] + repeat(')',depth);
      if (s != maxVal) //X
        ret += repeat('(',depth) + values[valVals[s+2]] + repeat(')',depth);
      else //(I)
        ret += repeat('(',depth+1) + values[valVals[0]] + repeat(')',depth+1);
      break;
      break;
    case 8: //VIII
      ret += repeat('(',depth) + values[valVals[s+1]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[s]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[s]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[s]] + repeat(')',depth);
      break;
    case 7: //VII
      ret += repeat('(',depth) + values[valVals[s+1]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[s]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[s]] + repeat(')',depth);
      break;
    case 6: //VI
      ret += repeat('(',depth) + values[valVals[s+1]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[s]] + repeat(')',depth);
      break;
    case 5: //V
      ret += repeat('(',depth) + values[valVals[s+1]] + repeat(')',depth);
      break;
    case 4: //IV
      ret += repeat('(',depth) + values[valVals[s]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[s+1]] + repeat(')',depth);
      break;
    case 3: //III
      ret += repeat('(',depth) + values[valVals[s]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[s]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[s]] + repeat(')',depth);
      break;
    case 2: //II
      ret += repeat('(',depth) + values[valVals[s]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[s]] + repeat(')',depth);
      break;
    case 1: //I
      ret += repeat('(',depth) + values[valVals[s]] + repeat(')',depth);
      break;
    default:
      break;
    }
    if (!s) {
      break;
    }
  }
  return ret;
}

//Lower-accuracy slower version for very large numbers
//1200000000000000000000000 becomes
//1200000000000000033554432
std::string toERoman(long double val, unsigned int depth, size_t maxVal)
{
  if (maxVal > 24) {
    return std::string("");
  }
  std::string ret;
  //Recurse if overly large)
         if (val >= 10*valVals[maxVal]) {
    ret += toERoman(val/(10*valVals[maxVal]), depth+1, maxVal);
  } else if (val >= 9*valVals[maxVal]) { //Y(I) or (in "c" mode, C(I)
    ret += repeat('(',depth) + values[valVals[maxVal]] + repeat(')',depth)
      + repeat('(',depth+1) + values[valVals[0]] + repeat(')',depth+1);
      val = fmod(val,(double)valVals[maxVal]);
  }
  val = fmod(val,(double)10*valVals[maxVal]);
  for (unsigned int s = maxVal; ; s-=2) {
    //std::cout<<s<<';'<<(val%(valVals[s]*10))/valVals[s]<<std::endl;
    //if val is greater than current char then get last digit
    switch ((int)trunc(fmod(val,(valVals[s]*10))/valVals[s])) {
    case 9: //IX
      ret += repeat('(',depth) + values[valVals[s]] + repeat(')',depth);
      if (s != maxVal) //X
        ret += repeat('(',depth) + values[valVals[s+2]] + repeat(')',depth);
      else //(I)
        ret += repeat('(',depth+1) + values[valVals[0]] + repeat(')',depth+1);
      break;
      break;
    case 8: //VIII
      ret += repeat('(',depth) + values[valVals[s+1]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[s]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[s]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[s]] + repeat(')',depth);
      break;
    case 7: //VII
      ret += repeat('(',depth) + values[valVals[s+1]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[s]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[s]] + repeat(')',depth);
      break;
    case 6: //VI
      ret += repeat('(',depth) + values[valVals[s+1]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[s]] + repeat(')',depth);
      break;
    case 5: //V
      ret += repeat('(',depth) + values[valVals[s+1]] + repeat(')',depth);
      break;
    case 4: //IV
      ret += repeat('(',depth) + values[valVals[s]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[s+1]] + repeat(')',depth);
      break;
    case 3: //III
      ret += repeat('(',depth) + values[valVals[s]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[s]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[s]] + repeat(')',depth);
      break;
    case 2: //II
      ret += repeat('(',depth) + values[valVals[s]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[s]] + repeat(')',depth);
      break;
    case 1: //I
      ret += repeat('(',depth) + values[valVals[s]] + repeat(')',depth);
      break;
    default:
      break;
    }
    if (!s) {
      break;
    }
  }
  return ret;
}

//Exact implementation for text input
std::string toERoman(std::string val, unsigned int depth, size_t maxVal)
{
  if (maxVal > 24) {
    return std::string("");
  }
  std::string ret;
  //Pad val to be a multiple of (maxVal/2) long
  if (val.length() % maxVal/2) {
    val = std::string(((val.length() / (maxVal/2) + 1)*maxVal/2) - val.length(), '0') + val;
  }
  std::string valR = val;
  std::reverse(valR.begin(), valR.end());
  //std::cout<<valR<<std::endl;
  //Recurse if overly large)
  if (val.length() > maxVal/2) {
    ret = toERoman(val.substr(0, val.size()-(maxVal/2+1)), depth+1, maxVal);
  }
  val = valR.substr(0, maxVal/2+1);
  for (unsigned int s = val.length() - 1; ; --s) {
    //std::cout<<s<<';'<<(val%(valVals[s]*10))/valVals[s]<<std::endl;
    //if val is greater than current char then get last digit
    switch (val[s]) {
    case '9': //IX
      ret += repeat('(',depth) + values[valVals[2*s]] + repeat(')',depth);
      if (s != maxVal/2) //X
        ret += repeat('(',depth) + values[valVals[2*s+2]] + repeat(')',depth);
      else //(I)
        ret += repeat('(',depth+1) + values[valVals[0]] + repeat(')',depth+1);
      break;
    case '8': //VIII
      ret += repeat('(',depth) + values[valVals[2*s+1]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[2*s]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[2*s]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[2*s]] + repeat(')',depth);
      break;
    case '7': //VII
      ret += repeat('(',depth) + values[valVals[2*s+1]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[2*s]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[2*s]] + repeat(')',depth);
      break;
    case '6': //VI
      ret += repeat('(',depth) + values[valVals[2*s+1]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[2*s]] + repeat(')',depth);
      break;
    case '5': //V
      ret += repeat('(',depth) + values[valVals[2*s+1]] + repeat(')',depth);
      break;
    case '4': //IV
      ret += repeat('(',depth) + values[valVals[2*s]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[2*s+1]] + repeat(')',depth);
      break;
    case '3': //III
      ret += repeat('(',depth) + values[valVals[2*s]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[2*s]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[2*s]] + repeat(')',depth);
      break;
    case '2': //II
      ret += repeat('(',depth) + values[valVals[2*s]] + repeat(')',depth)
        + repeat('(',depth) + values[valVals[2*s]] + repeat(')',depth);
      break;
    case '1': //I
      ret += repeat('(',depth) + values[valVals[2*s]] + repeat(')',depth);
      break;
    default:
      break;
    }
    if (!s) { //Because testing in the loop statement was giving odd results
      break;
    }
  }
  return ret;
}

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

//Sanitize numeral string
std::string sanitizeNumeral(std::string in)
{
  std::string tmp;
  for (auto i : in) {
    if (valuesR.count(i) || i == '(' || i == ')') {
      tmp += i;
    }
  }
  return tmp;
}

//Convert to standard numerals
std::string toSRoman(unsigned short int val, size_t maxVal) {
  return repeat(values[valVals[maxVal]], val/valVals[maxVal])
    + toERoman(val % valVals[maxVal], 0);
}

std::map<char,char> Lmap = {
    {'M','i'}, {'A','v'},
    {'B','x'}, {'E','l'},
    {'F','c'}, {'G','d'},
    //To simplify the code, at the cost of increased rigidity
    {'I','I'}, {'V','V'},
    {'X','X'}, {'L','L'},
    {'C','C'}, {'D','D'},
    {'(','('}, {')',')'},
  };

//Exact implementation for lowercase-extended numerals
std::string toLRoman(unsigned long long int val) {
  std::string tmp = toERoman(val, 0, 10);
  std::for_each(tmp.begin(), tmp.end(), [](char& i){i = Lmap[i];});
  return tmp;
}

//Floating-point implementation for lowercase-extended numerals
std::string toLRoman(long double val) {
  std::string tmp = toERoman(val, 0, 10);
  std::for_each(tmp.begin(), tmp.end(), [](char& i){i = Lmap[i];});
  return tmp;
}

//Exact implementation for lowercase-extended numerals
std::string toLRoman(std::string val) {
  std::string tmp = toERoman(val, 0, 10);
  std::for_each(tmp.begin(), tmp.end(), [](char& i){i = Lmap[i];});
  return tmp;
}

//Convert from roman numerals to integer
unsigned long long int fromERomanTI(std::string val, size_t maxVal)
{
  size_t run_length = 0;
  unsigned long long prev = 0, out = 0, curr = 0, mult = 1, sub = 0;
  for (auto i : val) {
    if (valuesR.count(i)) {
      curr = valuesR[i] * mult;
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
      mult *= valVals[maxVal]*10;
    } else if (i == ')') {
      mult /= valVals[maxVal]*10;
    }
  }
  out += prev * run_length;
  out -= sub; //sub is (by definition) less than out here
  return out;
}

//Convert from roman numerals to floating-point type
long double fromERomanTF(std::string val, size_t maxVal)
{
  size_t run_length = 0;
  long double prev = 0, out = 0, curr = 0, mult = 1;
  for (auto i : val) {
    if (valuesR.count(i)) {
      curr = valuesR[i] * mult;
             if (curr < prev) {
        out += prev * run_length;
        run_length = 0;
      } else if (curr > prev) {
        out -= prev * run_length;
        run_length = 0;
      }//If equal, do nothing special
      prev = curr;
      ++run_length;
    } else if (i == '(') {
      mult *= valVals[maxVal]*10;
    } else if (i == ')') {
      mult /= valVals[maxVal]*10;
    }
  }
  out += prev * run_length;
  return out;
}

//Convert from roman numerals to string
//(NYI)
std::string fromERomanTS(std::string val, size_t maxVal)
{
  size_t run_length = 0;
  unsigned long long prev = 0, out = 0, curr = 0, mult = 1, sub = 0;
  for (auto i : val) {
    if (valuesR.count(i)) {
      curr = valuesR[i] * mult;
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
      mult *= valVals[maxVal]*10;
    } else if (i == ')') {
      mult /= valVals[maxVal]*10;
    }
  }
  out += prev * run_length;
  out -= sub; //sub is (by definition) less than out here
  return toStr(out);
}
