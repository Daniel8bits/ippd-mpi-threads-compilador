@0xabcdefabcdefabcdf2;

using Cxx = import "/capnp/c++.capnp";

interface Pod {
    processWorkload @0 (workload :Workload) -> ();

    struct Workload {
        array @0 :List(Int32);
        operation @1 :Text;
        operand @2 :Int32;
    }
}