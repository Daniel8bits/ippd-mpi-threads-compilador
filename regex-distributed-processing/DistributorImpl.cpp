#include "DistributorImpl.h"

DistributorImpl::DistributorImpl(kj::Own<Scheduler::Client> sched)
    : scheduler(kj::mv(sched)) {
    schedulerThread = std::thread(&DistributorImpl::fetchAndDistributeWorkloads, this);
}

kj::Promise<void> DistributorImpl::registerPod(RegisterPodContext context) {
    auto podInfo = context.getParams().getPodInfo();
    std::string key = std::string(podInfo.getIpAddress().cStr()) + ":" + std::to_string(podInfo.getPort());

    std::lock_guard<std::mutex> lock(podsMutex);
    pods[key] = Pod(podInfo.getIpAddress(), podInfo.getPort(), podInfo.getCores(),
                    podInfo.getThreads(), podInfo.getRam());

    return kj::READY_NOW;
}

void DistributorImpl::distributeWorkload() {
    // Placeholder for the actual distribution logic
    std::lock_guard<std::mutex> lock(podsMutex);

    for (auto& [key, pod] : pods) {
        if (!pod.isBusy) {
            // Assign workload to this pod
            // For example, send a Cap'n Proto RPC to the Pod to start processing

            pod.isBusy = true; // Mark pod as busy
        }
    }
}

void DistributorImpl::fetchAndDistributeWorkloads() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Adjust the interval as needed

        // Fetch the workload from the Scheduler
        auto request = scheduler->getWorkloadRequest();

        request.send().then([this](Scheduler::WorkloadResponse::Reader response) {
            // Distribute the workloads across Pods
            distributeWorkload();
        }).wait();
    }
}

int main(int argc, const char* argv[]) {
    capnp::EzRpcServer server(kj::heap<DistributorImpl>(/* pass Scheduler client here */), "localhost", 12347);

    kj::NEVER_DONE.wait(server.getWaitScope());
}
