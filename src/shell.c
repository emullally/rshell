#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>

int parse(char *cmdLine, char **argv);
void execCmd(char **argVec);

// Note on parse: now any char after '#' is considered a comment and ignored
// by the parse function.
// Note 2: rather than having a pointer to the prev char to find || or &&,
// (as planned before), consider creating BOOL flags.
// ex: if encounter a '|', set Bool ORFLAG to 1. then upon next char inspection
// if (*cmdLine == '|') and ORFLAG == 1 (is set), you know you've encountered
// a '||'. otherwise, if the second *cmdLine != '|', reset ORFLAG to zero.
int parse(char *cmdLine, char **argv)
{
   int arguments = 0;
   while (*cmdLine != '\0') // traverse through cmdLine until NULL char reached
   {
      // ensure to skip all spaces, tabs, and newlines
      while ((*cmdLine == '\t') || (*cmdLine == ' ') || (*cmdLine == '\n'))
      {
         // replace this character with NULL
         *cmdLine = '\0';
         cmdLine++;
      }
      // now ensure a word, and not comments, follows, otherwise end argv array
      if ((*cmdLine == '\0') || (*cmdLine == '#'))
      {
         break;
      }
      // else, at the beginning of a word, so save the string starting here
      *argv = cmdLine;
      argv++;
      arguments++;
      // now traverse until the end of the word
      while ((*cmdLine != '\t') && (*cmdLine != ' ') && (*cmdLine != '\n') && (*cmdLine != '#') && (*cmdLine != '\0'))
      {
         cmdLine++;
      }
      // if encounter a '#' symbol, in this word, end the argv array here
      if (*cmdLine == '#')
      {
         *cmdLine = '\0';
         break;
      }
      // now restart loop to record next word until NULL reached
   }
   // set a NULL character at the end of argv list
   *argv = '\0';
   return arguments;
}

void execCmd(char **argVec)
{
   pid_t pid;
   // fork a child process and then verify the syscall worked
   if ((pid = fork()) < 0)
   {
      perror("Error forking child process:");
      exit(1);
   } else if (pid == 0) // now in child process, so call execvp
   {
       // call execvp and ensure syscall execvp completed successfully
      if ((execvp(argVec[0], argVec)) < 0)
      {
         perror("execution failed: ");
         exit(1);
      }
   }
   else // in parent process, so wait for child process to finish
   {
      // ensure syscall waitpid completes successfully
      if (-1 == waitpid(pid, NULL, 0))
      {
         perror("waitpid failed: ");
         exit(1);
      }
//      printf("Back to parent, now for new cmdline:\n");
   }
}

int main(void)
{
   char line[1024];
   char *arglist[128];
   int i = 0;
   int args = 0;
   while (1) // loop program infinitely until prompted to exit with 'exit"
   {
      printf("$: ");
      fgets(line, 1024, stdin);

      // now line is a NULL-terminated string ending with a newline.
      // let's parse the line:
      args = parse(line, arglist);
      // now arglist is a NULL-terminated array of char pointers to
      // NULL-terminated char arrays

      // now let's determine of any of these arguments are the exit command:
      // loop through arglist while (*arglist) and determine if **arglist
      // and "exit" == 0 in strcmp. If so, exit the program.
//      printf("the args here are:\n");
      for (i = 0; i < args; i++)
      {
         if (0 == strcmp(arglist[i], "exit"))
         {
            exit(0);
         }
       //  else
       //  {
       //     printf("%s\n", arglist[i]);
       //  }
      }
      // now, exit is not an argument, so send the arglist to execvp
      execCmd(arglist);
      // now restart while loop to execute next commandline entry
   }
   printf("You should never see this!");
   return 0;
}
