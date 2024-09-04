#include "ControllerImpl.h"

kj::Promise<void> ControllerImpl::processRequest(ProcessRequestContext context) {
    // Extract the request data
    auto request = context.getParams().getRequest();
    auto array = request.getArray();
    auto operation = request.getOperation();
    auto operand = request.getOperand();

    ::capnp::MallocMessageBuilder message;
    Scheduler::Request::Builder schedulerRequest = message.initRoot<Scheduler::Request>();

    schedulerRequest.setArray(array);
    schedulerRequest.setOperation(operation);
    schedulerRequest.setOperand(operand);
    //::capnp::List<int32_t>::Builder request = message.initRoot<Scheduler::Request>();

    // Create a Scheduler request (based on the scheduler schema)

    auto scheduleWorkRequest = scheduler->scheduleWorkRequestRequest();
    scheduleWorkRequest.setRequest(schedulerRequest);

    // Wait for Scheduler's response (async)
    return scheduleWorkRequest.send().then([context](Scheduler::ScheduleWorkRequestResults::Reader response) {
        // Send the processed result back to the client
        auto resultArray = response.getResultArray();
        auto responseBuilder = context.initResults();
        responseBuilder.setResultArray(resultArray);
    });
}

int main(int argc, const char* argv[]) {
    capnp::EzRpcClient client("localhost:12345");
    Scheduler::Client scheduler = client.getMain<Scheduler>();
    capnp::EzRpcServer server(kj::heap<ControllerImpl>(scheduler), "localhost", 12345);

    // Run the RPC server until it is manually terminated
    kj::NEVER_DONE.wait(server.getWaitScope());
}
