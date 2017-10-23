#include "SoftwareTable.hpp"
#include "../Query.hpp"
#include "../Record.hpp"
#include <cmath>

namespace rq {

	SoftwareTable::SoftwareTable(const TableSchema &s) : Table(s) {
		root.isLeaf = true;
		idCounter = 1;
	}

	RecordID SoftwareTable::nextID() {
		return idCounter++;
	}

	SoftwareTable::Node *SoftwareTable::getNodeToInsert(const Record &record, Node *_current) {
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
					if (isBoundsContainsPoint(child->bounds, record.getValues(), schema)) {
						return child;
					} else {
						auto bounds = child->bounds;
						auto sizeBefore = getBoundsSize(bounds, schema);
						addRecordToBounds(bounds, record, schema);
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
					if (isBoundsContainsPoint(child->bounds, record.getValues(), schema)) {
						return getNodeToInsert(record, child);
					} else {
						auto bounds = child->bounds;
						auto sizeBefore = getBoundsSize(bounds, schema);
						addRecordToBounds(bounds, record, schema);
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

	void SoftwareTable::splitNodeIfNeeded(Node *node) {
		const TableSchema &schema = getSchema();
		auto parent = node->parent;
		if (node->isLeaf && node->records.size() > schema.rTreeConfig.maxLeafRecordsNum) {
			double minSize = INFINITY;
			size_t selectedAxis = 0;
			std::vector<Record> selectedSortedRecords;
			size_t selectedHalfLen = 0;
			for (size_t axis = 0, axisNum = schema.types.size(); axis < axisNum; ++axis) {
				auto type = schema.types[axis];
				std::vector<Record> sortedRecords = node->records;
				auto comparator = [&](const Record &a, const Record &b) {
					return
							RecordTypedValue(type, a.getValue(axis)) >
							RecordTypedValue(type, b.getValue(axis));
				};
				std::sort(sortedRecords.begin(), sortedRecords.end(), comparator);
				auto boundsA = Bounds::fromRecord(sortedRecords.front());
				auto boundsB = Bounds::fromRecord(sortedRecords.back());
				size_t halfLen = sortedRecords.size() / 2;
				size_t r = 0;
				for (auto &record : sortedRecords) {
					auto &b = r < halfLen ? boundsA : boundsB;
					addRecordToBounds(b, record, schema);
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
					recordsParents[record.getID()] = nodeToAdd;
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
					recordsParents[record.getID()] = nodeToAdd;
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
							RecordTypedValue(type, a->bounds.min[axis]) >
							RecordTypedValue(type, b->bounds.min[axis]);
				};
				std::sort(sortedChilds.begin(), sortedChilds.end(), comparator);
				auto boundsA = sortedChilds.front()->bounds;
				auto boundsB = sortedChilds.back()->bounds;
				size_t halfLen = sortedChilds.size() / 2;
				size_t r = 0;
				for (auto child : sortedChilds) {
					auto &b = r < halfLen ? boundsA : boundsB;
					mergeBounds(b, child->bounds, schema);
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

	void SoftwareTable::recalculateNodeBounds(Node *node, bool recrUpdateTop) {
		const auto &schema = getSchema();
		if (node->isLeaf) {
			if (!node->records.empty()) {
				node->bounds = Bounds::fromRecord(node->records.front());
				for (auto &record : node->records) {
					addRecordToBounds(node->bounds, record, schema);
				}
			}
		} else {
			if (!node->children.empty()) {
				node->bounds = node->children.front()->bounds;
				for (auto child : node->children) {
					mergeBounds(node->bounds, child->bounds, schema);
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

	void SoftwareTable::set(const Record &record) {
		const auto &schema = getSchema();
		auto node = getNodeToInsert(record, &root);
		node->records.push_back(record);
		recordsParents[record.getID()] = node;
		splitNodeIfNeeded(node);
	}

	void SoftwareTable::remove(RecordID id) {
		auto it = recordsParents.find(id);
		if (it != recordsParents.end()) {
			auto node = it->second;
			for (auto rit = node->records.begin(), ritend = node->records.end(); rit < ritend; ++rit) {
				if (rit->getID() == id) {
					node->records.erase(rit);
					break;
				}
			}
			recalculateNodeBounds(node, true);
			recordsParents.erase(it);
		}
	}

	QueryResult SoftwareTable::query(const Query &query) {
		QueryResult result;
		const auto &schema = getSchema();
		auto axis = query.axis;

		switch (query.type) {
			case QueryType::list: break;
			case QueryType::min:
				result.value = RecordTypedValue::minValue(schema.types[axis]);
				break;
			case QueryType::max:
				result.value = RecordTypedValue::maxValue(schema.types[axis]);
				break;
			case QueryType::sum:
				result.value = RecordTypedValue::nullValue(schema.types[axis]);
				break;
			case QueryType::average:
				result.value = RecordTypedValue::nullValue(schema.types[axis]);
				break;
		}

		executeQuery(query, &root, result);

		if (query.type == QueryType::average) {
			double avg = result.value.asDouble() / (double)result.recordsNum;
			result.value = makeTypedValue(RecordValueType::d, avg);
		}

		return result;
	}

	void SoftwareTable::executeQuery(const Query &query, Node *node, QueryResult &result) {
		const auto &schema = getSchema();

		bool contains = false, overlaps = false;
		checkBoundsDisposition(query.bounds, node->bounds, schema, contains, overlaps);

		if (!overlaps) {
			return;
		}

		if (node->isLeaf) {
			for (auto &record : node->records) {
				auto &point = record.getValues();

				if (!contains) {
					if (!isBoundsContainsPoint(query.bounds, point, schema)) continue;
				}

				switch (query.type) {
					case QueryType::list:
						result.records.push_back(record);
						break;
					case QueryType::min:
						result.value = min(result.value, RecordTypedValue(schema.types[query.axis], point[query.axis]));
						break;
					case QueryType::max:
						result.value = max(result.value, RecordTypedValue(schema.types[query.axis], point[query.axis]));
						break;
					case QueryType::sum:
						result.value = result.value + RecordTypedValue(schema.types[query.axis], point[query.axis]);
						break;
					case QueryType::average:
						result.value = result.value + RecordTypedValue(schema.types[query.axis], point[query.axis]);
						break;
				}

				result.recordsNum += 1;
			}
		} else {
			for (auto &child : node->children) {
				executeQuery(query, child, result);
			}
		}
	}

}
