#ifndef DISTRIBUTORIMPL_H
#define DISTRIBUTORIMPL_H

#include <capnp/ez-rpc.h>
#include "schemas/distributor.capnp.h"
#include "schemas/scheduler.capnp.h"
#include <unordered_map>
#include <vector>
#include <thread>

struct Pod {
    std::string ipAddress;
    int port;
    int cores;
    int threads;
    int64_t ram;
    bool isBusy;

    Pod(std::string ip, int p, int c, int t, int64_t r)
        : ipAddress(std::move(ip)), port(p), cores(c), threads(t), ram(r), isBusy(false) {}
};

class DistributorImpl final : public Distributor::Server {
public:
    DistributorImpl(kj::Own<Scheduler::Client> scheduler);

    kj::Promise<void> registerPod(RegisterPodContext context) override;

    void distributeWorkload();

private:
    std::unordered_map<std::string, Pod> pods;
    kj::Own<Scheduler::Client> scheduler;
    std::thread schedulerThread;
    std::mutex podsMutex;

    void fetchAndDistributeWorkloads();
};

#endif // DISTRIBUTORIMPL_H
