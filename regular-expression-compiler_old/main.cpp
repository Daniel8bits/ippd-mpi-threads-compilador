#include <mpi/mpi.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <queue>
#include <mutex>
#include <functional>
#include "regular_expression.h"

#define LOG(M) std::cout << M << std::endl;
#define ISNULL(M, P) std::cout << M << std::string(P == nullptr ? "is null" : "not null") << std::endl;

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
    if (already_exists(regex_string)) return;
    auto symbol_table = regex.get_lexicon_tokenizer()->process(regex_string);
    if (!symbol_table->has_value()) {
      delete symbol_table;
      return;
    }
    symboltable_list[regex_string] = symbol_table->value();
  }

  void generate_intermediate_graph(std::string regex_string) {
    if (already_exists(regex_string)) return;
    auto metadata = regex.get_regex_semantic()->create_graph(symboltable_list[regex_string]);
    if (!metadata->has_value()) {
      delete metadata;
      return;
    }
    graph_list[regex_string] = metadata->value()->first;
    dictionary_list[regex_string] = metadata->value()->second;
  }

  void generate_grammar(std::string regex_string) {
    if (already_exists(regex_string)) return;
    auto params = new Pair(graph_list[regex_string], dictionary_list[regex_string]);
    auto grammar = regex.get_regex_semantic()->create_grammar(params);
    grammar_list[regex_string] = grammar;
  }

  void match(std::string value, std::string regex_string) {
    LOG("\tm - 1");
    auto grammar = grammar_list[regex_string];
    auto tokenizer = new LexiconTokenizer(grammar);
    LOG("\tm - 2 v: " + value + ", r: " + regex_string);
    ISNULL("grammar: ", grammar);
    auto symbol_table = tokenizer->process(value);
    LOG("\tm - 3");
    bool accepted = symbol_table->has_value() && !symbol_table->value()->empty();
    std::string status = accepted ? "ACCEPTED" : "NOT ACCEPTED";
    output->emplace_back("The text { " + value + " } was " + status + " using regex { " + regex_string + "} .");
    LOG("\tm - 4");
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
  LOG("ft0 - 1");
  if(!pipeline->t0q.empty()) {
      LOG("ft0 - 2");
    auto regex_string = pipeline->t0q.top();
    pipeline->generate_symbol_table(regex_string.second);
    LOG("ft0 - 3");
    pipeline->t0q.pop();
    pipeline->t1q.push(regex_string);
    LOG("ft0 - 4");
    if (pipeline->get_status(1) == PipelineStatus::DEAD) {
      pipeline->set_status(1, PipelineStatus::MUST_ALIVE);
      LOG("ft0 - 5");
    }
    LOG("ft0 - 6");
  }
  pipeline->set_status(0, pipeline->t0q.empty()
                       ? PipelineStatus::MUST_DIE : PipelineStatus::MUST_WORK);
  LOG("ft0 - 7");
}

void ft1(Pipeline * pipeline) {
  LOG("ft1 - 1");
  if(!pipeline->t1q.empty())  {
      LOG("ft1 - 2");
    auto regex_string = pipeline->t1q.top();
    pipeline->generate_intermediate_graph(regex_string.second);
    LOG("ft1 - 3");
    pipeline->t1q.pop();
    pipeline->t2q.push(regex_string);
    LOG("ft1 - 4");
    if (pipeline->get_status(2) == PipelineStatus::DEAD) {
      pipeline->set_status(2, PipelineStatus::MUST_ALIVE);
      LOG("ft1 - 5");
    }
    LOG("ft1 - 6");
  }
  pipeline->set_status(1, pipeline->t1q.empty()
                       ? PipelineStatus::MUST_DIE : PipelineStatus::MUST_WORK);
  LOG("ft1 - 7");
}

void ft2(Pipeline * pipeline) {
  LOG("ft2 - 1");
  if(!pipeline->t2q.empty()) {
      LOG("ft2 - 2");
    auto regex_string = pipeline->t2q.top();
    pipeline->generate_grammar(regex_string.second);
    LOG("ft2 - 3");
    pipeline->t2q.pop();
    pipeline->t3q.push(regex_string);
    LOG("ft2 - 4");
    if (pipeline->get_status(3) == PipelineStatus::DEAD) {
      pipeline->set_status(3, PipelineStatus::MUST_ALIVE);
      LOG("ft2 - 5");
    }
    LOG("ft2 - 6");
  }
  pipeline->set_status(2, pipeline->t2q.empty()
                       ? PipelineStatus::MUST_DIE : PipelineStatus::MUST_WORK);
  LOG("ft2 - 7");
}

void ft3(Pipeline * pipeline) {
  LOG("ft3 - 1");
  if(!pipeline->t3q.empty())  {
    auto regex_string = pipeline->t3q.top();
    LOG("ft3 - 2, rs: " + regex_string.second);
    pipeline->match(regex_string.first, regex_string.second);
    LOG("ft3 - 3");
    pipeline->t3q.pop();
    LOG("ft3 - 4");
  }
  pipeline->set_status(3, pipeline->t3q.empty()
                       ? PipelineStatus::MUST_DIE : PipelineStatus::MUST_WORK);
  LOG("ft3 - 5");
}

// Function to trim leading whitespace
std::string ltrim(const std::string& s) {
    auto start = std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    });
    return std::string(start, s.end());
}

// Function to trim trailing whitespace
std::string rtrim(const std::string& s) {
    auto end = std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base();
    return std::string(s.begin(), end);
}

// Function to trim both leading and trailing whitespace
std::string trim(const std::string& s) {
    return ltrim(rtrim(s));
}

void worker(Pipeline& pipeline) {

  LOG("aaaaa");

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
    LOG("aaaaa 1");
    pipeline->set_status(0, PipelineStatus::MUST_ALIVE);

    LOG("aaaaa 2");

    std::function<void(Pipeline*)> f[4] = {ft0, ft1, ft2, ft3};
    std::thread * t[4] = {nullptr, nullptr, nullptr, nullptr};

    LOG("aaaaa 3");

    PipelineStatus status[4];
    while(true) {
      for(int i = 0; i < 4; i++) {
          //LOG("aaaaa 4 [" + std::to_string(i) + "]");
        status[i] = pipeline->get_status(i);
        if(status[i] == PipelineStatus::MUST_ALIVE) {
            LOG("aaaaa 4.1");
          status[i] = PipelineStatus::WORKING;
          pipeline->set_status(i, PipelineStatus::WORKING);
          t[i] = new std::thread(f[i], pipeline);
        } else if (status[i] == PipelineStatus::MUST_DIE) {
            LOG("aaaaa 4.2");
          status[i] = PipelineStatus::DEAD;
          pipeline->set_status(i, PipelineStatus::DEAD);
          t[i]->join();
          delete t[i];
          t[i] = nullptr;
        } else if (status[i] == PipelineStatus::MUST_WORK) {
            LOG("aaaaa 4.3");
          status[i] = PipelineStatus::WORKING;
          pipeline->set_status(i, PipelineStatus::WORKING);
          t[i]->join();
          delete t[i];
          t[i] = new std::thread(f[i], pipeline);;
        }
      }

      //LOG("aaaaa 5");
      //LOG("\tstatus 0: " + std::to_string(status[0]));
      //LOG("\tstatus 1: " + std::to_string(status[1]));
      //LOG("\tstatus 2: " + std::to_string(status[2]));
      //LOG("\tstatus 3: " + std::to_string(status[3]));

      std::this_thread::yield();
      //LOG("aaaaa 6");

      if (status[0] == PipelineStatus::DEAD &&
          status[1] == PipelineStatus::DEAD &&
          status[2] == PipelineStatus::DEAD &&
          status[3] == PipelineStatus::DEAD) break;
    }

    LOG("HALT");

    pipeline->should_halt();
  }, &pipeline);

  t4.join();
  LOG("BBBBBBBBBBBB");
}

int main(int argc, char** argv) {

  if(argc <= 2) return -1;

  std::string filename(argv[1]);
  std::ifstream file(filename);

  if (!file) {
    std::cerr << "Error: Could not open the file " << filename << std::endl;
    return -1;
  }

  int workload_window = 1;

  try {
      workload_window *= std::stoi(argv[2]);
  } catch (const std::invalid_argument& e) {
      std::cerr << "Invalid argument: " << e.what() << std::endl;
  } catch (const std::out_of_range& e) {
      std::cerr << "Out of range: " << e.what() << std::endl;
  }

  int i = 0;
  Pipeline pipeline;

  std::string line;
  while (std::getline(file, line)) {
    std::string trimmed = trim(line);

    size_t pos = trimmed.find('=');
    std::string value = trimmed.substr(0, pos);
    std::string regex_string = trimmed.substr(pos + 1);
    i++;

    LOG("WORKER 2.3: v: " + value + ", rs: " + regex_string);

    pipeline.add_t0(value, regex_string);

    if (i == workload_window) worker(pipeline);

    i %= workload_window;
  }

  if (i > 0) worker(pipeline);

  std::ofstream output_file("output.txt");
  if (!output_file.is_open()) {
    std::cerr << "Failed to open file." << std::endl;
    return -1;
  }

  int output_size = pipeline.get_output_size();
  for (int k = 0; k < output_size; k++) {
      LOG("WORKER 3.3.1");
    std::string output = pipeline.get_output(k);
    output_file << output << std::endl;

  }

  file.close();
  output_file.close();

  return 0;
}
