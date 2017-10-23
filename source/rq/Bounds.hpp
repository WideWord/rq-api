#pragma once

#include "TableSchema.hpp"

namespace rq {

	class Record;

	struct Bounds {
		Point min;
		Point max;

		static Bounds fromRecord(const Record &r);
	};

	bool isBoundsContainsPoint(const Bounds &bounds, const Point &point, const TableSchema &schema);
	bool isBoundsOverlaps(const Bounds &a, const Bounds &b, const TableSchema &schema);
	void checkBoundsDisposition(const Bounds &a, const Bounds &b, const TableSchema &schema, bool &outContains, bool &outOverlaps);
	double getBoundsOverlappingSize(const Bounds &a, const Bounds &b, const TableSchema &schema);
	double getBoundsSize(const Bounds &bounds, const TableSchema &schema);
	void addRecordToBounds(Bounds &bounds, const Record &record, const TableSchema &schema);
	void mergeBounds(Bounds &a, const Bounds &b, const TableSchema &schema);


}