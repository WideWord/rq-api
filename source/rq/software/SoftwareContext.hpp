#pragma once

#include "../Context.hpp"
#include "SoftwareTable.hpp"
#include <list>

namespace rq {

	class SoftwareContext : public Context {
	public:
		Table *createTable(const TableSchema &) override;
		void destroyTable(Table *table) override;
	private:
		std::list<SoftwareTable> tables;
	};

}
