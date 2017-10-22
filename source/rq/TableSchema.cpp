#include "TableSchema.hpp"
#include "Record.hpp"

namespace rq {

	AnyBounds AnyBounds::fromRecord(const AnyRecord &record) {
		AnyBounds result;
		result.min = record.getValues();
		result.max = result.min;
		return result;
	}

}
