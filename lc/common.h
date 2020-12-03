#pragma once
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct TreeNode {
	int val;
	TreeNode *left;
	TreeNode *right;
	TreeNode(int x) : val(x), left(NULL), right(NULL) {}
};

TreeNode* getTreeNode(const std::string& str)
{
	std::string src(str);
//	src.erase(std::remove(src.begin(), src.end(), '['));
//	src.erase(std::remove(src.begin(), src.end(), ']'));

	std::istringstream stream(src);

	std::string field;
	std::vector<std::string> result;
	while (std::getline(stream, field, ','))
	{
		result.push_back(field);
	}

	TreeNode* root = new TreeNode(atoi(result[0].c_str()));
	vector<TreeNode*> vecs;
	vecs.push_back(root);

	for (int i = 1; i < result.size(); i++)
	{
		if (strstr(result[i].c_str(), "null") == nullptr)
		{
			TreeNode* n1 = new TreeNode(atoi(result[i].c_str()));

			if (i % 2 == 1) //left
			{
				vecs[(i - 1) / 2]->left = n1;
			}
			else
			{
				vecs[(i - 1) / 2]->right = n1;
			}
			vecs.push_back(n1);
		}
		else
		{
			vecs.push_back(nullptr);
		}
	}

	return root;
}

struct ListNode {
	int val;
	ListNode *next;
	ListNode(int x) : val(x), next(NULL) {}
};


void print(ListNode* l)
{
	while (l)
	{
		std::cout << l->val << "\t";
		l = l->next;
	}
	std::cout << std::endl;
}