#pragma once

namespace rq {

	class Table;
	class TableSchema;

	class Context {
	public:
		virtual ~Context();
		virtual Table *createTable(const TableSchema &) = 0;
		virtual void destroyTable(Table *table) = 0;
	};

}
