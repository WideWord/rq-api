#pragma once

#include <vector>
#include <functional>

namespace rq {

	enum class RecordValueType {
		d,
	};

	struct TableSchema {
		struct RTreeConfig {
			size_t minChildrenNum = 2;
			size_t maxChildrenNum = 8;
			size_t maxLeafRecordsNum = 20;
			RTreeConfig() = default;
		} rTreeConfig;
		std::vector<RecordValueType> types;
		TableSchema() = default;
	};

	using RecordID = uint64_t;

	union AnyRecordValue {
		double d;
	};

	struct AnyRecordTypedValue {
		AnyRecordValue value;
		RecordValueType type;

		AnyRecordTypedValue(RecordValueType t, AnyRecordValue v) : value(v), type(t) {}

		bool operator>(const AnyRecordTypedValue &o) const {
			switch (type) {
				case RecordValueType::d:
					return value.d > o.value.d;
			}
		}

		bool operator<(const AnyRecordTypedValue &o) const { return !(o > *this); }

		double asDouble() const {
			switch (type) {
				case RecordValueType::d:
					return value.d;
			}
		}
	};

	inline AnyRecordTypedValue max(const AnyRecordTypedValue &a, const AnyRecordTypedValue &b) {
		if (a > b) {
			return a;
		} else {
			return b;
		}
	}

	inline AnyRecordTypedValue min(const AnyRecordTypedValue &a, const AnyRecordTypedValue &b) {
		if (a < b) {
			return a;
		} else {
			return b;
		}
	}

	class AnyRecord;

	struct AnyBounds {
		std::vector<AnyRecordValue> min;
		std::vector<AnyRecordValue> max;

		static AnyBounds fromRecord(const AnyRecord &r);
	};

}
