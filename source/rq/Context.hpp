#pragma once

namespace rq {

	class AnyTable;
	class TableSchema;

	class Context {
	public:
		virtual ~Context();
		virtual AnyTable *createTable(const TableSchema &) = 0;
		virtual void destroyTable(AnyTable *table) = 0;
	};

}
