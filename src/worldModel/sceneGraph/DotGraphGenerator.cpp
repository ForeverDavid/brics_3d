/******************************************************************************
* BRICS_3D - 3D Perception and Modeling Library
* Copyright (c) 2011, GPS GmbH
*
* Author: Sebastian Blumenthal
*
*
* This software is published under a dual-license: GNU Lesser General Public
* License LGPL 2.1 and Modified BSD license. The dual-license implies that
* users of this code may choose which terms they prefer.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License LGPL and the BSD license for
* more details.
*
******************************************************************************/

#include "DotGraphGenerator.h"
#include "core/Logger.h"

namespace BRICS_3D {

namespace RSG {

DotGraphGenerator::DotGraphGenerator() {
	reset();
}

DotGraphGenerator::~DotGraphGenerator() {
	reset();
}

void DotGraphGenerator::reset() {
	alreadyVisitedNodes.clear();
	nodes.clear();
}

void DotGraphGenerator::visit(Node* node){
	assert (node != 0);

	vector<Node*>::iterator visitedNodes;
	visitedNodes = find (alreadyVisitedNodes.begin(), alreadyVisitedNodes.end(), node);

	if (visitedNodes == alreadyVisitedNodes.end()) { // if not in list: insert and handle node
		doHandleNode(node);
		alreadyVisitedNodes.push_back(node);
	}
}

void DotGraphGenerator::visit(Group* node){
	assert (node !=0);
	doHandleEdges(node); //all edges to children will be processed
	this->visit(dynamic_cast<Node*>(node)); //and feed forward to be handled as node
}

void DotGraphGenerator::visit(Transform* node){
	this->visit(dynamic_cast<Group*>(node)); //feed forward to be handled as group TODO a diffrent handling?
}

void DotGraphGenerator::visit(GeometricNode* node){
	this->visit(dynamic_cast<Node*>(node)); //just feed forward to be handled as node
}

void DotGraphGenerator::doHandleNode(Node* node) {
	assert (node !=0);
	LOG(DEBUG) << "Adding node " << node->getId() << " to dot graph.";

	//Each node should add a line like this: 2 [label = "ID [2]\n(name = point_cloud_1)\n"];

	std::stringstream aggregatedAttributes;
	vector<Attribute> attributeList = node->getAttributes();
	for (unsigned int i = 0; i < static_cast<unsigned int>(attributeList.size()); ++i) {
		aggregatedAttributes << attributeList[i];
		aggregatedAttributes << "\\n";
	}

//	nodes << node->getId() << ";" << std::endl;
	nodes << node->getId();
    nodes << " [label = \"";
    nodes << "ID [" << node->getId() << "]\\n";
    nodes << aggregatedAttributes.str();
    nodes << "\"]";
	nodes << ";" << std::endl;
}

void DotGraphGenerator::doHandleEdges(Group* node){
	assert (node !=0);

	// An egde has the following format: 3 -> 4;

	for (unsigned int childIndex = 0; childIndex < node->getNumberOfChildren(); ++childIndex) {
		edges << node->getId() << " -> " << (*node->getChild(childIndex)).getId() << ";" << std::endl;
	}
}

std::string DotGraphGenerator::getDotGraph() {
	std::string result;

	/* This e.g. would be a valid dot file:
	 * digraph {
	 *	1 [label = "ID [1]\n"];
	 *	2 [label = "ID [2]\n(name = point_cloud_1)\n"];
	 *	3 [label = "ID [3]\n"];
	 *	4 [label = "ID [4]\n(name = point_cloud_2)\n"];
	 *	5 [label = "ID [5]\n(name = point_cloud_reduced)\n(filterType = octree)\n"];
	 *	6 [label = "ID [6]\n(name = point_cloud_box_roi)\n"];
	 *	7 [label = "ID [7]\n"];
	 *	9 [label = "ID [9]\n"];
	 *	10 [label = "ID [10]\n"];
	 *	11 [label = "ID [11]\n(name = mesh_1)\n"];
	 *
	 *	1 -> 2;
	 *	1 -> 3;
	 *	1 -> 5;
	 *	1 -> 6;
	 *	1 -> 7;
	 *	1 -> 11;
	 *	3 -> 4;
	 *	7 -> 9;
	 *	9 -> 10;
	 *
	 *	}
	 */

	dotGraph << "digraph {" << std::endl;
	dotGraph << nodes.str();
	dotGraph << std::endl;
	dotGraph << edges.str();
	dotGraph << std::endl;
	dotGraph << "}" << std::endl;
	result = dotGraph.str();
	return result;
}

}

}

/* EOF */