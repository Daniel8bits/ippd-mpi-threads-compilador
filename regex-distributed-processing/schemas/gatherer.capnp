@0xabcdefabcdefabcdf1;

using Cxx = import "/capnp/c++.capnp";

interface Gatherer {
  gatherResults @0 (result :ProcessingResult) -> ();

  struct ProcessingResult {
    array @0 :List(Int32);
    clientAddress @1 :Text;
    clientPort @2 :Int32;
  }
}