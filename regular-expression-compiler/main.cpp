#include <mpi/mpi.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <queue>
#include <mutex>
#include <functional>
#include "regular_expression.h"

#define LOG(M) std::cout << M << std::endl;

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
    LOG("\tgst 1 rs: " + regex_string);
    if (already_exists(regex_string)) return;
    LOG("\tgst 2");
    std::lock_guard<std::mutex> lock(mutex);
    auto symbol_table = regex.get_lexicon_tokenizer()->process(regex_string);
    if (!symbol_table->has_value()) {
      delete symbol_table;
      return;
    }
    symboltable_list[regex_string] = symbol_table->value();
    LOG("\tgst 3");
  }
//std::lock_guard<std::mutex> lock(mutex);
  void generate_intermediate_graph(std::string regex_string) {
    LOG("\tgig 1 rs: " + regex_string);
    if (already_exists(regex_string)) return;
    LOG("\tgig 2");
    auto st = symboltable_list[regex_string];
    std::lock_guard<std::mutex> lock(mutex);
    LOG("\tst is null? " + std::string(st == nullptr ? "null" : "not null"));
    //std::lock_guard<std::mutex> lock(mutex);
    std::vector<Token> * symbol_table = new std::vector<Token>(st->cbegin(), st->cend());
    auto semantic = regex.get_regex_semantic();
    LOG("\tgig 3");
    if (!semantic) LOG("semantic n existe");
    LOG("\tgig 3");
    auto metadata = semantic->create_graph(symbol_table);
    LOG("\tgig 4");
    if (!metadata->has_value()) {
      delete metadata;
      return;
    }
    LOG("\tgig 5");
    graph_list[regex_string] = metadata->value()->first;
    LOG("\tgig 6");
    dictionary_list[regex_string] = metadata->value()->second;
    LOG("\tgig 7");
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
    bool accepted = symbol_table->has_value() && !symbol_table->value()->empty();
    std::string status = accepted ? "ACCEPTED" : "NOT ACCEPTED";
    output->emplace_back("The text { " + value + " } was " + status + " using regex { " + regex_string + "} .");
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
  if(!pipeline->t0q.empty()) {
      LOG("ft0 - 1");
    auto regex_string = pipeline->t0q.top();
    LOG("ft0 - 2");
    pipeline->generate_symbol_table(regex_string.second);
    LOG("ft0 - 3");
    pipeline->t0q.pop();
    LOG("ft0 - 4");
    pipeline->t1q.push(regex_string);
    LOG("ft0 - 5");
    if (pipeline->get_status(1) == PipelineStatus::DEAD) {
        LOG("ft0 - 6");
      pipeline->set_status(1, PipelineStatus::MUST_ALIVE);
    }
    LOG("ft0 - 7");
  }
  auto tq0_is_empty = pipeline->t0q.empty();
  pipeline->set_status(0, tq0_is_empty
                       ? PipelineStatus::MUST_DIE : PipelineStatus::MUST_WORK);
  LOG("ft0 - 8");
}

void ft1(Pipeline * pipeline) {
  if(!pipeline->t1q.empty())  {
      LOG("ft1 - 1");
    auto regex_string = pipeline->t1q.top();
    LOG("ft1 - 2 regex:"+regex_string.second);
    pipeline->generate_intermediate_graph(regex_string.second);
    LOG("ft1 - 3");
    pipeline->t1q.pop();
    LOG("ft1 - 4");
    pipeline->t2q.push(regex_string);
    LOG("ft1 - 5");
    if (pipeline->get_status(2) == PipelineStatus::DEAD) {
        LOG("ft1 - 6");
      pipeline->set_status(2, PipelineStatus::MUST_ALIVE);
    }
    LOG("ft1 - 7");
  }
  auto tq1_is_empty = pipeline->t1q.empty();
  pipeline->set_status(1, tq1_is_empty
                       ? PipelineStatus::MUST_DIE : PipelineStatus::MUST_WORK);
  LOG("ft1 - 8");
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
        status[i] = pipeline->get_status(i);
        //LOG("aaaaa 4 [" + std::to_string(i) + "] = " + std::to_string(status[i]));
        if(status[i] == PipelineStatus::MUST_ALIVE) {
            LOG("aaaaa 4.1.1");
          status[i] = PipelineStatus::WORKING;
          pipeline->set_status(i, PipelineStatus::WORKING);
          t[i] = new std::thread(f[i], pipeline);
          LOG("aaaaa 4.1.2");
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
    bool should_gather = false;

    // BROADCASTER
    std::thread broadcaster([&]() {
      std::string filename(argv[1]);
      std::ifstream file(filename);

      if (!file) {
        std::cerr << "Error: Could not open the file " << filename << std::endl;
        return;
      }

      int i = 0;
      std::string line;
      while (std::getline(file, line)) {

        if(line == "\n") continue;
        int rank = (i%worker_ranks)+1;
        MPI_Isend(&MUST_PROCESS_LINE, 1, MPI_INT, rank, 0, MPI_COMM_WORLD, &request);
        MPI_Isend(line.c_str(), line.size(), MPI_CHAR, rank, 0, MPI_COMM_WORLD, &request);
        i++;

        if(i == workload_window) {
          LOG("BROADCASTER 3");
          for (int k = 0; k < worker_ranks; k++)
            MPI_Isend(&MUST_DELIVERY_OUTPUT, 1, MPI_INT, k+1, 0, MPI_COMM_WORLD, &request);
          should_gather = true;
        }
        while(should_gather) std::this_thread::yield();
        i %= workload_window;
      }
      //int rank = (i%worker_ranks)+1;
      MPI_Isend(&END_OF_PROGRAM, 1, MPI_INT, i+1, 0, MPI_COMM_WORLD, &request);

      MPI_Wait(&request, &status);

      should_gather = true;
      std::this_thread::yield();

      file.close();
    });

    // GATHERER
    std::thread gatherer([&]() {
      std::ofstream file("output.txt");
      if (!file.is_open()) {
        std::cerr << "Failed to open file." << std::endl;
        MPI_Finalize();
        return;
      }


      int i = 0;
      std::vector<char> buffer(1000);
      int signal = 0;
      MPI_Recv(&signal, 1, MPI_INT, i+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      while(signal == MUST_PROCESS_LINE) {
        int rank = (i%worker_ranks)+1;
        MPI_Recv(buffer.data(), 1000, MPI_CHAR, rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        file << buffer.data() << std::endl;
        for(int k = 0; k < 1000; k++) buffer[k] = 0;
        i++;
        if(i == workload_window) should_gather = false;

        while(!should_gather) std::this_thread::yield();

        rank = (i%worker_ranks)+1;
        MPI_Recv(&signal, 1, MPI_INT, rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        i %= workload_window;
      }

      file.close();
    });

    broadcaster.join();
    gatherer.join();

    MPI_Finalize();
  } else {

    Pipeline pipeline;
    LOG("WORKER 0");
    std::vector<char> buffer(1000);
    int signal = 0;
    MPI_Recv(&signal, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    while(signal != END_OF_PROGRAM) {

        LOG("WORKER 1: rank: " + std::to_string(world_rank));

      if (signal == MUST_PROCESS_LINE) {
          LOG("WORKER 2.1");
        MPI_Recv(buffer.data(), 1000, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

          //std::transform(buffer.begin(), buffer.end(), buffer.begin(), ::toupper);

        LOG("WORKER 2.2");
        std::string result(buffer.begin(), buffer.end());
        std::string trimmed = trim(result);

        size_t pos = trimmed.find('=');
        std::string value = trimmed.substr(0, pos);
        std::string regex_string = trimmed.substr(pos + 1);

        LOG("WORKER 2.3: v: " + value + ", rs: " + regex_string);

        pipeline.add_t0(value, regex_string);
        for(int k = 0; k < 1000; k++) buffer[k] = 0;
        LOG("WORKER 2.4");
      } else {

          LOG("WORKER 3.1");

        worker(pipeline);
        LOG("WORKER 3.2");
        int output_size = pipeline.get_output_size();
        for (int k = 0; k < output_size; k++) {
            LOG("WORKER 3.3.1");
          std::string output = pipeline.get_output(k);
          MPI_Isend(&MUST_PROCESS_LINE, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &request);
          MPI_Isend(output.c_str(), output.size(), MPI_CHAR, 0, 0, MPI_COMM_WORLD, &request);
          LOG("WORKER 3.3.2");
        }
        LOG("WORKER 3.3");
        pipeline.clear_output();
      }

      MPI_Recv(&signal, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    MPI_Isend(&END_OF_PROGRAM, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &request);
    MPI_Wait(&request, &status);
    MPI_Finalize();
  }

  /*


  */

  return 0;
}
