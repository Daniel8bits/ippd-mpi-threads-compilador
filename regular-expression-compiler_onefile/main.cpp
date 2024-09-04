#include <mpi/mpi.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <queue>
#include <mutex>
#include <functional>
#include <map>
#include <stack>
#include <typeinfo>
//#include "regular_expression.h"

int rank_aux;

#define LOG(M) std::cout << M << "    ( rank: " << std::to_string(rank_aux) << " )" << std::endl;


//========================
// UTILS

template<class T1, class T2>
class Pair {
public:
  Pair(T1 first, T2 second): first(first), second(second) {};
  T1 first;
  T2 second;
};

template<class T>
class Optional {
private:
  bool initialized;
  T * value;
public:
  Optional(T * value): initialized(value != nullptr), value(value) {};
  Optional(): initialized(false), value(nullptr) {};
  constexpr bool has_value() {return initialized;}
  T * get_value() {return value;}
};

//========================
// COMMON


struct TokenType {
  TokenType(std::string name);
  virtual ~TokenType();
  std::string name;
};

class Dictionary {
protected:
  std::map<std::string, TokenType *> dictionary;
public:
  Dictionary();
  TokenType * get(std::string& name);
  bool has(std::string& name);
};

class Grammar;

struct GrammarNode {
private:
  static GrammarNode * EMPTY_VALUE;
public:
  GrammarNode(Grammar * grammar);
  GrammarNode(std::string token_type_name);
  Grammar * grammar = nullptr;
  std::string token_type_name = "";
  bool empty = false;
  static GrammarNode EMPTY();
};

struct GrammarBranch {
  std::vector<GrammarNode> nodes;
};

class Grammar {
private:
  std::vector<GrammarBranch *> branches;
  Dictionary * dictionary;
public:
  Grammar(Dictionary * dictionary);
  Grammar * pipe(GrammarBranch branch, bool allow_empty = false);
  Grammar * concat(GrammarNode node); // and
  Grammar * all_concat(GrammarNode node); //allAnd
  GrammarBranch * get_branch(size_t i); // getNode
  size_t size();
  Dictionary * get_dictionary();

  std::string counter = "";

  Grammar& operator&(std::string token_type_name);
  Grammar& operator&(GrammarNode b);
  Grammar& operator&(Grammar * b);

  Grammar& operator<<(std::string token_type_name);
  Grammar& operator<<(GrammarNode b);
  Grammar& operator<<(Grammar * b);

  Grammar& operator|(std::string token_type_name);
  Grammar& operator|(GrammarNode b);
  Grammar& operator|(Grammar * b);
};

struct Token {
  Token() = delete;
  size_t position;
  TokenType * token_type;
  std::string content;
};

class History {
private:
  size_t begin_token_backup;
  size_t symbol_table_index_backup;
  size_t branch_pointer;
  size_t node_pointer;
  Grammar * grammar;

public:
  History(Grammar * grammar);
  static History * create_from_root(Grammar * root);
  GrammarNode next();
  void update_pointers();
  void change_branch();
  bool is_next_a_grammar();
  bool is_empty();
  bool is_branch_end();
  bool is_last_branch();
  TokenType * get_token_type();
  History * create_from_next(size_t begin_token, size_t symbol_table_index);

  size_t get_begin_token_backup();
  size_t get_symbol_table_index_backup();
  size_t get_branch_pointer();
  size_t get_node_pointer();
  Grammar * get_grammar();
};

enum TokenizerStatus {
  Failed,
  PartiallyFailed,
  Accepted
};

struct TokenizerState {
  TokenizerState() = delete;
  std::string& language;
  std::string token;
  size_t begin_token;
  size_t symbol_table_index;
  size_t pointer;
};

class Tokenizer {
private:
  Grammar * root;
protected:
  virtual TokenizerStatus accepted(TokenizerState& state, TokenType * token_type) = 0;
public:
  Tokenizer(Grammar * root);
  Optional<std::vector<Token>> * process(std::string& language);
};


// TOKEN TYPE

TokenType::TokenType(std::string name): name(name) {}

TokenType::~TokenType() {}

// DICTIONARY

Dictionary::Dictionary() {}

TokenType * Dictionary::get(std::string& name) {
  return this->dictionary[name];
}

bool Dictionary::has(std::string& name) {
  return this->dictionary.find(name) != this->dictionary.end();
}

// GRAMMAR

GrammarNode::GrammarNode(Grammar * grammar):
  grammar(grammar)
{}

GrammarNode::GrammarNode(std::string token_type_name):
  token_type_name(token_type_name)
{}

GrammarNode * GrammarNode::EMPTY_VALUE = nullptr;

GrammarNode GrammarNode::EMPTY() {
  if (!EMPTY_VALUE) {
    EMPTY_VALUE = new GrammarNode(nullptr);
    EMPTY_VALUE->empty = true;
  }
  return *EMPTY_VALUE;
}

Grammar::Grammar(Dictionary * dictionary): dictionary(dictionary) {}

Grammar * Grammar::pipe(GrammarBranch branch, bool allow_empty) {
  auto b = new GrammarBranch; //new std::vector<GrammarNode>;
  std::copy_if(branch.nodes.begin(), branch.nodes.end(), std::back_inserter(b->nodes), [this](GrammarNode& node) {
    return node.grammar != nullptr || this->dictionary->has(node.token_type_name);
  });

  if (!b->nodes.empty() || allow_empty) this->branches.emplace_back(b);

  return this;
}

Grammar * Grammar::concat(GrammarNode node) {
  if (node.grammar != nullptr || this->dictionary->has(node.token_type_name))
    this->branches.back()->nodes.emplace_back(node);

  return this;
}

Grammar * Grammar::all_concat(GrammarNode node) {
  std::for_each(this->branches.begin(), this->branches.end(), [&node, this](GrammarBranch * branch) {
    if (node.grammar != nullptr || this->dictionary->has(node.token_type_name))
      branch->nodes.emplace_back(node);
  });

  return this;
}

GrammarBranch * Grammar::get_branch(size_t i) {
  return this->branches[i];
}

size_t Grammar::size() {
  return this->branches.size();
}

Dictionary * Grammar::get_dictionary() {
  return this->dictionary;
}

Grammar& Grammar::operator&(std::string token_type_name) {
  this->counter += " " + token_type_name;
  this->branches.back()->nodes.emplace_back(GrammarNode(token_type_name));
  return *this;
}

Grammar& Grammar::operator&(GrammarNode b) {
  this->counter += " <empty>";
  this->branches.back()->nodes.emplace_back(b);
  return *this;
}

Grammar& Grammar::operator&(Grammar * b) {
  this->counter += " <GRAMMAR>";
  this->branches.back()->nodes.emplace_back(GrammarNode(b));
  return *this;
}

Grammar& Grammar::operator<<(std::string token_type_name) {
  this->counter += "THIS := " + token_type_name;
  this->pipe(GrammarBranch(), true);
  this->branches.back()->nodes.emplace_back(GrammarNode(token_type_name));
  return *this;
}

Grammar& Grammar::operator<<(GrammarNode b) {
  this->counter += "THIS := <empty>";
  this->pipe(GrammarBranch(), true);
  this->branches.back()->nodes.emplace_back(b);
  return *this;
}

Grammar& Grammar::operator<<(Grammar * b) {
  this->counter += "THIS := <GRAMMAR>";
  this->pipe(GrammarBranch(), true);
  this->branches.back()->nodes.emplace_back(GrammarNode(b));
  return *this;
}

Grammar& Grammar::operator|(std::string token_type_name) {
  this->counter += " | " + token_type_name;
  this->pipe(GrammarBranch(), true);
  this->branches.back()->nodes.emplace_back(GrammarNode(token_type_name));
  return *this;
}

Grammar& Grammar::operator|(GrammarNode b) {
  this->counter += " | <empty>";
  this->pipe(GrammarBranch(), true);
  this->branches.back()->nodes.emplace_back(b);
  return *this;
}

Grammar& Grammar::operator|(Grammar * b) {
  this->counter += " | <GRAMMAR>";
  this->pipe(GrammarBranch(), true);
  this->branches.back()->nodes.emplace_back(GrammarNode(b));
  return *this;
}


// HISTORY

History::History(Grammar * grammar):
  begin_token_backup(0),
  symbol_table_index_backup(0),
  branch_pointer(0),
  node_pointer(0),
  grammar(grammar)
{}

History * History::create_from_root(Grammar * root) {
  auto grammar = root->get_branch(0)->nodes[0].grammar;
  if (grammar == nullptr)
    throw ("Root must contain a grammar object!");

  return new History(grammar);
}

GrammarNode History::next() {
  return this->grammar->get_branch(this->branch_pointer)->nodes[this->node_pointer];
}

void History::update_pointers() {
  if (this->branch_pointer == this->grammar->size()) return;
  if (this->node_pointer < grammar->get_branch(this->branch_pointer)->nodes.size()) {
    this->node_pointer++;
    return;
  }
}

void History::change_branch() {
  this->node_pointer = 0;
  this->branch_pointer++;
}

bool History::is_next_a_grammar() {
  return !this->next().empty && this->next().grammar != nullptr;
}

bool History::is_empty() {
  return this->next().empty;
}

bool History::is_branch_end() {
  return grammar->get_branch(this->branch_pointer)->nodes.size() == this->node_pointer;
}

bool History::is_last_branch() {
  return grammar->size() - 1 == this->branch_pointer;
}

TokenType * History::get_token_type() {
  auto token_type_name = this->next().token_type_name;
  return grammar->get_dictionary()->get(token_type_name);
}

History * History::create_from_next(size_t begin_token, size_t symbol_table_index) {
  if (this->next().grammar == nullptr)
    throw ("Next node must be a grammar object!");

  auto history = new History(this->next().grammar);
  history->begin_token_backup = begin_token;
  history->symbol_table_index_backup = symbol_table_index;

  return history;
}

size_t History::get_begin_token_backup() {
  return this->begin_token_backup;
}

size_t History::get_symbol_table_index_backup() {
  return this->symbol_table_index_backup;
}

size_t History::get_branch_pointer() {
  return this->branch_pointer;
}

size_t History::get_node_pointer() {
  return this->node_pointer;
}

Grammar *History::get_grammar() {
  return this->grammar;
}

// TOKENIZER

Tokenizer::Tokenizer(Grammar * root): root(root) {}

Optional<std::vector<Token>> * Tokenizer::process(std::string& language) {
  //std::cout << "\t\t\tCOMMON process: " << std::endl;
  auto symbol_table = new std::vector<Token>;

  std::stack<History *> stack;
  stack.push(History::create_from_root(this->root));

  auto state = TokenizerState {
    .language = language,
    .token = "",
    .begin_token = 0,
    .symbol_table_index = 0,
    .pointer = 0
  };

  bool failed = false;
  while (!stack.empty()) {
    auto history = stack.top();

    if (stack.empty()) break;

    if (failed) {
      if (history->is_last_branch()) {
        stack.pop();
        continue;
      }

      history->change_branch();
      state.begin_token = history->get_begin_token_backup();
      state.symbol_table_index = history->get_symbol_table_index_backup();
      state.pointer = state.begin_token;
      failed = false;
      symbol_table->erase(symbol_table->begin()+state.symbol_table_index, symbol_table->end());
    }

    if (history->is_branch_end()) {
      stack.pop();
      continue;
    }

    if (history->is_next_a_grammar()) {
      stack.push(history->create_from_next(state.begin_token, state.symbol_table_index));
      history->update_pointers();
      continue;
    }

    if (!history->is_empty()) {
      auto substr_length = (state.pointer + 1) - state.begin_token;
      state.token = language.substr(state.begin_token, substr_length);
      auto token_type = history->get_token_type();
      if (!state.token.empty()) {
        auto status = this->accepted(state, token_type);
        if (status == TokenizerStatus::Accepted) {
          history->update_pointers();
          symbol_table->emplace_back(Token {
            .position = state.begin_token,
            .token_type = token_type,
            .content = state.token
          });

          state.symbol_table_index++;
          state.pointer++;
          state.begin_token = state.pointer;

          if (history->is_branch_end()) stack.pop();

          continue;
        } else if (status == TokenizerStatus::PartiallyFailed) {
          state.pointer++;
          continue;
        }
      }
    }

    history->update_pointers();

    if (history->is_last_branch()) {
      failed = true;
      stack.pop();
      continue;
    }

    if (state.begin_token == language.size()) {
      failed = true;
      continue;
    }

    state.pointer = state.begin_token;
    history->change_branch();
  }

  //std::cout << language << std::endl;
  //std::cout << "begin_token: " << state.begin_token << ", language.size: " << language.size() << ", stack size: " << stack.size() << std::endl;
  //std::cout << "kkkkk: " << std::string(state.begin_token == language.size() ? "ok" : "q?") << std::endl;

  if (state.begin_token == language.size() && stack.size() == 0)
    return new Optional<std::vector<Token>>(symbol_table);

  return new Optional<std::vector<Token>>;

}

//========================
// LEXICON TOKENIZER


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

//========================
// REGULAR EXPRESSION


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
  Optional<Pair<RegexListNode *, LexiconDictionary *>> * create_graph(std::vector<Token> * symbol_table);
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
  Optional<Pair<Grammar *, LexiconDictionary *>> * create_grammar(std::string regex);
  Optional<Pair<RegexListNode *, LexiconDictionary *>> * create_graph(std::string regex);
  Optional<std::vector<Token>> * process(std::string regex);
  Grammar *getRegex_grammar() const;
  RegexSemantic * get_regex_semantic() const;
  LexiconTokenizer * get_lexicon_tokenizer() const;
};

// REGEX NODE

RegexNode::RegexNode(RegexOpType operation): operation(operation) {}

RegexNode::~RegexNode() {}

RegexOperandNode::RegexOperandNode(RegexOpType operation, std::string operand):
  RegexNode(operation),
  operand(operand)
{}

RegexBranch::RegexBranch() {}

RegexListNode::RegexListNode(RegexOpType operation): RegexNode(operation) {
  this->branches.emplace_back(new RegexBranch());
}

// REGEX SEMANTIC

Optional<Pair<RegexListNode *, LexiconDictionary *>> * RegexSemantic::create_graph(std::vector<Token> * symbol_table) {
  //LOG("\t\tcg 1");
  auto dictionary = new LexiconDictionary();
  std::stack<RegexListNode *> stack;
  stack.push(new RegexListNode(RegexOpType::None));

  //LOG("\t\tcg 2");

  bool failed = false;
  for (size_t i = 0; i < symbol_table->size(); i++) {
    auto symbol = symbol_table->at(i);
    auto node = stack.top();

    if (stack.empty())
      throw ("Grammar are wrong, make sure to pass a proper symbol!");

    auto branch = node->branches.back();

    auto operation = symbol.token_type->name;

    if        (operation == "op:or") {
      node->branches.emplace_back(new RegexBranch);
    } else if (operation == "op:zero_to_many") {
      branch->nodes.back()->operation = RegexOpType::ZeroToMany;
    } else if (operation == "op:one_to_many") {
      branch->nodes.back()->operation = RegexOpType::OneToMany;
    } else if (operation == "op:zero_or_one") {
      branch->nodes.back()->operation = RegexOpType::ZeroOrOne;
    } else if (operation == "alpnum") {
      branch->nodes.emplace_back(new RegexOperandNode(RegexOpType::None, symbol.content));
      if (!dictionary->has(symbol.content))
        dictionary->create_ascii_tt(symbol.content, symbol.content);
    }
  }

  //LOG("\t\tcg 3");

  auto graph = stack.top();
  bool is_empty = stack.empty();
  stack.pop();

  //LOG("\t\tcg 4");

  if (!is_empty && !failed) {
    return new Optional(new Pair(graph, dictionary));
  }

  //LOG("\t\tcg 6");

  return new Optional<Pair<RegexListNode *, LexiconDictionary *>>;
}

Grammar * RegexSemantic::create_grammar(Pair<RegexListNode *, LexiconDictionary *> * metadata) {
  auto graph = metadata->first;
  auto dictionary = metadata->second;
  auto root = new Grammar(dictionary);

  struct State {
    size_t sequence_pointer;
    size_t breadth_pointer;
    Grammar * depth;
    Grammar * breadth;
    State(size_t sequence_pointer, size_t breadth_pointer, Grammar * depth, Grammar * breadth):
      sequence_pointer(sequence_pointer),
      breadth_pointer(breadth_pointer),
      depth(depth),
      breadth(breadth)
    {}
  };

  std::stack<State *> stack;
  stack.push(new State(0, 0, root, root));

  while (!stack.empty()) {
    auto state = stack.top();

    auto branch = graph->branches[state->breadth_pointer];
    auto node = branch->nodes[state->sequence_pointer];

    if (state->depth->size() == 0) state->depth->pipe(GrammarBranch(), true);

    if (typeid(*node) == typeid(RegexOperandNode)) {
      switch (node->operation) {
        case RegexOpType::None: {
          auto operand = dynamic_cast<RegexOperandNode *>(node)->operand;
          *state->depth & operand;
          break;
        }
        case RegexOpType::ZeroToMany: {
          auto operand = dynamic_cast<RegexOperandNode *>(node)->operand;
          auto X0 = new Grammar(dictionary);
          auto X1 = new Grammar(dictionary);
          auto ZM = new Grammar(dictionary);

          *ZM << operand & ZM | operand;
          if (state->sequence_pointer + 1 < branch->nodes.size()) {
            (*X0 << X1 | ZM) & X1;
          } else {
            (*X0 << GrammarNode::EMPTY() | ZM) & GrammarNode::EMPTY();
          }

          *state->depth & X0;
          state->depth = X1;
          break;
        }
        case RegexOpType::OneToMany: {
          auto operand = dynamic_cast<RegexOperandNode *>(node)->operand;
          auto OM = new Grammar(dictionary);

          *OM << operand & OM | operand;

          *state->depth & OM;
          break;
        }
        case RegexOpType::ZeroOrOne: {
          auto operand = dynamic_cast<RegexOperandNode *>(node)->operand;
          auto X0 = new Grammar(dictionary);
          auto X1 = new Grammar(dictionary);

          if (state->sequence_pointer + 1 < branch->nodes.size()) {
            (*X0 << X1 | operand) & X1;
          } else {
            (*X0 << GrammarNode::EMPTY() | operand) & GrammarNode::EMPTY();
          }

          *state->depth & X0;
          state->depth = X1;
          break;
        }
        default:
          break;
      }

      state->sequence_pointer++;

      if (state->sequence_pointer == branch->nodes.size()) {
        state->breadth_pointer++;

        if (state->breadth_pointer == graph->branches.size()) {
          stack.pop();
          continue;
        }

        state->sequence_pointer = 0;
        state->depth = state->breadth;
        state->breadth->pipe(GrammarBranch(), true);
      }
    }
  }

  auto result = new Grammar(dictionary);
  *result << root;

  return result;
}

// REGULAR EXPRESSION

RegexSemantic * RegularExpression::get_regex_semantic() const {
  return regex_semantic;
}

LexiconTokenizer * RegularExpression::get_lexicon_tokenizer() const {
  return lexicon_tokenizer;
}

void RegularExpression::generate_dictionary() {
  this->lexicon_dictionary
    // policy
    .create_policy_tt("alpnum")
    .create_policy_tt("alpha", false)
    .create_policy_tt("upper", false, false)
    .create_policy_tt("lower", false, true, false)
    .create_policy_tt("digits", true, false, false)
    // string
    .create_ascii_tt("op:one_to_many", "+")
    .create_ascii_tt("op:zero_to_many", "*")
    .create_ascii_tt("op:zero_or_one", "?")
    .create_ascii_tt("op:any", ".")
    .create_ascii_tt("op:or", "|")
    .create_ascii_tt("node:open", "(")
    .create_ascii_tt("node:close", ")")
    .create_ascii_tt("repeat:open", "{")
    .create_ascii_tt("repeat:close", "}")
    .create_ascii_tt("repeat:separator", ",")
    .create_ascii_tt("pattern:open", "[")
    .create_ascii_tt("pattern:close", "]")
    .create_ascii_tt("dash", "-")
    .create_ascii_tt("underscore", "_")
    .create_ascii_tt("escape", "\\");
}

Grammar * RegularExpression::generate_grammar() {
  auto sym      = new Grammar(&this->lexicon_dictionary);
  auto op       = new Grammar(&this->lexicon_dictionary);
  auto exp1     = new Grammar(&this->lexicon_dictionary);
  auto pipe    = new Grammar(&this->lexicon_dictionary);

  *sym   << "alpnum" & sym | "alpnum";
  std::cout << sym->counter << std::endl;
  *op    << "op:one_to_many" | "op:zero_to_many" | "op:zero_or_one";
  std::cout << op->counter << std::endl;
  (*exp1 << sym & op & exp1 | sym) & op | sym;
  std::cout << exp1->counter << std::endl;
  *pipe  << exp1 & "op:or" & pipe | exp1;
  std::cout << pipe->counter << std::endl;

  auto result = new Grammar(&this->lexicon_dictionary);
  *result << pipe;

  return result;
}

RegularExpression::RegularExpression(std::string regex): regex_semantic(new RegexSemantic) {
  this->generate_dictionary();
  this->lexicon_tokenizer = new LexiconTokenizer(this->generate_grammar());

  if (!regex.empty()) this->generate(regex);
}

bool RegularExpression::match(std::string value) {
  if (!this->regex_grammar) throw ("Grammar not generated");
  auto tokenizer = new LexiconTokenizer(this->regex_grammar);
  auto symbol_table = tokenizer->process(value);
  return symbol_table->has_value() && !symbol_table->get_value()->empty();
}

void RegularExpression::generate(std::string regex) {
  if (regex == this->regex) return;
  auto data = this->create_grammar(regex);
  if (!data->has_value()) return;
  this->regex_grammar = data->get_value()->first;
  this->regex = regex;
  free(data->get_value());
  delete data;
}

Optional<Pair<Grammar *, LexiconDictionary *>> * RegularExpression::create_grammar(std::string regex) {
  auto symbol_table = this->lexicon_tokenizer->process(regex);
  if (!symbol_table->has_value()) {
    delete symbol_table;
    return new Optional<Pair<Grammar *, LexiconDictionary *>>;
  }
  auto metadata = regex_semantic->create_graph(symbol_table->get_value());
  if (!metadata->has_value()) {
    delete symbol_table->get_value();
    delete symbol_table;
    delete metadata;
    return new Optional<Pair<Grammar *, LexiconDictionary *>>;
  }
  auto grammar = regex_semantic->create_grammar(metadata->get_value());
  auto dictionary = metadata->get_value()->second;
  delete symbol_table->get_value();
  delete symbol_table;
  delete(metadata->get_value());
  delete metadata;
  return new Optional(new Pair(grammar, dictionary));
}

Optional<Pair<RegexListNode *, LexiconDictionary *>> * RegularExpression::create_graph(std::string regex) {
  auto symbol_table = this->lexicon_tokenizer->process(regex);
  if (!symbol_table->has_value()) return new Optional<Pair<RegexListNode *, LexiconDictionary *>>;
  return this->regex_semantic->create_graph(symbol_table->get_value());
}

Optional<std::vector<Token>> * RegularExpression::process(std::string regex) {
  return this->lexicon_tokenizer->process(regex);
}

//========================
// MAIN

template<class T, typename C> class ThreadSafePriorityQueue {
public:
  void push(T value) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(value);
  }

  const T top() {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.top();
  }

  void pop() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty()) return;
    queue_.pop();
  }

  bool empty() {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
  }

private:
  std::priority_queue<T, std::vector<T>, C> queue_;
  std::mutex mutex_;
};

struct CustomLess {
  bool operator()(const Pair<std::string, std::string>& l, const Pair<std::string, std::string>& r) const {
    return l.second.size() > r.second.size();
  }
};

enum PipelineStatus {
  DEAD,
  WORKING,
  MUST_ALIVE,
  MUST_DIE,
  MUST_WORK
};

class Pipeline {
  bool accepted = false;
  std::map<std::string, std::vector<Token> *> symboltable_list;
  std::map<std::string, RegexListNode *>      graph_list;
  std::map<std::string, LexiconDictionary *>  dictionary_list;
  std::map<std::string, Grammar *>            grammar_list;
  RegularExpression regex;
  std::mutex mutex;
  std::vector<std::string> * output = new std::vector<std::string>;

  bool already_exists(std::string regex_string) {
    return grammar_list.end() != grammar_list.find(regex_string);
  }
public:
  bool halt = false;
  PipelineStatus status[4] = {PipelineStatus::DEAD, PipelineStatus::DEAD, PipelineStatus::DEAD, PipelineStatus::DEAD};
  ThreadSafePriorityQueue<Pair<std::string, std::string>, CustomLess> t0q;
  ThreadSafePriorityQueue<Pair<std::string, std::string>, CustomLess> t1q;
  ThreadSafePriorityQueue<Pair<std::string, std::string>, CustomLess> t2q;
  ThreadSafePriorityQueue<Pair<std::string, std::string>, CustomLess> t3q;

  void generate_symbol_table(std::string regex_string) {
    //LOG("\tgst 1 rs: " + std::to_string(regex_string.size()));
    if (already_exists(regex_string)) return;
    //LOG("\tgst 2");
    std::lock_guard<std::mutex> lock(mutex);
    auto symbol_table = regex.get_lexicon_tokenizer()->process(regex_string);
    if (!symbol_table->has_value()) {
      delete symbol_table;
      return;
    }
    symboltable_list[regex_string] = symbol_table->get_value();
    //LOG("\tgst 3");
  }
//std::lock_guard<std::mutex> lock(mutex);
  void generate_intermediate_graph(std::string regex_string) {
    //LOG("\tgig 1 rs: " + regex_string);
    if (already_exists(regex_string)) return;
    //LOG("\tgig 2");
    auto st = symboltable_list[regex_string];
    //std::lock_guard<std::mutex> lock(mutex);
    //LOG("\tst is null? " + std::string(st == nullptr ? "null" : "not null"));
    //std::lock_guard<std::mutex> lock(mutex);
    //std::vector<Token> * symbol_table = new std::vector<Token>(st->cbegin(), st->cend());
    auto semantic = regex.get_regex_semantic();
    //LOG("\tgig 3");
    //if (!semantic) LOG("semantic n existe");
    //LOG("\tgig 3");
    auto metadata = semantic->create_graph(st);
    //LOG("\tgig 4");
    if (!metadata->has_value()) {
      delete metadata;
      return;
    }
    //LOG("\tgig 5");
    graph_list[regex_string] = metadata->get_value()->first;
    //LOG("\tgig 6");
    dictionary_list[regex_string] = metadata->get_value()->second;
    //LOG("\tgig 7");
  }

  void generate_grammar(std::string regex_string) {
    if (already_exists(regex_string)) return;
    auto params = new Pair(graph_list[regex_string], dictionary_list[regex_string]);
    auto grammar = regex.get_regex_semantic()->create_grammar(params);
    grammar_list[regex_string] = grammar;
  }

  void match(std::string value, std::string regex_string) {
    auto grammar = grammar_list[regex_string];
    auto tokenizer = new LexiconTokenizer(grammar);
    auto symbol_table = tokenizer->process(value);
    bool accepted = symbol_table->has_value() && !symbol_table->get_value()->empty();
    std::string status = accepted ? "ACCEPTED" : "NOT ACCEPTED";
    output->emplace_back("The text { " + value + " } was " + status + " using regex { " + regex_string + " } .");
  }

  void add_t0(std::string value, std::string regex_string) {
    t0q.push(Pair(value, regex_string));
  }

  void add_t1(std::string value, std::string regex_string) {
    t1q.push(Pair(value, regex_string));
  }

  void add_t2(std::string value, std::string regex_string) {
    t2q.push(Pair(value, regex_string));
  }

  void add_t3(std::string value, std::string regex_string) {
    t3q.push(Pair(value, regex_string));
  }

  bool is_halt() {
    std::lock_guard<std::mutex> lock(mutex);
    return halt;
  }

  void should_halt() {
    std::lock_guard<std::mutex> lock(mutex);
    halt = true;
  }

  PipelineStatus get_status(int i) {
    std::lock_guard<std::mutex> lock(mutex);
    return status[i];
  }

  void set_status(int i, PipelineStatus v) {
    std::lock_guard<std::mutex> lock(mutex);
    status[i] = v;
  }

  bool symboltable_list_has(std::string regex_string) {
    std::lock_guard<std::mutex> lock(mutex);
    return symboltable_list.end() != symboltable_list.find(regex_string);
  }

  bool graph_list_has(std::string regex_string) {
    std::lock_guard<std::mutex> lock(mutex);
    return graph_list.end() != graph_list.find(regex_string);
  }

  bool grammar_list_has(std::string regex_string) {
    std::lock_guard<std::mutex> lock(mutex);
    return grammar_list.end() != grammar_list.find(regex_string);
  }

  bool size() {
    std::lock_guard<std::mutex> lock(mutex);
    return grammar_list.size();
  }

  std::string get_output(int k) {
    std::lock_guard<std::mutex> lock(mutex);
    return (*output)[k];
  }

  int get_output_size() {
    std::lock_guard<std::mutex> lock(mutex);
    return output->size();
  }

  void clear_output() {
    std::lock_guard<std::mutex> lock(mutex);
    output->clear();
  }

  void add_output(std::string v) {
    std::lock_guard<std::mutex> lock(mutex);
    output->emplace_back(v);
  }

  bool is_all_queues_empty() {
    std::lock_guard<std::mutex> lock(mutex);
    return t0q.empty() && t1q.empty() && t2q.empty() && t3q.empty();
  }
};
/*
void test(std::string value, std::string regex_string, Pipeline& pipeline) {
  pipeline.generate_symbol_table(regex_string);
  pipeline.generate_intermediate_graph(regex_string);
  pipeline.generate_grammar(regex_string);
  pipeline.match(value, regex_string);
}
*/
void ft0(Pipeline * pipeline) {
  if(!pipeline->t0q.empty()) {
      //LOG("ft0 - 1");
    auto regex_string = pipeline->t0q.top();
    //LOG("ft0 - 2");
    pipeline->generate_symbol_table(regex_string.second);
    //LOG("ft0 - 3");
    pipeline->t0q.pop();
    //LOG("ft0 - 4");
    pipeline->t1q.push(regex_string);
    //LOG("ft0 - 5");
    if (pipeline->get_status(1) == PipelineStatus::DEAD) {
        //LOG("ft0 - 6");
      pipeline->set_status(1, PipelineStatus::MUST_ALIVE);
    }
    //LOG("ft0 - 7");
  }
  auto tq0_is_empty = pipeline->t0q.empty();
  pipeline->set_status(0, tq0_is_empty
                       ? PipelineStatus::MUST_DIE : PipelineStatus::MUST_WORK);
  //LOG("ft0 - 8");
}

void ft1(Pipeline * pipeline) {
  if(!pipeline->t1q.empty())  {
      //LOG("ft1 - 1");
    auto regex_string = pipeline->t1q.top();
    //LOG("ft1 - 2 regex:"+regex_string.second);
    pipeline->generate_intermediate_graph(regex_string.second);
    //LOG("ft1 - 3");
    pipeline->t1q.pop();
    //LOG("ft1 - 4");
    pipeline->t2q.push(regex_string);
    //LOG("ft1 - 5");
    if (pipeline->get_status(2) == PipelineStatus::DEAD) {
        //LOG("ft1 - 6");
      pipeline->set_status(2, PipelineStatus::MUST_ALIVE);
    }
    //LOG("ft1 - 7");
  }
  auto tq1_is_empty = pipeline->t1q.empty();
  pipeline->set_status(1, tq1_is_empty
                       ? PipelineStatus::MUST_DIE : PipelineStatus::MUST_WORK);
  //LOG("ft1 - 8");
}

void ft2(Pipeline * pipeline) {
  if(!pipeline->t2q.empty()) {
    auto regex_string = pipeline->t2q.top();
    pipeline->generate_grammar(regex_string.second);
    pipeline->t2q.pop();
    pipeline->t3q.push(regex_string);
    if (pipeline->get_status(3) == PipelineStatus::DEAD) {
      pipeline->set_status(3, PipelineStatus::MUST_ALIVE);
    }
  }
  auto tq2_is_empty = pipeline->t2q.empty();
  pipeline->set_status(2, tq2_is_empty
                       ? PipelineStatus::MUST_DIE : PipelineStatus::MUST_WORK);
}

void ft3(Pipeline * pipeline) {
  if(!pipeline->t3q.empty())  {
    auto regex_string = pipeline->t3q.top();
      pipeline->match(regex_string.first, regex_string.second);
      pipeline->t3q.pop();
  }
  auto tq3_is_empty = pipeline->t3q.empty();
  pipeline->set_status(3, tq3_is_empty
                       ? PipelineStatus::MUST_DIE : PipelineStatus::MUST_WORK);
}

// Function to trim leading whitespace
std::string ltrim(const std::string& s) {
    auto start = std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch) && ch > 0;
    });
    return std::string(start, s.end());
}

// Function to trim trailing whitespace
std::string rtrim(const std::string& s) {
    auto end = std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch) && ch > 0;
    }).base();
    return std::string(s.begin(), end);
}

// Function to trim both leading and trailing whitespace
std::string trim(const std::string& s) {
    return ltrim(rtrim(s));
}

void worker(Pipeline& pipeline) {

  //LOG("aaaaa");

  std::thread t4([](Pipeline * pipeline) {
/*
    pipeline->add_t0("ab",            "ab+|z");
    pipeline->add_t0("a",             "ab+|z");
    pipeline->add_t0("abbbb",         "ab+|z");
    pipeline->add_t0("bbbbb",         "ab+|z");
    pipeline->add_t0("zzzzz",         "ab+|z");
    pipeline->add_t0("z",             "ab+|z");
    pipeline->add_t0("az",            "ab+|z");
    pipeline->add_t0("azzzzz",        "ab+|az+");
    pipeline->add_t0("bshajudshyuad", "ab+|z");
*/
    //LOG("aaaaa 1");
    pipeline->set_status(0, PipelineStatus::MUST_ALIVE);

    //LOG("aaaaa 2");

    std::function<void(Pipeline*)> f[4] = {ft0, ft1, ft2, ft3};
    std::thread * t[4] = {nullptr, nullptr, nullptr, nullptr};

    //LOG("aaaaa 3");

    PipelineStatus status[4];
    while(true) {
      for(int i = 0; i < 4; i++) {
        status[i] = pipeline->get_status(i);
        //LOG("aaaaa 4 [" + std::to_string(i) + "] = " + std::to_string(status[i]));
        if(status[i] == PipelineStatus::MUST_ALIVE) {
            //LOG("aaaaa 4.1.1");
          status[i] = PipelineStatus::WORKING;
          pipeline->set_status(i, PipelineStatus::WORKING);
          t[i] = new std::thread(f[i], pipeline);
          //LOG("aaaaa 4.1.2");
        } else if (status[i] == PipelineStatus::MUST_DIE) {
            //LOG("aaaaa 4.2");
          status[i] = PipelineStatus::DEAD;
          pipeline->set_status(i, PipelineStatus::DEAD);
          t[i]->join();
          delete t[i];
          t[i] = nullptr;
        } else if (status[i] == PipelineStatus::MUST_WORK) {
            //LOG("aaaaa 4.3");
          status[i] = PipelineStatus::WORKING;
          pipeline->set_status(i, PipelineStatus::WORKING);
          t[i]->join();
          delete t[i];
          t[i] = new std::thread(f[i], pipeline);;
        }
      }

      //LOG("aaaaa 5");
      std::this_thread::yield();

      //LOG("aaaaa 6");

      if (status[0] == PipelineStatus::DEAD &&
          status[1] == PipelineStatus::DEAD &&
          status[2] == PipelineStatus::DEAD &&
          status[3] == PipelineStatus::DEAD &&
          pipeline->is_all_queues_empty()) break;
    }

    //LOG("HALT");

    pipeline->should_halt();
  }, &pipeline);

  t4.join();
  //LOG("BBBBBBBBBBBB");
}

int main(int argc, char** argv) {

  int provided;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

  if (provided < MPI_THREAD_MULTIPLE) {
    std::cerr << "Warning: MPI does not provide full thread support." << std::endl;
    return -1;
  }

  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  rank_aux = world_rank;

  MPI_Request request;
  MPI_Status status;

  int MUST_DELIVERY_OUTPUT = 2;
  int MUST_PROCESS_LINE = 1;
  int END_OF_PROGRAM = 0;

  if(argc <= 2) return -1;

  int worker_ranks = world_size - 1;
  int workload_window = world_size*2;
  try {
      workload_window = worker_ranks*std::stoi(argv[2]);
  } catch (const std::invalid_argument& e) {
      std::cerr << "Invalid argument: " << e.what() << std::endl;
  } catch (const std::out_of_range& e) {
      std::cerr << "Out of range: " << e.what() << std::endl;
  }

  if (world_rank == 0) {
    //bool should_gather = false;

    std::string filename(argv[1]);
    std::ifstream file(filename);

    if (!file) {
      std::cerr << "Error: Could not open the file " << filename << std::endl;
      MPI_Finalize();
      return -1;
    }

    std::ofstream outfile("output.txt");
    if (!outfile.is_open()) {
      std::cerr << "Failed to open file." << std::endl;
      MPI_Finalize();
      return -1;
    }

    int i = 0;
    //int amount = 0;
    int signal = 0;
    std::string line;
    std::vector<char> buffer(1000);

    std::function<void(void)> gather = [&]() {
        LOG("GATHERER 1");
      int j = 0;
      while(j < workload_window) {
        int rank = (j%worker_ranks)+1;
        LOG("GATHERER 2, worker rank: " + std::to_string(rank));
        //MPI_Recv(&signal, 1, MPI_INT, rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        //if (signal == MUST_PROCESS_LINE) {

          MPI_Recv(buffer.data(), 1000, MPI_CHAR, rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          //LOG("GATHERER 2");
          LOG(buffer.data());
          outfile << buffer.data() << std::endl;
          for(int k = 0; k < 1000; k++) buffer[k] = 0;
        //}
        j++;


        //amount++;



        //LOG("GATHERER 3");

        //LOG("GATHERER 4");

        //rank = (j%worker_ranks)+1;
        //MPI_Recv(&signal, 1, MPI_INT, rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        //j %= workload_window;
        //LOG("GATHERER 5");
      }

      //LOG(std::string("[GATHERER] output amount: ") + std::to_string(amount));
    };

    int how_much = 0;

    // consome arquivo
    while (std::getline(file, line)) {
      if(line == "\n") continue;
      //LOG("BROADCASTER 3.1");
      // envia sinal para consumir linha
      int rank = (i%worker_ranks)+1;
      MPI_Isend(&MUST_PROCESS_LINE, 1, MPI_INT, rank, 0, MPI_COMM_WORLD, &request);

      //LOG("BROADCASTER 3.2");
      // envia a linha
      for (size_t k = 0; k < 1000; k++) {
        if (k < line.size()) buffer[k] = line[k];
        else buffer[k] = 0;
      }
      MPI_Isend(buffer.data(), buffer.size(), MPI_CHAR, rank, 0, MPI_COMM_WORLD, &request);
      i++;
      //LOG("BROADCASTER 3.3");

      // ao completar janela, envia sinal para retornar resultado
      if(i == workload_window) {
        //LOG("BROADCASTER 3.4.1");
        for (int k = 0; k < worker_ranks; k++)
          MPI_Isend(&MUST_DELIVERY_OUTPUT, 1, MPI_INT, k+1, 0, MPI_COMM_WORLD, &request);

        //LOG("BROADCASTER 3.4.1");

        gather();

        MPI_Wait(&request, &status);

      }
      //LOG("BROADCASTER 3.5");

      how_much++;
      if (i == workload_window)
        LOG(std::to_string(how_much));

      i %= workload_window;
      //LOG("BROADCASTER 3.6");
    }
    if(i > 0) {
        LOG(std::to_string(how_much));
      //LOG("BROADCASTER 4");
      for (int k = 0; k < worker_ranks; k++)
        MPI_Isend(&MUST_DELIVERY_OUTPUT, 1, MPI_INT, k+1, 0, MPI_COMM_WORLD, &request);

      gather();
    }
    //LOG("BROADCASTER 4");
    //int rank = (i%worker_ranks)+1;
    for (int k = 0; k < worker_ranks; k++)
      MPI_Isend(&END_OF_PROGRAM, 1, MPI_INT, k+1, 0, MPI_COMM_WORLD, &request);

    MPI_Wait(&request, &status);

    file.close();

    MPI_Finalize();
  } else {

    Pipeline pipeline;
    //LOG("WORKER 0");
    std::vector<char> buffer(1000);
    //int output_amount = 0;
    int signal = 0;
    int r = 0;
    MPI_Recv(&signal, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    while(signal != END_OF_PROGRAM) {

        //LOG("WORKER 1: rank: " + std::to_string(world_rank));

      if (signal == MUST_PROCESS_LINE) {
          //LOG("WORKER 2.1");
        MPI_Recv(buffer.data(), 1000, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

          //std::transform(buffer.begin(), buffer.end(), buffer.begin(), ::toupper);

        //LOG("WORKER 2.2");
        std::string result(buffer.begin(), buffer.end());
        std::string trimmed = trim(result);

        size_t pos = trimmed.find('=');
        std::string value = trimmed.substr(0, pos);
        std::string regex_string = trimmed.substr(pos + 1);

        //LOG("WORKER 2.3: v: " + value + ", rs: " + std::to_string(regex_string.size()));

        pipeline.add_t0(value, regex_string);
        //pipeline.add_output("mock mock mock");
        for(int k = 0; k < 1000; k++) buffer[k] = 0;
        //LOG("WORKER 2.4");



        r++;

      } else if (signal == MUST_DELIVERY_OUTPUT) {

          //LOG("WORKER 3.1");

        worker(pipeline);
        //pipeline.add_output("mock mock mock");
        //pipeline.add_output("mock mock mock");
        //pipeline.add_output("mock mock mock");
        //pipeline.add_output("mock mock mock");

        //LOG("WORKER 3.2");
        int output_size = pipeline.get_output_size();
        /*
        if (world_rank == 4) {
          output_amount += output_size;
          LOG(std::string("output amount: ") + std::to_string(output_amount));
        }
        */
        for (int k = 0; k < output_size; k++) {
            LOG("WORKER 3.3.1, output size: " + std::to_string(output_size)
                + ", qtd entrada: " + std::to_string(r));
          std::string output = pipeline.get_output(k);
          //MPI_Isend(&MUST_PROCESS_LINE, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &request);
          MPI_Isend(output.c_str(), output.size(), MPI_CHAR, 0, 0, MPI_COMM_WORLD, &request);
          //LOG("WORKER 3.3.2");
        }
        MPI_Wait(&request, &status);
        //LOG("WORKER 3.3");
        pipeline.clear_output();
        r = 0;
        //MPI_Isend(&END_OF_PROGRAM, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &request);
      }
      //LOG("WORKER 3.4");
      MPI_Recv(&signal, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      //LOG("WORKER 3.4");
    }
    //LOG("WORKER 3.5");
    //MPI_Isend(&END_OF_PROGRAM, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &request);

    //LOG("WORKER 3.6");
    MPI_Wait(&request, &status);
    MPI_Finalize();
  }

  /*


  */

  return 0;
}
