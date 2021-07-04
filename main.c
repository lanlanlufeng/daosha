#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>

#define total 13
#define filled 11
#define maxlognum 500

typedef struct operation
{
	int send;
	int receive;
	int sdepth;
	int rdepth;
	int step;
	int color;
}operation;

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

/*
//Subscript of bottles:
	[0,filled-1]: full whit sand
	[filled,total-1]: empty
	eg 0-10 11-12
*/
bottle bottles[total];

//about log
int lognum;
operation oplog[maxlognum];

//hopest
int hopest;

bool check()
{
	int i;
	for(i = 0; i < total; i++)
	{
		if((bottles[i].depth < 4) && (!bottles[i].flag))
			return false;
	}
	return true;
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

void printlog()
{
	int i;
	for(i = 0; i < lognum; i++)
		printf("%d %d\n", oplog[i].send, oplog[i].receive);
	printf("Print Finished!\n");
}

void printbottles()
{
	int i, j;
	system("cls");
	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < total; j++)
		{
			printf("%02d ", bottles[j].color[i]);
		}
		printf("\n");
	}
	printf("\n");
	for(j = 0; j < total; j++)
	{
		printf("%02d ", j);
	}
	printf("\n");
	Sleep(500);
	//getchar();
}

//cala stepnum
//can move:return op.step <> 0
//cannot move:return op.step = 0
operation calastep(int send, int receive)
{
	int i;
	operation op;
	//init
	op.send = send;
	op.receive = receive;
	op.sdepth = bottles[op.send].depth;
	op.rdepth = bottles[op.receive].depth;
	op.step = 0;
	op.color = bottles[op.receive].color[op.rdepth];
	/*
	//find the first number that not equal to 0
	//(0111) depth = 1
	//(1111) depth = 0
	//depth must be in the set{0, 1, 2, 3, 4}
	while((op.sdepth < 4) && (!bottles[op.send].color[op.sdepth]))
		op.sdepth += 1;
	while((op.rdepth < 4) && (!bottles[op.receive].color[op.rdepth]))
		op.rdepth += 1;
	*/
	//when receive is empty, any colors are available, so it is decided by sand
	if(op.color == 0)
		op.color = bottles[op.send].color[op.sdepth];
	//judge the color of send and receive
	//when send is empty: op.sdepth equal to 4, so (op.sdepth + op.step < 4) equal to false
	//when receive is full: op.rdepth equal to 0, so (op.step < op.rdepth) equal to false
	while((op.sdepth + op.step < 4) && (op.step < op.rdepth) && (bottles[op.send].color[op.sdepth + op.step] == op.color))
		op.step++;
	//most of the cases go out here
	if(!op.step)
		return op;
	//avoid unused move, go out here when something about bottle.flag wrong
	if(op.step == 4)
	{
		op.step = 0;
		printf("step equal to 4.\n");
		return op;
	}
	//here do judge about future: is this op discover a usable state
	hopest = next_usable(op.send, op.receive, bottles[op.send].color[op.sdepth + op.step]);
	if(hopest < 0)
	{
		op.step = 0;
		return op;
	}
	//here do judge with log to avoid dead loop
	for(i = 0; i < lognum; i++)
	{
		if(
			(op.color   == oplog[i].color  ) &&
			(op.step    == oplog[i].step   ) &&
			//(op.send    == oplog[i].receive) &&
			(op.receive == oplog[i].send) &&
			//(op.sdepth  == oplog[i].rdepth - op.step) &&
			(op.rdepth  == oplog[i].sdepth + op.step)
		)
		{
			op.step = 0;
			return op;
		}
	}
	return op;
}

int move(operation op)
{
	int i;
	//Exceptions
	if(op.color == 0)
		return -4;//due to calastep function, op.color must not equal to 0
	//color
	for(i = 0; i < op.step; i++)
	{
		//bottles[op.receive].color[op.rdepth - 1 - i] = bottles[op.send].color[op.sdepth + i];
		bottles[op.receive].color[op.rdepth - 1 - i] = op.color;
		bottles[op.send].color[op.sdepth + i] = 0;
	}
	//depth, of course it could cala by op.sdepth/op.rdepth
	bottles[op.send].depth += op.step;
	bottles[op.receive].depth -= op.step;
	//flag
	if((bottles[op.receive].color[0] == op.color) && (bottles[op.receive].color[1] == op.color) && (bottles[op.receive].color[2] == op.color) && (bottles[op.receive].color[3] == op.color))
		bottles[op.receive].flag = 1;
	//log
	oplog[lognum++] = op;
	if(lognum >= maxlognum)
	{
		printlog();
		printf("Log not enough!\n");
		getchar();
		exit(0);
	}
	return 0;
}

int unmove()
{
	int i;
	operation op;
	//log
	op = oplog[--lognum];
	//color
	for(i = op.step - 1; i >= 0; i--)
	{
		//bottles[op.send].color[op.sdepth + i] = bottles[op.receive].color[op.rdepth - 1 - i];
		bottles[op.send].color[op.sdepth + i] = op.color;
		bottles[op.receive].color[op.rdepth - 1 - i] = 0;
	}
	//depth, of crouse it could cover by op.sdepth/op.rdepth
	bottles[op.send].depth -= op.step;
	bottles[op.receive].depth += op.step;
	//flag
	bottles[op.receive].flag = 0;
	return 0;
}

int go(int sendhome, int receivehome)
{
	int send;
	int receive;
	operation op;

	if(check())
	{
		printlog();
		printf("Success!\n");
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
			op = calastep(send % total, receive % total);
			if(op.step)
			{
				move(op);
				printbottles();
				go(hopest, send);
				unmove();
				printbottles();
			}
		}
	}
	return 1;
}

int main()
{
	int i;
	FILE *in;
	in = fopen("in.txt", "r");
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
	//init log
	lognum = 0;
	//print
	printbottles();
	fclose(in);
	go(0, filled);
    printf("No solution!\n");
    getchar();
    exit(0);
    return 0;
}
