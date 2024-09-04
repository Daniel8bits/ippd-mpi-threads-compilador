#include "GathererImpl.h"

GathererImpl::GathererImpl() {
    // Initialization if necessary
}

kj::Promise<void> GathererImpl::gatherResults(GatherResultsContext context) {
    auto result = context.getParams().getResult();
    auto array = result.getArray();
    auto clientAddress = result.getClientAddress();
    int clientPort = result.getClientPort();

    std::vector<int32_t> resultVector(array.begin(), array.end());

    std::thread sendThread(&GathererImpl::sendResultToClient, this, clientAddress, clientPort, resultVector);
    sendThread.detach();  // Handle sending in a separate thread

    return kj::READY_NOW;
}

void GathererImpl::sendResultToClient(const std::string& clientAddress, int clientPort, const std::vector<int32_t>& result) {
    // Implement sending the result back to the client
    // This can be done using a simple TCP connection, Cap'n Proto, or any other method

    // Example: (Pseudocode)
    /*
    capnp::EzRpcClient client(clientAddress, clientPort);
    auto controller = client.getMain<Controller>();
    auto request = controller.sendResultRequest();
    request.setArray(result);
    request.send().wait(client.getWaitScope());
    */
}

int main(int argc, const char* argv[]) {
    capnp::EzRpcServer server(kj::heap<GathererImpl>(), "localhost", 12349);

    kj::NEVER_DONE.wait(server.getWaitScope());
}
