#include "rq/software/SoftwareContext.hpp"

using namespace rq;

int main() {

	SoftwareContext ctx;
	TableSchema schema;
	schema.rTreeConfig.maxLeafRecordsNum = 2;
	schema.rTreeConfig.maxChildrenNum = 2;
	schema.rTreeConfig.minChildrenNum = 1;
	schema.types = { RecordValueType::d, RecordValueType::d };
	auto table = ctx.createTable(schema);

	table->insert(0.0, 1.0);
	table->insert(1.0, 0.0);
	table->insert(1.0, 1.0);
	table->insert(0.0, 0.0);

	return 0;
}