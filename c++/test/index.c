#include <iostream>
#include <stdio.h>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

#include <vector>

using namespace std;
using namespace boost;


int main(int argc, char *argv[])
{
	shared_ptr<vector<int> > spv = make_shared<vector<int> >(10, 2);
	vector<int>::iterator it;
	for (it = spv->begin(); it != spv->end(); ++it) {
		cout << *it << endl;
	}	
	

	return 0;
}
