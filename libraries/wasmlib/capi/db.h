#pragma once

extern "C" {

	__attribute__((wasm_wasm_import))
	int32_t db_store( const void* key, uint32_t key_len,  const void* value, uint32_t value_len);

	__attribute__((wasm_wasm_import))
	int32_t db_remove( const void* key, uint32_t key_len);

	__attribute__((wasm_wasm_import))
	int32_t db_get( const void* key, uint32_t key_len, void* value, uint32_t value_len);

	__attribute__((wasm_wasm_import))
	int32_t db_update( const void* key, uint32_t key_len, const void* value, uint32_t value_len);

}