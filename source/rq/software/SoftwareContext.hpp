#pragma once

#include "../Context.hpp"
#include "SoftwareAnyTable.hpp"
#include <list>

namespace rq {

	class SoftwareContext : public Context {
	public:
		AnyTable *createTable(const TableSchema &) override;
		void destroyTable(AnyTable *table) override;
	private:
		std::list<SoftwareAnyTable> tables;
	};

}
