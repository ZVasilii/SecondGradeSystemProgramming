/*
Program that writes pid and ppid into stdout
 */
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
	printf("PID: %lld\n", (long long) getpid);
	printf("PPID: %lld\n", (long long) getppid);

	return 0;
}