#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <math.h>
#include <limits.h>
#define PIPE(fd) socketpair(AF_UNIX, SOCK_STREAM, PF_UNIX, fd)

typedef struct coordinate {
	int x;
	int y;
} coordinate;

typedef struct server_message {
	coordinate pos;
	coordinate adv_pos;
	int object_count;
	coordinate object_pos[4];
} server_message;

typedef struct Hunters {
	coordinate hunterPosition;
	int hunterEnergy;
	int aliveHunter;
} Hunters;

typedef struct Preys {
	coordinate preyPosition;
	int preyEnergy;
	int alivePrey;
} Preys;

typedef struct ph_message {
	coordinate move_request;
} ph_message;

void printMap(char *mapArray, int mapWidth, int mapHeight) {
	printf("%c", '+');
	for (int i = 0; i < mapWidth; i++) 
		printf("%c", '-');
	printf("%c\n",'+' );

	for (int i = 0; i < mapHeight; i++) { 
		printf("%c", '|');
		for (int j = 0; j < mapWidth; j++) { 
			printf("%c", *((mapArray+i*mapWidth) + j));
		}
		printf("%c\n", '|');
	}	
	printf("%c", '+');
	for (int i = 0; i < mapWidth; i++)
		printf("%c", '-');
	printf("%c\n", '+');	
}

int calculateManhattanDistance(int x_hunter, int y_hunter, int x_prey, int y_prey) {
	int manhattanDistance = abs(x_hunter - x_prey) + abs(y_prey - y_hunter);
	return manhattanDistance;
}

coordinate findNearestPrey_forHunter(coordinate hunterPosition, Preys *preyArray, int preyArrayLength) {
	int mh = INT_MAX;
	int x_hunter = hunterPosition.x;
	int y_hunter = hunterPosition.y;
	coordinate nearestPrey;
	nearestPrey.x = -1;
	nearestPrey.y = -1;
	for (int i = 0; i < preyArrayLength; i++) {
		int x_prey = preyArray[i].preyPosition.x;
		int y_prey = preyArray[i].preyPosition.y;
		if (x_prey != INT_MAX && y_prey != INT_MAX) {
			if(calculateManhattanDistance(x_hunter, y_hunter, x_prey, y_prey) < mh) {
				mh = calculateManhattanDistance(x_hunter, y_hunter, x_prey, y_prey);
				nearestPrey.x = x_prey;
				nearestPrey.y = y_prey;
			}
		}
	}
	return nearestPrey;
}

coordinate findNearestHunter_forPrey(coordinate preyPosition, Hunters *hunterArray, int hunterArrayLength) {
	int mh = INT_MAX;
	int x_prey = preyPosition.x;
	int y_prey = preyPosition.y;
	coordinate nearestHunter;
	nearestHunter.x = -1;
	nearestHunter.y = -1;
	for (int i = 0; i < hunterArrayLength; i++) {
		int x_hunter = hunterArray[i].hunterPosition.x;
		int y_hunter = hunterArray[i].hunterPosition.y;
		if (x_hunter != INT_MAX && y_hunter != INT_MAX) {
			if (calculateManhattanDistance(x_hunter, y_hunter, x_prey, y_prey) < mh) {
				mh = calculateManhattanDistance(x_hunter, y_hunter, x_prey, y_prey);
				nearestHunter.x = x_hunter;
				nearestHunter.y = y_hunter;
			}
		}
	}
	return nearestHunter;
}

int objectCountAroundHunter(coordinate hunterPosition, Hunters *hunterArray, int hunterArrayLength, coordinate *obstacleArray, int obstacleArrayLength, coordinate objectPositionArray[4]) {
	int count = 0; 
	int x_hunter = hunterPosition.x; 
	int y_hunter = hunterPosition.y;
	coordinate tmpObjectCoordinate;
	for (int i = 0; i < obstacleArrayLength; i++) {
		if ((x_hunter + 1 == obstacleArray[i].x) && (y_hunter == obstacleArray[i].y)) {
			tmpObjectCoordinate.x = obstacleArray[i].x;
			tmpObjectCoordinate.y = obstacleArray[i].y;
			objectPositionArray[count] = tmpObjectCoordinate;
			count++;
		}
		else if ((x_hunter - 1 == obstacleArray[i].x) && (y_hunter == obstacleArray[i].y)) {
			tmpObjectCoordinate.x = obstacleArray[i].x;
			tmpObjectCoordinate.y = obstacleArray[i].y;
			objectPositionArray[count] = tmpObjectCoordinate;
			count++;
		}
		else if ((x_hunter == obstacleArray[i].x) && (y_hunter + 1 == obstacleArray[i].y)) {
			tmpObjectCoordinate.x = obstacleArray[i].x;
			tmpObjectCoordinate.y = obstacleArray[i].y;
			objectPositionArray[count] = tmpObjectCoordinate;
			count++;
		}
		else if ((x_hunter == obstacleArray[i].x) && (y_hunter - 1 == obstacleArray[i].y)) {
			tmpObjectCoordinate.x = obstacleArray[i].x;
			tmpObjectCoordinate.y = obstacleArray[i].y;
			objectPositionArray[count] = tmpObjectCoordinate;
			count++;
		}
	}

	for (int i = 0; i < hunterArrayLength; i++) {
		if (x_hunter + 1 == hunterArray[i].hunterPosition.x && y_hunter == hunterArray[i].hunterPosition.y) {
			tmpObjectCoordinate.x = x_hunter + 1;
			tmpObjectCoordinate.y = y_hunter;
			objectPositionArray[count] = tmpObjectCoordinate;
			count++;
		}
		else if (x_hunter - 1 == hunterArray[i].hunterPosition.x && y_hunter == hunterArray[i].hunterPosition.y) {
			tmpObjectCoordinate.x = x_hunter - 1;
			tmpObjectCoordinate.y = y_hunter;
			objectPositionArray[count] = tmpObjectCoordinate;
			count++;
		}
		else if (x_hunter == hunterArray[i].hunterPosition.x && y_hunter + 1 == hunterArray[i].hunterPosition.y) {
			tmpObjectCoordinate.x = x_hunter;
			tmpObjectCoordinate.y = y_hunter + 1;
			objectPositionArray[count] = tmpObjectCoordinate;
			count++;
		}
		else if (x_hunter == hunterArray[i].hunterPosition.x && y_hunter - 1 == hunterArray[i].hunterPosition.y) {
			tmpObjectCoordinate.x = x_hunter;
			tmpObjectCoordinate.y = y_hunter - 1;
			objectPositionArray[count] = tmpObjectCoordinate;
			count++;
		}
	}
	return count;
}

int objectCountAroundPrey(coordinate preyPosition, Preys *preyArray, int preyArrayLength, coordinate *obstacleArray, int obstacleArrayLength, coordinate objectPositionArray[4]) {
	int count = 0;
	int x_prey = preyPosition.x; 
	int y_prey = preyPosition.y;
	coordinate tmpObjectCoordinate;
	for (int i = 0; i < obstacleArrayLength; i++) {
		if (x_prey + 1 == obstacleArray[i].x && y_prey == obstacleArray[i].y) {
			tmpObjectCoordinate.x = obstacleArray[i].x;
			tmpObjectCoordinate.y = obstacleArray[i].y;
			objectPositionArray[count] = tmpObjectCoordinate;
			count++;
		}
		else if (x_prey - 1 == obstacleArray[i].x && y_prey == obstacleArray[i].y) {
			tmpObjectCoordinate.x = obstacleArray[i].x;
			tmpObjectCoordinate.y = obstacleArray[i].y;
			objectPositionArray[count] = tmpObjectCoordinate;
			count++;
		}
		else if (x_prey == obstacleArray[i].x && y_prey + 1 == obstacleArray[i].y) {
			tmpObjectCoordinate.x = obstacleArray[i].x;
			tmpObjectCoordinate.y = obstacleArray[i].y;
			objectPositionArray[count] = tmpObjectCoordinate;
			count++;
		}
		else if (x_prey == obstacleArray[i].x && y_prey - 1 == obstacleArray[i].y) {
			tmpObjectCoordinate.x = obstacleArray[i].x;
			tmpObjectCoordinate.y = obstacleArray[i].y;
			objectPositionArray[count] = tmpObjectCoordinate;
			count++;
		}
	}

	for (int i = 0; i < preyArrayLength; i++) {
		if (x_prey + 1 == preyArray[i].preyPosition.x && y_prey == preyArray[i].preyPosition.y) {
			tmpObjectCoordinate.x = preyArray[i].preyPosition.x;
			tmpObjectCoordinate.y = preyArray[i].preyPosition.y;
			objectPositionArray[count] = tmpObjectCoordinate;
			count++;
		}
		else if (x_prey - 1 == preyArray[i].preyPosition.x && y_prey == preyArray[i].preyPosition.y) {
			tmpObjectCoordinate.x = preyArray[i].preyPosition.x;
			tmpObjectCoordinate.y = preyArray[i].preyPosition.y;
			objectPositionArray[count] = tmpObjectCoordinate;
			count++;
		}
		else if (x_prey == preyArray[i].preyPosition.x && y_prey + 1 == preyArray[i].preyPosition.y) {
			tmpObjectCoordinate.x = preyArray[i].preyPosition.x;
			tmpObjectCoordinate.y = preyArray[i].preyPosition.y;
			objectPositionArray[count] = tmpObjectCoordinate;
			count++;
		}
		else if (x_prey == preyArray[i].preyPosition.x && y_prey - 1 == preyArray[i].preyPosition.y) {
			tmpObjectCoordinate.x = preyArray[i].preyPosition.x;
			tmpObjectCoordinate.y = preyArray[i].preyPosition.y;
			objectPositionArray[count] = tmpObjectCoordinate;
			count++;
		}
	}

	return count;
}

int isValidMove_Hunter(int requestedPosx, int requestedPosy, coordinate *obstacleArray, int obstacleArrayLength, Hunters *hunterArray, int hunterArrayLength) {
	for (int i = 0; i < hunterArrayLength; i++) {
		if (requestedPosx == hunterArray[i].hunterPosition.x && requestedPosy == hunterArray[i].hunterPosition.y) {
			return 0;
		}
	}

	// not necessary, but i did it
	for (int i = 0; i < obstacleArrayLength; i++) {
		if (requestedPosx == obstacleArray[i].x && requestedPosy == obstacleArray[i].y) 
			return 0;
	}
	return 1;
}

int isValidMove_Prey(int requestedPosx, int requestedPosy, coordinate *obstacleArray, int obstacleArrayLength, Preys *preyArray, int preyArrayLength) {
	for (int i = 0; i < preyArrayLength; i++) {
		if (requestedPosx == preyArray[i].preyPosition.x && requestedPosy == preyArray[i].preyPosition.y) {
			return 0;
		}
	}

	for (int i = 0;  i < obstacleArrayLength; i++) {
		
		if (requestedPosx == obstacleArray[i].x && requestedPosy == obstacleArray[i].y) {
			return 0;
		}
	}
	return 1;
}

int getHunterIdx_withGivenPoisiton(int posx, int posy, Hunters *hunterArray, int hunterArrayLength) {
	for (int i = 0; i < hunterArrayLength; i++) {
		if (posx == hunterArray[i].hunterPosition.x && posy == hunterArray[i].hunterPosition.y)
			return i; 
	}
	return -1;  // error
}

int getPreyIdx_withGivenPosition(int posx, int posy, Preys *preyArray, int preyArrayLength) {
	for (int i = 0; i < preyArrayLength; i++) {
		if (posx == preyArray[i].preyPosition.x && posy == preyArray[i].preyPosition.y)
			return i; 
	}
}

int isOccupiedByHunter(int posx, int posy, Hunters *hunterArray, int hunterArrayLength) {
	for (int i = 0; i < hunterArrayLength; i++) {
		if (hunterArray[i].hunterPosition.x == posx && hunterArray[i].hunterPosition.y == posy) {
			return 1;
		}
	}
	return 0;
}

int isOccupiedByPrey(int posx, int posy, Preys *preyArray, int preyArrayLength) {
	for (int i = 0; i < preyArrayLength; i++) {
		if (preyArray[i].preyPosition.x == posx && preyArray[i].preyPosition.y == posy) {
			return 1;
		}
	}
	return 0;
}

int main(int argc, char const *argv[])
{
	// *******************************************************************************************************************************************************************************
	// Reading Input From Terminal ***************************************************************************************************************************************************
	// *******************************************************************************************************************************************************************************

	int mapWidth;
	int mapHeight;
	int numberOfObstacles;
	int numberOfHunters;
	int numberOfPreys;
	int tmpx, tmpy, tmpEnergy;

	scanf("%d %d", &mapWidth, &mapHeight);
	char mapArray[mapHeight][mapWidth];
	
	// Filling Map with Space Character
	for (int i = 0; i < mapHeight; i++) {
		for (int j = 0; j < mapWidth; j++) {
			mapArray[i][j] = ' ';
		}
	}

	// Reading Obstacles and Create Obstacle Array
	scanf("%d", &numberOfObstacles);
	coordinate obstacleArray[numberOfObstacles];
	for (int i = 0; i < numberOfObstacles; i++) {
		scanf("%d %d", &tmpx, &tmpy);
		obstacleArray[i].x = tmpx;
		obstacleArray[i].y = tmpy;
		mapArray[tmpx][tmpy] = 'X';
	}

	// Reading Hunters and Create Hunter Array
	scanf("%d", &numberOfHunters);
	Hunters hunterArray[numberOfHunters];
	for (int i = 0; i < numberOfHunters; i++) {
		scanf("%d %d %d", &tmpx, &tmpy, &tmpEnergy);
		hunterArray[i].hunterPosition.x = tmpx;
		hunterArray[i].hunterPosition.y = tmpy;
		hunterArray[i].hunterEnergy = tmpEnergy;
		hunterArray[i].aliveHunter = 1;
		mapArray[tmpx][tmpy] = 'H';
	}

	// Reading Prey and Create Prey Array
	scanf("%d", &numberOfPreys);
	Preys preyArray[numberOfPreys];
	for (int i = 0; i < numberOfPreys; i++) {
		scanf("%d %d %d", &tmpx, &tmpy, &tmpEnergy);
		preyArray[i].preyPosition.x = tmpx;
		preyArray[i].preyPosition.y = tmpy;
		preyArray[i].preyEnergy = tmpEnergy;
		preyArray[i].alivePrey = 1;
		mapArray[tmpx][tmpy] = 'P';
	}

	// *******************************************************************************************************************************************************************************
	// *******************************************************************************************************************************************************************************
	// *******************************************************************************************************************************************************************************
	//
	//
	// *******************************************************************************************************************************************************************************
	// ICP(Intercommunication Process) ***********************************************************************************************************************************************
	// *******************************************************************************************************************************************************************************

	// file descritors of hunters and preys
	//int fd_hunters[numberOfHunters][2];
	//int fd_preys[numberOfPreys][2];
	int file_desc[numberOfPreys + numberOfHunters][2];
	int hunterArrayLength = sizeof(hunterArray)/sizeof(hunterArray[0]);
	int preyArrayLength   = sizeof(preyArray)/sizeof(preyArray[0]);
	int obstacleArrayLength = sizeof(obstacleArray)/sizeof(obstacleArray[0]);

	// CREATING PIPES FOR HUNTER AND PREY
	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	for (int i = 0; i < numberOfHunters + numberOfPreys; i++) {
		PIPE(file_desc[i]);
	}
	
	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//
	//
	// CREATING HUNTER AND PREY CHILD PROCESSES
	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------	

	pid_t hunterPidArray[numberOfHunters];
	for (int i = 0; i < numberOfHunters; i++) {
		if ((hunterPidArray[i] = fork()) == 0) {
			dup2(file_desc[i][0], 0);
			dup2(file_desc[i][0], 1);
			char *newargs[4];
			char mapW[20];
			sprintf(mapW, "%d", mapWidth);
			char mapH[20];
			sprintf(mapH, "%d", mapHeight);
			newargs[0] = "hunter";
			newargs[1] = mapW;
			newargs[2] = mapH;
			newargs[3] = NULL;
			execv(newargs[0], newargs);
		}
	}

	pid_t preyPidArray[numberOfPreys];
	for (int i = 0; i < numberOfPreys; i++) {
		if ((preyPidArray[i] = fork()) == 0) {
			//close(file_desc[i + numberOfHunters][1]);
			dup2(file_desc[i + numberOfHunters][0], 0);
			dup2(file_desc[i + numberOfHunters][0], 1);
			char *newargs[4];
			char mapW[20];
			sprintf(mapW, "%d", mapWidth);
			char mapH[20];
			sprintf(mapH, "%d", mapHeight);
			newargs[0] = "prey";
			newargs[1] = mapW;
			newargs[2] = mapH;
			newargs[3] = NULL;
			execv(newargs[0], newargs);
		}
	}

	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// --------
	// --------
	// SENDING INITIAL MESSAGES TO HUNTER AND PREY
	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	for (int i = 0; i < numberOfHunters; i++) {
		server_message message;
		message.pos = hunterArray[i].hunterPosition;
		coordinate manhattanDistance = findNearestPrey_forHunter(hunterArray[i].hunterPosition, preyArray, preyArrayLength);
		message.adv_pos = manhattanDistance;
		coordinate objectPositionArray[4];
		int objectCount = objectCountAroundHunter(hunterArray[i].hunterPosition ,hunterArray, hunterArrayLength, 
														obstacleArray, obstacleArrayLength, objectPositionArray);
		message.object_count = objectCount;
		for (int j = 0; j < objectCount; j++) {
			message.object_pos[j] = objectPositionArray[j];
		}

		write(file_desc[i][1], &message, sizeof(server_message));
	}

	for (int i = 0; i < numberOfPreys; i++) {
		server_message message;
		message.pos = preyArray[i].preyPosition;
		coordinate manhattanDistance = findNearestHunter_forPrey(preyArray[i].preyPosition, hunterArray, hunterArrayLength);
		message.adv_pos = manhattanDistance;
		coordinate objectPositionArray[4];
		int objectCount = objectCountAroundPrey(preyArray[i].preyPosition, preyArray, preyArrayLength,
														obstacleArray, obstacleArrayLength, objectPositionArray);
		message.object_count = objectCount;
		for (int j = 0; j < objectCount; j++) {
			message.object_pos[j] = objectPositionArray[j];
		}

		write(file_desc[i + numberOfHunters][1], &message, sizeof(server_message));
	}


	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	printMap((char*)mapArray, mapWidth, mapHeight);
	fflush(stdout);

	fd_set readset;
	int hunterCount = numberOfHunters;
	int preyCount   = numberOfPreys;
	int fd_size = sizeof(file_desc)/sizeof(file_desc[0]);
	//fprintf(stderr, "%d %d %d\n", hunterCount, preyCount, fd_size);

	int max = 0;
	for (int i = 0; i < numberOfHunters + numberOfPreys; i++) {
		if (file_desc[i][1] > max) {
			max = file_desc[i][1];
		}
	}
	//fprintf(stderr, "%d\n", max);
	max += 1;
	//fprintf(stderr, "%d\n", max);

	while(1) {
		if (hunterCount == 0 || preyCount == 0) {
			for (int i = 0; i < numberOfHunters + numberOfPreys; i++) {
				close(file_desc[i][1]);
				close(file_desc[i][0]);
			}

			for (int i = 0; i < numberOfHunters; i++) {
				if (hunterArray[i].aliveHunter) {
					kill(hunterPidArray[i], SIGTERM);
				}
			}

			for (int i = 0; i < numberOfPreys; i++) {
				if (preyArray[i].alivePrey) {
					kill(preyPidArray[i], SIGTERM);
				}
			}
			break;
		}

		FD_ZERO(&readset);

		for (int i = 0; i < numberOfHunters; i++) {
			if (hunterArray[i].aliveHunter) {
				FD_SET(file_desc[i][1], &readset);
			}
		}
		for (int i = 0; i < numberOfPreys; i++) {
			if (preyArray[i].alivePrey) {
				FD_SET(file_desc[i + numberOfHunters][1], &readset);
			}
		}

		select(max, &readset, NULL, NULL, NULL);

		for (int i = 0; i < numberOfHunters; i++) { 

			if (FD_ISSET(file_desc[i][1], &readset)) {
				ph_message request;
				if (read(file_desc[i][1], &request, sizeof(ph_message)) >= 0) {
					int oldPosx = hunterArray[i].hunterPosition.x;
					int oldPosy = hunterArray[i].hunterPosition.y;
					int requestedPosx = request.move_request.x;
					int requestedPosy = request.move_request.y;
					server_message message;
					if (isValidMove_Hunter(requestedPosx, requestedPosy, obstacleArray, obstacleArrayLength, hunterArray, hunterArrayLength)) {
						if (isOccupiedByPrey(requestedPosx, requestedPosy, preyArray, preyArrayLength)) {
							int preyIdx = getPreyIdx_withGivenPosition(requestedPosx, requestedPosy, preyArray, preyArrayLength);
							preyArray[preyIdx].preyPosition.x = INT_MAX;
							preyArray[preyIdx].preyPosition.y = INT_MAX;
							hunterArray[i].hunterPosition.x = requestedPosx;
							hunterArray[i].hunterPosition.y = requestedPosy;
							message.pos = hunterArray[i].hunterPosition;
							findNearestPrey_forHunter(hunterArray[i].hunterPosition, preyArray, preyArrayLength);
							coordinate objectPositionArray[4];
							int objectCount = objectCountAroundHunter(hunterArray[i].hunterPosition, hunterArray, hunterArrayLength, 
																		obstacleArray, obstacleArrayLength, objectPositionArray);
							message.object_count = objectCount;
							for (int j = 0; j < objectCount; j++) {
								message.object_pos[j] = objectPositionArray[j];
							}

							hunterArray[i].hunterEnergy += (preyArray[preyIdx].preyEnergy - 1);								

							preyArray[preyIdx].alivePrey = 0;
							preyCount--;
							mapArray[oldPosx][oldPosy] = ' ';
							mapArray[requestedPosx][requestedPosy] = 'H';
							printMap((char*)mapArray, mapWidth, mapHeight);
							fflush(stdout);
							close(file_desc[preyIdx + numberOfHunters][1]);
							close(file_desc[preyIdx + numberOfHunters][0]);
							kill(preyPidArray[preyIdx], SIGTERM);  
							write(file_desc[i][1], &message, sizeof(server_message));
						}

						else {
							hunterArray[i].hunterPosition.x = requestedPosx;
							hunterArray[i].hunterPosition.y = requestedPosy;
							message.pos = hunterArray[i].hunterPosition;
							message.adv_pos = findNearestPrey_forHunter(hunterArray[i].hunterPosition, preyArray, preyArrayLength);
							coordinate objectPositionArray[4];
							int objectCount = objectCountAroundHunter(hunterArray[i].hunterPosition, hunterArray, hunterArrayLength, 
																		obstacleArray, obstacleArrayLength, objectPositionArray);
							message.object_count = objectCount;
							for (int j = 0; j < objectCount; j++) {
								message.object_pos[j] = objectPositionArray[j];
							}

							hunterArray[i].hunterEnergy -= 1;
							if (hunterArray[i].hunterEnergy > 0) {
								mapArray[oldPosx][oldPosy] = ' ';
								mapArray[requestedPosx][requestedPosy] = 'H';
								printMap((char*)mapArray, mapWidth, mapHeight);
								fflush(stdout);
								write(file_desc[i][1], &message, sizeof(server_message));
							}

							else {
								hunterArray[i].hunterPosition.x = INT_MAX;
								hunterArray[i].hunterPosition.y = INT_MAX;
								hunterArray[i].aliveHunter = 0;
								hunterCount--;
								mapArray[oldPosx][oldPosy] = ' ';
								mapArray[requestedPosx][requestedPosy] = ' ';
								printMap((char*)mapArray, mapWidth, mapHeight);
								fflush(stdout);
								close(file_desc[i][1]);
								close(file_desc[i][0]);
								kill(hunterPidArray[i], SIGTERM);
								write(file_desc[i][1], &message, sizeof(server_message));
								
							}
						}
					}
					else {
						// NOT VALID MOVE
						hunterArray[i].hunterPosition.x = oldPosx;
						hunterArray[i].hunterPosition.y = oldPosy;
						message.pos = hunterArray[i].hunterPosition;
						message.adv_pos = findNearestPrey_forHunter(hunterArray[i].hunterPosition, preyArray, preyArrayLength);
						coordinate objectPositionArray[4];
						int objectCount = objectCountAroundHunter(hunterArray[i].hunterPosition, hunterArray, hunterArrayLength, 
																	obstacleArray, obstacleArrayLength, objectPositionArray);
						message.object_count = objectCount;
						for (int j = 0; j < objectCount; j++) {
							message.object_pos[j] = objectPositionArray[j];
						}

						write(file_desc[i][1], &message, sizeof(server_message));
					}
				}
			}
		}

		
		for (int i = 0; i < numberOfPreys; i++) {

			if (FD_ISSET(file_desc[i + numberOfHunters][1], &readset)) {
				//sleep(1);
				ph_message request;
				if (read(file_desc[i + numberOfHunters][1], &request, sizeof(ph_message))>= 0) {
					int oldPosx = preyArray[i].preyPosition.x;
					int oldPosy = preyArray[i].preyPosition.y;
					int requestedPosx = request.move_request.x;
					int requestedPosy = request.move_request.y;
					server_message message;
					if (isValidMove_Prey(requestedPosx, requestedPosy, obstacleArray, obstacleArrayLength, preyArray, preyArrayLength)) {
						if (isOccupiedByHunter(requestedPosx, requestedPosy, hunterArray, hunterArrayLength)) {
							// PREY GOES TO HIS OWN DEATH
							preyArray[i].preyPosition.x = INT_MAX;
							preyArray[i].preyPosition.y = INT_MAX;
							preyArray[i].alivePrey = 0;
							preyCount--;
							close(file_desc[i + numberOfHunters][1]);
							close(file_desc[i + numberOfHunters][0]);
							kill(preyPidArray[i], SIGTERM);
							//----------------------------------------------------------------
							mapArray[oldPosx][oldPosy] = ' ';
							printMap((char*)mapArray, mapWidth, mapHeight);
							fflush(stdout);
							write(file_desc[i + numberOfHunters][1], &message, sizeof(server_message));
						}
						else {
							preyArray[i].preyPosition.x = requestedPosx;
							preyArray[i].preyPosition.y = requestedPosy;
							message.pos = preyArray[i].preyPosition;
							message.adv_pos = findNearestHunter_forPrey(preyArray[i].preyPosition, hunterArray, hunterArrayLength);
							coordinate objectPositionArray[4];
							int objectCount = objectCountAroundPrey(preyArray[i].preyPosition, preyArray, preyArrayLength,
																		obstacleArray, obstacleArrayLength, objectPositionArray);
							message.object_count = objectCount;
							for (int j = 0; j < objectCount; j++) {
								message.object_pos[j] = objectPositionArray[j];
							}

							mapArray[oldPosx][oldPosy] = ' ';
							mapArray[requestedPosx][requestedPosy] = 'P';
							printMap((char*)mapArray, mapWidth, mapHeight);
							fflush(stdout);
							write(file_desc[i + numberOfHunters][1], &message, sizeof(server_message));
						}

					}		
					else {
						message.pos.x = oldPosx;
						message.pos.y = oldPosy;
						message.adv_pos = findNearestHunter_forPrey(preyArray[i].preyPosition, hunterArray, hunterArrayLength);
						coordinate objectPositionArray[4];
						int objectCount = objectCountAroundPrey(preyArray[i].preyPosition, preyArray, preyArrayLength, 
																	obstacleArray, obstacleArrayLength, objectPositionArray);
						message.object_count = objectCount;
						for (int j = 0; j < objectCount; j++) {
							message.object_pos[j] = objectPositionArray[j];
						}

						write(file_desc[i + numberOfHunters][1], &message, sizeof(server_message));
					}		
				}
			}
		}
		
	}

	return 0;
}
