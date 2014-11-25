//验证类实例化 test t
//g++ -g test_class.cpp  -o test_class
#include <iostream>
using namespace std;


class test
{
public:
	test()
	{
		cout << "test" << endl;
	}
	~test()
	{
		cout << "test destruct" << endl;
	}
};

int  main()
{

	test *t = new test();
	return 0;

}
