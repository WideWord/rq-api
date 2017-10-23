#pragma once

#include "Point.hpp"
#include <vector>

namespace rq {

	class AnyRecord {
	public:
		AnyRecord() = default;
		AnyRecord(const AnyRecord &o) : id(o.id), values(o.values) {}
		AnyRecord(RecordID id, const AnyPoint &values) : id(id), values(values) {}

		template<typename ...V>
		static AnyRecord create(uint32_t id, V... values) {
			AnyRecord r;
			r.id = id;
			unfoldValuesList(r.values, values...);
			return r;
		}

		RecordID getID() const { return id; }
		AnyRecordValue getValue(size_t i) const { return values[i]; }
		const AnyPoint &getValues() const { return values; }

		void setID(RecordID _id) { id = _id; }
		void setValue(size_t i, const AnyRecordValue &v) { values[i] = v; }
	private:
		RecordID id = 0;
		AnyPoint values;

		template<typename ...V>
		static void unfoldValuesList(std::vector<AnyRecordValue> &vec, double d, V... values) {
			AnyRecordValue anyValue;
			anyValue.d = d;
			vec.push_back(anyValue);
			unfoldValuesList(vec, values...);
		}

		template<typename ...V>
		static void unfoldValuesList(std::vector<AnyRecordValue> &vec, double d) {
			AnyRecordValue anyValue;
			anyValue.d = d;
			vec.push_back(anyValue);
		}
	};

}
