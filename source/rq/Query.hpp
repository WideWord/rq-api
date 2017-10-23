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

	struct AnyQuery {
		AnyBounds bounds;
		QueryType type;
		size_t axis;
	};

	struct AnyQueryResult {
		std::vector<AnyRecord> records;
		AnyRecordTypedValue value;
		size_t recordsNum = 0;

		AnyQueryResult() = default;
	};

}
