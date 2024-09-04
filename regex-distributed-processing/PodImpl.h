#ifndef PODIMPL_H
#define PODIMPL_H

#include <capnp/ez-rpc.h>
#include "schemas/pod.capnp.h"
#include "schemas/distributor.capnp.h"
#include <thread>

class PodImpl final : public Pod::Server {
public:
    PodImpl(std::string distributorIp, int distributorPort);

    kj::Promise<void> processWorkload(ProcessWorkloadContext context) override;

private:
    void registerWithDistributor(const std::string& distributorIp, int distributorPort);

    std::thread registrationThread;
    bool isBusy = false;
};

#endif // PODIMPL_H
