#include "capi_tester.hpp"

using namespace duckdb;
using namespace std;

TEST_CASE("Test C API VARIANT type support", "[capi]") {
	CAPITester tester;
	duckdb::unique_ptr<CAPIResult> result;

	REQUIRE(tester.OpenDatabase(nullptr));
	REQUIRE_NO_FAIL(tester.Query("CREATE TABLE t1 (data VARIANT)"));
	REQUIRE_NO_FAIL(tester.Query("INSERT INTO t1 VALUES (42::VARIANT)"));
	REQUIRE_NO_FAIL(tester.Query("INSERT INTO t1 VALUES ('hello world'::VARIANT)"));

	result = tester.Query("SELECT data FROM t1");
	REQUIRE_NO_FAIL(*result);

	REQUIRE(result->ColumnType(0) == DUCKDB_TYPE_VARIANT);

	auto chunk = result->FetchChunk(0);
	REQUIRE(chunk);

	auto vec = duckdb_data_chunk_get_vector(chunk->GetChunk(), 0);

	auto type = duckdb_vector_get_column_type(vec);

	REQUIRE(duckdb_get_type_id(type) == DUCKDB_TYPE_VARIANT);

	duckdb_destroy_logical_type(&type);
}

TEST_CASE("Test C API VARIANT logical type creation", "[capi]") {
	auto variant_type = duckdb_create_logical_type(DUCKDB_TYPE_VARIANT);
	REQUIRE(variant_type);
	REQUIRE(duckdb_get_type_id(variant_type) == DUCKDB_TYPE_VARIANT);

	// LIST(VARIANT) value with mixed element types
	duckdb_value elements[2] = {duckdb_create_int32(1), duckdb_create_varchar("two")};
	auto list_value = duckdb_create_list_value(variant_type, elements, 2);
	REQUIRE(list_value);
	REQUIRE(duckdb_get_list_size(list_value) == 2);
	duckdb_destroy_value(&list_value);
	duckdb_destroy_value(&elements[0]);
	duckdb_destroy_value(&elements[1]);

	// ARRAY(VARIANT) value
	duckdb_value array_elements[2] = {duckdb_create_int32(1), duckdb_create_varchar("two")};
	auto array_value = duckdb_create_array_value(variant_type, array_elements, 2);
	REQUIRE(array_value);
	duckdb_destroy_value(&array_value);
	duckdb_destroy_value(&array_elements[0]);
	duckdb_destroy_value(&array_elements[1]);

	// bind a value to a ?::VARIANT parameter
	CAPITester tester;
	REQUIRE(tester.OpenDatabase(nullptr));
	duckdb_prepared_statement prepared;
	REQUIRE(duckdb_prepare(tester.connection, "SELECT ?::VARIANT", &prepared) == DuckDBSuccess);
	auto int_value = duckdb_create_int32(42);
	REQUIRE(duckdb_bind_value(prepared, 1, int_value) == DuckDBSuccess);
	duckdb_destroy_value(&int_value);
	duckdb_result result;
	REQUIRE(duckdb_execute_prepared(prepared, &result) == DuckDBSuccess);
	duckdb_destroy_result(&result);
	duckdb_destroy_prepare(&prepared);
	duckdb_destroy_logical_type(&variant_type);
}
