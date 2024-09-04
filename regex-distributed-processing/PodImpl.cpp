#include "PodImpl.h"

PodImpl::PodImpl(std::string distributorIp, int distributorPort) {
    // Register the Pod with the Distributor
    registrationThread = std::thread(&PodImpl::registerWithDistributor, this, distributorIp, distributorPort);
}

void PodImpl::registerWithDistributor(const std::string& distributorIp, int distributorPort) {
    // Establish connection to the Distributor
    capnp::EzRpcClient client(distributorIp, distributorPort);
    auto distributor = client.getMain<Distributor>();

    // Prepare PodInfo with the Pod's details
    auto request = distributor.registerPodRequest();
    request.getPodInfo().setIpAddress("localhost");  // Use actual IP address
    request.getPodInfo().setPort(12348);             // Use actual port number
    request.getPodInfo().setCores(std::thread::hardware_concurrency());  // Number of cores
    request.getPodInfo().setThreads(std::thread::hardware_concurrency()); // Number of threads
    request.getPodInfo().setRam(8 * 1024 * 1024 * 1024);  // Example RAM value, 8GB

    // Send the registration request
    request.send().wait(client.getWaitScope());
}

kj::Promise<void> PodImpl::processWorkload(ProcessWorkloadContext context) {
    if (isBusy) {
        // Reject the workload if already busy (optional based on your design)
        return kj::READY_NOW;
    }

    isBusy = true;

    // Extract workload details
    auto workload = context.getParams().getWorkload();
    auto array = workload.getArray();
    auto operation = workload.getOperation();
    auto operand = workload.getOperand();

    // Process the array according to the operation
    std::vector<int32_t> result;
    result.reserve(array.size());

    if (operation == "sum") {
        for (auto value : array) {
            result.push_back(value + operand);
        }
    } else if (operation == "subtract") {
        for (auto value : array) {
            result.push_back(value - operand);
        }
    }
    // Handle other operations as needed...

    // Simulate processing delay
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Send the result back to the Gatherer (assumed to be implemented separately)

    isBusy = false;

    return kj::READY_NOW;
}

int main(int argc, const char* argv[]) {
    capnp::EzRpcServer server(kj::heap<PodImpl>("localhost", 12347), "localhost", 12348);

    kj::NEVER_DONE.wait(server.getWaitScope());
}
