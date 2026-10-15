#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
int main(int argc,char** argv){
	system("clear");
	char cmd[100];
	while(1){
		printf("~myshell$ ");
		scanf(" %[^\n]",cmd);
		/*printf("%s\n",cmd);*/
		if(strcmp(cmd,"exit")==0){
			break;
		}
		/*here we use these code for tokenisation which we pass input*/	
		char* args[10];
		char* tok = strtok(cmd," ");
		int i=0;
		while(tok!=NULL){
			args[i++]=tok;
			tok= strtok(NULL," ");
		}
		args[i]=NULL;
		int j;
		/*for(j=0;j<i;j++){
			printf("%s\n",args[j]);
		}*/
		/*now calling here fork system call if pid==0 it means in child if >0 it means in parent other wise  did create*/
		/*pid_t pid = fork();
		if(pid==0){
			printf("child process : pid=%d, input = %s\n",getpid(),cmd);
			exit(0);
		}else if(pid>0){
			printf("parent process : pid=%d, waiting for child.....",getpid());
			wait(NULL);
		}else{
			perror("process not created");
		}
		printf("\n");*/
		/* here i make fork + exe jis se ki ek program run ho ske*/
		pid_t pid=fork();
		if(pid==0){
			if(execvp(args[0],args)==-1){
			perror("myshell");
			}
			exit(1);/*agar exe fail ho gya to*/
		}else if(pid>0){
			wait(NULL);
		}else{
			perror("fork failed");
		}
	}
	return 0;
}
