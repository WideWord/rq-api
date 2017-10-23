#pragma once

#include <vector>
#include <functional>
#include <cmath>

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

	class AnyRecordTypedValue;

	template<typename V>
	inline AnyRecordTypedValue typedValue(RecordValueType t, V value);

	struct AnyRecordTypedValue {
		AnyRecordValue value;
		RecordValueType type;

		AnyRecordTypedValue() = default;
		AnyRecordTypedValue(RecordValueType t, AnyRecordValue v) : value(v), type(t) {}

		inline static AnyRecordTypedValue maxValue(RecordValueType t) {
			switch (t) {
				case RecordValueType::d:
					return typedValue(t, INFINITY);
			}
		}

		inline static AnyRecordTypedValue minValue(RecordValueType t) {
			switch (t) {
				case RecordValueType::d:
					return typedValue(t, -INFINITY);
			}
		}

		inline static AnyRecordTypedValue nullValue(RecordValueType t) {
			switch (t) {
				case RecordValueType::d:
					return typedValue(t, 0);
			}
		}

		bool operator>(const AnyRecordTypedValue &o) const {
			switch (type) {
				case RecordValueType::d:
					return value.d > o.value.d;
			}
		}

		bool operator<(const AnyRecordTypedValue &o) const { return (o > *this); }

		double asDouble() const {
			switch (type) {
				case RecordValueType::d:
					return value.d;
			}
		}

		AnyRecordTypedValue operator+(const AnyRecordTypedValue &o) {
			switch (type) {
				case RecordValueType::d:
					return typedValue(type, value.d + o.value.d);
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


	template<typename V>
	inline AnyRecordTypedValue typedValue(RecordValueType t, V value) {
		AnyRecordValue v;
		switch (t) {
			case RecordValueType::d: v.d = value; break;
		}
		return AnyRecordTypedValue(t, v);
	}

}
