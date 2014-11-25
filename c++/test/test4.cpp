#include <iostream>

using namespace std;

class A
{
public:
	virtual void test()=0;
};

class B : public A
{
public:
	virtual void test()
	{
		cout << "lijianwei" << endl;
	}

};

int main()
{
	B *b = new B();
	
	A *a = (A *)b;
	
	a->test();

	return 0;


}
