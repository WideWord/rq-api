#pragma once

#include "TableSchema.hpp"
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
	};

	struct AnyQueryResult {
		std::vector<AnyRecord> records;
		AnyRecordValue value;
	};

}
