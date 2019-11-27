#include "pch.h"
#include <algorithm>
#include <iostream>
#include <vector>
#include <random>
#include <time.h>

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

	inline float splitLine() const
	{
		return val.get(level);
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
	sort(origin.begin(), origin.end(), [=](Point& p1, Point& p2) { return p1.get(plane) < p2.get(plane); });

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
vector<Node*> nodes;
void search_nearest(Node* n, const Point& target, Point& ret, float& DIS)
{
	if (!n) return;

	float dis = n->val.dis(target);

	if (dis <= DIS)
	{
		DIS = dis;
		ret = n->val;
	}
	nodes.push_back(n);

	float line = n->splitLine();

	if (target.get(n->level) + DIS < line)
	{
		search_nearest(n->l, target, ret, DIS);
	}
	else if (target.get(n->level) - DIS > line)
	{
		search_nearest(n->r, target, ret, DIS);
	}
	else
	{
		search_nearest(n->l, target, ret, DIS);
		search_nearest(n->r, target, ret, DIS);
	}
}

int main()
{
	std::default_random_engine		   eng(time(NULL));
	std::uniform_int_distribution<int> rand(0, 100000);

	for (int i = 0; i < 10000; i++)
	{
		float x = rand(eng);
		float y = rand(eng);

		PTs.push_back(Point(x, y));
	}

	Node* root	= new Node;
	root->level = 0;
	build(PTs, 0, root);

	POINTS pts;
	POINTS ret1, ret2;

	for (int i = 0; i < 3000; i++)
	{
		float x = rand(eng);
		float y = rand(eng);
		pts.push_back(Point(x, y));
	}
	vector<float> dis1(pts.size()), dis2(pts.size());

	//--------------------------------------------------TEST KD TREE
	auto t = clock();
	{
		int i = 0;
		for (const Point& pt : pts)
		{
			Node* n = search(root, pt);

			Point ret = n->val;
			float dis = ret.dis(pt);
			search_nearest(root, pt, ret, dis);
			dis1[i++] = dis;
			ret1.push_back(ret);
		}

		cout << "KD: " << (clock() - t) << endl;
	}

	//--------------------------------------------------NORMAL
	t = clock();
	{
		int j = 0;
		for (const Point& pt : pts)
		{
			Point ret;
			float dis = FLT_MAX;
			for (int i = 0; i < PTs.size(); i++)
			{
				float tmp = PTs[i].dis(pt);
				if (dis > tmp)
				{
					dis		= tmp;
					dis2[j] = dis;
					ret		= PTs[i];
				}
			}
			j++;
			ret2.push_back(ret);
		}

		cout << "NORMAL: " << (clock() - t) << endl;
	}

	for (size_t i = 0; i < ret1.size(); i++)
	{
		if (ret1[i] != ret2[i])
		{
			cout << "DIS: " << dis1[i] << "\t" << dis2[i] << "    ";
			cout << "TAR: " << pts[i].x << "\t" << pts[i].y << "  \t  RET1: ";
			cout << ret1[i].x << "\t" << ret1[i].y << "\t     RET2: " << ret2[i].x << "\t" << ret2[i].y << endl;
		}
	}

	cout << "FINISHED\n";
	getchar();
}