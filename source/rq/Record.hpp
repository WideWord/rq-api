#pragma once

#include "TableSchema.hpp"
#include <vector>

namespace rq {

	class Record {
	public:
		Record() = default;
		Record(const Record &o) : id(o.id), values(o.values) {}
		Record(RecordID id, const Point &values) : id(id), values(values) {}

		RecordID getID() const { return id; }
		RecordValue getValue(size_t i) const { return values[i]; }
		const Point &getValues() const { return values; }

		void setID(RecordID _id) { id = _id; }
		void setValue(size_t i, const RecordValue &v) { values[i] = v; }
	private:
		RecordID id = 0;
		Point values;
	};

}
