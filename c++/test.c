#include <boost/shared_ptr.hpp>
#include <iostream>

using namespace std;
using namespace boost;

int main(int argc, char **argv, char **envp)
{
	shared_ptr<string> ptr(new string("lijianwei"));	
	
	return 0;

}
