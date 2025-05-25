/*
	header file
*/
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
/*
	macros
*/
#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

/*
	Function Declarations for rudimentary tasks

*/

char *lsh_read_line(void);
char **lsh_split_line(char *line);

/*
	Function Declarations for builtin shell commands

*/

int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);
int lsh_cp(char **args);

/*
	List of builtin commands, followed by their corresponding functions
*/
void lsh_loop(void);
char *builtin_str[] ={
	"cd",
	"help",
	"exit",
	"cp"
};

int main()
{
	//load config files, incase if any

	//run command shell loop
	lsh_loop();

	//perform shutdown/cleanup, if any
	return EXIT_SUCCESS;
}


void lsh_loop()
{
	char *line;
	char **args;
	int status;

	do{
		printf("$ ");
		line = lsh_read_line();
		args = lsh_split_line(line);
		status = lsh_execute(args);

		free(line);
		free(args);
	}while(status);

}

char *lsh_read_line(void)
{
	int bufsize = LSH_RL_BUFSIZE;
	int position = 0;
	char* buffer = malloc(sizeof(char)*bufsize);
	int c;

	if(!buffer){
		fprintf(stderr, "lsh: allocation error\n");
		exit(EXIT_FAILURE);
	}
	while(1){
		c = getchar();

		if(c == EOF || c == '\n'){ // for no input
			buffer[position] = '\0';
			return  buffer;
		}else{ // fi anything is typed on the input line
			buffer[position] = c;
		}
		position++;
		if(position >= bufsize){ // for not running out of space
			bufsize += LSH_RL_BUFSIZE;
			buffer = realloc(buffer,bufsize);
			if(!buffer){ // for checking allocation errors if buffer actually runs out of space
				fprintf(stderr,  "lsh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
	}

}

char **lsh_split_line(char *line)
{
	int bufsize = LSH_TOK_BUFSIZE,position=0;
	char **tokens = malloc(bufsize * sizeof(char*));
	char *token;

	if(!tokens){
		fprintf(stderr, "lsh: allocation error\n");
		exit(EXIT_FAILURE);
	}
	token = strtok(line, LSH_TOK_DELIM);
	while(token != NULL){
		tokens[position] = token;
		position++;
		if(position >= bufsize){
			bufsize += LSH_TOK_BUFSIZE;
			tokens = realloc(tokens,bufsize*sizeof(char*));
			if(!tokens){
				fprintf(stderr, "lsh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL,LSH_TOK_DELIM);
	}
	tokens[position] = NULL;
	return tokens;
}

int lsh_launch(char **args)
{
	pid_t pid,wpid;
	int status;

	pid = fork();
	if(pid == 0){
		//child process
		if(execvp(args[0],args) == -1){
			perror("lsh");
		}
		exit(EXIT_FAILURE);
	}else if(pid <0){
		//Error forking
		perror("lsh");
	}else{
		//Parent process
		do{
			wpid = waitpid(pid,&status,WUNTRACED);
		}while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1;
}





int (*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_help,
  &lsh_exit,
  &lsh_cp
};

int lsh_num_builtins() {
	return sizeof(builtin_str)/ sizeof(char *);
}

/*
	Builtin function implementations
*/
int lsh_cd(char **args)
{
    if (args[1] == NULL) {
    	fprintf(stderr, "lsh: expected argument to \"cd\"\n");
    } else {
    if (chdir(args[1]) != 0) {
      	perror("lsh");
    }
  }
  return 1;
}

int lsh_help(char **args)
{
	int i;
	printf("LSH \n");
	printf("Type program names and arguments, andhit enter.\n");
	printf("The following are built in:\n");
	for(i = 0;i<lsh_num_builtins();i++){
		printf(" %s\n", builtin_str[i]);
	}
	printf("Use the man command for information on other programs.\n");
	return 1;
}

int lsh_exit(char **args){
	return 0;
}

int lsh_cp(char **args){
    FILE *in,*out;
    in = fopen(args[1],"r");
    out = fopen(args[2],"w");
    if(in == NULL || out == NULL){
        printf("Error while opening file(s)\n");
        return 1;
    }

    char c;
    while( (c = fgetc(in)) != EOF){
        fputc(c,out);
    }
    fclose(in);
    fclose(out);
    return 1;
}

int lsh_execute(char **args)
{
	int i;
	if(args[0] == NULL){
		// an empty command was entered.
		return 1;
	}

	for(i = 0;i < lsh_num_builtins(); i++){
		if(strcmp(args[0], builtin_str[i]) == 0){ //compares first term (command) with the string array
			return (*builtin_func[i])(args);
		}
	}
	return lsh_launch(args);

}
