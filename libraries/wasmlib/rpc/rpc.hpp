#pragma once
#include <vector>

namespace wasm {

/**
 *  set the result for wasm rpc.
 *
 *  @ingroup set_rpc_result
 */
void set_rpc_result(const string &name, const string &type, const std::vector<char> &value);


/**
 *  set the result for wasm rpc.
 *
 *  @ingroup emit_result
 */
  template <typename T>
  inline void emit_result(const string &name, const string &type, const T &value) {
    auto v = wasm::pack<T>(value);
    set_rpc_result(name, type, v);
  }

} // namespace wasm
