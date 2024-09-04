#include "lexicon_tokenizer.h"

ASCIITokenType::ASCIITokenType(std::string name, std::string ascii):
  TokenType(name),
  ascii(ascii)
{}

PolicyTokenType::PolicyTokenType(std::string name, bool allowUpperCase, bool allowLowerCase, bool allowDigits):
  TokenType(name),
  allowUpperCase(allowUpperCase),
  allowLowerCase(allowLowerCase),
  allowDigits(allowDigits)
{}

RangeTokenType::RangeTokenType(std::string name, std::string min, std::string max):
  TokenType(name),
  min(min),
  max(max)
{}

NotTokenType::NotTokenType(std::string name, std::vector<std::string> exclude): TokenType(name) {
  this->exclude = exclude;
}

// LEXICON DICTIONARY

LexiconDictionary::LexiconDictionary(): Dictionary() {}

LexiconDictionary& LexiconDictionary::create_ascii_tt(std::string name, std::string ascii) {
  this->dictionary[name] = new ASCIITokenType(name, ascii);
  return *this;
}

LexiconDictionary& LexiconDictionary::create_policy_tt(std::string name, bool allowUpperCase, bool allowLowerCase, bool allowDigits) {
  this->dictionary[name] = new PolicyTokenType(name, allowUpperCase, allowLowerCase, allowDigits);
  return *this;
}

LexiconDictionary& LexiconDictionary::create_range_tt(std::string name, std::string min, std::string max) {
  this->dictionary[name] = new RangeTokenType(name, min, max);
  return *this;
}

LexiconDictionary& LexiconDictionary::create_not_tt(std::string name, std::vector<std::string> exclude) {
  this->dictionary[name] = new NotTokenType(name, exclude);
  return *this;
}

// LEXICON TOKENIZER

TokenizerStatus LexiconTokenizer::accepted(TokenizerState& state, TokenType * token_type) {
  if        (typeid(*token_type) == typeid(ASCIITokenType)) {
    auto tt = dynamic_cast<ASCIITokenType *>(token_type);
    return state.token == tt->ascii
      ? TokenizerStatus::Accepted
      : TokenizerStatus::Failed;
  } else if (typeid(*token_type) == typeid(PolicyTokenType)) {
    auto tt = dynamic_cast<PolicyTokenType *>(token_type);
    char t = state.token.c_str()[0];
    return (tt->allowDigits && t >= '0' && t <= '9') ||
        (tt->allowLowerCase && t >= 'a' && t <= 'z') ||
        (tt->allowUpperCase && t >= 'A' && t <= 'Z')
        ? TokenizerStatus::Accepted
        : TokenizerStatus::Failed;
  } else if (typeid(*token_type) == typeid(RangeTokenType)) {
    // auto tt = dynamic_cast<RangeTokenType *>(token_type);
  } else if (typeid(*token_type) == typeid(NotTokenType)) {
    // auto tt = dynamic_cast<NotTokenType *>(token_type);
  }

  return TokenizerStatus::Failed;
}

LexiconTokenizer::LexiconTokenizer(Grammar * root): Tokenizer(root) {}


