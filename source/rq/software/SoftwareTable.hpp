#pragma once

#include "../Table.hpp"
#include "../Record.hpp"
#include "../Bounds.hpp"
#include <unordered_map>

namespace rq {

	class TableSchema;

	class SoftwareTable : public Table {
	public:
		SoftwareTable(const TableSchema &s);

		RecordID nextID() override;
		void set(const Record &record) override;
		void remove(RecordID id) override;
		QueryResult query(const Query &) override;
	private:
		struct Node {
			bool isLeaf = false;
			std::vector<Node*> children;
			std::vector<Record> records;
			Bounds bounds;
			double overlapping = 0;
			Node *parent = nullptr;

			Node() = default;
			~Node() {
				for (auto &child : children) {
					delete child;
				}
			}

			Node *createChild() {
				auto child = new Node;
				children.push_back(child);
				child->parent = this;
				return child;
			}
		};
		Node root;
		RecordID idCounter;
		std::unordered_map<RecordID, Node*> recordsParents;

		Node *getNodeToInsert(const Record &record, Node *current);
		void splitNodeIfNeeded(Node *node);
		void recalculateNodeBounds(Node *node, bool recrUpdateTop);

		void executeQuery(const Query &query, Node *node, QueryResult &result);
	};

}
