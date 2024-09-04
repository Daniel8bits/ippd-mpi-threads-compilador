#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <algorithm>
#include <mpi/mpi.h>

/*

For rank 0:
  THREAD 1
  - read from the file
  - broadcast evenly across ranks
  THREAD 2
  - gather from ranks
  - write to other file
For each rank:
  - process text


*/

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);

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

  int workload_window = world_size*2;
  try {
      workload_window = world_size*std::stoi(argv[2]);
  } catch (const std::invalid_argument& e) {
      std::cerr << "Invalid argument: " << e.what() << std::endl;
  } catch (const std::out_of_range& e) {
      std::cerr << "Out of range: " << e.what() << std::endl;
  }

  if (world_rank == 0) {
    int worker_ranks = world_size - 1;
    bool should_gather = false;
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
          std::cout << "BROADCASTER 1" << std::endl;
          std::cout << line << std::endl;

        if(line == "\n") continue;
        int rank = (i%worker_ranks)+1;
        MPI_Isend(&MUST_PROCESS_LINE, 1, MPI_INT, rank, 0, MPI_COMM_WORLD, &request);
        MPI_Isend(line.c_str(), line.size(), MPI_CHAR, rank, 0, MPI_COMM_WORLD, &request);
        i++;

          std::cout << "BROADCASTER 2" << std::endl;

        if(i == workload_window) {
          for (int k = 0; k < worker_ranks; k++)
            MPI_Isend(&MUST_DELIVERY_OUTPUT, 1, MPI_INT, k, 0, MPI_COMM_WORLD, &request);
          should_gather = true;
        }
        while(should_gather) std::this_thread::yield();
          std::cout << "BROADCASTER 3" << std::endl;
        i %= workload_window;
      }
      int rank = (i%worker_ranks)+1;
      MPI_Isend(&END_OF_PROGRAM, 1, MPI_INT, rank, 0, MPI_COMM_WORLD, &request);

      std::cout << "BROADCASTER 4" << std::endl;

      MPI_Wait(&request, &status);

      std::cout << "BROADCASTER 5" << std::endl;

      should_gather = true;
      std::this_thread::yield();

      std::cout << "BROADCASTER 6" << std::endl;

      file.close();
    });

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

          std::cout << "GATHERER 1: worker_ranks = " << worker_ranks<< ", i = " << i << std::endl;
          std::cout << "GATHERER 2: shouldn't gather? " << !should_gather << std::endl;

        while(!should_gather) std::this_thread::yield();

          std::cout << "GATHERER 3" << std::endl;

        rank = (i%worker_ranks)+1;
        MPI_Recv(&signal, 1, MPI_INT, rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        i %= workload_window;
      }

      std::cout << "GATHERER 4" << std::endl;

      file.close();
    });

    broadcaster.join();
    gatherer.join();

    MPI_Finalize();
  } else {
    std::vector<char> buffer(1000);
    int signal = 0;
    MPI_Recv(&signal, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    while(signal == MUST_PROCESS_LINE) {
        std::cout << "WORKER 1" << std::endl;

      MPI_Recv(buffer.data(), 1000, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        std::cout << "WORKER 2" << std::endl;
        std::transform(buffer.begin(), buffer.end(), buffer.begin(), ::toupper);
        std::cout << "WORKER 3" << std::endl;

      MPI_Isend(&MUST_PROCESS_LINE, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &request);
      MPI_Isend(buffer.data(), buffer.size(), MPI_CHAR, 0, 0, MPI_COMM_WORLD, &request);
      for(int k = 0; k < 1000; k++) buffer[k] = 0;
        std::cout << "WORKER 4" << std::endl;
      MPI_Recv(&signal, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    MPI_Isend(&END_OF_PROGRAM, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &request);
    MPI_Wait(&request, &status);
    MPI_Finalize();
  }

  return 0;
}
