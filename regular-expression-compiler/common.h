#ifndef COMMON_H
#define COMMON_H

#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <stack>
#include <optional>

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
  GrammarNode() = delete;
  Grammar * grammar = nullptr;
  std::string token_type_name = "";
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
  Grammar& operator&(Grammar * b);

  Grammar& operator<<(std::string token_type_name);
  Grammar& operator<<(Grammar * b);

  Grammar& operator|(std::string token_type_name);
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
  std::optional<std::vector<Token> *> * process(std::string& language);
};

#endif // COMMON_H
