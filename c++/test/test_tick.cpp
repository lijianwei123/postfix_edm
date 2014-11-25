#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdlib.h>

using namespace std;

unsigned long long  get_tick_count()
{

	struct timeval tval;
	unsigned long long  ret_tick;

	gettimeofday(&tval, NULL);

	ret_tick = tval.tv_sec * 1000L + tval.tv_usec / 1000L;
	return ret_tick;
}




int main()
{
	unsigned long long tick = 0;
	tick = get_tick_count();

	cout << tick << endl;
	sleep(3);
	
	tick = get_tick_count();
	cout << tick << endl;



}
