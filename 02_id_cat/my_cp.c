/*
  Program that copy one file onto another directory (support console arguments)
*/
#include <stdio.h>
#include <unistd.h>
#include <errno.h> 
#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>

const size_t BUFFSIZE = 4096;
const int PERMISS = 0666;

//Flags -v -i -p -f (read man foe more information)
char verbose_flag = 0;
char interact_flag = 0;
char preserve_flag = 0;
char force_flag = 0;

void error(int source_fd, int dest_fd, char* buf);

size_t read_file(int fd, char** buf, size_t* cur_buf_size);

size_t write_to_file(int fd, const char*  buf, size_t count);

int force_open(const char* path, int flags);

int open_file(const char* path, int flags);

int close_file(int fd);

//Function that reads arguments from command line
int read_flags(int argc, char **argv);

int main(int argc, char** argv)
{
  if (argc < 3)
  {
    printf("Too few arguments. Expected: mycp <src> <dest> [parameters]\n");
    exit(-1);
  }

   if (argc > 3)
    read_flags(argc, argv);
  
  // Arguments
  int source_flags  = O_RDONLY;
  int dest_flags = O_WRONLY | O_CREAT;

  int source_fd  = 0;
  int dest_fd = 0;
  
  size_t read_symbols = 0;
  char*   buf          = NULL;
  size_t  buf_size     = 0;
  
  //Prompt before owerwriting
  if (interact_flag && (access(argv[2], F_OK) != -1))
  {
    printf("mycp: overwrite '%s'?\n Choose 'y' or 'n' ", argv[2]);
    char answ = 'n';
    scanf("%c", &answ);

    if (answ == 'n')
      exit(0);
  }

  // open files
  if ((source_fd = open_file(argv[1], source_flags)) < 0)
    error(source_fd, dest_fd, buf);

  if ((dest_fd = open_file(argv[2], dest_flags)) < 0)
    //Force mode
    if (force_flag)                                             
      if ((dest_fd = force_open(argv[2], dest_flags)) < 0)
         error(source_fd, dest_fd, buf);

  // Read and write (main work here)
  read_symbols = read_file(source_fd, &buf, &buf_size);
  write_to_file(dest_fd, buf, (size_t)read_symbols);

  
  //Closing everything
  close_file(source_fd);
  close_file(dest_fd);
  free(buf);

  //Verbose mode
  if (verbose_flag)                                         
    printf("\'%s\' -> \'%s\'\n", argv[1], argv[2]);

  return 0;
}

void error(int source_fd, int dest_fd, char* buf)
{
  assert(buf);
  perror("Something went wrong, goodbye!\n");
  close_file(source_fd);
  close_file(dest_fd);
  free(buf);
  exit(-1);
}

size_t read_file(int fd, char** buf, size_t* cur_buf_size)
{
  assert(cur_buf_size); 
  assert(buf);
  assert(fd >= 0);

  size_t succ_read = 0;
  if (*cur_buf_size < BUFFSIZE)
  {
      *buf = (char*) realloc(*buf, BUFFSIZE);
      assert(buf && "Troubles with realloc");
      *cur_buf_size = BUFFSIZE;
  }

  while (1)
  {
    //Lack of memory operation
    if (succ_read != 0)
    {
      *buf = (char*) realloc (*buf, *cur_buf_size + BUFFSIZE);
      *cur_buf_size += BUFFSIZE;
      assert(buf && "Troubles with realloc");
    }

    // reading
    size_t status = read(fd, *buf + succ_read, BUFFSIZE);
    ///Error management (read)
    if (status < 1)                 
    {                               
      if (errno == EINTR)           
        continue;                  
      if (status == 0 && errno == 0)
        break;                      
                                      
      printf("Error: path=%d\n", fd);
      perror("my_read failed");     
      exit(-1);                  
    }                               
    
    succ_read += (size_t) status;
  }
    
  return (size_t) succ_read;
}

size_t write_to_file(int fd, const char* buf, size_t count)
{
  assert(buf);
  assert(fd >= 0);

  size_t succ_write = 0;
  while (succ_write < count)
  {
    size_t status = write(fd, buf + succ_write, count - succ_write);

     ///Error management (write)
    if (status < 1)               
    {                             
      if (!errno || errno == EINTR)
        continue;                  
                                  
      printf("Error: path=%d\n", fd);
      perror("write_to_file failed");   
      return -1;                   
    }                              
                                   
    succ_write += (size_t)status;
  }

  return 0;
}


//Force opening
int force_open(const char* path, int flags)
{
  assert(path);

  //Deleting file
  if (unlink(path) < 0)
  {
    printf("Error: path=%s\n", path);
    perror("force_open failed");         
    exit(-1); 
  }
  
  //Creating file
  return open_file(path, flags | O_CREAT);
}

int open_file(const char* path, int flags)
{
  assert(path && "Open_file");

  int fd = 0;
  while (1)
  {
    fd = open(path, flags, PERMISS); 
    //Error management (open)
    if (fd < 0)                         
    {                                   
      if (errno == EINTR)               
        continue;                        
                                        
      printf("Error: path=%s\n", path);
      perror("open_file failed");        
      exit(-1);                     
    }                                 
    break;
  }

  return fd;
}


int close_file(int fd)
{
  assert(fd >= 0);
  
  int status = 0;
  //Error management (closing)
  while (1)
  {
    status = close(fd);
    
    if (status < 0)                 
    {                              
      if (errno == EINTR)          
        continue;                        
                                    
      printf("Error: path=%d\n", fd);
      perror("close_file failed");   
      exit(-1);               
    }                              
    break;
  }

  return fd;
}

int read_flags(int argc, char **argv)
{
  optind = 3;    //Start with 3 argument
  int ret_arg = 0;
  int opt_index = 0;
  static struct option long_options[] =
        {
          {"verbose",     no_argument, 0, 'v'},
          {"force",       no_argument, 0, 'f'},
          {"interactive", no_argument, 0, 'i'},
          {"p",           no_argument, 0, 'p'},
          {0, 0, 0, 0}
        };

  while (1)
  {
    ret_arg = getopt_long(argc, argv, "vfip",
                    long_options, &opt_index);

    //End of argv line
    if (ret_arg == -1)
      break;

    switch (ret_arg)
    {
      case 'v':
        verbose_flag = 1;
        break;

      case 'i':
        interact_flag = 1;
        break;

      case 'p':
        preserve_flag = 1;
        break;

      case 'f':
        force_flag = 1;
        break;

      default:
        printf("Unknown argument: %c\n", ret_arg);
        exit(-1);
    }
  }

  return 0;
}

