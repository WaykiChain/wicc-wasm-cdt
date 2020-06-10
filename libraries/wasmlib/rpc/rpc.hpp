#pragma once
#include <vector>

namespace wasm {

/**
 *  set the result for wasm rpc.
 *
 *  @ingroup set_rpc_result
 */
void set_rpc_result(const uint64_t &name, const std::vector<char> &value);

} // namespace wasm
