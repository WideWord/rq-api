#include <rq/Query.hpp>
#include "rq/software/SoftwareContext.hpp"
#include "rq/Bounds.hpp"
#include <iostream>

using namespace rq;

int main() {

	SoftwareContext ctx;

	TableSchema schema;
	schema.types = { RecordValueType::d, RecordValueType::d, RecordValueType::d };
	schema.rTreeConfig.maxLeafRecordsNum = 2;
	schema.rTreeConfig.maxChildrenNum = 2;
	schema.rTreeConfig.minChildrenNum = 1;

	auto table = ctx.createTable(schema);

	table->insert(0, 0, 0.5);
	table->insert(1, 0, 0.5);
	table->insert(1, 1, 0.5);
	table->insert(0, 1, 0.5);

	Query q;
	q.bounds.min = table->makePoint(-0.5, -0.5, -INFINITY);
	q.bounds.max = table->makePoint(1.5, 1.5, INFINITY);
	q.type = QueryType::sum;
	q.axis = 2;

	auto result = table->query(q);
	std::cout << result.value.asDouble();

	return 0;
}