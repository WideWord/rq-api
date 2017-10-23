#pragma once

#include <functional>
#include "TableSchema.hpp"
#include "Record.hpp"

namespace rq {

	class Query;
	class QueryResult;

	struct MaxValue {};
	struct MinValue {};
	struct NullValue {};

	class Table {
	public:
		Table(const TableSchema &s) : schema(s) {}

		virtual RecordID nextID() = 0;
		virtual void set(const Record &record) = 0;
		virtual void remove(RecordID id) = 0;
		virtual QueryResult query(const Query &) = 0;

		const TableSchema &getSchema() const { return schema; }

		template<typename ...V>
		RecordID insert(V... values) {
			auto id = nextID();
			auto point = makePoint(values...);
			set(Record(id, point));
			return id;
		}

		template<typename ...V> Point makePoint(V... values) {
			return schema.makePoint(values...);
		}
	private:
		TableSchema schema;



	};

}
