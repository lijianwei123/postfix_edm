/*
 *  g++ -I ./base -g  daemon.cpp ./base/libprename.a -o daemon
 *
 * 	daemon.cpp
 *
 *  Created on: 2014-11-4
 *  Author: lijianwei
 */
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "prename.h"

using namespace std;

extern char **environ;

int main(int argc, char **argv, char **envp)
{
	if (argc < 2) {
		cerr << "argc error!" << endl;
		return EXIT_FAILURE;
	}

	if (daemon(1, 1) < 0) {
		perror("daemon error");
		return EXIT_FAILURE;
	}

	char **p = NULL;

	int i = 0, j = 0;
	if (argc >= 3) {
		p = (char **)calloc(argc - 1, sizeof(char *));
		for (i = 2; i < argc; i ++) {
			p[j] = argv[i];
			j++;
		}
	} else {
		p = (char **)calloc(1, sizeof(char *));
		p[0] = NULL;
	}

	if (execvp(argv[1], p) < 0) {
		perror("execvp error");
		return EXIT_FAILURE;
	}

	//修改程序名称
	prename_setproctitle_init(argc, argv, envp);
	prename_setproctitle("%s", argv[0]);

	return EXIT_SUCCESS;
}

