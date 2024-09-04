@0xabcdefabcdefabcdf3;

using Cxx = import "/capnp/c++.capnp";

interface Scheduler {
  scheduleWorkRequest @0 (request :Request) -> ();

  getWorkload @1 () -> (response :WorkloadResponse);

  struct Request {
    array @0 :List(Int32);
    operation @1 :Text;
    operand @2 :Int32;
    timestamp @3 :Float64;
  }

  struct WorkloadResponse {
    workloads @0 :List(Workload);
  }

  struct Workload {
    arrays @0 :List(Request);
  }
}