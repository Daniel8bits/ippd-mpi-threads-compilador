#ifndef REGULAR_EXPRESSION_H
#define REGULAR_EXPRESSION_H

#include <string>
#include <vector>
#include <utility>
#include <optional>
#include <iostream>

#include "utils.h"
#include "lexicon_tokenizer.h"

enum RegexOpType {
  None,
  OneToMany,
  ZeroToMany,
  ZeroOrOne
};

struct RegexNode {
  RegexNode(RegexOpType operation);
  virtual ~RegexNode();
  RegexOpType operation;
};

struct RegexOperandNode : public RegexNode {
  RegexOperandNode(RegexOpType operation, std::string operand);
  std::string operand;
};

struct RegexBranch {
  RegexBranch();
  std::vector<RegexNode *> nodes;
};

struct RegexListNode : public RegexNode {
  RegexListNode(RegexOpType operation);
  std::vector<RegexBranch *> branches;
};

class RegexSemantic {
public:
  std::optional<Pair<RegexListNode *, LexiconDictionary *> *> * create_graph(std::vector<Token> * symbol_table);
  Grammar * create_grammar(Pair<RegexListNode *, LexiconDictionary *> * metadata);
};

class RegularExpression {
private:
  LexiconDictionary lexicon_dictionary;
  LexiconTokenizer * lexicon_tokenizer;
  RegexSemantic * regex_semantic;

  std::string regex;
  Grammar * regex_grammar;

  void generate_dictionary();
  Grammar * generate_grammar();
public:
  RegularExpression(std::string regex = "");
  bool match(std::string value);
  void generate(std::string regex);
  std::optional<Pair<Grammar *, LexiconDictionary *> *> * create_grammar(std::string regex);
  std::optional<Pair<RegexListNode *, LexiconDictionary *> *> * create_graph(std::string regex);
  std::optional<std::vector<Token> *> * process(std::string regex);
  Grammar *getRegex_grammar() const;
  RegexSemantic * get_regex_semantic() const;
  LexiconTokenizer * get_lexicon_tokenizer() const;
};



#endif // REGULAR_EXPRESSION_H
