#include "pch.h"
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

struct Point
{
	float x, y, z;
	
	Point() {}
	Point(float x_, float y_, float z_ = 0) : x(x_), y(y_), z(z_)
	{
	}

	float get(int plane)
	{
		if (plane % 2 == 0) return x;
		else return y;
	}
};

using POINTS = vector<Point>;
POINTS PTs;

struct Node
{
	Node() {}

	Point val;
	int plane;

	Node* l = nullptr;
	Node* r = nullptr;
	Node* p = nullptr;
};

//--------------------------------------------------split
void split(POINTS& origin, int plane, Node* node, POINTS& left, POINTS& right)
{
	sort(origin.begin(), origin.end(), [=](Point& p1, Point& p2) { p1.get(plane) < p2.get(plane); });
	int mid   = origin.size() / 2;
	node->val = origin[mid];

	left	   = POINTS(origin.begin(), origin.end() + mid);
	node->l	= new Node;
	node->l->p = node;

	if (mid == 0) return;

	right	  = POINTS(origin.begin() + mid, origin.end());
	node->r	= new Node;
	node->r->p = node;
}

//--------------------------------------------------build
void build(POINTS& pts, int plane, Node* node)
{
	if (pts.size() == 0) return;
	else if (pts.size() == 1)
	{
		node->val = pts[0];
		return;
	}

	POINTS l, r;
	split(pts, plane, node, l, r);
	build(l, plane + 1, node->l);
	build(r, plane + 1, node->r);
}

int main()
{
	PTs.push_back(Point(7, 2));
	PTs.push_back(Point(5, 4));
	PTs.push_back(Point(2, 3));
	PTs.push_back(Point(4, 7));
	PTs.push_back(Point(8, 1));
	PTs.push_back(Point(9, 6));
	
	Node* root = new Node;
	root->plane = 0;

	build(PTs, 0, root);

	getchar();
}