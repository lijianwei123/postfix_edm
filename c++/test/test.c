#include <boost/shared_ptr.hpp>
#include <iostream>
#include <stdio.h>
#include <errno.h>

using namespace std;
using namespace boost;

int main(int argc, char **argv, char **envp)
{
	shared_ptr<string> ptr(new string("lijianwei"));	
	
	cout << EWOULDBLOCK << endl;	
	cout << EINPROGRESS << endl;
	return 0;

}
