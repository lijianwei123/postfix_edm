/*
 * test_mailq.cpp   计算mailq数量
 *
 *  Created on: 2014-11-26
 *      Author: Administrator
 */

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


using namespace std;



int64_t GetMailqNum()
{
	int ret = -1;
	long byteNum = 0;
	long mailqNum = 0;


	FILE *mailFp = NULL;
	const char *command = "/usr/bin/mailq 2>/dev/null |tail -n 1";
	char lastLineStr[1024] = {0};
	mailFp = popen(command, "r");
	if (mailFp == NULL) {
		return 0;
	}
	fgets(lastLineStr, 1024, mailFp);
	pclose(mailFp);

	if (strstr(lastLineStr, "empty")) {
		return 0;
	}

	ret = sscanf(lastLineStr, "-- %ld Kbytes in %ld Requests.", &byteNum, &mailqNum);
	if (ret == -1) {
		return 0;
	}

	return mailqNum;
}

int main()
{
	int64_t num =  GetMailqNum();
	cout << num << endl;


	return 0;

}
