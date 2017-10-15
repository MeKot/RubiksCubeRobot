#include "executors.h"
#include "BrickPi.h"
#include "tick.h"

typedef struct move* list_iter;

typedef struct move {
	char type;
	uint8_t isDouble;
	uint8_t isReverse;
	struct move* next;
} move_s;

struct move_list {
	struct move* header;
	struct move* footer;
};

void removeExtraCharacters(char *str) {
    int ctr = 0;
		int i;
    for (i = 0; str[i]; i++) {
        if (str[i] != ' ' && str[i] != '#' && str[i] != '\n') {
            str[ctr] = str[i];
            ctr++;
        }
    }
    str[ctr] = '\0';
}

struct move* list_alloc_elem(void) {
	struct move* elem = (struct move*) malloc(sizeof(struct move));
	if (elem == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	return elem;
}

void list_free_elem(struct move* elem) {
	free(elem);
}

void add_move(struct move_list* l, char moveType, uint8_t isD, uint8_t isR) {
	struct move* new_move = list_alloc_elem();
	new_move->type = moveType;
	new_move->isDouble = isD;
	new_move->isReverse = isR;

	l->footer->next = new_move;
	l->footer = new_move;
	new_move->next = NULL;
}

void list_init(struct move_list* l) {
	l->header = list_alloc_elem();
	l->footer = l->header;
	l->footer->next = NULL;
}

void list_destroy(struct move_list* l) {
	list_iter iter = l->header;
	while(iter != NULL) {
		list_iter next = iter->next;
		list_free_elem(iter);
		iter = next;
	}
}

void remapVert(move_s* toRemap) {
	if (toRemap == NULL) return;
  
	switch(toRemap->type) {
		case 'U' : toRemap->type = 'L'; break;
		case 'L' : toRemap->type = 'D'; break;
		case 'D' : toRemap->type = 'R'; break;
		case 'R' : toRemap->type = 'U'; break;
	}
  if (toRemap->next != NULL) {
    remapVert(toRemap->next);
  }
}

void remapClock(move_s* toRemap) {
	if (toRemap == NULL) return;

	switch(toRemap->type) {
		case 'F' : toRemap->type = 'L'; break;
		case 'L' : toRemap->type = 'B'; break;
		case 'B' : toRemap->type = 'R'; break;
		case 'R' : toRemap->type = 'F'; break;
	}
  if (toRemap->next != NULL) {
    remapClock(toRemap->next);
  }
}

void remapAnti(move_s* toRemap) {
	if (toRemap == NULL) return;

	switch(toRemap->type) {
		case 'F' : toRemap->type = 'R'; break;
		case 'L' : toRemap->type = 'F'; break;
		case 'B' : toRemap->type = 'L'; break;
		case 'R' : toRemap->type = 'B'; break;
	}
  if (toRemap->next != NULL) {
    remapAnti(toRemap->next);
  }
}

void executeMove(move_s toRemap) {
  lock();
  if(toRemap.isReverse) {
    horizontalClockwise();
  } else if (toRemap.isDouble) {
    horizontalDouble();
  } else {
    horizontalAnticlockwise();
  }
  unlock();
}

void execute(move_s* toRemap) {
	switch(toRemap->type) {
		case 'R':
      flip();
		  flip();
      flip();
      executeMove(*toRemap);
			if (toRemap->next != NULL) {
        remapVert(toRemap->next);
        remapVert(toRemap->next);
        remapVert(toRemap->next);
			}
		 	break;
		case 'U':
			flip();
			flip();
		  executeMove(*toRemap);
			if (toRemap->next != NULL) {
        remapVert(toRemap->next);
        remapVert(toRemap->next);
			}
	    break;
		case 'L':
		  flip();
			executeMove(*toRemap);
			if (toRemap->next != NULL) {
        remapVert(toRemap->next);
			}
			break;
		case 'D':
      executeMove(*toRemap);
			break;
		case 'F':
      horizontalClockwiseFree();
	  	flip();
      executeMove(*toRemap);
			if (toRemap->next != NULL) {
        remapVert(toRemap->next);
        remapClock(toRemap->next);
			}
	  	break;
		case 'B':
      horizontalAnticlockwiseFree();
		  flip();
      executeMove(*toRemap);
			if (toRemap->next != NULL) {
        remapVert(toRemap->next);
        remapAnti(toRemap->next);
			}
			break;
	}
	printf("%c\n", toRemap->type);
  if(toRemap->next == NULL) {
    return;
  }
//recursive step
  execute(toRemap->next);
}

struct move_list parse(struct move_list* moves, char* s) {

  while(*s != '\0') {

    if (*s == '2' || *s == '\'') {
      s++;
    } else {
      if (*(s+1) == '2') {
        add_move(moves, *s, 1, 0);
      } else if (*(s+1) == '\'') {
        add_move(moves, *s, 0, 1);
      } else {
        add_move(moves, *s, 0, 0);
      }
      s++;
    }

  }

}



int main(int argc, char** argv) {
  FILE* fp;
  char string[100];
  fp = fopen(argv[1], "r");
  if (fp) {
    fgets(string, 100, fp);
  } else {
    perror("Could not open file");
    exit(EXIT_FAILURE);
  }

  struct move_list moves_struct;
	struct move_list* moves = &moves_struct;
  list_init(moves);

	removeExtraCharacters(string);

  parse(moves, string);

	//Actual main
	ClearTick();
	int result = BrickPiSetup();
	printf("BrickPiSetup: %d \n", result);

	if (result) return EXIT_FAILURE;

	BrickPi.Address[0] = 1;
	BrickPi.Address[1] = 2;

	BrickPi.MotorEnable[ARM_MOTOR]  = 1;
	BrickPi.MotorEnable[BASE_MOTOR] = 1;

	result = BrickPiSetupSensors();
	printf("BrickPiSetupSensors; %d \n", result);

	BrickPi.Timeout = 100;
	BrickPiSetTimeout();
	execute(moves->header);
  //lock();
  //printf("Angle after lock:%d\n", (BrickPi.Encoder[ARM_MOTOR] % 720) / 2);
	//horizontalDouble();
	//printf("Angle before unlock: %d\n", BrickPi.Encoder[ARM_MOTOR] % 720 / 2);
	//unlock();
  //printf("Angle after unlock:%d\n", BrickPi.Encoder[ARM_MOTOR] % 720 / 2);
	//flip();
	//lock();
	//horizontalClockwise();
	//unlock();
	//flip();
	//flip();
//horizontalClockwiseFree();
//horizontalDoubleFree();










	return EXIT_SUCCESS;


}
//anchor
