#ifndef CONTROLLERIMPL_H
#define CONTROLLERIMPL_H


#include <capnp/ez-rpc.h>
#include "schemas/controller.capnp.h"
#include "schemas/scheduler.capnp.h" // Assuming you'll have Scheduler schema similarly defined
#include <kj/async-io.h>

class ControllerImpl final : public Controller::Server {
public:
    explicit ControllerImpl(kj::Own<Scheduler::Client> scheduler)
        : scheduler(kj::mv(scheduler)) {}

    // Implement the processRequest method from the schema
    kj::Promise<void> processRequest(ProcessRequestContext context) override;

private:
    kj::Own<Scheduler::Client> scheduler;
};

#endif // CONTROLLERIMPL_H
