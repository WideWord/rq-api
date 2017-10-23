#pragma once

#include "../Table.hpp"
#include "../Record.hpp"
#include "../Bounds.hpp"
#include <unordered_map>

namespace rq {

	class TableSchema;

	class SoftwareAnyTable : public AnyTable {
	public:
		SoftwareAnyTable(const TableSchema &s);

		RecordID nextID() override;
		void set(const AnyRecord &record) override;
		void remove(RecordID id) override;
		AnyQueryResult query(const AnyQuery &) override;
	private:
		struct Node {
			bool isLeaf = false;
			std::vector<Node*> children;
			std::vector<AnyRecord> records;
			AnyBounds bounds;
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

		Node *getNodeToInsert(const AnyRecord &record, Node *current);
		void splitNodeIfNeeded(Node *node);
		void recalculateNodeBounds(Node *node, bool recrUpdateTop);

		void executeQuery(const AnyQuery &query, Node *node, AnyQueryResult &result);
	};

}
