/*******************************************************************
******* Name: 		(Ebrahimi Meymand, Fahimeh) ****************
******* Project: 	PA-1 (Programming)          ****************
*******	Instructor:	Feng Chen                   ****************
*******	Class:		CSC7103-au18		    ****************
*******	LogonID:	cs710302		    ****************
*******	Date:		10 November 2018	    ****************
********************************************************************/

#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>

#define HISTORY_MAX_VALUE 100
#define HISTAT_MAX_VALUE 10
#define LINE_BUFSIZE 1024
#define TOKEN_BUFSIZE 64
#define TOKEN_DELIM " \t\r\n\a"

char *historyArr[HISTORY_MAX_VALUE];
int history_count = 0;
int histat_count = 0;

typedef struct  
{
	 char* command;
	 int count;
} histatStruct;

histatStruct histat_struct[HISTORY_MAX_VALUE];
histatStruct temp[HISTORY_MAX_VALUE];


int cd_handler(char **args)
{
	if (args[1] == NULL) 
	{
		fprintf(stderr, "Not enough argument\n");
       	}
       	else
       	{
		if (chdir(args[1]) != 0) 
		{
      			perror("Error in changing directory");
			return -1;
   		}
  	}
  	return 1;
}

int pwd_handler(char **args)
{
       	char * env;
	if((env = getenv("PWD")) == NULL)
 	{
		fprintf(stderr, "pwd failed");
	       	return -1;
	}
       	else
 	{
 		 printf(" %s\n", env);
 	}
	 return 1;
}

int exit_handler(char **args)
{
	printf("\nBye.....\n");
	return 0;
}

int history_handler(char **args)
{
	printf("\n   History Count = %d\n\n", history_count);
  	for(int i = 0; i < history_count; i++)
  	{
		 printf("   %d.", i + 1);
      		 printf("   %s\n", historyArr[i]);
    	}
	printf("\n");
	return 1;
}

int histat_handler(char **args)
{
	printf("\n   Histat Count = %d\n\n",histat_count);
  	for (int k = 0; k < histat_count; k++)
       	{
	       for (int j = 0; j < histat_count - 1; j++)
	       {
		       int j1 = histat_struct[j].count;
		       int j2 = histat_struct[j+1].count;
	       
		       if (j1 < j2)
		       {
				temp[0] = histat_struct[j+1];
				histat_struct[j+1] = histat_struct[j];
				histat_struct[j] = temp[0];
		       } 
	       }
       }
	for (int i = 0; i< 10; i++)
  	{
		if (i >= histat_count) 
		{
			printf("\n");
			return 1;
		}
   		printf("   %d. ", i+1);
    		printf("  %s\t", histat_struct[i].command);
    		printf("count = %d\n", histat_struct[i].count);
   		
  	}
	printf("\n");
  	return 1;
}

int help_handler()
{
	printf("\nHere is the list of commanded that I have tested\n");
	printf("1. Implemented Commands: cd, pwd, exit, help\n");
	printf("2. history\n");
	printf("3. histat\n");
	printf("4. simple commands like: mkdir, ls, vim, more, rm\n");
	printf("5. cat t1 (t1 must be existed)\n");
	printf("6. instructions | like\n");
	printf("7. diff a.txt b.txt\n");
	printf("8. ps ax, ps -ef\n");
	printf("9. head instructions, haed -n 2 instructions\n");
	printf("10. tail instructions, tail -n 2 instructions\n");
	printf("11. echo helloooo\n");
	printf("12. echo hello > file.txt\n");
	printf("13. ls | grep ins | wc\n");
	printf("14. sort < b.txt\n");
	printf("15. wc instructions, ls -l | wc -l\n");
	printf("16. grep like instructions\n");
	printf("17. date\n");
	return 1;
}

int redirection_handler(char * args[], char* inputFile, char* outputFile, int type)
{
	pid_t pid = -10; 
	int err = -1;
	
	int inputFileDescriptor = -1; // between 0 and 19, describing the output or input file
	int outputFileDescriptor = -1;
 
	pid = fork();
	if(pid == -1)
	{
		printf("Child process could not be created\n");
		return -1;
	}
 
	if(pid == 0)
	{
		// Option 0: output redirection
		if (type == 0)
		{
	
			outputFileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
			dup2(outputFileDescriptor, 1); 
			close(outputFileDescriptor);
	
			}
		else if (type == 1)
		{
		// Option 1: input and output redirection
			inputFileDescriptor = open(inputFile, O_RDONLY, 0600);
			dup2(inputFileDescriptor, STDIN_FILENO);
			close(inputFileDescriptor);		
		}
		else if (type == 2)
		{
			inputFileDescriptor = open(inputFile, O_RDONLY, 0600);
			dup2(inputFileDescriptor, STDIN_FILENO);
			close(inputFileDescriptor);

			outputFileDescriptor = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
			dup2(outputFileDescriptor, STDOUT_FILENO);
			close(outputFileDescriptor);
		}
		 
		if (execvp(args[0],args)==err)
		{
			printf("err");
			kill(getpid(),SIGTERM);
		}	
	}
	
	waitpid(pid,NULL,0);
	return 1;
}

int pipe_handler(char **args)
{

	int filedes[2]; // pos. 0 output, pos. 1 input of the pipe
	int filedes2[2];
	int num_cmds = 0;
	char *command[256];
	pid_t pid;
	
	int err = -1;
	int end = 0;
	
	int i = 0;
	int j = 0;
	int k = 0;
	int l = 0;
	
	while (args[l] != NULL)
	{
		if (strcmp(args[l],"|") == 0)
		{
			num_cmds++;
		}
		l++;
	}
	num_cmds++;
	while (args[j] != NULL && end != 1)
	{
		k = 0;
		while (strcmp(args[j],"|") != 0)
		{
			command[k] = args[j];
			j++;	
			if (args[j] == NULL)
			{
				end = 1;
				k++;
				break;
			}
			k++;
		}
		command[k] = NULL;
		j++;		
		
		if (i % 2 != 0)
		{
			pipe(filedes); 
		}
		else
		{
			pipe(filedes2); 
		}
		pid=fork();
		if(pid==-1)
		{			
			if (i != num_cmds - 1)
			{
				if (i % 2 != 0)
				{
					close(filedes[1]); 
				}
				else
				{
					close(filedes2[1]); 
				} 
			}			
			printf("Child process could not be created\n");
			return -1;
		}
		if(pid==0)
		{
			if (i == 0)
			{
				dup2(filedes2[1], STDOUT_FILENO);
			}
			
			else if (i == num_cmds - 1)
			{
				if (num_cmds % 2 != 0)
				{ 
			 		dup2(filedes[0],STDIN_FILENO);
				}
				else
				{ 
					dup2(filedes2[0],STDIN_FILENO);
				}
			}
			else
			{ 
				if (i % 2 != 0)
				{
					dup2(filedes2[0],STDIN_FILENO); 
					dup2(filedes[1],STDOUT_FILENO);
				}
				else
				{ 
					dup2(filedes[0],STDIN_FILENO); 
					dup2(filedes2[1],STDOUT_FILENO);					
				} 
			}
			
			if (execvp(command[0],command)==err)
			{
				kill(getpid(),SIGTERM);
			}		
		}
				
		
		if (i == 0)
		{
			close(filedes2[1]);
		}
		else if (i == num_cmds - 1)
		{
			if (num_cmds % 2 != 0)
			{					
				close(filedes[0]);
			}
			else
			{					
				close(filedes2[0]);
			}
		}
		else
		{
			if (i % 2 != 0)
			{					
				close(filedes2[0]);
				close(filedes[1]);
			}
			else
			{					
				close(filedes[0]);
				close(filedes2[1]);
			}
		}
				
		waitpid(pid,NULL,0);
				
		i++;	
	}
	return 1;
}

void add_command_to_history(char *command)
{
	if (history_count < HISTORY_MAX_VALUE)
	{
		historyArr[history_count] = strdup(command);
		history_count++;
	}
	else
	{
		free(historyArr[0]);
		for (unsigned index = 1; index < HISTORY_MAX_VALUE; index++)
    		{
     			 historyArr[index - 1] = historyArr[index];
		}
	       	historyArr[HISTORY_MAX_VALUE - 1] = strdup(command);
	}
}


int search_in_history(char *command)
{
	for (int i = 0; i< 50; i++)
  	{
	
		if(strcmp(histat_struct[i].command, command) == 0)
    		{
       			 return i;
   		}
  	}
 	 return -1;
}

void add_command_to_histat(char *command)
{
       	int searchResultIndex = -2;
	searchResultIndex = search_in_history( command);
 	 if ( searchResultIndex == -1)
 	 {
 
 		 histat_struct[histat_count].command = strdup(command);
    		 histat_struct[histat_count].count++;
  		 histat_count++;
  	}
  	else
  	{
   		 histat_struct[searchResultIndex].count++;
  	}
}

/*
void print_current_directory()
{
	char hostname[1024] = "";
	gethostname(hostname, sizeof(hostname));
	printf("SimpleShell: %s@%s %s > ", getenv("LOGNAME"), hostname,getcwd(currentDirectory, 1024));
	
}*/


int execute_command(char **args)
{
	pid_t pid, wpid;
       	int status;
      	pid = fork();
       	
	if (pid == 0) 
	{
   		 // Child process
    		if (execvp(args[0], args) == -1) 
		{
     			 perror("Shell: Child Error ");
    		}
    		exit(EXIT_FAILURE);
	}
       	else if (pid < 0) 
	{
		// Error forking
    		perror("Shell: Fork Error");
  	}
       	else 
	{
		// Parent process
		do
	       	{
     			 wpid = waitpid(pid, &status, WUNTRACED);
	       	} while (!WIFEXITED(status) && !WIFSIGNALED(status));
  	}
	 return 1;
}


int command_handler(char **args)
{
	int i = 0;
       	int j = 0;
      	char *args_aux[TOKEN_BUFSIZE];
       	int aux=0;
	if (args[0] == NULL) 
	{
		
		// An empty command was entered.
		 return 1;
	}
        while (args[j] != NULL)
       	{
		if ( (strcmp(args[j],">") == 0) || (strcmp(args[j],"<") == 0) || (strcmp(args[j],"&") == 0))
		{
			break;
		}
		args_aux[j] = args[j];
		j++;
  	}
  	args_aux[j] = NULL;


	if (args[0] != NULL)
	{
		
		if (strcmp(args[0], "cd") == 0) 
		{
			return cd_handler(args);
		}
		else if (strcmp(args[0], "pwd") == 0)
		{
			return pwd_handler(args);
		}
		else if (strcmp(args[0], "exit") == 0)
		{
			return exit_handler(args);
		}
		else if (strcmp(args[0], "history") == 0)
		{
			return history_handler(args);
		}
		else if (strcmp(args[0], "histat") == 0)
		{
			return histat_handler(args);
		}
		else if (strcmp(args[0], "help") == 0)
		{
			return help_handler();
		}

	}
	

	for (i = 0; i < TOKEN_BUFSIZE; i++)
  	{
		if (args[i] != NULL)
		{
   	 		if(strcmp(args[i],"|") == 0)
    			{
      				return pipe_handler(args);
    			}
 	
			if(strcmp(args[i],"<>") == 0)
    			{
      			//	printf("\nYEEEEEEEEEES\n");
			//	return -1;
				aux = i + 1;
				if (args[aux] == NULL)
				{
					printf("\nNot enough input arguments\n");
					return -1;
				}
			
				return redirection_handler(args_aux, args[i-1],args[i+1],2);

    			}

	    		if (strcmp(args[i],"<") == 0)
    			{
				aux = i+1;
				if (args[aux] == NULL)
				{
					printf("\nNot enough input arguments\n");
					return -1;
				}
				else
				{
					if (strcmp(args[aux],">") == 0)
					{
					//	printf("\nargs[aux +1]= %s\n", args[aux+1]);
					//	printf("\nUsage: Expected '>' and found %s\n",args[aux]);
						return -1;
					}
				}	
				return redirection_handler(args_aux,args[i+1],NULL,1);
    			}

			if (strcmp(args[i],">") == 0)
    			{
     
				if (args[i+1] == NULL)
				{
					printf("\nNot enough input arguments\n");
					return -1;
				}
	
	
				return redirection_handler(args_aux,NULL,args[i+1],0);
    			}
		}
		

	}
	
  	return execute_command(args);
}

char *read_line_from_input(void)
{
       	int bufsize = LINE_BUFSIZE;
	int position = 0;
	char *buffer = malloc(sizeof(char) * bufsize);
	int c;
	
	if (!buffer)
       	{
    		fprintf(stderr, "Shell: allocation error\n");
    		exit(EXIT_FAILURE);
  	}

  	while (1) 
	{

    		c = getchar();


    		if (c == EOF || c == '\n')
	       	{
      			buffer[position] = '\0';
      			return buffer;
    		} 
		else
	       	{
      			buffer[position] = c;
    		}
    		position++;


    		if (position >= bufsize) 
		{
      			bufsize += LINE_BUFSIZE;
     			buffer = realloc(buffer, bufsize);
     			 if (!buffer) 
			 {
        			fprintf(stderr, "Shell: allocation error\n");
        			exit(EXIT_FAILURE);
			 }
	    	}
      	}
}

char **tokenize_line(char *line)
{
      int bufsize = TOKEN_BUFSIZE;
      int  position = 0;
      char **tokens = malloc(bufsize * sizeof(char*));
      char *token;

      if (!tokens) 
      {
	      fprintf(stderr, "Shell: allocation error\n");
	      exit(EXIT_FAILURE);
      }
      token = strtok(line, TOKEN_DELIM);
      while (token != NULL) 
      {
	      tokens[position] = token;
	      position++;
	      
	      if (position >= bufsize)
	      {
		      bufsize += TOKEN_BUFSIZE;
		      tokens = realloc(tokens, bufsize * sizeof(char*));
		      if (!tokens)
		      {
			      fprintf(stderr, "Shell: allocation error\n");
			      exit(EXIT_FAILURE);
		      }
	      }
	      
	      token = strtok(NULL, TOKEN_DELIM);
      }
      tokens[position] = NULL;
      return tokens;
}


void init()
{
	for (int i = 0; i < HISTORY_MAX_VALUE; i++)
      	{
		histat_struct[i].command = "";
	       	histat_struct[i].count = 0;
	}

}

int main(int argc, char **argv)
{
	char *line;
	char **args;
	int success;

	init();	
              	
	do 
	{
		printf("SimpleShell--->> ");

		line = read_line_from_input();
	

		if (line != NULL && line[0] != '\0')
		{
			add_command_to_history(line);
		}
		
		args = tokenize_line(line);
		
		if (args[0] != NULL)
		{
			add_command_to_histat(args[0]);
		}
		
		success = command_handler(args);
		
		free(line);
		free(args);

	} while (success);
   
      	return EXIT_SUCCESS;
	
}
