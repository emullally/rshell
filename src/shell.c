#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>

int G_ISOR = 0;
//int G_ISAND = 0;
int G_ISNEXT = 0;

char* parse(char *cmdLine, char **argv);
int execCmd(char **argVec);

// Note on parse: now any char after '#' is considered a comment and ignored
// by the parse function.
// Note 2: rather than having a pointer to the prev char to find || or &&,
// (as planned before), consider creating BOOL flags.
// ex: if encounter a '|', set Bool ORFLAG to 1. then upon next char inspection
// if (*cmdLine == '|') and ORFLAG == 1 (is set), you know you've encountered
// a '||'. otherwise, if the second *cmdLine != '|', reset ORFLAG to zero.
char* parse(char *cmdLine, char **argv)
{
   char *nextLine = cmdLine;
   int ORFLAG = 0;
//   int ANDFLAG = 0;

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
         *cmdLine = '\0';
         break;
      }
      // else, at the beginning of a word, so save the string starting here
      // check if the ; connector is at the beginning of a word or alone
      if (*cmdLine == ';')
      {
         cmdLine++;
         nextLine = cmdLine;
         *argv = '\0';
         G_ISNEXT = 1;
         return nextLine;
      }
      if (*cmdLine == '|')
      {
         if (ORFLAG)
         {
            // if here, a || connector has been detected, so prep and return
            // decrement cmdLine pointer and set this char to 0, then 
            // incrment cmdLine pointer to where nextLine starts, after the 
            // || connector.
            cmdLine--;
            *cmdLine = '\0';
            cmdLine++;
            *cmdLine = '\0';
            cmdLine++;
            nextLine = cmdLine;
            *argv = '\0';
            G_ISOR = 1;
            return nextLine;
         } 
         // else
         ORFLAG = 1;
      } else
      {
         ORFLAG = 0;
      }
      if ((*cmdLine != '|') && (*cmdLine != '&'))
      {
         // if at the beginning of a word and not a connector:
         *argv = cmdLine;
         argv++;
      }
      // now traverse until the end of the word
      while ((*cmdLine != '\t') && (*cmdLine != ' ') && (*cmdLine != '\n') && (*cmdLine != '#') && (*cmdLine != '\0') && (*cmdLine != '|') && (*cmdLine != '&'))
      {
         // look for connectors ; && and ||
         // if encounter a ';', update nextLine passed current execution
         // and return with argv containing the current execution
         if (*cmdLine == ';')
         {
            *cmdLine = '\0';
            cmdLine++;
            // update nextLine with cmdLine passed the current execution args
            nextLine = cmdLine;
            *argv = '\0';
            // set flag that ; connector encountered
            G_ISNEXT = 1;
            return nextLine;
         }
         cmdLine++;
      }
      // if encounter a '#' symbol, in this word, end the argv array here
      if (*cmdLine == '#')
      {
         *cmdLine = '\0';
         break;
      }
      if (*cmdLine == '|')
      {
         ORFLAG = 1;
         cmdLine++;
      }
/*
      if (*cmdLine == '&')
      {
         ANDFLAG = 1;
         cmdLine++;
      }
*/
      // now restart loop to record next word until NULL reached
   }
   // set a NULL character at the end of argv list
   *argv = '\0';
   nextLine = cmdLine;
   return nextLine;
}

int execCmd(char **argVec)
{
   int execSuccess = 1;
   pid_t pid;
   // fork a child process and then verify the syscall worked
   if ((pid = fork()) < 0)
   {
      perror("Error with fork");
      exit(1);
   } else if (pid == 0) // now in child process, so call execvp
   {
       // call execvp and ensure syscall execvp completed successfully
      if ((execvp(argVec[0], argVec)) < 0)
      {
         perror("execution failed");
         execSuccess = 0;
//         return execSuccess;
    //     exit(1);
      }
      execSuccess = 0;
      return execSuccess;
   }
   else // in parent process, so wait for child process to finish
   {
      // ensure syscall waitpid completes successfully
      if (-1 == waitpid(pid, NULL, 0))
      {
         perror("waitpid failed");
         exit(1);
      }
//      printf("Back to parent, now for new cmdline:\n");
   }
 //  printf("End of execCmd func with: %d\n", execSuccess);
   return execSuccess;
}

int main(void)
{
   char line[1024];
//   char *startLine = line;
   char *nxtLine = NULL;
   char *arglist[128];
   char *startArg = *arglist;
   int execRtrn = 0;
   while (1) // loop program infinitely until prompted to exit with 'exit"
   {
//      line = startLine;
   NEWPROMPT:
      *arglist = startArg;
      printf("$: ");
      fgets(line, 1024, stdin);
//      if (0 == strcmp(line, "\n"))
 //     {
//         printf("Enter a commmand!\n");
   //      continue;
     // }
      // now line is a NULL-terminated string ending with a newline.
      // let's parse the line:
      nxtLine = parse(line, arglist);
      // call parse until an arglist provided so long as there's something
      // to parse
      while ((!*arglist) && (*nxtLine != '\0'))
      {
    //     printf("No arglist; looking to next\n");
//         continue; 
         nxtLine = parse(nxtLine, arglist);
      }
      if (!*arglist)
      {
      //   printf("no args and no more to check\n");
         // if here, no argument is to be executed and no new commands await
        G_ISNEXT = 0;
        G_ISOR = 0;
//      G_ISAND = 0; 
        continue;

      }
      // check for exit command
      if (0 == strcmp(arglist[0], "exit"))
      {
      //   exit(0);
          return 0;
      }
      // in ; connector case:
      while (G_ISNEXT || G_ISOR) // add || G_ISOR || G_ISAND when those work...
      {
         // clear the ; connector flag for next parse
      //   printf("In ; connector loop.\n");
         G_ISNEXT = 0;
         // G_ISOR = 0;
         // G_ISAND = 0;
         // ensure arglist is filled before sending to execvp, then put
         // arglist pointer back to original position.
         
         execRtrn = execCmd(arglist);
         
         *arglist = startArg;   
//         if (0 == strcmp(nxtLine, "\n"))
//         {
//         printf("Enter a commmand!\n");
//            continue;
//         }
   
         // take action depending on connector and execCmd return
         if (G_ISOR && (execRtrn == 1))
         {
            // here, we skip the RHS of || since the LHS passed
            G_ISOR = 0;
            goto NEWPROMPT;
         }
//         if (G_ISAND && (execRtrn == 0))
//         {
//            goto NEWPROMPT;
//         }
         // now, clear global && and || flags
         G_ISOR = 0;
//         G_ISAND = 0;
         // otherwise, take in next command to execute
         nxtLine = parse(nxtLine, arglist);
         while ((!*arglist)  && (*nxtLine != '\0'))
         {
         //   printf("No arglist; looking to next\n");
//            continue; 
            nxtLine = parse(nxtLine, arglist);
         }
         if (!*arglist)
         {
        //    printf("no args and no more to check\n");
            // if here, no argument is to be executed and no new commands await
            G_ISNEXT = 0;  
            G_ISOR = 0;
//            G_ISAND = 0;
            goto NEWPROMPT;
         } 
/*         while (!*arglist)
         {
            printf("No arglist again\n");
            goto NEWPROMPT;
            nxtLine = parse(nxtLine, arglist);
         }
*/
         // if the first command argument is "exit", then exit
         if (0 == strcmp(arglist[0], "exit"))
         {
         //   exit(0);
            return 0;
         }
//         printf("end of connector loop in main\n");
      }
      // now ensure there is an argument in the argument list, arglist 
      // send the arglist to execvp
      execRtrn = execCmd(arglist);
      // now restart while loop to execute next commandline entry
 //     printf("The first char of the last command: %c\n", line[0]); 
   }
   printf("You should never see this!");
   return 0;
}
