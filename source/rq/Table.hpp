#pragma once

#include <functional>
#include "TableSchema.hpp"
#include "Record.hpp"

namespace rq {

	class AnyQuery;
	class AnyQueryResult;

	class AnyTable {
	public:
		AnyTable(const TableSchema &s) : schema(s) {}

		virtual RecordID nextID() = 0;
		virtual void set(const AnyRecord &record) = 0;
		virtual void remove(RecordID id) = 0;
		virtual AnyQueryResult query(const AnyQuery &) = 0;

		const TableSchema &getSchema() const { return schema; }

		template<typename ...V>
		RecordID insert(V... values) {
			auto id = nextID();
			set(AnyRecord::create(id, values...));
			return id;
		}
	private:
		TableSchema schema;
	};

}
