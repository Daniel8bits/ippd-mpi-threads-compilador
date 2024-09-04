#include "SchedulerImpl.h"
#include <chrono>
#include <cmath>

kj::Promise<void> SchedulerImpl::scheduleWorkRequest(ScheduleWorkRequestContext context) {
    auto request = context.getParams().getRequest();
    double timestamp = request.getTimestamp();

    // Add the request to the priority queue
    requestQueue.push({ request, timestamp });

    // Sort workloads if necessary
    if (timestamp - lastSortedTime >= 0.25) { // 250 milliseconds
        sortWorkloads();
        lastSortedTime = timestamp;
    }

    return kj::READY_NOW;
}

kj::Promise<void> SchedulerImpl::getWorkload(GetWorkloadContext context) {
    // Return the most recent workload to the Distributor
    auto responseBuilder = context.initResults();

    auto workloadList = responseBuilder.initResponse();//sortedWorkloads.size());
    int workloadIndex = 0;

    for (const auto& [operation, requests] : sortedWorkloads) {
        auto workloadBuilder = workloadList[workloadIndex].initArrays(requests.size());
        for (size_t i = 0; i < requests.size(); ++i) {
            workloadBuilder[i] = requests[i];
        }
        ++workloadIndex;
    }

    // Clear sorted workloads after sending them
    sortedWorkloads.clear();

    return kj::READY_NOW;
}

void SchedulerImpl::sortWorkloads() {
    while (!requestQueue.empty()) {
        auto requestWithTime = requestQueue.top();
        requestQueue.pop();

        auto operation = requestWithTime.request.getOperation();
        sortedWorkloads[operation].push_back(requestWithTime.request);
    }
}

int main(int argc, const char* argv[]) {
    capnp::EzRpcServer server(kj::heap<SchedulerImpl>(), "localhost", 12346);

    kj::NEVER_DONE.wait(server.getWaitScope());
}
