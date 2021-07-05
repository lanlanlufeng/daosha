#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>

#define total 13
#define filled 11

typedef struct bottle
{
	//color: 0 means any colors are all right, [1,filled] means the kinds of color, eg 0 1-11
	//subscript[0..3] means the four sub from top to bottom, subscript[4] is always equal to 0
	int color[5];
	//cover color: the cover color of this bottle, 0 means this bottle is empty
	//depth: the subscript of cover color, so (cover == color[depth])
	int depth;
	//flag: 1 means all of c0, c1, c2, c3 of this bottle are same, this bottle cannot move after
	int flag;
}bottle;

typedef struct bottles_status
{
	//Subscript of bottles:
	//[0,filled-1]: full whit sand
	//[filled,total-1]: empty
	//eg 0-10 11-12
	bottle bottles[total];
	//op
	int send;
	int receive;
	//father
	struct bottles_status *father;
	//sort
	//struct bottles_status *previous;
	struct bottles_status *next;
}bottles_status/*, *bottles_status_pointer*/;

//Global Variables
bottle bottles[total];
int hopest;
FILE *logfile;

bottles_status* new_bottles_status(bottle *b, bottles_status *father, int send, int receive)
{
	bottles_status *new_node;
	new_node = (bottles_status*)malloc(sizeof(bottles_status));
	memcpy(new_node->bottles, b, sizeof(bottle) * total);
	new_node->send = send;
	new_node->receive = receive;
	new_node->father = father;
	new_node->next = NULL;
	return new_node;
}

/*__inline*/
void free_bottles_status(bottles_status *unused_node)
{
	free(unused_node);
}

void free_bottles_status_linklist(bottles_status *head)
{
	bottles_status *next;
	if(!head)
		return;
	while(head)
	{
		next = head->next;
		free_bottles_status(head);
		head = next;
	}
}

int compare_bottle(bottle *a, bottle *b)
{
	int ret = 0;
	int *p = a->color + 3;
	int *q = b->color + 3;
    while((q >= b->color) && !(ret = *p - *q))
	{
		--p;
		--q;
	}
    if(ret < 0)
        ret = -1;
    else if(ret > 0)
        ret = 1;
    return ret;
}

int compare_bottles_status(bottles_status *a, bottles_status *b)
{
	int ret = 0;
	bottle *p = a->bottles;
	bottle *q = b->bottles;
	bottle *o = q;
    while((q - o < total) && !(ret = compare_bottle(p, q)))
	{
		++p;
		++q;
	}
    return ret;
}

//inline sort
void Standardize(bottle *b)
{
	int i, j;
	bottle temp;
	for(i = 0; i < total - 1; i++)
		for(j = 0; j < total - i - 1; j++)
			if(compare_bottle(b + j, b + j + 1) > 0)
			{
				temp = b[j];
				b[j] = b[j + 1];
				b[j + 1] = temp;
			}
}

//Insertion Sort
bottles_status* locate_status(bottles_status *head, bottles_status *current, int *result)
{
	bottles_status *p, *q;
	if(!head || !current || !result)
		return head;
	//insert the current into the middle of p and q
	for(p = NULL, q = head; q && ((*result = compare_bottles_status(q, current)) < 0); p = q, q = q->next);
	if(*result == 0)
		*result = -1;
	else
	{
		*result = 1;
        if (q == head)
            head = current;
        else
            p->next = current;
        current->next = q;
	}
	return head;
}

bool check()
{
	int i;
	for(i = 0; i < total; i++)
	{
		//not empty && have not flag
		if((bottles[i].depth < 4) && (!bottles[i].flag))
			return false;
	}
	return true;
}

void printbottles(bottles_status *p)
{
	int i, j;
	bottle *target;
	if(p)
		target = p->bottles;
	else
		target = bottles;
	//system("cls");
	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < total; j++)
		{
			fprintf(logfile, "%02d ", target[j].color[i]);
		}
		fprintf(logfile, "\n");
	}
	/*
	fprintf(logfile, "\n");
	for(j = 0; j < total; j++)
	{
		fprintf(logfile, "%02d ", j);
	}
	*/
	fprintf(logfile, "\n");
	//Sleep(500);
	//getchar();
}

void printstatus_address_from_son(bottles_status *p)
{
	while(p)
	{
		fprintf(logfile, "%p ", p);
		p = p->father;
	}
	fprintf(logfile, "\n");
}

void printstatus_address_from_head(bottles_status *p)
{
	while(p)
	{
		fprintf(logfile, "%p ", p);
		p = p->next;
	}
	fprintf(logfile, "\n");
}

void printstatus_log_number(bottles_status *p)
{
	char buffer[1000];
	int i = 0;
	int l;
	memset(buffer, 0, 100);
	while(p)
	{
		fprintf(logfile, "%d %d  ", p->send, p->receive);
		l = sprintf(buffer + i, "%d %d  ", p->send, p->receive);
		i += l;
		p = p->father;
	}
	fprintf(logfile, "\n");
	if(memcmp(buffer, "0 1  10 1  9 11  1 3  5 12  1 0  1 10  10 5  5 7  7 12  3 11  1 12  0 11  0 0  ", 79) == 0)
		fprintf(logfile, "get\n");
}

void printstatus_log_bottles(bottles_status *p)
{
	printbottles(NULL);
	while(p)
	{
		printbottles(p);
		p = p->father;
	}
}

void printlog(bottles_status *head, bottles_status *current)
{
	fprintf(logfile, "This is printlog:\n");
	fprintf(logfile, "This is printstatus_address_from_head:\n");
	printstatus_address_from_head(head);
	fprintf(logfile, "This is printstatus_address_from_son:\n");
	printstatus_address_from_son(current);
	fprintf(logfile, "This is printstatus_log_number:\n");
	printstatus_log_number(current);
	fprintf(logfile, "Now bottles is:\n");
	printbottles(NULL);
	fprintf(logfile, "\n\n");
}

//return: hopest bottle
int next_usable(int send, int receive, int nextcolor)
{
	int i;
	if(!nextcolor)
		return 0;
	for(i = 0; i < total; i++)
	{
		if((i == send) || (i == receive))
			continue;
		if((!bottles[i].color[bottles[i].depth]) || (nextcolor == bottles[i].color[bottles[i].depth]))
			return i;
	}
	return -1;
}

//cala step
//can move:return step  [1,3]
//cannot move:return step = 0
int calastep(int send, int receive)
{
	int sdepth;
	int rdepth;
	int step;
	int color;
	//init
	sdepth = bottles[send].depth;
	rdepth = bottles[receive].depth;
	step = 0;
	color = bottles[receive].color[rdepth];
	//when receive is empty, any colors are available, so it is decided by sand
	if(color == 0)
		color = bottles[send].color[sdepth];
	//judge the color of send and receive
	//when send is empty: sdepth equal to 4, so (sdepth + step < 4) equal to false
	//when receive is full: rdepth equal to 0, so (step < rdepth) equal to false
	while((sdepth + step < 4) && (step < rdepth) && (bottles[send].color[sdepth + step] == color))
		step++;
	//most of the cases go out here
	if(!step)
		return step;
	//avoid unused move, go out here when something about bottle.flag wrong
	if(step == 4)
	{
		step = 0;
		printf("step equal to 4.\n");
		return step;
	}
	//here do judge about future: is this op discover a usable state
	hopest = next_usable(send, receive, bottles[send].color[sdepth + step]);
	if(hopest < 0)
	{
		//step = 0;
		return step;
	}
	return step;
}

int move(int send, int receive, int step)
{
	int i;
	//color
	for(i = 0; i < step; i++)
	{
		bottles[receive].color[bottles[receive].depth - 1 - i] = bottles[send].color[bottles[send].depth + i];
		bottles[send].color[bottles[send].depth + i] = 0;
	}
	//depth, of course it could cala by bottles[send].depth/bottles[receive].depth
	bottles[send].depth += step;
	bottles[receive].depth -= step;
	//flag
	if((bottles[receive].color[1] == bottles[receive].color[0]) && (bottles[receive].color[2] == bottles[receive].color[0]) && (bottles[receive].color[3] == bottles[receive].color[0]))
		bottles[receive].flag = 1;
	//fprintf(logfile, "move:%d %d %d\n", send, receive, step);
	//printbottles(NULL);
	return 0;
}

int unmove(int send, int receive, int step)
{
	int i;
	//flag
	bottles[receive].flag = 0;
	//depth, of crouse it could cover by bottles[send].depth/bottles[receive].depth
	bottles[send].depth -= step;
	bottles[receive].depth += step;
	//color
	for(i = step - 1; i >= 0; i--)
	{
		bottles[send].color[bottles[send].depth + i] = bottles[receive].color[bottles[receive].depth - 1 - i];
		bottles[receive].color[bottles[receive].depth - 1 - i] = 0;
	}
	//fprintf(logfile, "unmove:%d %d %d\n", send, receive, step);
	//printbottles(NULL);
	return 0;
}

int go(int sendhome, int receivehome, bottles_status **head, bottles_status *father)
{
	int send;
	int receive;
	int step;
	int result = 0;
	bottles_status *new_node;

	//log
	printlog(*head, father);
	if(check())
	{
		printf("Success!\n");
		printstatus_log_number(father);
		getchar();
		exit(0);
		//return 0;
	}
	for(send = sendhome; send < sendhome + total; send++)
	{
		if(bottles[send % total].flag)
			continue;
		for(receive = receivehome; receive < receivehome + total; receive++)
		{
			if(receive % total == send % total)
				continue;
			step = calastep(send % total, receive % total);
			if(step)
			{
				move(send % total, receive % total, step);
				new_node = new_bottles_status(bottles, father, send % total, receive % total);
				Standardize(new_node->bottles);
				*head = locate_status(*head, new_node, &result);
				if(result > 0)
				{
					go(hopest, send, head, new_node);
				}
				else
				{
					free_bottles_status(new_node);
					new_node = NULL;
				}
				unmove(send % total, receive % total, step);
			}
		}
	}
	return 1;
}

int main()
{
	int i;
	FILE *in;
	bottles_status *head;
	in = fopen("in.txt", "r");
	logfile = fopen("log.txt", "w+");
	//init bottles
	memset(bottles, 0, total * sizeof(bottle));
	for(i = 0; i < filled; i++)
	{
		fscanf(in, "%d %d %d %d", &bottles[i].color[0], &bottles[i].color[1], &bottles[i].color[2], &bottles[i].color[3]);
	}
	for(; i < total; i++)
	{
		bottles[i].depth = 4;
	}
	fclose(in);
	//print
	printf("Init data:\n");
	printbottles(NULL);
	printf("Start:\n");
	//init head
	head = new_bottles_status(bottles, NULL, 0, 0);
	Standardize(head->bottles);
	go(0, filled, &head, head);
	if(head)
	{
		free_bottles_status_linklist(head);
		head = NULL;
	}
    printf("No solution!\n");
    fclose(logfile);
    getchar();
    exit(0);
    return 0;
}
