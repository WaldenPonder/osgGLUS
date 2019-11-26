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

	bool operator==(const Point& o)
	{
		return o.x == x && o.y == y && o.z == z;
	}

	bool operator!=(const Point& o)
	{
		return !operator==(o);
	}

	float get(int level) const
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
		left	  = POINTS(origin.begin(), origin.begin() + mid);
		Node* ln  = new Node;
		node->l	  = ln;
		ln->p	  = node;
		ln->level = plane + 1;

		right	  = POINTS(origin.begin() + mid + 1, origin.end());
		Node* rn  = new Node;
		node->r	  = rn;
		rn->p	  = node;
		rn->level = plane + 1;
	}
	else if (origin.size() == 2)
	{
		left	  = POINTS(origin.begin(), origin.begin() + mid);
		Node* ln  = new Node;
		node->l	  = ln;
		ln->p	  = node;
		ln->level = plane + 1;
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

//--------------------------------------------------search
Node* search(Node* node, const Point& pt)
{
	if (node->val == pt) return node;

	float val	 = node->val.get(node->level);
	float target = pt.get(node->level);

	if (val == target)
	{
		return node;
	}
	else if (val > target)
	{
		return search(node->l, pt);
	}
	else
	{
		return search(node->r, pt);
	}
}

//--------------------------------------------------search nearest
void search_nearest(Node* node, const Point& pt)
{
	Node* n = search(node, pt);



}

int main()
{
	PTs.push_back(Point(7, 2));
	PTs.push_back(Point(5, 4));
	PTs.push_back(Point(2, 3));
	PTs.push_back(Point(4, 7));
	PTs.push_back(Point(8, 1));
	PTs.push_back(Point(9, 6));
	PTs.push_back(Point(1, 9));

	Node* root	= new Node;
	root->level = 0;
	build(PTs, 0, root);

	getchar();
}