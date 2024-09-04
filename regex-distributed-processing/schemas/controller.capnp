@0xabcdefabcdefabcdef;

using Cxx = import "/capnp/c++.capnp";

interface Controller {
  processRequest @0 (request :Request) -> (response :Response);

  struct Request {
    array @0 :List(Int32);
    operation @1 :Text;
    operand @2 :Int32;
  }

  struct Response {
    resultArray @0 :List(Int32);
  }
}