// Generated by Cap'n Proto compiler, DO NOT EDIT
// source: pod.capnp

#pragma once

#include <capnp/generated-header-support.h>
#include <kj/windows-sanity.h>
#if !CAPNP_LITE
#include <capnp/capability.h>
#endif  // !CAPNP_LITE

#ifndef CAPNP_VERSION
#error "CAPNP_VERSION is not defined, is capnp/generated-header-support.h missing?"
#elif CAPNP_VERSION != 1000002
#error "Version mismatch between generated code and library headers.  You must use the same version of the Cap'n Proto compiler and library."
#endif


CAPNP_BEGIN_HEADER

namespace capnp {
namespace schemas {

CAPNP_DECLARE_SCHEMA(def431fec444a116);
CAPNP_DECLARE_SCHEMA(d6f97574fccf8990);
CAPNP_DECLARE_SCHEMA(bb3a09993d644074);
CAPNP_DECLARE_SCHEMA(a42d8f7c3d012003);

}  // namespace schemas
}  // namespace capnp


struct Pod {
  Pod() = delete;

#if !CAPNP_LITE
  class Client;
  class Server;
#endif  // !CAPNP_LITE

  struct Workload;
  struct ProcessWorkloadParams;
  struct ProcessWorkloadResults;

  #if !CAPNP_LITE
  struct _capnpPrivate {
    CAPNP_DECLARE_INTERFACE_HEADER(def431fec444a116)
    static constexpr ::capnp::_::RawBrandedSchema const* brand() { return &schema->defaultBrand; }
  };
  #endif  // !CAPNP_LITE
};

struct Pod::Workload {
  Workload() = delete;

  class Reader;
  class Builder;
  class Pipeline;

  struct _capnpPrivate {
    CAPNP_DECLARE_STRUCT_HEADER(d6f97574fccf8990, 1, 2)
    #if !CAPNP_LITE
    static constexpr ::capnp::_::RawBrandedSchema const* brand() { return &schema->defaultBrand; }
    #endif  // !CAPNP_LITE
  };
};

struct Pod::ProcessWorkloadParams {
  ProcessWorkloadParams() = delete;

  class Reader;
  class Builder;
  class Pipeline;

  struct _capnpPrivate {
    CAPNP_DECLARE_STRUCT_HEADER(bb3a09993d644074, 0, 1)
    #if !CAPNP_LITE
    static constexpr ::capnp::_::RawBrandedSchema const* brand() { return &schema->defaultBrand; }
    #endif  // !CAPNP_LITE
  };
};

struct Pod::ProcessWorkloadResults {
  ProcessWorkloadResults() = delete;

  class Reader;
  class Builder;
  class Pipeline;

  struct _capnpPrivate {
    CAPNP_DECLARE_STRUCT_HEADER(a42d8f7c3d012003, 0, 0)
    #if !CAPNP_LITE
    static constexpr ::capnp::_::RawBrandedSchema const* brand() { return &schema->defaultBrand; }
    #endif  // !CAPNP_LITE
  };
};

// =======================================================================================

#if !CAPNP_LITE
class Pod::Client
    : public virtual ::capnp::Capability::Client {
public:
  typedef Pod Calls;
  typedef Pod Reads;

  Client(decltype(nullptr));
  explicit Client(::kj::Own< ::capnp::ClientHook>&& hook);
  template <typename _t, typename = ::kj::EnableIf< ::kj::canConvert<_t*, Server*>()>>
  Client(::kj::Own<_t>&& server);
  template <typename _t, typename = ::kj::EnableIf< ::kj::canConvert<_t*, Client*>()>>
  Client(::kj::Promise<_t>&& promise);
  Client(::kj::Exception&& exception);
  Client(Client&) = default;
  Client(Client&&) = default;
  Client& operator=(Client& other);
  Client& operator=(Client&& other);

  ::capnp::Request< ::Pod::ProcessWorkloadParams,  ::Pod::ProcessWorkloadResults> processWorkloadRequest(
      ::kj::Maybe< ::capnp::MessageSize> sizeHint = nullptr);

protected:
  Client() = default;
};

class Pod::Server
    : public virtual ::capnp::Capability::Server {
public:
  typedef Pod Serves;

  ::capnp::Capability::Server::DispatchCallResult dispatchCall(
      uint64_t interfaceId, uint16_t methodId,
      ::capnp::CallContext< ::capnp::AnyPointer, ::capnp::AnyPointer> context)
      override;

protected:
  typedef  ::Pod::ProcessWorkloadParams ProcessWorkloadParams;
  typedef  ::Pod::ProcessWorkloadResults ProcessWorkloadResults;
  typedef ::capnp::CallContext<ProcessWorkloadParams, ProcessWorkloadResults> ProcessWorkloadContext;
  virtual ::kj::Promise<void> processWorkload(ProcessWorkloadContext context);

  inline  ::Pod::Client thisCap() {
    return ::capnp::Capability::Server::thisCap()
        .template castAs< ::Pod>();
  }

  ::capnp::Capability::Server::DispatchCallResult dispatchCallInternal(
      uint16_t methodId,
      ::capnp::CallContext< ::capnp::AnyPointer, ::capnp::AnyPointer> context);
};
#endif  // !CAPNP_LITE

class Pod::Workload::Reader {
public:
  typedef Workload Reads;

  Reader() = default;
  inline explicit Reader(::capnp::_::StructReader base): _reader(base) {}

  inline ::capnp::MessageSize totalSize() const {
    return _reader.totalSize().asPublic();
  }

#if !CAPNP_LITE
  inline ::kj::StringTree toString() const {
    return ::capnp::_::structString(_reader, *_capnpPrivate::brand());
  }
#endif  // !CAPNP_LITE

  inline bool hasArray() const;
  inline  ::capnp::List< ::int32_t,  ::capnp::Kind::PRIMITIVE>::Reader getArray() const;

  inline bool hasOperation() const;
  inline  ::capnp::Text::Reader getOperation() const;

  inline  ::int32_t getOperand() const;

private:
  ::capnp::_::StructReader _reader;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::List;
  friend class ::capnp::MessageBuilder;
  friend class ::capnp::Orphanage;
};

class Pod::Workload::Builder {
public:
  typedef Workload Builds;

  Builder() = delete;  // Deleted to discourage incorrect usage.
                       // You can explicitly initialize to nullptr instead.
  inline Builder(decltype(nullptr)) {}
  inline explicit Builder(::capnp::_::StructBuilder base): _builder(base) {}
  inline operator Reader() const { return Reader(_builder.asReader()); }
  inline Reader asReader() const { return *this; }

  inline ::capnp::MessageSize totalSize() const { return asReader().totalSize(); }
#if !CAPNP_LITE
  inline ::kj::StringTree toString() const { return asReader().toString(); }
#endif  // !CAPNP_LITE

  inline bool hasArray();
  inline  ::capnp::List< ::int32_t,  ::capnp::Kind::PRIMITIVE>::Builder getArray();
  inline void setArray( ::capnp::List< ::int32_t,  ::capnp::Kind::PRIMITIVE>::Reader value);
  inline void setArray(::kj::ArrayPtr<const  ::int32_t> value);
  inline  ::capnp::List< ::int32_t,  ::capnp::Kind::PRIMITIVE>::Builder initArray(unsigned int size);
  inline void adoptArray(::capnp::Orphan< ::capnp::List< ::int32_t,  ::capnp::Kind::PRIMITIVE>>&& value);
  inline ::capnp::Orphan< ::capnp::List< ::int32_t,  ::capnp::Kind::PRIMITIVE>> disownArray();

  inline bool hasOperation();
  inline  ::capnp::Text::Builder getOperation();
  inline void setOperation( ::capnp::Text::Reader value);
  inline  ::capnp::Text::Builder initOperation(unsigned int size);
  inline void adoptOperation(::capnp::Orphan< ::capnp::Text>&& value);
  inline ::capnp::Orphan< ::capnp::Text> disownOperation();

  inline  ::int32_t getOperand();
  inline void setOperand( ::int32_t value);

private:
  ::capnp::_::StructBuilder _builder;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  friend class ::capnp::Orphanage;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
};

#if !CAPNP_LITE
class Pod::Workload::Pipeline {
public:
  typedef Workload Pipelines;

  inline Pipeline(decltype(nullptr)): _typeless(nullptr) {}
  inline explicit Pipeline(::capnp::AnyPointer::Pipeline&& typeless)
      : _typeless(kj::mv(typeless)) {}

private:
  ::capnp::AnyPointer::Pipeline _typeless;
  friend class ::capnp::PipelineHook;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
};
#endif  // !CAPNP_LITE

class Pod::ProcessWorkloadParams::Reader {
public:
  typedef ProcessWorkloadParams Reads;

  Reader() = default;
  inline explicit Reader(::capnp::_::StructReader base): _reader(base) {}

  inline ::capnp::MessageSize totalSize() const {
    return _reader.totalSize().asPublic();
  }

#if !CAPNP_LITE
  inline ::kj::StringTree toString() const {
    return ::capnp::_::structString(_reader, *_capnpPrivate::brand());
  }
#endif  // !CAPNP_LITE

  inline bool hasWorkload() const;
  inline  ::Pod::Workload::Reader getWorkload() const;

private:
  ::capnp::_::StructReader _reader;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::List;
  friend class ::capnp::MessageBuilder;
  friend class ::capnp::Orphanage;
};

class Pod::ProcessWorkloadParams::Builder {
public:
  typedef ProcessWorkloadParams Builds;

  Builder() = delete;  // Deleted to discourage incorrect usage.
                       // You can explicitly initialize to nullptr instead.
  inline Builder(decltype(nullptr)) {}
  inline explicit Builder(::capnp::_::StructBuilder base): _builder(base) {}
  inline operator Reader() const { return Reader(_builder.asReader()); }
  inline Reader asReader() const { return *this; }

  inline ::capnp::MessageSize totalSize() const { return asReader().totalSize(); }
#if !CAPNP_LITE
  inline ::kj::StringTree toString() const { return asReader().toString(); }
#endif  // !CAPNP_LITE

  inline bool hasWorkload();
  inline  ::Pod::Workload::Builder getWorkload();
  inline void setWorkload( ::Pod::Workload::Reader value);
  inline  ::Pod::Workload::Builder initWorkload();
  inline void adoptWorkload(::capnp::Orphan< ::Pod::Workload>&& value);
  inline ::capnp::Orphan< ::Pod::Workload> disownWorkload();

private:
  ::capnp::_::StructBuilder _builder;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  friend class ::capnp::Orphanage;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
};

#if !CAPNP_LITE
class Pod::ProcessWorkloadParams::Pipeline {
public:
  typedef ProcessWorkloadParams Pipelines;

  inline Pipeline(decltype(nullptr)): _typeless(nullptr) {}
  inline explicit Pipeline(::capnp::AnyPointer::Pipeline&& typeless)
      : _typeless(kj::mv(typeless)) {}

  inline  ::Pod::Workload::Pipeline getWorkload();
private:
  ::capnp::AnyPointer::Pipeline _typeless;
  friend class ::capnp::PipelineHook;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
};
#endif  // !CAPNP_LITE

class Pod::ProcessWorkloadResults::Reader {
public:
  typedef ProcessWorkloadResults Reads;

  Reader() = default;
  inline explicit Reader(::capnp::_::StructReader base): _reader(base) {}

  inline ::capnp::MessageSize totalSize() const {
    return _reader.totalSize().asPublic();
  }

#if !CAPNP_LITE
  inline ::kj::StringTree toString() const {
    return ::capnp::_::structString(_reader, *_capnpPrivate::brand());
  }
#endif  // !CAPNP_LITE

private:
  ::capnp::_::StructReader _reader;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::List;
  friend class ::capnp::MessageBuilder;
  friend class ::capnp::Orphanage;
};

class Pod::ProcessWorkloadResults::Builder {
public:
  typedef ProcessWorkloadResults Builds;

  Builder() = delete;  // Deleted to discourage incorrect usage.
                       // You can explicitly initialize to nullptr instead.
  inline Builder(decltype(nullptr)) {}
  inline explicit Builder(::capnp::_::StructBuilder base): _builder(base) {}
  inline operator Reader() const { return Reader(_builder.asReader()); }
  inline Reader asReader() const { return *this; }

  inline ::capnp::MessageSize totalSize() const { return asReader().totalSize(); }
#if !CAPNP_LITE
  inline ::kj::StringTree toString() const { return asReader().toString(); }
#endif  // !CAPNP_LITE

private:
  ::capnp::_::StructBuilder _builder;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
  friend class ::capnp::Orphanage;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::_::PointerHelpers;
};

#if !CAPNP_LITE
class Pod::ProcessWorkloadResults::Pipeline {
public:
  typedef ProcessWorkloadResults Pipelines;

  inline Pipeline(decltype(nullptr)): _typeless(nullptr) {}
  inline explicit Pipeline(::capnp::AnyPointer::Pipeline&& typeless)
      : _typeless(kj::mv(typeless)) {}

private:
  ::capnp::AnyPointer::Pipeline _typeless;
  friend class ::capnp::PipelineHook;
  template <typename, ::capnp::Kind>
  friend struct ::capnp::ToDynamic_;
};
#endif  // !CAPNP_LITE

// =======================================================================================

#if !CAPNP_LITE
inline Pod::Client::Client(decltype(nullptr))
    : ::capnp::Capability::Client(nullptr) {}
inline Pod::Client::Client(
    ::kj::Own< ::capnp::ClientHook>&& hook)
    : ::capnp::Capability::Client(::kj::mv(hook)) {}
template <typename _t, typename>
inline Pod::Client::Client(::kj::Own<_t>&& server)
    : ::capnp::Capability::Client(::kj::mv(server)) {}
template <typename _t, typename>
inline Pod::Client::Client(::kj::Promise<_t>&& promise)
    : ::capnp::Capability::Client(::kj::mv(promise)) {}
inline Pod::Client::Client(::kj::Exception&& exception)
    : ::capnp::Capability::Client(::kj::mv(exception)) {}
inline  ::Pod::Client& Pod::Client::operator=(Client& other) {
  ::capnp::Capability::Client::operator=(other);
  return *this;
}
inline  ::Pod::Client& Pod::Client::operator=(Client&& other) {
  ::capnp::Capability::Client::operator=(kj::mv(other));
  return *this;
}

#endif  // !CAPNP_LITE
inline bool Pod::Workload::Reader::hasArray() const {
  return !_reader.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS).isNull();
}
inline bool Pod::Workload::Builder::hasArray() {
  return !_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS).isNull();
}
inline  ::capnp::List< ::int32_t,  ::capnp::Kind::PRIMITIVE>::Reader Pod::Workload::Reader::getArray() const {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::int32_t,  ::capnp::Kind::PRIMITIVE>>::get(_reader.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}
inline  ::capnp::List< ::int32_t,  ::capnp::Kind::PRIMITIVE>::Builder Pod::Workload::Builder::getArray() {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::int32_t,  ::capnp::Kind::PRIMITIVE>>::get(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}
inline void Pod::Workload::Builder::setArray( ::capnp::List< ::int32_t,  ::capnp::Kind::PRIMITIVE>::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::int32_t,  ::capnp::Kind::PRIMITIVE>>::set(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS), value);
}
inline void Pod::Workload::Builder::setArray(::kj::ArrayPtr<const  ::int32_t> value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::int32_t,  ::capnp::Kind::PRIMITIVE>>::set(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS), value);
}
inline  ::capnp::List< ::int32_t,  ::capnp::Kind::PRIMITIVE>::Builder Pod::Workload::Builder::initArray(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::int32_t,  ::capnp::Kind::PRIMITIVE>>::init(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS), size);
}
inline void Pod::Workload::Builder::adoptArray(
    ::capnp::Orphan< ::capnp::List< ::int32_t,  ::capnp::Kind::PRIMITIVE>>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::List< ::int32_t,  ::capnp::Kind::PRIMITIVE>>::adopt(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::List< ::int32_t,  ::capnp::Kind::PRIMITIVE>> Pod::Workload::Builder::disownArray() {
  return ::capnp::_::PointerHelpers< ::capnp::List< ::int32_t,  ::capnp::Kind::PRIMITIVE>>::disown(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}

inline bool Pod::Workload::Reader::hasOperation() const {
  return !_reader.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS).isNull();
}
inline bool Pod::Workload::Builder::hasOperation() {
  return !_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS).isNull();
}
inline  ::capnp::Text::Reader Pod::Workload::Reader::getOperation() const {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(_reader.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS));
}
inline  ::capnp::Text::Builder Pod::Workload::Builder::getOperation() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::get(_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS));
}
inline void Pod::Workload::Builder::setOperation( ::capnp::Text::Reader value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::set(_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS), value);
}
inline  ::capnp::Text::Builder Pod::Workload::Builder::initOperation(unsigned int size) {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::init(_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS), size);
}
inline void Pod::Workload::Builder::adoptOperation(
    ::capnp::Orphan< ::capnp::Text>&& value) {
  ::capnp::_::PointerHelpers< ::capnp::Text>::adopt(_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::capnp::Text> Pod::Workload::Builder::disownOperation() {
  return ::capnp::_::PointerHelpers< ::capnp::Text>::disown(_builder.getPointerField(
      ::capnp::bounded<1>() * ::capnp::POINTERS));
}

inline  ::int32_t Pod::Workload::Reader::getOperand() const {
  return _reader.getDataField< ::int32_t>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS);
}

inline  ::int32_t Pod::Workload::Builder::getOperand() {
  return _builder.getDataField< ::int32_t>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS);
}
inline void Pod::Workload::Builder::setOperand( ::int32_t value) {
  _builder.setDataField< ::int32_t>(
      ::capnp::bounded<0>() * ::capnp::ELEMENTS, value);
}

inline bool Pod::ProcessWorkloadParams::Reader::hasWorkload() const {
  return !_reader.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS).isNull();
}
inline bool Pod::ProcessWorkloadParams::Builder::hasWorkload() {
  return !_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS).isNull();
}
inline  ::Pod::Workload::Reader Pod::ProcessWorkloadParams::Reader::getWorkload() const {
  return ::capnp::_::PointerHelpers< ::Pod::Workload>::get(_reader.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}
inline  ::Pod::Workload::Builder Pod::ProcessWorkloadParams::Builder::getWorkload() {
  return ::capnp::_::PointerHelpers< ::Pod::Workload>::get(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}
#if !CAPNP_LITE
inline  ::Pod::Workload::Pipeline Pod::ProcessWorkloadParams::Pipeline::getWorkload() {
  return  ::Pod::Workload::Pipeline(_typeless.getPointerField(0));
}
#endif  // !CAPNP_LITE
inline void Pod::ProcessWorkloadParams::Builder::setWorkload( ::Pod::Workload::Reader value) {
  ::capnp::_::PointerHelpers< ::Pod::Workload>::set(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS), value);
}
inline  ::Pod::Workload::Builder Pod::ProcessWorkloadParams::Builder::initWorkload() {
  return ::capnp::_::PointerHelpers< ::Pod::Workload>::init(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}
inline void Pod::ProcessWorkloadParams::Builder::adoptWorkload(
    ::capnp::Orphan< ::Pod::Workload>&& value) {
  ::capnp::_::PointerHelpers< ::Pod::Workload>::adopt(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS), kj::mv(value));
}
inline ::capnp::Orphan< ::Pod::Workload> Pod::ProcessWorkloadParams::Builder::disownWorkload() {
  return ::capnp::_::PointerHelpers< ::Pod::Workload>::disown(_builder.getPointerField(
      ::capnp::bounded<0>() * ::capnp::POINTERS));
}


CAPNP_END_HEADER

