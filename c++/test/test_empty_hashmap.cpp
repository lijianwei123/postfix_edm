//g++ -g test_empty_hashmap.cpp -Wno-deprecated  -o test_empty_hashmap
#include <iostream>
#include <ext/hash_map>

using namespace std;
using namespace __gnu_cxx;


int main()
{
	hash_map<int, int> m;
	m.insert(make_pair(1, 2));
	m.erase(1);	
	
	for (hash_map<int, int>::iterator mt = m.begin(); mt != m.end(); ++mt) {
		cout << "lijianwei" << endl;
	}
	return 0;
}
