#pragma once

#include "Point.hpp"

namespace rq {

	class AnyRecord;

	struct AnyBounds {
		AnyPoint min;
		AnyPoint max;

		static AnyBounds fromRecord(const AnyRecord &r);
	};

	bool isBoundsContainsPoint(const AnyBounds &bounds, const AnyPoint &point, const TableSchema &schema);
	bool isBoundsOverlaps(const AnyBounds &a, const AnyBounds &b, const TableSchema &schema);
	void checkBoundsDisposition(const AnyBounds &a, const AnyBounds &b, const TableSchema &schema, bool &outContains, bool &outOverlaps);
	double getBoundsOverlappingSize(const AnyBounds &a, const AnyBounds &b, const TableSchema &schema);
	double getBoundsSize(const AnyBounds &bounds, const TableSchema &schema);
	void addRecordToBounds(AnyBounds &bounds, const AnyRecord &record, const TableSchema &schema);
	void mergeBounds(AnyBounds &a, const AnyBounds &b, const TableSchema &schema);


}