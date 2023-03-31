#ifndef SILENCE_BEHAVIOR_TREE_BUILDER_H
#define SILENCE_BEHAVIOR_TREE_BUILDER_H

#include "behavior_tree.h"
#include "tree_node.h"
#include "leaf_node.h"
#include "decorator_node.h"
#include "composite_node.h"
#include <stack>

enum CurrentParent {
	COMPOSITE,
	DECORATOR
};

class BehaviorTreeBuilder {
private:
	std::shared_ptr<TreeNode> currentParent;
	std::stack<CurrentParent> treeHierarchy;
public:
	BehaviorTreeBuilder() {
		currentParent = nullptr;
	};

	/**
	 * create a leaf node and add it to the current parent.
	 *
	 * @tparam leafType  - class extending the LeafNode class
	 * @tparam Types
	 * @param arguments
	 * @return
	 */
	template<typename leafType, typename ... Types>
	BehaviorTreeBuilder &leaf(Types... arguments) {
		// create a new leaf node
		std::shared_ptr<LeafNode> new_leaf = std::make_shared<leafType>(arguments...);
		// set the leaf nodes parent
		new_leaf->set_parent(currentParent);

		if (treeHierarchy.top() == COMPOSITE) {
			// add the leaf node to the composite node
			std::static_pointer_cast<CompositeNode>(currentParent)->add_child(new_leaf);
		} else if (treeHierarchy.top() == DECORATOR) {
			// set the leaf node as the decorator_nodes child
			std::static_pointer_cast<DecoratorNode>(currentParent)->set_child(new_leaf);
		}

		return *this;
	};

	/**
	 * create a decorator node and add it to the current parent.
	 *
	 * @tparam compositeType  - class extending the CompositeNode class
	 * @tparam Types
	 * @param arguments
	 * @return
	 */
	template<class compositeType, typename ... Types>
	BehaviorTreeBuilder &composite(Types... arguments) {
		std::shared_ptr<CompositeNode> new_composite = std::make_shared<compositeType>(arguments...);
		new_composite->set_parent(currentParent);

		if (!treeHierarchy.empty() && treeHierarchy.top() == COMPOSITE) {
			std::static_pointer_cast<CompositeNode>(currentParent)->add_child(new_composite);
		} else if (!treeHierarchy.empty() && treeHierarchy.top() == DECORATOR) {
			std::static_pointer_cast<DecoratorNode>(currentParent)->set_child(new_composite);
		}

		currentParent = new_composite;
		treeHierarchy.emplace(COMPOSITE);

		return *this;
	};

	/**
	 * create a decorator node and add it to the current parent.
	 *
	 * @tparam decoratorType  - class extending the DecoratorNode class
	 * @tparam Types
	 * @param arguments
	 * @return
	 */
	template<class decoratorType, typename ... Types>
	BehaviorTreeBuilder &decorator(Types... arguments) {
		std::shared_ptr<DecoratorNode> new_decorator = std::make_shared<decoratorType>(arguments...);
		new_decorator->set_parent(currentParent);

		if (!treeHierarchy.empty() && treeHierarchy.top() == COMPOSITE) {
			std::static_pointer_cast<CompositeNode>(currentParent)->add_child(new_decorator);
		} else if (!treeHierarchy.empty() && treeHierarchy.top() == DECORATOR) {
			std::static_pointer_cast<DecoratorNode>(currentParent)->set_child(new_decorator);
		}

		currentParent = new_decorator;
		treeHierarchy.emplace(DECORATOR);

		return *this;
	};

	/**
	 * end the creation process of the builder
	 *
	 * @return
	 */
	BehaviorTreeBuilder &end() {
		if (currentParent->get_parent() != nullptr) {
			currentParent = currentParent->get_parent();
			treeHierarchy.pop();
		}

		return *this;
	}

	/**
	 * build the behavior tree
	 *
	 * @return
	 */
	std::shared_ptr<BehaviorTree> build() {
		return std::make_shared<BehaviorTree>(currentParent);
	}

};
#endif //SILENCE_BEHAVIOR_TREE_BUILDER_H
