//g++ -g -Wall test5.cpp -o test5
#include <iostream>

using namespace std;

typedef struct test
{
	char name[20];
} test_t;




int main()
{
	string name("lijianwei");
	cout << "name:" << name << endl;

	test_t t;

	return 0;
}
