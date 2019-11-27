#include "pch.h"
#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

//https://blog.csdn.net/ye1215172385/article/details/80214776

#define DIM 2

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

	inline float get(int level) const
	{
		int dim = level % DIM;

		if (dim == 0)
			return x;
		else if (dim == 1)
			return y;
		else
			return z;
	}

	inline float dis(const Point& pt) const
	{
		return sqrt(dis2(pt));
	}

	inline float dis2(const Point& pt) const
	{
		float v = pow((pt.x - x), 2) + pow((pt.y - y), 2) + pow((pt.z - z), 2);
		return v;
	}
};

using POINTS = vector<Point>;
POINTS PTs;

struct Node
{
	Node() {}

	inline float disPlane(const Point& pt) const
	{
		int dim = level % DIM;
		if (dim == 0)
		{
			return fabs(pt.x - val.x);
		}
		else if (dim == 1)
		{
			return fabs(pt.y - val.y);
		}
		else if (dim == 2)
		{
			return fabs(pt.z - val.z);
		}
	}

	inline Node* sibing() const
	{
		if (parent)
		{
			if (this == parent->l)
				return parent->r;
			else
				return parent->l;
		}

		return nullptr;
	}

	Point val;
	int	  level;

	Node* l		 = nullptr;
	Node* r		 = nullptr;
	Node* parent = nullptr;
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
		left	   = POINTS(origin.begin(), origin.begin() + mid);
		Node* ln   = new Node;
		node->l	   = ln;
		ln->parent = node;
		ln->level  = plane + 1;

		right	   = POINTS(origin.begin() + mid + 1, origin.end());
		Node* rn   = new Node;
		node->r	   = rn;
		rn->parent = node;
		rn->level  = plane + 1;
	}
	else if (origin.size() == 2)
	{
		left	   = POINTS(origin.begin(), origin.begin() + mid);
		Node* ln   = new Node;
		node->l	   = ln;
		ln->parent = node;
		ln->level  = plane + 1;
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
	if (!node) return nullptr;

	if (node->val == pt) return node;

	float val	 = node->val.get(node->level);
	float target = pt.get(node->level);

	if (val == target)
	{
		return node;
	}
	else if (val > target)
	{
		if (node->l)
			return search(node->l, pt);
	}
	else
	{
		if (node->r)
			return search(node->r, pt);
	}

	return node;
}

//--------------------------------------------------search nearest
void search_nearest(Node* n, const Point& pt, Point& ret)
{
	if (!n) return;

	float dis = n->val.dis(pt);

	if (dis == 0)
	{
		ret = pt;
	}	
	else if (dis < n->disPlane(pt) || n->sibing() == nullptr)	 //减枝， 不需要搜索兄弟节点
	{
		if (n->parent)
		{
			float dp = n->parent->val.dis(pt);
			if (dp < dis)
			{
				ret = n->parent->val;
			}

			search_nearest(n->parent, pt, ret);
		}
		else
		{
			if (n->val.dis(pt) < ret.dis(pt))
				ret = n->val;
		}
	}
	else if (n->sibing())
	{
		float dp = n->sibing()->val.dis(pt);
		if (dp < dis)
		{
			ret = n->sibing()->val;
			search_nearest(n->sibing(), pt, ret);
		}
		else
		{
			if (n->val.dis(pt) < ret.dis(pt))
				ret = n->val;
			search_nearest(n->parent, pt, ret);
		}		
	}
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

	//找到叶节点
	Point pt(7, 3.5);
	Node* n = search(root, pt);

	Point ret;
	search_nearest(n, pt, ret);

	getchar();
}