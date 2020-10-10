#include <sys/types.h>
#include <unistd.h>
#include <string.h>

const char* buff = "Hello, world!\n";

int main()
{

	size_t size = strlen(buff);
	write(1, buff, size);
	return 0;
}