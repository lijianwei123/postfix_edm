#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

#define LOG_FILE_NAME "./log/client.log"

int main()
{
	char *clone1 = strdup(LOG_FILE_NAME);
	char *clone2 = strdup(LOG_FILE_NAME);

	char *p1 = strrchr(clone1, '.');
	char *p2 = strrchr(clone2, '.');
	*p1 = '\0';	

	char new_log_name[200] = {0};
	
	sprintf(new_log_name, "%s_%d.%s", clone1, 10, ++p2);

	cout << new_log_name << endl;
	
	free(clone1);
	free(clone2);	

	return 0;

}
