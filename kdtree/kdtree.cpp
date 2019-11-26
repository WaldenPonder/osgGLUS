#include "pch.h"
#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

//https://blog.csdn.net/ye1215172385/article/details/80214776

struct Point
{
	float x, y, z;

	Point() {}
	Point(float x_, float y_, float z_ = 0) : x(x_), y(y_), z(z_)
	{
	}

	bool operator==(Point& o)
	{
		return o.x == x && o.y == y && o.z == z;
	}

	bool operator!=(Point& o)
	{
		return !operator==(o);
	}

	float get(int level)
	{
		if (level % 2 == 0)
			return x;
		else
			return y;
	}
};

using POINTS = vector<Point>;
POINTS PTs;

struct Node
{
	Node() {}

	Point val;
	int	  level;

	Node* l = nullptr;
	Node* r = nullptr;
	Node* p = nullptr;
};

//--------------------------------------------------split
void split(POINTS& origin, int plane, Node* node, POINTS& left, POINTS& right)
{
	//sort(origin.begin(), origin.end(), [=](auto& p1, auto& p2) { p1.get(plane) < p2.get(plane); });

	for (int i = 0; i < origin.size(); i++)
	{
		for (int j = i + 1; j < origin.size(); j++)
		{
			if (origin[j].get(plane) < origin[i].get(plane))
			{
				std::swap(origin[i], origin[j]);
			}
		}
	}

	int mid	  = origin.size() / 2;
	node->val = origin[mid];

	if (origin.size() >= 3)
	{
		left		   = POINTS(origin.begin(), origin.begin() + mid);
		node->l		   = new Node;
		node->l->p	   = node;
		node->l->level = plane + 1;

		right		   = POINTS(origin.begin() + mid + 1, origin.end());
		node->r		   = new Node;
		node->r->p	   = node;
		node->r->level = plane + 1;
	}
	else if (origin.size() == 2)
	{
		left		   = POINTS(origin.begin(), origin.begin() + mid);
		node->l		   = new Node;
		node->l->p	   = node;
		node->l->level = plane + 1;
	}
}

//--------------------------------------------------build
void build(POINTS& pts, int plane, Node* node)
{
	if (pts.size() == 0)
		return;
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
	root->level = 0;
	build(PTs, 0, root);

	getchar();
}