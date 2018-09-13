/*
 * Program Name: Game of life
 * Developer: LiaoChen
 * ID: 17211401
 * School: BJTU
 *
 * Change log:
 * YYYY/MM/DD xxxxxxxxxx
 * 2018/05/01 Create it and build based structure.
 * 2018/05/03 Change the based structure, create new structure. Code several functions.
 * 2018/05/04 Code all of the functions. Fix some bug. Create command user interface.
 * 2018/05/07 Test Thread(using <pthread.h>) for linux(incomplete).
 * 2018/05/10 Create new NONE_CREATE function based on existed function to raise the program efficency.
 * 2018/05/14 Finish using the Thread and it run perfect.
 * 2018/05/26 Fix some bugs, code ShowMap()
*/

#include <stdio.h>
#include <stdlib.h>

#define RUN_IN_LINUX		0
#define THREADS_NUM		10

#define	UP	0
#define	DOWN	1
#define	RIGHT	2
#define	LEFT	3

#define	OK	1
#define	ERROR	0

#define	TRUE	1
#define	FALSE	0

#define BIGGER	0
#define EQUAL	1
#define SMALLER	2

#define NONE	0
#define ALIVE	1
#define TOALIVE	2
#define DIED	-1

#define MAXSIZE 10

#if RUN_IN_LINUX
#include <pthread.h>
#include <unistd.h>
#endif

typedef char Bool;
typedef char Status;
typedef char Cell;
typedef char Compare;

typedef struct Map{
	int x, y;
	Cell map[MAXSIZE][MAXSIZE];
	struct Map *list[4];
}Map;

typedef struct MapIndex{
	struct Map *data;
	struct MapIndex *next;
}MapIndex;

typedef struct MapIndexHead{
	Status hasSort;
	int num;
	struct MapIndex *next;
}MapIndexHead;

typedef struct CellLocation{
	struct Map *m;
	int x, y;
}CellLocation;

MapIndexHead mapIndexHead;

Status Debug();								//A function which show all the cell to debug
Status SetDefault(Map* m);						//Set *m map[*][*]=NONE and list[*]=NULL
Status CreateMap(Map *base, Map *new, int direction);			//Create link from base to new by direction
Status SortMap();							//Sort all the Maps for(x = min; x < max; x++){for(y = min; y < max; y++)}
Status ChangeCellStatus(CellLocation *c);				//Change cell status by judge how many cells around it
Status ChangeAroundStatus(CellLocation *c);				//Change cell around area status by judge how many cells around it
Status ChangeCellStatus_NoneCreate(CellLocation *c);			//Similar to ChangeCellStatus(), but it will not create new map, and it used in which we know it will not create new map
Status ChangeAroundStatus_NoneCreate(CellLocation *c);			//Similar to ChangeAroundStatus(), but it will not create new map, and it used in which we kone it will not create new map
Status NextStep(int *e);						//Do all of a step and return alive cell to *e
Status ShowMap();							//Print all the cell
int AmountAroundCell(CellLocation c);					//Return how many cells around this cell
int AmountAroundCell_NoneCreate(CellLocation c);			//Similar to AmountAroundCell(), but it will not create new map, and it used in which we konw it will not create new map
Map *NewMap();								//New a Map, add it to MapIndex and return itself
CellLocation CellDirection(CellLocation c, int direction);		//Retrun cell which is (direction) of it, if has no map in the direction, create new one
Compare CompareMap(Map a, Map b);					//Compare Map a(x, y) and Map b(x, y)
void ChooseSort();							//ChooseSort


int main(){
	int e;
	int i, j;
	char user;
	int mode = 0;

	//freopen("in.txt", "r", stdin);

	//Initialize
	mapIndexHead.next = NULL;
	mapIndexHead.hasSort = TRUE;
	mapIndexHead.num = 0;

	Map* base = NewMap();
	base->x = 0;
	base->y = 0;

	e = 0;

	//Input the cell
	printf("You can input the coordinate(x, y) to create cell.\n");
	printf("----------->\n");
	printf("|          y\n");
	printf("|           \n");
	printf("|           \n");
	printf("|           \n");
	printf("v x         \n");
	printf("\n");
	printf("0<= x <=%d\n0<= y <=%d\n", MAXSIZE-1, MAXSIZE-1);
	printf("Such as input:\n2 3\nmeans x=2 and y=3\n");
	printf("Input -1 to end input\n");
	printf("Please input the coordinate:\n");
	while(TRUE){
		scanf("%d", &i);
		if(i == -1)
			break;
		scanf("%d", &j);
		if(i < 0 || j < 0 || i >= MAXSIZE || j >= MAXSIZE){
			printf("OVER FLOW\n");
			continue;
		}else{
			base->map[i][j] = ALIVE;
			e++;
		}
	}
	getchar();			//Clear "\n"

	while(TRUE){
		if(mode == 0){
			printf("Now you have %d cells, and you want: \n", e);
		}else if(mode == 1){
			SortMap();
			ShowMap();
		}
		printf("p ---------- Print all of the cells\n");
		printf("n ---------- Go to the next step(Or you can also just press ENTER)\n");
		printf("c ---------- Change mode between text and picture\n");
		printf("h ---------- Get help\n");

		user = getchar();
		for(i = 0; i < 5; i++)
			printf(".\n");
		if(user == 'p'){
			SortMap();
			ShowMap();
		}else if(user == 'n'){
			printf("Running next step, please wait...\n");
			NextStep(&e);
			printf("Finish running!\n");
		}else if(user == 'h'){
			printf("================Developer: Liao Chen================\n");
		}else if(user == '\n'){
			printf("Running next step, please wait...\n");
			NextStep(&e);
			printf("Finish running!\n");
		}else if(user == 'c'){
			if(mode == 0)
				mode = 1;
			else if(mode == 1)
				mode = 0;
		}else{
			printf("ERROR ON INPUT, please input \'p\', \'n\' or \'h\'\n");
		}

		while(user != '\n' && getchar() != '\n');		//Clear the input flow
			for(i = 0; i < 100; i++)
			printf(".\n");
	}


	while(e < 10000){
		NextStep(&e);
		SortMap();
		//Debug();
		ShowMap();
		printf("//////%d\n", e);
		getchar();
	}
	return 0;
}

Status SetDefault(Map* m){
	int i, j;
	for(i = 0; i < MAXSIZE; i++)
		for(j = 0; j < MAXSIZE; j++)
			m->map[i][j] = NONE;
	for(i = 0; i < 4; i++)
		m->list[i] = NULL;
	return OK;
}

Status CreateMap(Map *base, Map *new, int direction){
	if(direction > 4 || direction < 0){
		printf("ERROR ON DIRECTION\n");
		return ERROR;
	}
	base->list[direction] = new;
	switch(direction){
		case UP:
			new->list[DOWN] = base;
			if(base->list[LEFT])
				if(base->list[LEFT]->list[UP]){
					base->list[LEFT]->list[UP]->list[RIGHT] = new;
					new->list[LEFT] = base->list[LEFT]->list[UP];
				}
			if(base->list[RIGHT])
				if(base->list[RIGHT]->list[UP]){
					base->list[RIGHT]->list[UP]->list[LEFT] = new;
					new->list[RIGHT] = base->list[RIGHT]->list[UP];
				}
			new->x = base->x-1;
			new->y = base->y;
			break;
		case DOWN:
			new->list[UP] = base;
			if(base->list[LEFT])
				if(base->list[LEFT]->list[DOWN]){
					base->list[LEFT]->list[DOWN]->list[RIGHT] = new;
					new->list[LEFT] = base->list[LEFT]->list[DOWN];
				}
			if(base->list[RIGHT])
				if(base->list[RIGHT]->list[UP]){
					base->list[RIGHT]->list[UP]->list[LEFT] = new;
					new->list[RIGHT] = base->list[RIGHT]->list[DOWN];
				}
			new->x = base->x+1;
			new->y = base->y;
			break;
		case LEFT:
			new->list[RIGHT] = base;
			if(base->list[UP])
				if(base->list[UP]->list[LEFT]){
					base->list[UP]->list[LEFT]->list[DOWN] = new;
					new->list[UP] = base->list[UP]->list[LEFT];
				}
			if(base->list[DOWN])
				if(base->list[DOWN]->list[LEFT]){
					base->list[DOWN]->list[LEFT]->list[UP] = new;
					new->list[DOWN] = base->list[DOWN]->list[LEFT];
				}
			new->x = base->x;
			new->y = base->y-1;
			break;
		case RIGHT:
			new->list[LEFT] = base;
			if(base->list[UP])
				if(base->list[UP]->list[RIGHT]){
					base->list[UP]->list[RIGHT]->list[DOWN] = new;
					new->list[UP] = base->list[UP]->list[RIGHT];
				}
			if(base->list[DOWN])
				if(base->list[DOWN]->list[RIGHT]){
					base->list[DOWN]->list[RIGHT]->list[UP] = new;
					new->list[DOWN] = base->list[DOWN]->list[RIGHT];
				}
			new->x = base->x;
			new->y = base->y+1;
			break;
		default:
			printf("ERROR ON DIRECTION\n");
			return ERROR;
			break;
	}
	return OK;
}

Map *NewMap(){
	Map* tmp = (Map*)malloc(sizeof(Map));
	MapIndex* tmpIndex = (MapIndex*)malloc(sizeof(MapIndex));
	SetDefault(tmp);
	tmpIndex->data = tmp;
	tmpIndex->next = mapIndexHead.next;
	mapIndexHead.next = tmpIndex;
	mapIndexHead.hasSort = FALSE;
	mapIndexHead.num++;
	return tmp;
}

CellLocation CellDirection(CellLocation c, int direction){
	CellLocation tmp;
	switch(direction){
		case UP:
			if(c.x == 0){
				if(c.m->list[UP] == NULL)
					CreateMap(c.m, NewMap(), UP);
				tmp.m = c.m->list[UP];
				tmp.x = MAXSIZE-1;
				tmp.y = c.y;
				return tmp;
			}else{
				tmp.m = c.m;
				tmp.x = c.x-1;
				tmp.y = c.y;
				return tmp;
			}
			break;
		case DOWN:
			if(c.x == MAXSIZE-1){
				if(c.m->list[DOWN] == NULL)
					CreateMap(c.m, NewMap(), DOWN);
				tmp.m = c.m->list[DOWN];
				tmp.x = 0;
				tmp.y = c.y;
				return tmp;
			}else{
				tmp.m = c.m;
				tmp.x = c.x+1;
				tmp.y = c.y;
				return tmp;
			}
			break;
		case LEFT:
			if(c.y == 0){
				if(c.m->list[LEFT] == NULL)
					CreateMap(c.m, NewMap(), LEFT);
				tmp.m = c.m->list[LEFT];
				tmp.x = c.x;
				tmp.y = MAXSIZE-1;
				return tmp;
			}else{
				tmp.m = c.m;
				tmp.x = c.x;
				tmp.y = c.y-1;
				return tmp;
			}
			break;
		case RIGHT:
			if(c.y == MAXSIZE-1){
				if(c.m->list[RIGHT] == NULL)
					CreateMap(c.m, NewMap(), RIGHT);
				tmp.m = c.m->list[RIGHT];
				tmp.x = c.x;
				tmp.y = 0;
				return tmp;
			}else{
				tmp.m = c.m;
				tmp.x = c.x;
				tmp.y = c.y+1;
				return tmp;
			}
			break;
		default:
			tmp.m = NULL;
			tmp.x = tmp.y = 0;
			return tmp;
			break;
	}
}

int AmountAroundCell(CellLocation c){
	int sum = 0;
	CellLocation up = CellDirection(c, UP);
	CellLocation down = CellDirection(c, DOWN);
	CellLocation left = CellDirection(c, LEFT);
	CellLocation right = CellDirection(c, RIGHT);
	CellLocation upl = CellDirection(up, LEFT);
	CellLocation upr = CellDirection(up, RIGHT);
	CellLocation downl = CellDirection(down, LEFT);
	CellLocation downr = CellDirection(down, RIGHT);

	if(up.m->map[up.x][up.y] == ALIVE || up.m->map[up.x][up.y] == DIED)
		sum++;
	if(down.m->map[down.x][down.y] == ALIVE || down.m->map[down.x][down.y] == DIED)
		sum++;
	if(left.m->map[left.x][left.y] == ALIVE || left.m->map[left.x][left.y] == DIED)
		sum++;
	if(right.m->map[right.x][right.y] == ALIVE || right.m->map[right.x][right.y] == DIED)
		sum++;
	if(upl.m->map[upl.x][upl.y] == ALIVE || upl.m->map[upl.x][upl.y] == DIED)
		sum++;
	if(upr.m->map[upr.x][upr.y] == ALIVE || upr.m->map[upr.x][upr.y] == DIED)
		sum++;
	if(downl.m->map[downl.x][downl.y] == ALIVE || downl.m->map[downl.x][downl.y] == DIED)
		sum++;
	if(downr.m->map[downr.x][downr.y] == ALIVE || downr.m->map[downr.x][downr.y] == DIED)
		sum++;
	return sum;
}

int AmountAroundCell_NoneCreate(CellLocation c){
	int sum = 0;
	Map* cMap = c.m;
	int cX = c.x;
	int cY = c.y;
	CellLocation up = {.m = cMap, .x = cX-1, .y = cY-1};
	CellLocation down = {.m = cMap, .x = cX-1, .y = cY};
	CellLocation left = {.m = cMap, .x = cX-1, .y = cY+1};
	CellLocation right = {.m = cMap, .x = cX, .y = cY-1};
	CellLocation upl = {.m = cMap, .x = cX, .y = cY+1};
	CellLocation upr = {.m = cMap, .x = cX+1, .y = cY-1};
	CellLocation downl = {.m = cMap, .x = cX+1, .y = cY};
	CellLocation downr = {.m = cMap, .x = cX+1, .y = cY+1};

	if(up.m->map[up.x][up.y] == ALIVE || up.m->map[up.x][up.y] == DIED)
		sum++;
	if(down.m->map[down.x][down.y] == ALIVE || down.m->map[down.x][down.y] == DIED)
		sum++;
	if(left.m->map[left.x][left.y] == ALIVE || left.m->map[left.x][left.y] == DIED)
		sum++;
	if(right.m->map[right.x][right.y] == ALIVE || right.m->map[right.x][right.y] == DIED)
		sum++;
	if(upl.m->map[upl.x][upl.y] == ALIVE || upl.m->map[upl.x][upl.y] == DIED)
		sum++;
	if(upr.m->map[upr.x][upr.y] == ALIVE || upr.m->map[upr.x][upr.y] == DIED)
		sum++;
	if(downl.m->map[downl.x][downl.y] == ALIVE || downl.m->map[downl.x][downl.y] == DIED)
		sum++;
	if(downr.m->map[downr.x][downr.y] == ALIVE || downr.m->map[downr.x][downr.y] == DIED)
		sum++;
	return sum;
}


Status ChangeAroundStatus(CellLocation *c){
	int i;
	CellLocation up = CellDirection(*c, UP);
	CellLocation down = CellDirection(*c, DOWN);
	CellLocation left = CellDirection(*c, LEFT);
	CellLocation right = CellDirection(*c, RIGHT);
	CellLocation upl = CellDirection(up, LEFT);
	CellLocation upr = CellDirection(up, RIGHT);
	CellLocation downl = CellDirection(down, LEFT);
	CellLocation downr = CellDirection(down, RIGHT);
	CellLocation list[8] = {up, down, left, right, upl, upr, downl, downr};

	for(i = 0; i < 8; i++)
		if(list[i].m->map[list[i].x][list[i].y] == NONE)
			switch(AmountAroundCell(list[i])){
				case 3:
					list[i].m->map[list[i].x][list[i].y] = TOALIVE;
					break;
				default:
					//Do nothing
					break;
			}

	return OK;
}

Status ChangeAroundStatus_NoneCreate(CellLocation *c){
	int i, cX, cY;
	Map *cMap = c->m;
	cX = c->x;
	cY = c->y;
	CellLocation aC = {.m = cMap, .x = cX-1, .y = cY-1};
	CellLocation bC = {.m = cMap, .x = cX-1, .y = cY};
	CellLocation cC = {.m = cMap, .x = cX-1, .y = cY+1};
	CellLocation dC = {.m = cMap, .x = cX, .y = cY-1};
	CellLocation eC = {.m = cMap, .x = cX, .y = cY+1};
	CellLocation fC = {.m = cMap, .x = cX+1, .y = cY-1};
	CellLocation gC = {.m = cMap, .x = cX+1, .y = cY};
	CellLocation hC = {.m = cMap, .x = cX+1, .y = cY+1};
	CellLocation list[8] = {aC, bC, cC, dC, eC, fC, gC, hC};

	for(i = 0; i < 8; i++)
		if(list[i].m->map[list[i].x][list[i].y] == NONE)
			switch(AmountAroundCell_NoneCreate(list[i])){
				case 2:
				case 3:
					list[i].m->map[list[i].x][list[i].y] = TOALIVE;
					break;
				default:
					//Do nothing
					break;
			}
	return OK;
}



Status ChangeCellStatus(CellLocation *c){
	switch(AmountAroundCell(*c)){
		case 0:
		case 1:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
			c->m->map[c->x][c->y] = DIED;
			break;
		case 2:
		case 3:
			break;
		default:
			printf("AMOUNT ERROR\n");
			return ERROR;
			break;
	}
	return OK;
}

Status ChangeCellStatus_NoneCreate(CellLocation *c){
	switch(AmountAroundCell_NoneCreate(*c)){
		case 0:
		case 1:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
			c->m->map[c->x][c->y] = DIED;
			break;
		case 2:
		case 3:
			break;
		default:
			printf("AMOUNT ERROR\n");
			return ERROR;
			break;
	}
	return OK;
}

#if RUN_IN_LINUX
int sum;
int pthread_alive;

MapIndex *mapIndex;
pthread_t threads[THREADS_NUM];
pthread_mutex_t flag;


void *GoWithThreadsOne(void *threadid){
	int i, j;
	CellLocation cellLocation;
	MapIndex *mapIndex_;

	//Judge the amount of cell or its area and change their status
	while(TRUE){
		pthread_mutex_trylock(&flag);
		mapIndex_ = mapIndex;
		if(mapIndex_ != NULL)
			mapIndex = mapIndex->next;
		else{
			pthread_mutex_unlock(&flag);
			break;
		}
		for(i = 0, j = 0; j < MAXSIZE; j++){
			cellLocation = (CellLocation){.m=mapIndex_->data, .x=i, .y=j};
			if(cellLocation.m->map[cellLocation.x][cellLocation.y] == ALIVE){
				ChangeAroundStatus(&cellLocation);
				ChangeCellStatus(&cellLocation);
			
			}
		}

		for(i = 1, j = 0; i < MAXSIZE-1; i++){
			cellLocation = (CellLocation){.m=mapIndex_->data, .x=i, .y=j};
			if(cellLocation.m->map[cellLocation.x][cellLocation.y] == ALIVE){
				ChangeAroundStatus(&cellLocation);
				ChangeCellStatus(&cellLocation);
				
			}
		}

		for(i = 1, j = MAXSIZE-1; i < MAXSIZE-1; i++){
			cellLocation = (CellLocation){.m=mapIndex_->data, .x=i, .y=j};
			if(cellLocation.m->map[cellLocation.x][cellLocation.y] == ALIVE){
				ChangeAroundStatus(&cellLocation);
				ChangeCellStatus(&cellLocation);
				
			}
		}

		for(i = MAXSIZE-1, j = 0; j < MAXSIZE; j++){
			cellLocation = (CellLocation){.m=mapIndex_->data, .x=i, .y=j};
			if(cellLocation.m->map[cellLocation.x][cellLocation.y] == ALIVE){
				ChangeAroundStatus(&cellLocation);
				ChangeCellStatus(&cellLocation);
				
			}
		}
		pthread_mutex_unlock(&flag);
		for(i = 1; i < MAXSIZE-1; i++){
			for(j = 1; j < MAXSIZE-1; j++){
			cellLocation = (CellLocation){.m=mapIndex_->data, .x=i, .y=j};
			if(cellLocation.m->map[cellLocation.x][cellLocation.y] == ALIVE){
				ChangeAroundStatus_NoneCreate(&cellLocation);
				ChangeCellStatus_NoneCreate(&cellLocation);
				}
			}
		}
	}
	pthread_alive--;
	pthread_exit(NULL);
}

void *GoWithThreadsTwo(void *threadid){
	int i, j;
	MapIndex *mapIndex_;

	//Change all the cell(TOALIVE->ALIVE, DIED->NONE)
	while(TRUE){
		pthread_mutex_trylock(&flag);
		mapIndex_ = mapIndex;
		if(mapIndex_ != NULL)
			mapIndex = mapIndex->next;
		else{
			pthread_mutex_unlock(&flag);
			break;
		}
		pthread_mutex_unlock(&flag);
		for(i = 0; i < MAXSIZE; i++){
			for(j = 0; j < MAXSIZE; j++){
				switch(mapIndex_->data->map[i][j]){
					case TOALIVE:
						mapIndex_->data->map[i][j] = ALIVE;
						sum++;
						break;
					case DIED:
						mapIndex_->data->map[i][j] = NONE;
						break;
					case ALIVE:
						sum++;
						break;
					default:
						break;
				}
			}
		}
	}
	pthread_alive--;
	pthread_exit(NULL);
}

Status NextStepOne(){
	int rc;
	long i;
	pthread_alive = THREADS_NUM;
	
	pthread_mutex_init(&flag, NULL);
	mapIndex = mapIndexHead.next;
	for(i = 0; i < THREADS_NUM; i++){
		rc = pthread_create(&threads[i], NULL, GoWithThreadsOne, (void*)i);
		if(rc){
			printf("ERROR on create pthread %d\n", rc);
			exit(-1);
		}
	}
}

Status NextStepTwo(int *e){
	int rc;
	long i;
	sum = 0;
	pthread_alive = THREADS_NUM;

	mapIndex = mapIndexHead.next;
	for(i = 0; i < THREADS_NUM; i++){
		rc = pthread_create(&threads[i], NULL, GoWithThreadsTwo, (void*)i);
		if(rc){
			printf("ERROR on create pthread_ %d\n", rc);
			exit(-1);
		}
	}
	*e = sum;
}



Status NextStep(int *e){
		NextStepOne();
		while(pthread_alive != 0);
		NextStepTwo(e);
		while(pthread_alive != 0);
		return OK;
}
#else
Status NextStep(int *e){
	int i, j;
	int sum = 0;
	MapIndex *mapIndex;
	CellLocation cellLocation;

	//Judge the amount of cell or its area and change their status
	mapIndex = mapIndexHead.next;
	while(mapIndex){
		for(i = 0; i < MAXSIZE; i++){
			for(j = 0; j < MAXSIZE; j++){
				cellLocation = (CellLocation){.m=mapIndex->data, .x=i, .y=j};
				if(cellLocation.m->map[cellLocation.x][cellLocation.y] == ALIVE){
					ChangeAroundStatus(&cellLocation);
					ChangeCellStatus(&cellLocation);
				}
			}
		}
		mapIndex = mapIndex->next;
	}

	//Change all the cell(TOALIVE->ALIVE, DIED->NONE)
	mapIndex = mapIndexHead.next;
	while(mapIndex){
		for(i = 0; i < MAXSIZE; i++){
			for(j = 0; j < MAXSIZE; j++){
				switch(mapIndex->data->map[i][j]){
					case TOALIVE:
						mapIndex->data->map[i][j] = ALIVE;
						sum++;
						break;
					case DIED:
						mapIndex->data->map[i][j] = NONE;
						break;
					case ALIVE:
						sum++;
						break;
					default:
						break;
				}
			}
		}
		mapIndex = mapIndex->next;
	}
	*e = sum;
	return OK;
}
#endif

Status Debug(){
	MapIndex* p = mapIndexHead.next;
	int num = 1;
	int i, j;
	while(p != NULL){
		printf("(NUM %d(%d, %d)):\n", num, p->data->x, p->data->y);
		for(i = 0; i < MAXSIZE; i++){
			for(j = 0; j < MAXSIZE; j++){
				printf("%d", p->data->map[i][j]);
			}
			printf("\n");
		}
		num++;
		p = p->next;
	}
	return OK;
}

void ChooseSort(){
	MapIndex *p, *end, *big;
	Map* tmp;
	if(mapIndexHead.next == NULL)
		return;

	for(end = mapIndexHead.next; end->next != NULL; end = end->next);
	while(mapIndexHead.next != end){
		big = end;

		for(p = mapIndexHead.next; ; p = p->next){
			if(CompareMap(*(p->data), *(big->data)) == BIGGER)
				big = p;
			if(p->next == end)
				break;
		}


		
		tmp = big->data;
		big->data = end->data;
		end->data = tmp;

		end = p;
	}
	return;
}

Status SortMap(){
	if(mapIndexHead.hasSort)
		return OK;
	ChooseSort();
	mapIndexHead.hasSort = TRUE;
	return OK;
}

Compare CompareMap(Map a, Map b){
	if(a.x > b.x){
		return BIGGER;
	}else if(a.x < b.x){
		return SMALLER;
	}else{
		if(a.y > b.y)
			return BIGGER;
		else if(a.y < b.y)
			return SMALLER;
		else
			return EQUAL;
	}
}

Status ShowMap(){
	int X_min, Y_min, X_max, Y_max;
	int x, y, mapX, mapY;
	MapIndex* mapIndex = mapIndexHead.next;
	MapIndex* mapIndex_tmp;
	X_min = X_max = mapIndex->data->x;
	Y_min = Y_max = mapIndex->data->y;
	if(mapIndexHead.hasSort == FALSE){
		printf("ERROR: The maps have not sort.\n");
		return ERROR;
	}
	while(mapIndex != NULL){
		if(mapIndex->data->x < X_min)
			X_min = mapIndex->data->x;
		if(mapIndex->data->x > X_max)
			X_max = mapIndex->data->x;
		if(mapIndex->data->y < Y_min)
			Y_min = mapIndex->data->y;
		if(mapIndex->data->y > Y_max)
			Y_max = mapIndex->data->y;
		mapIndex = mapIndex->next;
	}

	mapIndex = mapIndexHead.next;
	for(x = X_min; x <= X_max; x++){
		while(mapIndex->data->x != x)
			mapIndex = mapIndex->next;
		for(mapX = 0; mapX < MAXSIZE; mapX++){
			mapIndex_tmp = mapIndex;
			printf("|*");
			for(y = Y_min; y <= Y_max; y++){
				if(mapIndex_tmp == NULL){
					for(mapY = 0; mapY < MAXSIZE; mapY++)
						printf("0");
				}else if(mapIndex_tmp->data->y != y || mapIndex_tmp->data->x != x){
					for(mapY = 0; mapY < MAXSIZE; mapY++)
						printf("0");
				}else{
					for(mapY = 0; mapY < MAXSIZE; mapY++)
						printf("%d", mapIndex_tmp->data->map[mapX][mapY]);
					mapIndex_tmp = mapIndex_tmp->next;
				}
			}
			printf("*|\n");
		}
	}
}
