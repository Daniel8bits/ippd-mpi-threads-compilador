#ifndef LEXICON_TOKENIZER_H
#define LEXICON_TOKENIZER_H

#include <typeinfo>

#include "common.h"

struct ASCIITokenType : public TokenType {
  ASCIITokenType(std::string name, std::string ascii);
  std::string ascii;
};

struct PolicyTokenType : public TokenType {
  PolicyTokenType(std::string name, bool allowUpperCase, bool allowLowerCase, bool allowDigits);
  bool allowUpperCase;
  bool allowLowerCase;
  bool allowDigits;
};

struct RangeTokenType : public TokenType {
  RangeTokenType(std::string name, std::string min, std::string max);
  std::string min;
  std::string max;
};

struct NotTokenType : public TokenType {
  NotTokenType(std::string name, std::vector<std::string> exclude);
  std::vector<std::string> exclude;
};

class LexiconDictionary : public Dictionary {
public:
  LexiconDictionary();
  LexiconDictionary& create_ascii_tt(std::string name, std::string ascii);
  LexiconDictionary& create_policy_tt(std::string name, bool allowUpperCase = true, bool allowLowerCase = true, bool allowDigits = true);
  LexiconDictionary& create_range_tt(std::string name, std::string min, std::string max);
  LexiconDictionary& create_not_tt(std::string name, std::vector<std::string> exclude);
};

class LexiconTokenizer : public Tokenizer {
protected:
  TokenizerStatus accepted(TokenizerState& state, TokenType * type);
public:
  LexiconTokenizer(Grammar * root);
};

#endif // LEXICON_TOKENIZER_H
