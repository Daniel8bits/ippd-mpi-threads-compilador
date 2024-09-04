#include "regular_expression.h"

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

std::optional<Pair<RegexListNode *, LexiconDictionary *> *> * RegexSemantic::create_graph(std::vector<Token> * symbol_table) {
  auto dictionary = new LexiconDictionary();
  std::stack<RegexListNode *> stack;
  stack.push(new RegexListNode(RegexOpType::None));

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

  auto graph = stack.top();
  bool is_empty = stack.empty();
  stack.pop();

  if (!is_empty && !failed) {
    return new std::optional(new Pair(graph, dictionary));
  }

  return new std::optional<Pair<RegexListNode *, LexiconDictionary *> *>;
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
  return symbol_table->has_value() && !symbol_table->value()->empty();
}

void RegularExpression::generate(std::string regex) {
  if (regex == this->regex) return;
  auto data = this->create_grammar(regex);
  if (!data->has_value()) return;
  this->regex_grammar = data->value()->first;
  this->regex = regex;
  free(data->value());
  delete data;
}

std::optional<Pair<Grammar *, LexiconDictionary *> *> * RegularExpression::create_grammar(std::string regex) {
  auto symbol_table = this->lexicon_tokenizer->process(regex);
  if (!symbol_table->has_value()) {
    delete symbol_table;
    return new std::optional<Pair<Grammar *, LexiconDictionary *> *>;
  }
  auto metadata = regex_semantic->create_graph(symbol_table->value());
  if (!metadata->has_value()) {
    delete symbol_table->value();
    delete symbol_table;
    delete metadata;
    return new std::optional<Pair<Grammar *, LexiconDictionary *> *>;
  }
  auto grammar = regex_semantic->create_grammar(metadata->value());
  auto dictionary = metadata->value()->second;
  delete symbol_table->value();
  delete symbol_table;
  delete(metadata->value());
  delete metadata;
  return new std::optional(new Pair(grammar, dictionary));
}

std::optional<Pair<RegexListNode *, LexiconDictionary *> *> * RegularExpression::create_graph(std::string regex) {
  auto symbol_table = this->lexicon_tokenizer->process(regex);
  if (!symbol_table->has_value()) return new std::optional<Pair<RegexListNode *, LexiconDictionary *> *>;
  return this->regex_semantic->create_graph(symbol_table->value());
}

std::optional<std::vector<Token> *> * RegularExpression::process(std::string regex) {
  return this->lexicon_tokenizer->process(regex);
}
