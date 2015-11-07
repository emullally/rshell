#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>

int G_ISOR = 0;
int G_ISAND = 0;
int G_ISNEXT = 0;
int G_WASAND = 0;
int G_WASOR = 0;

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
   int ANDFLAG = 0;

   while (*cmdLine != '\0') // traverse through cmdLine until NULL char reached
   {
      // if global G_ISAND set at time of parse

      // ensure to skip all spaces, tabs, and newlines
      while ((*cmdLine == '\t') || (*cmdLine == ' ') || (*cmdLine == '\n'))
      {
         // replace this character with NULL
         *cmdLine = '\0';
         cmdLine++;
         ORFLAG = 0;
         ANDFLAG = 0;
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
         *cmdLine = '\0';
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
      if (*cmdLine == '&')
      {
         if (ANDFLAG)
         {
            // if here, a && connector has been detected, so prep and return
            // decrement cmdLine pointer and set this char to 0, then 
            // incrment cmdLine pointer to where nextLine starts, after the 
            // && connector.
            cmdLine--;
            *cmdLine = '\0';
            cmdLine++;
            *cmdLine = '\0';
            cmdLine++;
            nextLine = cmdLine;
            *argv = '\0';
            G_ISAND = 1;
            return nextLine;
         } 
         // else
         ANDFLAG = 1;
      } else
      {
         ANDFLAG = 0;
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
      if (*cmdLine == '&')
      {
         ANDFLAG = 1;
         cmdLine++;
      }
      // now restart loop to record next word until NULL reached
   }
   // set a NULL character at the end of argv list
   *argv = '\0';
   nextLine = cmdLine;
   return nextLine;
}

int execCmd(char **argVec)
{
   int execSuccess = 0;
   pid_t pid;
   int status;
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
//         execSuccess = 0;
//         return execSuccess;
         exit(1);
      }
//      execSuccess = 0;
     // return execSuccess;
   }
   else // in parent process, so wait for child process to finish
   {
      // ensure syscall waitpid completes successfully
      do
      {
         if (-1 == waitpid(pid, &status, WUNTRACED))
         {
            perror("waitpid failed");
            exit(1);
         }
         if (WIFEXITED(status))
         {
            if (1 == WEXITSTATUS(status))
            {
               execSuccess = 0;
            //   printf("exited status %d/n", WEXITSTATUS(status));
            } else
            {
               execSuccess = 1;
             //  printf("exited status %d/n", WEXITSTATUS(status));
            }
         }
      } while (!WIFEXITED(status));
      // check that the child terminated normally, otherwise set execRtrn to 0
/*      if (!WIFEXITED(status))
      {
         printf("child process did not finish normally!\n");
         execSuccess = 0;
      } else
      {
         execSuccess = 1;
      }
*/
 //     printf("Back to parent, now for new cmdline:\n");
   }
  // printf("End of execCmd func with: %d\n", execSuccess);
   return execSuccess;
}

int main(void)
{
   char line[1024];
//   char *startLine = line;
   char *nxtLine = line;
   char *arglist[128];
   char *startArg = *arglist;
   int execRtrn = 0;
   int firstLoop = 1;
   while (1) // loop program infinitely until prompted to exit with 'exit"
   {
   NEWPROMPT:
      firstLoop = 1;
      execRtrn = 0;
      nxtLine = line;
      printf("$: ");
      if (!fgets(line, 1024, stdin))
      {
         printf("fgets failed\n");
         exit(1);
      }
     // now line is a NULL-terminated string ending with a newline.
      // let's parse the line:
   NEXTCMD:
// do while (connector present) 


      if (G_WASOR) // if connector detected...
      {
         G_WASOR = 0;
         // clear the ; connector flag for next parse
   //      printf("In OR connector loop.\n");
 //        G_ISNEXT = 0;
         
   //      execRtrn = execCmd(arglist);
         
         // arglist pointer back to original position.
         //*arglist = startArg;   
   
         // take action depending on connector and execCmd return
         if (execRtrn == 1)
         {
            // here, we skip the RHS of || since the LHS passed
    //        G_ISOR = 0;
 //           printf("encountered OR with first success\n");
            *arglist = startArg;   
            nxtLine = parse(nxtLine, arglist);
            execRtrn = 0;
            goto CONTCMD;
    //        if (*nxtLine != '\0')
      //      {
        //       firstLoop = 0;
          //     goto CONTCMD;
   //         } else
     //       {
       //        goto NEWPROMPT;
         //   }
// Note: instead of ignoring RHS, let's check if && connector follows:
// if && connector....
         } else // if execRtrn == 0
         {
            if (0 == firstLoop)
            {
               // here if previous OR  command failed
               firstLoop = 0;
      //         G_ISOR = 0;
               printf("encountered command ORd with fail\n");
 //              *arglist = startArg;   
   //            nxtLine = parse(nxtLine, arglist);
         //      execRtrn = 0;
      //         execRtrn = execCmd(arglist);
               goto CONTCMD;
           //    if (*nxtLine != '\0')
             //  {
              //    firstLoop = 0;
               //   goto CONTCMD;
               //} else
              // {
              //    goto NEWPROMPT;
              // }
            } else // if 1 == firstLoop and 0 == execRtrn
            {
               // here if first command
               //G_ISOR = 0;
               firstLoop = 0;
               printf("encountered first command with OR\n");
       //        execRtrn = execCmd(arglist);
               goto CONTCMD;
            }
         }
      }
      if (G_WASAND)
      {
       //  *arglist = startArg;   
         G_WASAND = 0;
         if (execRtrn == 0)
         {
           // G_ISAND = 0; 
            printf("encountered AND with first fail\n");
//CHANGE?! 
           *arglist = startArg;   
            nxtLine = parse(nxtLine, arglist);
            execRtrn = 0;
            goto CONTCMD;
//            if (*nxtLine != '\0')
 //           {
  //             firstLoop = 0;
   //            goto CONTCMD;
    //        } else
     //       {
      //         goto NEWPROMPT;
       //     }
         } else
         {      
//            G_ISAND = 0;
            firstLoop = 0;
            printf("encountered a AND after successful call\n");  
//            execRtrn = execCmd(arglist);
       //     printf("encountered AND with first success\n");
            goto CONTCMD;
         }
      }
CONTCMD:
  // printf("at NEXTCMD\n"); 
      *arglist = startArg;
     // keep global flags if set already:
      nxtLine = parse(nxtLine, arglist);
      // call parse until an arglist provided
      while ((!*arglist) && (*nxtLine != '\0'))
      {
    //     printf("No arglist; looking to next\n");
//         continue;
         // do not call parse if G_ISAND set but no call to execCmd was made
         if (G_ISAND == 0)
         { 
            *arglist = startArg;
            nxtLine = parse(nxtLine, arglist);
         } else
         {
            G_ISNEXT = 0;
            G_ISOR = 0;
            G_ISAND = 0;
            goto NEWPROMPT;
         }
      }
      // if no arguments given, ask for new command
      if (!*arglist)
      {
      //   printf("no args and no more to check\n");
         // if here, no argument is to be executed and no new commands await
        G_ISNEXT = 0;
        G_ISOR = 0;
        G_ISAND = 0; 
        goto NEWPROMPT;

      }
      // check for exit command
      if (0 == strcmp(arglist[0], "exit"))
      {
      //   exit(0);
          return 0;
      }
      // in ; connector case:
      // execute the command list
      //execRtrn = execCmd(arglist);
      //if (G_ISNEXT)
      //{
       //  G_ISNEXT = 0;
        // firstLoop = 0;
         //execRtrn = execCmd(arglist);
         //goto NEXTCMD;
     // }
      execRtrn = execCmd(arglist);
      if (G_ISOR)
      {
         printf("prev command had a OR conditional operator\n");
         G_WASOR = 1;
         G_ISOR = 0;
         firstLoop = 0;
         goto NEXTCMD;
      }
      if (G_ISAND)
      {
         printf("prev command had a AND conditional operator\n");
         G_WASAND = 1;
         G_ISAND = 0;
         firstLoop = 0;
         goto NEXTCMD;
      }
      
      if (G_ISNEXT)
      {
         G_ISNEXT = 0;
         firstLoop = 0;
         goto NEXTCMD;
      }
/*      if (G_ISOR) // if connector detected...
      {
         // clear the ; connector flag for next parse
   //      printf("In OR connector loop.\n");
 //        G_ISNEXT = 0;
         
   //      execRtrn = execCmd(arglist);
         
         // arglist pointer back to original position.
         // *arglist = startArg;   
   
         // take action depending on connector and execCmd return
         if (execRtrn == 1)
         {
            // here, we skip the RHS of || since the LHS passed
            G_ISOR = 0;
 //           printf("encountered OR with first success\n");
            *arglist = startArg;   
            nxtLine = parse(nxtLine, arglist);
            execRtrn = 0;
            if (*nxtLine != '\0')
            {
               firstLoop = 0;
               goto NEXTCMD;
            } else
            {
               goto NEWPROMPT;
            }
// Note: instead of ignoring RHS, let's check if && connector follows:
// if && connector....
         } else // if execRtrn == 0
         {
            if (0 == firstLoop)
            {
               // here if previous OR  command failed
               firstLoop = 0;
               G_ISOR = 0;
               printf("encountered command ORd with fail\n");
               *arglist = startArg;   
               nxtLine = parse(nxtLine, arglist);
         //      execRtrn = 0;
               execRtrn = execCmd(arglist);
               if (*nxtLine != '\0')
               {
                  firstLoop = 0;
                  goto NEXTCMD;
               } else
               {
                  goto NEWPROMPT;
               }
            } else // if 1 == firstLoop and 0 == execRtrn
            {
               // here if first command
               G_ISOR = 0;
               firstLoop = 0;
               printf("encountered first command with OR\n");
               execRtrn = execCmd(arglist);
               goto NEXTCMD;
            }
         }
      }
      if (G_ISAND)
      {
       //  *arglist = startArg;   
         if (execRtrn == 0)
         {
            G_ISAND = 0; 
       //     printf("encountered AND with first fail\n");
            *arglist = startArg;   
            nxtLine = parse(nxtLine, arglist);
            execRtrn = 0;
            if (*nxtLine != '\0')
            {
               firstLoop = 0;
               goto NEXTCMD;
            } else
            {
               goto NEWPROMPT;
            }
         } else
         {      
            G_ISAND = 0;
            firstLoop = 0;
            printf("encountered a AND after successful call\n");  
            execRtrn = execCmd(arglist);
       //     printf("encountered AND with first success\n");
            goto NEXTCMD;
         }
      }
 */     // now ensure there is an argument in the argument list, arglist 
      // send the arglist to execvp
   //   execRtrn = execCmd(arglist);
      // now restart while loop to execute next commandline entry
 //     printf("The first char of the last command: %c\n", line[0]); 
   }
   printf("You should never see this!");
   return 0;
}
