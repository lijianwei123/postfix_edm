#include <iostream>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

//根据进程号获取父进程id
pid_t getParentProcessIdByChildId(pid_t childId)
{
	char filePath[100] = {0};
	char commandLine[100] = {0};
	FILE *fp = NULL;
	char buffer[100] = {0};

	sprintf(filePath, "/proc/%d/status", childId);

	//进程不存在
	if (access(filePath, F_OK))
			return -1;

	sprintf(commandLine, "ps -p %d -o ppid=", childId);
	fp = popen(commandLine, "r");
	if (fp == NULL)
		return -1;
	fgets(buffer, sizeof(buffer), fp);
	pclose(fp);
	return	atoi(buffer);
}


int main(int argc, char *argv[])
{
	printf("parent pid %d\n", getParentProcessIdByChildId(atoi(argv[1])));
	return 0;

}
