#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

/**
 * Executes the command "cat scores | grep Lakers".  In this quick-and-dirty
 * implementation the parent doesn't wait for the child to finish and
 * so the command prompt may reappear before the child terminates.
 *
 */

int main(int argc, char **argv)
{

  if (argc != 2)
  {
    fprintf(stderr, "Usage: %s <grep_argument>\n", argv[0]);
    return 1;
  }
  int cat_to_grep[2];
  int grep_to_sort[2];
  pid_t pid;

  char *cat_args[] = {"cat", "scores", NULL};
  char *grep_args[] = {"grep", argv[1], NULL};
  char *sort_args[] = {"sort", NULL};

  if (pipe(cat_to_grep) == -1)
  {
    perror("Failed to create cat-to-grep pipe");
    return 1;
  }
  if (pipe(grep_to_sort) == -1)
  {
    perror("Failed to create grep-to-sort pipe");
    return 1;
  }

  // make a pipe (fds go in pipefd[0] and pipefd[1])

  pid = fork();
  
  if (pid < 0)
  {
    perror("Failed to create P2");
    return 1;
  }

  if (pid == 0)
    {
      // child gets here and handles "grep Villanova"

      // replace standard input with input part of pipe
      pid_t sort_pid = fork();
      
      if (sort_pid < 0)
      {
        perror("Failed to create P3");
        return 1;
      }
      if  (sort_pid == 0)
      {
        if (dup2(grep_to_sort[0],STDIN_FILENO) == -1)
        {
          perror("Failed to redirect sort input");
          return 1;
        }

        close(cat_to_grep[0]);
        close(cat_to_grep[1]);
        close(grep_to_sort[0]);
        close(grep_to_sort[1]);
        
        execvp("sort", sort_args);
        perror("Failed to execute sort");
        return 1;
      }
      else
      {
        if (dup2(cat_to_grep[0], STDIN_FILENO) == -1)
        {
          perror("Failed to redirect grep input");
          return 1;
        }

        if(dup2(grep_to_sort[1], STDOUT_FILENO) == -1)
        {
          perror("Failed to redirect grep output");
          return 1;
        }

        close(cat_to_grep[0]);
        close(cat_to_grep[1]);
        close(grep_to_sort[0]);
        close(grep_to_sort[1]);
        
        execvp("grep", grep_args);
        perror("Failed to execute grep");
        return 1;
      }

    }
  else
    {

     if (dup2(cat_to_grep[1], STDOUT_FILENO) == -1)
     {
       perror("Failed to redirect cat output");
       return 1;
     }

      close(cat_to_grep[0]);
      close(cat_to_grep[1]);
      close(grep_to_sort[0]);
      close(grep_to_sort[1]);
      
      execvp("cat", cat_args);
      perror("Failed to execute cat");
      return 1;
    }
    return 0;
}