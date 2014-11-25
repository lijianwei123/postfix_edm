//验证cin需要先申请内存
#include <iostream>
#include <stdio.h>

using namespace std;

int main()
{
	char name[200] = {0};
	cin >> name;
	printf("%s\n", name);
	return 0;

}
