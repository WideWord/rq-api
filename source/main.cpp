#include <rq/Query.hpp>
#include "rq/software/SoftwareContext.hpp"
#include "rq/Bounds.hpp"

using namespace rq;

int main() {

	SoftwareContext ctx;

	TableSchema schema;
	schema.types = { RecordValueType::d, RecordValueType::d };
	schema.rTreeConfig.maxLeafRecordsNum = 2;
	schema.rTreeConfig.maxChildrenNum = 2;
	schema.rTreeConfig.minChildrenNum = 1;

	auto table = ctx.createTable(schema);

	table->insert(0, 0);
	table->insert(1, 0);
	table->insert(1, 1);
	table->insert(0, 1);

	AnyQuery q;
	q.bounds.min = { typedValue(RecordValueType::d, 0.5).value, typedValue(RecordValueType::d, 0.5).value };
	q.bounds.max = { typedValue(RecordValueType::d, 1.5).value, typedValue(RecordValueType::d, 1.5).value };
	q.type = QueryType::list;


	auto result = table->query(q);

	return 0;
}