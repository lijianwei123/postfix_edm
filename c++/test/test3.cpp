#include <iostream>

using namespace std;


class test
{
public:
	test()
	{
		test::num = 1;
	}
	static int num;
	void t()
	{
		num = 2;	
	}	
};


int main()
{
	test t;
	t.t();
	cout << test::num << endl;
	return 0;


}
