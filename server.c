//Name: Yehuda Daniel
//ID: 211789680
//Name: Ilan Brailovich
//ID: 322525072

#include <stdio.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>

int ReadLine(int fd, char* buff) {
	int i = 0;
	int temp = 0;
	do {
		temp = read(fd, &(buff[i++]), 1);
		if (temp < 1)return 0;
	} while (buff[i - 1] != '\n');
	buff[i - 1] = '\0';
	i = 0;
	while (buff[i] != '\0') {
		i++;
	}
	return i;
}
void printToScreen(char* stage) {
	char prints[50];
	strcpy(prints, "End of Stage ");
	strcat(prints, stage);
	strcat(prints, "\n");
	write(STDOUT_FILENO, prints, strlen(prints));
}

void handlerToServer(int dummy) {
	alarm(0);
	alarm(60);  // Scheduled alarm after 60 seconds
	char pidClient[7];
	char* Firstinput = (char*)malloc(50 * sizeof(char));
	char* secondInput = (char*)malloc(50 * sizeof(char));
	char action[10];
	int calc = 0;
	char errormsg[50];
	int errorDivided = 0; // 0 means false 1 means true
	char msg[50];

	int fdin = open("toSrv.txt", O_RDONLY);
	if (fdin < 0) {
		// if client didn't create this file yet
		perror("error from EX2\n");
		exit(-1);
	}

	// open fileToServer.txt read details . create a calcluation and return result to client by using signal
	ReadLine(fdin, pidClient);
	ReadLine(fdin, Firstinput);
	ReadLine(fdin, action);
	ReadLine(fdin, secondInput);
	close(fdin);
	//create fork child to remove the filetoSrv for next user
	int pid = fork();

	if (pid < 0) {
		perror("error from ex2\n");
		exit(-1);
	}
	else {
		if (pid == 0) {
			char* argcRemove[] = { "rm","toSrv.txt",NULL };
			execvp(argcRemove[0], argcRemove);
		}
		else {
			wait(NULL);

			// create calcluation and save this in file toClientXXXXX when XXXXX = pid of client
			switch (action[0]) {
			case '1':
				calc = atoi(Firstinput) + atoi(secondInput);
				break;
			case '2':
				calc = atoi(Firstinput) - atoi(secondInput);
				break;
			case '3':
				calc = atoi(Firstinput) * atoi(secondInput);
				break;
			case '4':
				if (atoi(secondInput) == 0) errorDivided = 1;
				else calc = atoi(Firstinput) / atoi(secondInput);
				break;
			default:
				strcpy(errormsg, "Error From ex2\n");
				write(STDOUT_FILENO, errormsg, strlen(errormsg));
				exit(-1);
				break;
			}
			char* clientFileName = (char*)malloc(50 * sizeof(char));
			strcpy(clientFileName, "toClient");
			strcat(clientFileName, pidClient);
			strcat(clientFileName, ".txt");

			int fToClient = open(clientFileName, O_CREAT | O_RDWR, 0666);

			if (errorDivided == 1) {
				strcpy(errormsg, "cannot divide by zero");
				write(fToClient, errormsg, strlen(errormsg));
			}
			else {
				sprintf(msg, "%d", calc);
				write(fToClient, msg, strlen(msg));
			}
			close(fToClient);
		}
	}
	kill(atoi(pidClient), SIGUSR2); 

	signal(SIGUSR1, handlerToServer);
}

void signalHandle(int signum) {
	char* closeAns = (char*)malloc(110 * sizeof(char));
	strcpy(closeAns, "the server was closed beacuse of no service request was recieved was recieved for the last 60 seconds");
	strcat(closeAns, "\n");

	write(STDOUT_FILENO, closeAns, strlen(closeAns));
	exit(0);
}



int main(int argc, char* argv[]) {
	signal(SIGUSR1, handlerToServer);
	signal(SIGALRM, signalHandle); // Register signal handler

	alarm(60);  // Scheduled alarm after 60 seconds
	while (1)
	{
		pause();
	}
	exit(0);
}