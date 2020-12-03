// lc.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include "common.h"

/*
给定一个按照升序排列的整数数组 nums，和一个目标值 target。找出给定目标值在数组中的开始位置和结束位置。

你的算法时间复杂度必须是 O(log n) 级别。

如果数组中不存在目标值，返回 [-1, -1]。

示例 1:

输入: nums = [5,7,7,8,8,10], target = 8
输出: [3,4]
示例 2:

输入: nums = [5,7,7,8,8,10], target = 6
输出: [-1,-1]

Line 17: Char 6: fatal error: control may reach end of non-void function [-Wreturn-type]
	 }
	 ^
1 error generated.

*/
class Solution
{
 public:
	vector<int> searchRange(vector<int>& nums, int target)
	{
		vector<int> res(2, -1);
		if (nums.empty()) return res;
		int n = nums.size(), l = 0, r = n - 1;
		while (l < r)
		{
			int m = l + (r - l) / 2;
			if (nums[m] >= target)
				r = m;
			else
				l = m + 1;
		}
		if (nums[l] != target) return res;
		res[0] = l;
		r	   = n;
		while (l < r)
		{
			int m = l + (r - l) / 2;
			if (nums[m] <= target)
				l = m + 1;
			else
				r = m;
		}
		res[1] = l - 1;
		return res;
	}
};

int main()
{
	vector<int> v = { 1 };

	Solution	s;
	vector<int> ret = s.searchRange(v, 1);

	getchar();
}
