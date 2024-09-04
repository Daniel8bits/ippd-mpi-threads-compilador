@0xabcdefabcdefabcdf0;

using Cxx = import "/capnp/c++.capnp";

interface Distributor {
  registerPod @0 (podInfo :PodInfo) -> ();

  struct PodInfo {
    ipAddress @0 :Text;
    port @1 :Int32;
    cores @2 :Int32;
    threads @3 :Int32;
    ram @4 :Int64;
  }
}