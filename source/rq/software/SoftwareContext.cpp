#include "SoftwareContext.hpp"

namespace rq {

	Table* SoftwareContext::createTable(const TableSchema &s) {
		tables.emplace_back(s);
		return &(tables.back());
	}

	void SoftwareContext::destroyTable(Table *table) {
		for (auto it = tables.begin(), itend = tables.end(); it != itend; ++it) {
			if (&(*it) == table) {
				tables.erase(it);
				return;
			}
		}
	}

}
