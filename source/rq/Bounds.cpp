#include "Bounds.hpp"
#include "Record.hpp"

namespace rq {

	AnyBounds AnyBounds::fromRecord(const AnyRecord &record) {
		AnyBounds r;
		r.min = record.getValues();
		r.max = record.getValues();
		return r;
	}

	bool isBoundsContainsPoint(const AnyBounds &bounds, const AnyPoint &point, const TableSchema &schema) {
		for (size_t i = 0, iend = schema.types.size(); i < iend; ++i) {
			auto type = schema.types[i];
			auto min = AnyRecordTypedValue(type, bounds.min[i]);
			auto max = AnyRecordTypedValue(type, bounds.max[i]);
			auto value = AnyRecordTypedValue(type, point[i]);
			if (value < min || value > max) {
				return false;
			}
		}
		return true;
	}

	bool isBoundsOverlaps(const AnyBounds &a, const AnyBounds &b, const TableSchema &schema) {
		for (size_t i = 0, iend = schema.types.size(); i < iend; ++i) {
			auto type = schema.types[i];
			auto amin = AnyRecordTypedValue(type, a.min[i]);
			auto amax = AnyRecordTypedValue(type, a.max[i]);
			auto bmin = AnyRecordTypedValue(type, b.min[i]);
			auto bmax = AnyRecordTypedValue(type, b.max[i]);
			if (amin < bmin) {
				if (amax > bmin) {
					return true;
				}
			} else {
				if (bmax > amin) {
					return true;
				}
			}
		}
		return false;
	}

	void checkBoundsDisposition(const AnyBounds &a, const AnyBounds &b, const TableSchema &schema, bool &outContains, bool &outOverlaps) {
		outContains = true;
		outOverlaps = true;
		for (size_t i = 0, iend = schema.types.size(); i < iend; ++i) {
			auto type = schema.types[i];
			auto amin = AnyRecordTypedValue(type, a.min[i]);
			auto amax = AnyRecordTypedValue(type, a.max[i]);
			auto bmin = AnyRecordTypedValue(type, b.min[i]);
			auto bmax = AnyRecordTypedValue(type, b.max[i]);
			if (amin < bmin) {
				if (amax > bmin) {
					if (amax < bmax) {
						outContains = false;
					}
				} else {
					outContains = false;
					outOverlaps = false;
					return;
				}
			} else {
				if (bmax > amin) {
					if (bmax < amax) {
						outContains = false;
					}
				} else {
					outContains = false;
					outOverlaps = false;
					return;
				}
			}
		}
	}

	double getBoundsOverlappingSize(const AnyBounds &a, const AnyBounds &b, const TableSchema &schema) {
		double size = 1;
		for (size_t i = 0, iend = schema.types.size(); i < iend; ++i) {
			auto type = schema.types[i];
			auto amin = AnyRecordTypedValue(type, a.min[i]);
			auto amax = AnyRecordTypedValue(type, a.max[i]);
			auto bmin = AnyRecordTypedValue(type, b.min[i]);
			auto bmax = AnyRecordTypedValue(type, b.max[i]);
			double axisSize = 0;
			if (amin < bmin) {
				if (amax > bmin) {
					if (amax > bmax) {
						axisSize = bmax.asDouble() - bmin.asDouble();
					} else {
						axisSize = amax.asDouble() - bmin.asDouble();
					}
				} else {
					return 0;
				}
			} else {
				if (bmax > amin) {
					if (bmax > amax) {
						axisSize = amax.asDouble() - amin.asDouble();
					} else {
						axisSize = bmax.asDouble() - amin.asDouble();
					}
				} else {
					return 0;
				}
			}
			size *= axisSize;
		}
		return size;
	}

	double getBoundsSize(const AnyBounds &bounds, const TableSchema &schema) {
		double size = 1;
		for (size_t i = 0, iend = schema.types.size(); i < iend; ++i) {
			auto type = schema.types[i];
			auto maxBound = AnyRecordTypedValue(type, bounds.max[i]);
			auto minBound = AnyRecordTypedValue(type, bounds.min[i]);
			auto axisSize = maxBound.asDouble() - minBound.asDouble();
			size *= axisSize;
		}
		return size;
	}

	void addRecordToBounds(AnyBounds &bounds, const AnyRecord &record, const TableSchema &schema) {
		for (size_t i = 0, iend = schema.types.size(); i < iend; ++i) {
			auto type = schema.types[i];
			auto maxBound = AnyRecordTypedValue(type, bounds.max[i]);
			auto minBound = AnyRecordTypedValue(type, bounds.min[i]);
			auto value = AnyRecordTypedValue(type, record.getValue(i));
			bounds.max[i] = max(maxBound, value).value;
			bounds.min[i] = min(minBound, value).value;
		}
	}

	void mergeBounds(AnyBounds &a, const AnyBounds &b, const TableSchema &schema) {
		for (size_t i = 0, iend = schema.types.size(); i < iend; ++i) {
			auto type = schema.types[i];
			auto amin = AnyRecordTypedValue(type, a.min[i]);
			auto amax = AnyRecordTypedValue(type, a.max[i]);
			auto bmin = AnyRecordTypedValue(type, b.min[i]);
			auto bmax = AnyRecordTypedValue(type, b.max[i]);
			a.min[i] = min(amin, bmin).value;
			a.max[i] = max(amax, bmax).value;
		}
	}


}
