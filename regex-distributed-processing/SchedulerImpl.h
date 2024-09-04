#ifndef SCHEDULERIMPL_H
#define SCHEDULERIMPL_H

#include <capnp/ez-rpc.h>
#include "schemas/scheduler.capnp.h"
#include <queue>
#include <unordered_map>
#include <vector>

struct RequestWithTime {
    Scheduler::Request::Reader request;
    double timestamp;

    bool operator<(const RequestWithTime& other) const {
        return timestamp > other.timestamp; // Min-heap based on timestamp
    }
};

class SchedulerImpl final : public Scheduler::Server {
public:
    SchedulerImpl() = default;

    kj::Promise<void> scheduleWorkRequest(ScheduleWorkRequestContext context) override;
    kj::Promise<void> getWorkload(GetWorkloadContext context) override;

private:
    std::priority_queue<RequestWithTime> requestQueue;
    std::unordered_map<std::string, std::vector<Scheduler::Request::Reader>> sortedWorkloads;
    double lastSortedTime = 0.0;

    void sortWorkloads();
};

#endif // SCHEDULERIMPL_H
