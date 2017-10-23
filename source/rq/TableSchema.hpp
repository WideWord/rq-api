#pragma once

#include <vector>
#include <functional>
#include <cmath>

namespace rq {

	enum class RecordValueType {
		d,
	};

	using RecordID = uint64_t;

	union RecordValue {
		double d;
	};

	using Point = std::vector<RecordValue>;

	struct TableSchema {
		struct RTreeConfig {
			size_t minChildrenNum = 2;
			size_t maxChildrenNum = 8;
			size_t maxLeafRecordsNum = 20;
			RTreeConfig() = default;
		} rTreeConfig;
		std::vector<RecordValueType> types;
		TableSchema() = default;


		template<typename ...V>
		Point makePoint(V... values) {
			Point r;
			fillPoint(types.begin(), r, values...);
			return r;
		}
	private:
		template<typename H, typename ...V>
		void fillPoint(std::vector<RecordValueType>::const_iterator schemaType, Point &p, H d, V... values) {
			fillPointValue(*schemaType, p, d);
			fillPoint(schemaType + 1, p, values...);
		};

		template<typename H, typename ...V>
		void fillPoint(std::vector<RecordValueType>::const_iterator schemaType, Point &p, H d) {
			fillPointValue(*schemaType, p, d);
		};

		template<typename H>
		void fillPointValue(RecordValueType type, Point &p, H d) {
			p.push_back(makeTypedValue(type, d).value);
		}
	};





	class RecordTypedValue;

	template<typename V>
	inline RecordTypedValue makeTypedValue(RecordValueType t, V value);

	struct RecordTypedValue {
		RecordValue value;
		RecordValueType type;

		RecordTypedValue() = default;
		RecordTypedValue(RecordValueType t, RecordValue v) : value(v), type(t) {}

		inline static RecordTypedValue maxValue(RecordValueType t) {
			switch (t) {
				case RecordValueType::d:
					return makeTypedValue(t, INFINITY);
			}
		}

		inline static RecordTypedValue minValue(RecordValueType t) {
			switch (t) {
				case RecordValueType::d:
					return makeTypedValue(t, -INFINITY);
			}
		}

		inline static RecordTypedValue nullValue(RecordValueType t) {
			switch (t) {
				case RecordValueType::d:
					return makeTypedValue(t, 0);
			}
		}

		bool operator>(const RecordTypedValue &o) const {
			switch (type) {
				case RecordValueType::d:
					return value.d > o.value.d;
			}
		}

		bool operator<(const RecordTypedValue &o) const { return (o > *this); }

		double asDouble() const {
			switch (type) {
				case RecordValueType::d:
					return value.d;
			}
		}

		RecordTypedValue operator+(const RecordTypedValue &o) {
			switch (type) {
				case RecordValueType::d:
					return makeTypedValue(type, value.d + o.value.d);
			}
		}
	};

	inline RecordTypedValue max(const RecordTypedValue &a, const RecordTypedValue &b) {
		if (a > b) {
			return a;
		} else {
			return b;
		}
	}

	inline RecordTypedValue min(const RecordTypedValue &a, const RecordTypedValue &b) {
		if (a < b) {
			return a;
		} else {
			return b;
		}
	}


	template<typename V>
	inline RecordTypedValue makeTypedValue(RecordValueType t, V value) {
		RecordValue v;
		switch (t) {
			case RecordValueType::d: v.d = value; break;
		}
		return RecordTypedValue(t, v);
	}

}
