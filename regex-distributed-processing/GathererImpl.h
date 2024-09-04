#ifndef GATHERERIMPL_H
#define GATHERERIMPL_H


#include <capnp/ez-rpc.h>
#include "schemas/gatherer.capnp.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <thread>
#include <mutex>

class GathererImpl final : public Gatherer::Server {
public:
    GathererImpl();

    kj::Promise<void> gatherResults(GatherResultsContext context) override;

private:
    void sendResultToClient(const std::string& clientAddress, int clientPort, const std::vector<int32_t>& result);

    std::mutex resultsMutex;
};

#endif // GATHERERIMPL_H
