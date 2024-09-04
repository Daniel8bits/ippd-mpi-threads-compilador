#include "common.h"
#include <iostream>

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
  this->branches.back()->nodes.emplace_back(GrammarNode{.grammar = nullptr, .token_type_name = token_type_name});
  return *this;
}

Grammar& Grammar::operator&(Grammar * b) {
  this->counter += " <GRAMMAR>";
  this->branches.back()->nodes.emplace_back(GrammarNode{.grammar = b, .token_type_name = ""});
  return *this;
}

Grammar& Grammar::operator<<(std::string token_type_name) {
  this->counter += "THIS := " + token_type_name;
  this->pipe(GrammarBranch(), true);
  this->branches.back()->nodes.emplace_back(GrammarNode{.grammar = nullptr, .token_type_name = token_type_name});
  return *this;
}

Grammar& Grammar::operator<<(Grammar * b) {
  this->counter += "THIS := <GRAMMAR>";
  this->pipe(GrammarBranch(), true);
  this->branches.back()->nodes.emplace_back(GrammarNode{.grammar = b, .token_type_name = ""});
  return *this;
}

Grammar& Grammar::operator|(std::string token_type_name) {
  this->counter += " | " + token_type_name;
  this->pipe(GrammarBranch(), true);
  this->branches.back()->nodes.emplace_back(GrammarNode{.grammar = nullptr, .token_type_name = token_type_name});
  return *this;
}

Grammar& Grammar::operator|(Grammar * b) {
  this->counter += " | <GRAMMAR>";
  this->pipe(GrammarBranch(), true);
  this->branches.back()->nodes.emplace_back(GrammarNode{.grammar = b, .token_type_name = ""});
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
  return this->next().grammar != nullptr;
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

std::optional<std::vector<Token> *> * Tokenizer::process(std::string& language) {
  std::cout << "\t\t\tCOMMON process: " << std::endl;
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

  std::cout << "begin_token: " << state.begin_token << ", stack size: " << stack.size() << std::endl;

  if (state.begin_token == language.size() && stack.size() == 0)
    return new std::optional(symbol_table);

  return new std::optional<std::vector<Token> *>;

}
