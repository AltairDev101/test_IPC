#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <cstring>

int main(int argc, char* argv[])
{
	int fd[2], nbytes;
	pid_t childpid;
	char string[] = "Hello from child!\n";
	char readbuffer[80];

	std::cout<<"Test IPC"<<std::endl;
	pipe(fd);
	if((childpid = fork()) == -1)
	{
		perror("fork");
		exit(1);
	}

	if(childpid == 0){
		//child process
		close(fd[0]);
		write(fd[1], string, strlen(string));
	}
	else{
		//parent process
		close(fd[1]);
		nbytes = read(fd[0], readbuffer, sizeof(readbuffer));
		std::cout<<"Received string: "<<readbuffer<<std::endl;
	}

	return 0;
}	

