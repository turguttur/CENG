#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <limits.h>
#include <math.h>

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

typedef struct ph_message {
	coordinate move_request;
} ph_message;

int calculateManhattanDistance(int x_hunter, int y_hunter, int x_prey, int y_prey) {
	int manhattanDistance = abs(x_hunter - x_prey) + abs(y_prey - y_hunter);
	return manhattanDistance;
}

int req_positionIsEmpty(int requestedPosition_x, int requestedPosition_y, int objectCount, coordinate objectPosition[4]) {
	int isEmpty = 1;
	for (int i = 0; i < objectCount; i++) {
		if (requestedPosition_x == objectPosition[i].x && requestedPosition_y == objectPosition[i].y) {
			isEmpty = 0;
			break;
		}
	}
	return isEmpty;
}

int main(int argc, char const *argv[])
{
	int mapWidth, mapHeight;
	mapWidth = atoi(argv[1]);
	mapHeight = atoi(argv[2]);
	while(1) {
		server_message message;
		if (read(0, &message, sizeof(server_message))) {
			ph_message request;
			int currentPosx = message.pos.x;
			int currentPosy = message.pos.y;
			int currentDistance = calculateManhattanDistance(currentPosx, currentPosy, message.adv_pos.x, message.adv_pos.y);
			int objectCount = message.object_count;

			if (currentPosx + 1 < mapHeight && req_positionIsEmpty(currentPosx + 1, currentPosy, objectCount, message.object_pos)
				&& calculateManhattanDistance(currentPosx + 1, currentPosy, message.adv_pos.x, message.adv_pos.y) < currentDistance) {
				request.move_request.x = currentPosx + 1;
				request.move_request.y = currentPosy;
			}

			else if (currentPosx - 1 >= 0 && req_positionIsEmpty(currentPosx - 1, currentPosy, objectCount, message.object_pos)
				&& calculateManhattanDistance(currentPosx - 1, currentPosy, message.adv_pos.x, message.adv_pos.y) < currentDistance) {
				request.move_request.x = currentPosx - 1;
				request.move_request.y = currentPosy;
			}

			else if (currentPosy + 1 < mapWidth && req_positionIsEmpty(currentPosx, currentPosy + 1, objectCount, message.object_pos)
				&& calculateManhattanDistance(currentPosx, currentPosy + 1, message.adv_pos.x, message.adv_pos.y) < currentDistance) {
				request.move_request.x = currentPosx;
				request.move_request.y = currentPosy + 1;
			}

			else if (currentPosy - 1 >= 0 && req_positionIsEmpty(currentPosx, currentPosy - 1, objectCount, message.object_pos)
				&& calculateManhattanDistance(currentPosx, currentPosy - 1, message.adv_pos.x, message.adv_pos.y) < currentDistance) {
				request.move_request.x = currentPosx;
				request.move_request.y = currentPosy - 1;
			}

			else {
				request.move_request.x = currentPosx;
				request.move_request.y = currentPosy;
			}

			write(1, &request, sizeof(ph_message));
		}

		else {
			fprintf(stderr, "%s\n", "ERROR: Reading server message from server is failed! (Hunter)");
		}
		usleep(10000*(1+rand()%9));
	}
	return 0;
}