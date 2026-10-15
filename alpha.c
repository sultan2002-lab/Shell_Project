#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include<signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#define MAX_ARGS 32
#define MAX_HISTORY 100
#define MAX_CMD_LEN 1024
char history[MAX_HISTORY][MAX_CMD_LEN];
int history_count = 0;
void add_to_history(const char* cmd);
void print_history();
/*HERE I MAKE USER FRIENDLY PROMPT*/
static void show_prompt(void) {
    char cwd[1024];
    char host[256] = "host";
    const char *user = getenv("USER");
    if (!user) user = "user";

    gethostname(host, sizeof(host));

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        strcpy(cwd, "?");
    }

    const char *home = getenv("HOME");
    const char *disp = cwd;
    char display[1024];
    if (home && strncmp(cwd, home, strlen(home)) == 0) {
        if (cwd[strlen(home)] == '\0') {
            strcpy(display, "~");
        } else {
            snprintf(display, sizeof(display), "~%s", cwd + strlen(home));
        }
        disp = display;
    }

    printf("\033[1;32m%s@%s\033[0m:\033[1;34m%s\033[0m \033[1;33malpha>\033[0m ",
           user, host, disp);
    fflush(stdout);
}
/*HERE I M MAKING FUNCTION FOR SIGNAL HANDELING*/
void sigint_handler(int signo) {
    	printf("\n[Shell] Ctrl+C pressed! But shell is still running.\nalpha> ");
    	fflush(stdout);
}
void sigchld_handler(int signo) {
    	int status;
    	/*HERE I M CLEANING ALL FINISHED BACKGROUND PROCESS*/
    	while (waitpid(-1, &status, WNOHANG) > 0) {
        	printf("\n[Background process finished]\nalpha> ");
        	fflush(stdout);
    	}
}
void sigtstp_handler(int signo) {
    	printf("\n[Shell] Ctrl+Z pressed! Ignoring.\nalpha> ");
    	fflush(stdout);
}
int main(int argc,char** argv){
	signal(SIGINT, sigint_handler);     /*Ctrl+C*/
    	signal(SIGCHLD, sigchld_handler);   /*Background cleanup*/
    	signal(SIGTSTP, sigtstp_handler);   /*Ctrl+Z*/
	char cmd[MAX_CMD_LEN];
	char cwd[1024];
	int first_prompt=1;
	system("clear");
	while(1){
		/* YHA PR MAI CD COMMAND USE KR K JIS DIRECTORY PR JA RHA HU TO USKE BAD VO DR AUR PROMPT EK SATH AAYEGA*/
		/*char* home = getenv("HOME");
		if(first_prompt){
			printf("~alpha> ");
		}else{
			if(getcwd(cwd,sizeof(cwd))!=NULL){
				if(home!=NULL && strncmp(cwd,home,strlen(home))==0){
					printf("~%salpha> ",cwd+strlen(home));
				}else{
					printf("%s alpha> ",cwd);
				}
			}else{
				perror("getcwd() error");
			}
		}*/
		show_prompt();
		/*HERE I am TAKING INPUT BY fgets*/
		if (fgets(cmd, sizeof(cmd), stdin) == NULL) {
    			break; /*Ctrl+D (EOF) handle karne ke liye*/
		}
		/*remove last newline character jo fgets() dalta hai*/
		cmd[strcspn(cmd, "\n")] = '\0';
		if (strcmp(cmd, "exit") == 0) {
    		break;
		}
		if (strcmp(cmd, "cd") == 0 || strncmp(cmd, "cd ", 3) == 0) {
    		first_prompt = 0;
		}
		/*scanf("%[^\n]%*c",cmd);
		if(strcmp(cmd,"cd")==0 || strncmp(cmd,"cd ",3)==0){
			first_prompt=0;
		}
		scanf(" %[^\n]",cmd);
		if(strcmp(cmd,"exit")==0){
			break;
		}*/
		/*yha history k liye commands likhe ja rhe hai*/
		if(strlen(cmd)==0)continue;
		add_to_history(cmd);
		if(strcmp(cmd,"history")==0){
			print_history();
			continue;
		}
		/*YHA PR MAI WORDS/SENTENCES KO TOKENS ME DIVIDE KR RHA THA AUR SPACE KO SPLIT KR RHA THA
		char* arg[10];
		char* tok = strtok(cmd," ");
		int i=0;
		while(tok!=NULL){
			arg[i++]=tok;
			tok=strtok(NULL," ");
		}
		arg[i]=NULL;
		int j;*/
		/*for(j=0;j<i;j++){
			printf("%s\n",arg[j]);
		}*/
		/*NOW YHA PR SPACES AUR TABS DONO KO SPLITS KRENGE AUR WORDS/COMMANDS KO TOKENISE KRENGE*/
		int token(char* line,char* argv[],int maxarg){
			int i=0;
			char* tok = strtok(line," \t");/*space ya tab ko splits kr dega*/
			while(tok && i < maxarg-1){
				argv[i++] = tok;
				tok = strtok(NULL," \t");
			}
			argv[i]=NULL;
			return i; /* total token return ho rha hai yha*/
		}
		char* arg[MAX_ARGS];
		int i = token(cmd,arg,MAX_ARGS);
		if( i == 0)continue;
		/*check if last argument is "&"*/
                i = 0;
		int background = 0;
                while (arg[i] != NULL) i++;
                if (i > 0 && strcmp(arg[i-1], "&") == 0) {
                        background = 1;
                arg[i-1] = NULL;  /* "&" ko remove kar do*/
                }

		/*now here i make function for cd command*/
		/*if(strcmp(arg[0],"cd")==0){
			if(arg[1]==NULL){
				printf("cd command is missing\n");
			}else{
				if(chdir(arg[1])!=0){
					perror("cd failed");
				}
			}
			continue;
		}*/
		/*NOW HERE I MAKE FUNCTION FOR INTERNAL COMMAND WHICH IS APPLICABLE ON USER HOME DIRECTORY*/
		if(strcmp(arg[0],"cd")==0){
			char path[200];
			if(arg[1]==NULL){
				char* home = getenv("HOME");
				if(home!=NULL){
					chdir(home);
				}
			}else{
				if(arg[1][0]=='~'){
					char* home = getenv("HOME");
					if(home!=NULL){
						snprintf(path,sizeof(path),"%s%s",home,arg[1]+1);
						if(chdir(path)!=0){
							perror("cd failed");
						}
					}
				}else{
					if(chdir(arg[1]) != 0){
						perror("failed");
					}
				}
			}
			continue;
		}
		/*here i call fork system call*/
		/*pid_t pid = fork();
		if(pid==0){
			if(execvp(arg[0],arg)==-1){
				perror("alpha");
			}
			exit(1);
		}else if(pid>0){
			wait(NULL);
		}else{
			perror("fork failed");
		}*/
		/*HERE I MAKE FUNCTION FOR BACK GROUND PROCESS */
		pid_t pid = fork();
		 background = 0;
		if (pid == 0) {
    		/*child process*/
    			if (execvp(arg[0], arg) == -1) {
        			perror("exec failed");
    			}
    			exit(1);
		} else if (pid > 0) {
    		/*parent process*/
    			if (background) {
        		printf("[Running in background: PID=%d]\n", pid);
    			} else {
        		waitpid(pid, NULL, 0);  /*foreground ke liye wait*/
    			}
		} else {
    		perror("fork failed");
		}
	}
	return 0;
}
/*here i make function definition of history command*/
void add_to_history(const char* cmd) {
    if (history_count < MAX_HISTORY) {
        strcpy(history[history_count], cmd);
        history_count++;
    } else {
        /* purane sabko ek step upar shift karo*/
	int i;
        for ( i = 1; i < MAX_HISTORY; i++) {
            strcpy(history[i - 1], history[i]);
        }
        /*last slot me latest command daalo*/
        strcpy(history[MAX_HISTORY - 1], cmd);
    }
}

void print_history(){
		int i;
		for( i=0;i<history_count;i++){
			printf("%d %s\n",i+1,history[i]);
		}
}
