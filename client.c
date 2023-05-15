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
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>

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
void writeTosrv(char* argv[]) {
	int fsToServer = open("toSrv.txt", O_CREAT | O_RDWR, 0666);
	if (fsToServer < 0) {
		perror("error from ex2\n");
		exit(-1);
	}

	char* data = (char*)malloc(50 * sizeof(char));
	char pid[7];
	sprintf(pid, "%d", getpid());
	strcpy(data, pid);
	strcat(data, "\n");
	strcat(data, argv[2]);
	strcat(data, "\n");
	strcat(data, argv[3]);
	strcat(data, "\n");
	strcat(data, argv[4]);
	write(fsToServer, data, strlen(data));
	free(data);
	alarm(30); 
	kill(atoi(argv[1]), SIGUSR1);
}

int getRandom() {
	return rand() % 6;
}
void removeToSrv() {
	if (access("toSrv.txt", F_OK) == 0) {
		int pidfork = fork();
		if (pidfork < 0) {
			perror("error from ex2\n");
			exit(-1);
		}
		else {
			if (pidfork == 0) {
				// remove toSrv.txt
				char* argcRemove[] = { "rm","toSrv.txt",NULL };
				execvp(argcRemove[0], argcRemove);
			}
			else {
				wait(NULL);
				exit(0);
			}
		}
	}
}
void createToSrvFile(char* argv[]) {
	int r;
	int i;

	for (i = 0; i < 10; i++) {
		if (access("toSrv.txt", F_OK) == 0) {
			// file already exists
			r = getRandom();
			printf("failed to create toSrv.txt in the %d time\n", i + 1);
			sleep(r);
		}else {
			writeTosrv(argv);		
			break;
		}
	}
	if (i == 10) {
		printf("error from ex2\n");
		removeToSrv();
		exit(0);
	}
}


void resultHandler(int dummy) {
	char result[50] = { 0 };
	char* clientFileName = (char*)malloc(50 * sizeof(char));
	strcpy(clientFileName, "toClient");
	char pid[7];
	sprintf(pid, "%d", getpid());
	strcat(clientFileName, pid);
	strcat(clientFileName, ".txt");


	int serverResult = open(clientFileName, O_RDWR, 0666);
	if (serverResult < 0) {
		perror("error from ex2\n");
		exit(-1);
	}
	ReadLine(serverResult, result);
	
	int pidfork = fork();
	if (pidfork < 0) {
		perror("error from ex2\n");
		exit(-1);
	}
	else {
		if (pidfork == 0) {
			// remove toSrv.txt
			char* argcRemove[] = { "rm",clientFileName,NULL };
			execvp(argcRemove[0], argcRemove);
		}
		else {
			wait(NULL);
			char* resultAns = (char*)malloc(50 * sizeof(char));
			strcpy(resultAns, "Result:");
			strcat(resultAns, result);
			strcat(resultAns, "\n");
			write(STDOUT_FILENO, resultAns, strlen(resultAns));
			alarm(0);
			alarm(30);
			free(resultAns);
		}
	}
	exit(0);
}
void signalHandle(int signum) {
	char* closeAns = (char*)malloc(110 * sizeof(char));
	strcpy(closeAns, "Client closed beacuse of no response was recieved from the server for 30 seconds");
	strcat(closeAns, "\n");
	write(STDOUT_FILENO, closeAns, strlen(closeAns));
	removeToSrv();
}

int main(int argc, char* argv[]) {
	time_t t;
	srand((unsigned)time(&t));
	signal(SIGALRM, signalHandle); 
	signal(SIGUSR2, resultHandler);
	sleep(10);

	createToSrvFile(argv); 	// create file with 4 params
	pause();
	exit(0);
}