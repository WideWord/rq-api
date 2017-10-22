#include "SoftwareAnyTable.hpp"
#include "../Query.hpp"
#include "../Record.hpp"
#include <cmath>

namespace rq {

	SoftwareAnyTable::SoftwareAnyTable(const TableSchema &s) : AnyTable(s) {
		root.isLeaf = true;
		idCounter = 1;
	}

	RecordID SoftwareAnyTable::nextID() {
		return idCounter++;
	}

	static bool isBoundsContainsRecord(const AnyBounds &bounds, const AnyRecord &record, const TableSchema &schema) {
		for (size_t i = 0, iend = schema.types.size(); i < iend; ++i) {
			auto type = schema.types[i];
			auto min = AnyRecordTypedValue(type, bounds.min[i]);
			auto max = AnyRecordTypedValue(type, bounds.max[i]);
			auto value = AnyRecordTypedValue(type, record.getValue(i));
			if (value < min || value > max) {
				return false;
			}
		}
		return true;
	}

	static bool isBoundsOverlaps(const AnyBounds &a, const AnyBounds &b, const TableSchema &schema) {
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

	static double getBoundsOverlappingSize(const AnyBounds &a, const AnyBounds &b, const TableSchema &schema) {
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

	static double getBoundsSize(const AnyBounds &bounds, const TableSchema &schema) {
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

	static void addRecordToBounds(const AnyBounds &bounds, const AnyRecord &record, AnyBounds &result, const TableSchema &schema) {
		for (size_t i = 0, iend = schema.types.size(); i < iend; ++i) {
			auto type = schema.types[i];
			auto maxBound = AnyRecordTypedValue(type, bounds.max[i]);
			auto minBound = AnyRecordTypedValue(type, bounds.min[i]);
			auto value = AnyRecordTypedValue(type, record.getValue(i));
			result.max[i] = max(maxBound, value).value;
			result.min[i] = min(minBound, value).value;
		}
	}

	static void mergeBounds(const AnyBounds &a, const AnyBounds &b, AnyBounds &result, const TableSchema &schema) {
		for (size_t i = 0, iend = schema.types.size(); i < iend; ++i) {
			auto type = schema.types[i];
			auto amin = AnyRecordTypedValue(type, a.min[i]);
			auto amax = AnyRecordTypedValue(type, a.max[i]);
			auto bmin = AnyRecordTypedValue(type, b.min[i]);
			auto bmax = AnyRecordTypedValue(type, b.max[i]);
			result.min[i] = min(amin, bmin).value;
			result.max[i] = max(amax, bmax).value;
		}
	}

	SoftwareAnyTable::Node *SoftwareAnyTable::getNodeToInsert(const AnyRecord &record, Node *_current) {
		const TableSchema &schema = getSchema();
		auto &current = *_current;
		if (current.isLeaf) {
			return _current;
		} else {
			if (current.children.front()->isLeaf) {

				double selectedOverlapping = -1;
				double selectedDeltaSize = -1;
				Node *selectedNode = nullptr;

				for (auto child : current.children) {
					if (isBoundsContainsRecord(child->bounds, record, schema)) {
						return child;
					} else {
						auto bounds = child->bounds;
						auto sizeBefore = getBoundsSize(bounds, schema);
						addRecordToBounds(bounds, record, bounds, schema);
						auto sizeAfter = getBoundsSize(bounds, schema);
						double deltaSize = sizeAfter - sizeBefore;
						double overlappingAfter = 0;
						for (auto anotherChild : current.children) {
							if (anotherChild == child) continue;
							overlappingAfter += getBoundsOverlappingSize(bounds, anotherChild->bounds, schema);
						}
						if (selectedNode == nullptr) {
							selectedOverlapping = overlappingAfter;
							selectedDeltaSize = deltaSize;
							selectedNode = child;
						} else if (selectedOverlapping - overlappingAfter < 0.01) {
							if (selectedDeltaSize > deltaSize) {
								selectedOverlapping = overlappingAfter;
								selectedDeltaSize = deltaSize;
								selectedNode = child;
							}
						} else if (selectedOverlapping > overlappingAfter) {
							selectedOverlapping = overlappingAfter;
							selectedDeltaSize = deltaSize;
							selectedNode = child;
						}
					}
				}

				if (selectedNode != nullptr) {
					return selectedNode;
				} else {
					return _current;
				}
			} else {
				double selectedDeltaSize = -1;
				Node *selectedNode = nullptr;

				for (Node *child : current.children) {
					if (isBoundsContainsRecord(child->bounds, record, schema)) {
						return getNodeToInsert(record, child);
					} else {
						auto bounds = child->bounds;
						auto sizeBefore = getBoundsSize(bounds, schema);
						addRecordToBounds(bounds, record, bounds, schema);
						auto sizeAfter = getBoundsSize(bounds, schema);
						double deltaSize = sizeAfter - sizeBefore;
						if (selectedNode == nullptr) {
							selectedDeltaSize = deltaSize;
							selectedNode = child;
						} else if (selectedDeltaSize > deltaSize) {
							selectedDeltaSize = deltaSize;
							selectedNode = child;
						}
					}
				}

				if (selectedNode != nullptr) {
					return getNodeToInsert(record, selectedNode);
				} else {
					return _current;
				}
			}
		}
	}

	void SoftwareAnyTable::splitNodeIfNeeded(Node *node) {
		const TableSchema &schema = getSchema();
		auto parent = node->parent;
		if (node->isLeaf && node->records.size() > schema.rTreeConfig.maxLeafRecordsNum) {
			double minSize = INFINITY;
			size_t selectedAxis = 0;
			std::vector<AnyRecord> selectedSortedRecords;
			size_t selectedHalfLen = 0;
			for (size_t axis = 0, axisNum = schema.types.size(); axis < axisNum; ++axis) {
				auto type = schema.types[axis];
				std::vector<AnyRecord> sortedRecords = node->records;
				auto comparator = [&](const AnyRecord &a, const AnyRecord &b) {
					return
							AnyRecordTypedValue(type, a.getValue(axis)) >
							AnyRecordTypedValue(type, b.getValue(axis));
				};
				std::sort(sortedRecords.begin(), sortedRecords.end(), comparator);
				auto boundsA = AnyBounds::fromRecord(sortedRecords.front());
				auto boundsB = AnyBounds::fromRecord(sortedRecords.back());
				size_t halfLen = sortedRecords.size() / 2;
				size_t r = 0;
				for (auto &record : sortedRecords) {
					auto &b = r < halfLen ? boundsA : boundsB;
					addRecordToBounds(b, record, b, schema);
					r += 1;
				}
				auto size = getBoundsSize(boundsA, schema) + getBoundsSize(boundsB, schema);
				if (size < minSize) {
					minSize = size;
					selectedAxis = axis;
					selectedSortedRecords = sortedRecords;
					selectedHalfLen = halfLen;
				}
			}

			bool addChildsToCurrentNode = false;
			if (parent == nullptr) {
				addChildsToCurrentNode = true;
			} else if (parent->children.size() >= schema.rTreeConfig.maxChildrenNum) {
				addChildsToCurrentNode = true;
			}

			if (addChildsToCurrentNode) {
				node->isLeaf = false;
				node->records.clear();
				auto nodeA = node->createChild();
				auto nodeB = node->createChild();
				nodeA->isLeaf = true;
				nodeB->isLeaf = true;

				size_t r = 0;
				for (auto &record : selectedSortedRecords) {
					auto &nodeToAdd = r < selectedHalfLen ? nodeA : nodeB;
					nodeToAdd->records.push_back(record);
					r += 1;
				}

				recalculateNodeBounds(nodeA, false);
				recalculateNodeBounds(nodeB, true);
			} else {
				node->records.clear();
				auto newNode = parent->createChild();

				size_t r = 0;
				for (auto &record : selectedSortedRecords) {
					auto &nodeToAdd = r < selectedHalfLen ? node : newNode;
					nodeToAdd->records.push_back(record);
					r += 1;
				}

				if (parent != nullptr) {
					splitNodeIfNeeded(parent);
				}

				recalculateNodeBounds(newNode, false);
				recalculateNodeBounds(node, true);
			}
		} else if (!node->isLeaf && node->children.size() > schema.rTreeConfig.maxChildrenNum) {
			double minSize = INFINITY;
			size_t selectedAxis = 0;
			std::vector<Node*> selectedSortedChilds;
			size_t selectedHalfLen = 0;
			for (size_t axis = 0, axisNum = schema.types.size(); axis < axisNum; ++axis) {
				auto type = schema.types[axis];
				std::vector<Node*> sortedChilds = node->children;
				auto comparator = [&](const Node *a, const Node *b) {
					return
							AnyRecordTypedValue(type, a->bounds.min[axis]) >
							AnyRecordTypedValue(type, b->bounds.min[axis]);
				};
				std::sort(sortedChilds.begin(), sortedChilds.end(), comparator);
				auto boundsA = sortedChilds.front()->bounds;
				auto boundsB = sortedChilds.back()->bounds;
				size_t halfLen = sortedChilds.size() / 2;
				size_t r = 0;
				for (auto child : sortedChilds) {
					auto &b = r < halfLen ? boundsA : boundsB;
					mergeBounds(b, child->bounds, b, schema);
					r += 1;
				}
				auto size = getBoundsSize(boundsA, schema) + getBoundsSize(boundsB, schema);
				if (size < minSize) {
					minSize = size;
					selectedAxis = axis;
					selectedSortedChilds = sortedChilds;
					selectedHalfLen = halfLen;
				}
			}


			auto nodeA = node->createChild();
			auto nodeB = node->createChild();

			size_t r = 0;
			for (auto &child : selectedSortedChilds) {
				auto &nodeToAdd = r < selectedHalfLen ? nodeA : nodeB;
				nodeToAdd->children.push_back(child);
				r += 1;
			}

			node->children = { nodeA, nodeB };

			recalculateNodeBounds(nodeA, false);
			recalculateNodeBounds(nodeB, true);
		} else {
			recalculateNodeBounds(node, true);
		}
	}

	void SoftwareAnyTable::recalculateNodeBounds(Node *node, bool recrUpdateTop) {
		const auto &schema = getSchema();
		if (node->isLeaf) {
			if (!node->records.empty()) {
				node->bounds = AnyBounds::fromRecord(node->records.front());
				for (auto &record : node->records) {
					addRecordToBounds(node->bounds, record, node->bounds, schema);
				}
			}
		} else {
			if (!node->children.empty()) {
				node->bounds = node->children.front()->bounds;
				for (auto child : node->children) {
					mergeBounds(node->bounds, child->bounds, node->bounds, schema);
					child->overlapping = 0;
					for (auto anotherChild : node->children) {
						if (child == anotherChild) continue;
						child->overlapping +=
							getBoundsOverlappingSize(child->bounds, anotherChild->bounds, schema);
					}
				}
			}
		}
		auto parent = node->parent;
		if (recrUpdateTop && parent != nullptr) {
			recalculateNodeBounds(parent, recrUpdateTop);
		}
	}

	void SoftwareAnyTable::set(const AnyRecord &record) {
		const auto &schema = getSchema();
		auto node = getNodeToInsert(record, &root);
		node->records.push_back(record);
		splitNodeIfNeeded(node);
	}

	void SoftwareAnyTable::remove(RecordID id) {

	}

	AnyQueryResult SoftwareAnyTable::query(const AnyQuery &) {
		return AnyQueryResult();
	}



}