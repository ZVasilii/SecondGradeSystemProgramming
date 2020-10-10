#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <assert.h>
#include <pwd.h>
#include <grp.h>

/// распечатки

const int max_groups = 100;
const int max_len = 100;


struct id
{
	unsigned int gid;
	unsigned int uid;
	unsigned int* list_groups;
	int groups_num;
	struct passwd* buffer_uid;
	struct group* buffer_group;

};


struct id* getid(struct id* tmp)
{
	assert(tmp != NULL);

	tmp -> uid = getuid();
	tmp -> gid = getgid();
	tmp -> buffer_uid = getpwuid(tmp -> uid);
	tmp -> buffer_group = getgrgid(tmp -> gid);
	tmp -> groups_num = getgroups(max_groups, tmp -> list_groups);
	return tmp;
}

void printing(struct id* tmp)
{
	assert(tmp != NULL);

	printf("uid=%u", tmp -> uid);
	printf("(%s) ", tmp -> buffer_uid -> pw_name);
	printf("gid=%u", tmp -> gid);
	printf("(%s) ", tmp -> buffer_group -> gr_name);
	printf("groups= ");
	printf("%u", tmp -> list_groups[tmp -> groups_num - 1]);
	printf("(%s), ", tmp -> buffer_group -> gr_name);
	for (int i = 0; i < (tmp -> groups_num - 1); i++)
	{
		printf("%u", tmp -> list_groups[i]);
		tmp -> buffer_group = getgrgid(tmp -> list_groups[i]);
		printf("(%s)", tmp -> buffer_group -> gr_name);
		if (i != (tmp -> groups_num - 2)) 
			printf(", ");

	}
	printf("\n");	
}

int main(int argc, char** argv)
{
	if (argc == 1) //without arguments
	{
		struct id myid;
		myid.list_groups = (unsigned int*) calloc (max_groups, sizeof(unsigned int));
		assert(myid.list_groups);
		myid.uid = 0;
		myid.gid = 0;
		myid.groups_num = 0; 

		getid(&myid);
		printing(&myid);

		free(myid.list_groups);
	}
	
	return 0;
}