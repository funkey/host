#include <util/assert.h>
#include "Skeleton.h"

Skeleton::Skeleton() {

	createGraph();
}

Skeleton::Skeleton(const ExplicitVolume<unsigned char>& skeleton) {

	createGraph();

	ExplicitVolume<unsigned char> temp = skeleton;
	createFromVolume(temp);
}

Skeleton::Skeleton(ExplicitVolume<unsigned char>&& skeleton) {

	createGraph();
	createFromVolume(skeleton);
}

Skeleton::Skeleton(Skeleton&& other) :
	_graph(other._graph),
	_positions(other._positions),
	_segments(other._segments) {

	other._graph     = 0;
	other._positions = 0;
	other._segments  = 0;
}

Skeleton::Skeleton(const Skeleton& other) {

	createGraph();
	copyGraph(other);
}

Skeleton&
Skeleton::operator=(const Skeleton& other) {

	deleteGraph();
	createGraph();
	copyGraph(other);

	return *this;
}

Skeleton::~Skeleton() {

	deleteGraph();
}

void
Skeleton::createGraph() {

	_graph     = new Graph();
	_positions = new Positions(*_graph);
	_segments  = new Segments(*_graph);
}

void
Skeleton::copyGraph(const Skeleton& other) {

	lemon::GraphCopy<Graph, Graph> copy(*other._graph, *_graph);

	copy.nodeMap(*_positions, *other._positions);
	copy.edgeMap(*_segments, *other._segments);
	copy.run();
}

void
Skeleton::deleteGraph() {

	if (_segments)
		delete _segments;
	if (_positions)
		delete _positions;
	if (_graph)
		delete _graph;
}

void
Skeleton::createFromVolume(ExplicitVolume<unsigned char>& skeleton) {

	setBoundingBox(skeleton.getBoundingBox());

	vigra::Shape3 root = findFirstNode(skeleton);

	ExplicitVolume<bool> visited = skeleton;
	visited.data() = false;

	traverse(root, skeleton, visited);
}

vigra::Shape3
Skeleton::findFirstNode(ExplicitVolume<unsigned char>& skeleton) {

	vigra::MultiCoordinateIterator<3> i(skeleton.data().shape());

	for (; i != i.getEndIterator(); i++)
		if (skeleton[i] > 0 && numNeighbors(i, skeleton) != 2)
			return i;

	UTIL_THROW_EXCEPTION(
			InvalidSkeleton,
			"skeleton image does not contain a valid starting point");
}

int
Skeleton::numNeighbors(const vigra::Shape3& pos, ExplicitVolume<unsigned char>& skeleton) {

	int num = 0;

	int sx = (pos[0] == 0 ? 0 : -1);
	int sy = (pos[1] == 0 ? 0 : -1);
	int sz = (pos[2] == 0 ? 0 : -1);
	int ex = (pos[0] == skeleton.width()  - 1 ? 0 : 1);
	int ey = (pos[1] == skeleton.height() - 1 ? 0 : 1);
	int ez = (pos[2] == skeleton.depth()  - 1 ? 0 : 1);

	for (int dz = sz; dz <= ez; dz++)
	for (int dy = sy; dy <= ey; dy++)
	for (int dx = sx; dx <= ex; dx++) {

		if (skeleton(pos[0] + dx, pos[1] + dy, pos[2] + dz) > 0)
			num++;
	}

	if (skeleton[pos] > 0)
		num--;

	return num;
}

void
Skeleton::traverse(vigra::Shape3 pos, ExplicitVolume<unsigned char>& skeleton, ExplicitVolume<bool>& visited) {

	visited[pos] = true;

	float x, y, z;
	skeleton.getRealLocation(pos[0], pos[1], pos[2], x, y, z);
	Position realPos(x, y, z);

	int neighbors = numNeighbors(pos, skeleton);
	bool isNode = (neighbors != 2);

	if (isNode)
		openNode(realPos);
	else
		extendEdge(realPos);

	int sx = (pos[0] == 0 ? 0 : -1);
	int sy = (pos[1] == 0 ? 0 : -1);
	int sz = (pos[2] == 0 ? 0 : -1);
	int ex = (pos[0] == skeleton.width()  - 1 ? 0 : 1);
	int ey = (pos[1] == skeleton.height() - 1 ? 0 : 1);
	int ez = (pos[2] == skeleton.depth()  - 1 ? 0 : 1);

	// as soon as 'neighbors' is negative, we know that there are no more 
	// neighbors left to test (0 is not sufficient, since we count ourselves as 
	// well)
	for (int dz = sz; dz <= ez && neighbors >= 0; dz++)
	for (int dy = sy; dy <= ey && neighbors >= 0; dy++)
	for (int dx = sx; dx <= ex && neighbors >= 0; dx++) {

		vigra::Shape3 p = pos + vigra::Shape3(dx, dy, dz);

		if (skeleton[p] > 0) {

			neighbors--;

			if (!visited[p])
				traverse(p, skeleton, visited);
		}
	}

	if (isNode)
		closeNode(realPos);
}

void
Skeleton::openNode(Position pos) {

	Node node = _graph->addNode();

	if (_currentSegment.size() > 0) {

		Node prev = _currentPath.top();
		Edge edge = _graph->addEdge(prev, node);

		(*_segments)[edge] = _currentSegment;
		_currentSegment.clear();
	}

	(*_positions)[node] = pos;
	_currentPath.push(node);
}

void
Skeleton::extendEdge(Position pos) {

	_currentSegment.push_back(pos);
}

void
Skeleton::closeNode(Position) {

	_currentPath.pop();
}
