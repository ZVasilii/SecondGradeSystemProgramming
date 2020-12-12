#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define CHK(cmd) \
  do { if (cmd < 0) {\
    perror(#cmd);\
    exit(EXIT_FAILURE);} \
  } while (0)

volatile int f;

void hand_0(int signal_num) { f++;}
void hand_1(int signal_num) { f++;}

int main()
{
    int fd[2];
    int fd1[2];
    CHK( pipe(fd) );
    CHK( pipe(fd1) );
    
    char child_str[] = "Hello, i'm child";
    char parent_str[] = "Hello, i'm parent";
    pid_t parent_pid = getpid();
    pid_t child_pid = fork();
    CHK( child_pid );
    
    // CHILD
    if (!child_pid) {
        sigset_t set;
        int sig;
        signal(SIGUSR1, &hand_0);
        //Close pipe
        close(fd[0]);
        sigemptyset(&set);
        sigaddset(&set, SIGUSR1);
        
        //WRITING
        size_t size = write(fd[1], child_str, strlen(child_str));
        if (size != strlen(child_str))
        { 
          perror("Something wrong with write\n");
          exit(EXIT_FAILURE);
        }
        close(fd[1]);

        sigwait(&set, &sig); 
        printf ("Parent had read\n");
        close(fd1[1]);

        //READING
        size = read(fd1[0], parent_str, strlen(parent_str)); 
        if (size < strlen(parent_str)) 
        { 
          perror("Something wrong with read");
          exit(EXIT_FAILURE);
        }

        printf("Read from parent: %s\n", parent_str);
        CHK(close(fd1[0]));
        
        kill(parent_pid, SIGUSR2);
        exit(EXIT_SUCCESS);
    }


    // PARENT
    if (child_pid > 0) {
        CHK(close(fd[1]));

        //READING
        size_t size = read(fd[0], child_str, 20);
        CHK(size); 
        printf("Read from child : %s\n", child_str);
        
        CHK(close(fd[0])); 
        kill(child_pid, SIGUSR1);
        sigset_t set1;
        
        int sig1;
        signal(SIGUSR2, &hand_1);
        CHK(close(fd1[0]));
        sigemptyset(&set1);
        sigaddset(&set1, SIGUSR2);

        //WRITING
        size = write(fd1[1], parent_str, strlen(parent_str)); 
        if (size != strlen(parent_str)) 
        {
          perror("Something wrong with write\n");
          exit(EXIT_FAILURE); 
        }
        CHK(close(fd1[1]));
        sigwait(&set1, &sig1);
        printf ("Child read from parent, goodbye!\n");
      }



    return (0);
}