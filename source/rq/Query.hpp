#pragma once

#include "TableSchema.hpp"
#include "Bounds.hpp"
#include <vector>

namespace rq {

	enum class QueryType {
		list,
		min,
		max,
		average,
		sum,
	};

	struct Query {
		Bounds bounds;
		QueryType type;
		size_t axis;
	};

	struct QueryResult {
		std::vector<Record> records;
		RecordTypedValue value;
		size_t recordsNum = 0;

		QueryResult() = default;
	};

}
