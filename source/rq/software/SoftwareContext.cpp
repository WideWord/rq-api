#include "SoftwareContext.hpp"

namespace rq {

	AnyTable* SoftwareContext::createTable(const TableSchema &s) {
		tables.emplace_back(s);
		return &(tables.back());
	}

	void SoftwareContext::destroyTable(AnyTable *table) {
		for (auto it = tables.begin(), itend = tables.end(); it != itend; ++it) {
			if (&(*it) == table) {
				tables.erase(it);
				return;
			}
		}
	}

}
