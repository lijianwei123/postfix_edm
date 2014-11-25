//g++ -g test_free_string.cpp -o test_free_string
//valgrind --tool=memcheck --leak-check=full ./test_free_string
//验证string(char *) 不会自动释放 char *内存
#include <iostream>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


using namespace std;

int main()
{
	char *p = (char *)calloc(10, 1);

	const char *name = "lijianwei";

	strncpy(p, name, 10);

	string *s = new string(p);
	delete s;
	return 1;
}
