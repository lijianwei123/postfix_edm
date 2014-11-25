//map中如果存储的是类对象，实际上存储的是对象的副本， 原始对象可以释放内存,  在 erarse   clear 或者自动结束的时候，会调用类对象的析构函数
//如果存储的是指针，不能释放掉内存，需要自己  delete  然后  erarse 下
#include <iostream>
#include <map>
#include <vector>

using namespace std;


int main()
{

	map<string, string *> ss_map;
	ss_map.insert(make_pair(string("123"), new string("lijianwei")));
	map<string, string *>::iterator it;
	it = ss_map.find(string("123"));
	delete it->second;
	ss_map.erase(string("123"));	
	
	vector<string> a;
	a.push_back(string("123"));
	
	return 0;
}
