#ifndef NODE_H__
#define NODE_H__


struct Node_s;
struct Line_s;
struct Sprite_s;

typedef struct Node_s
{
	short x1;
	short y1;
	short x2;
	short y2;
	int args1;
	int args2;
	struct Sprite_s* sprites;
	int floorHeight;
	int ceilingHeight;
	int floorTexelID;
	int ceilingTexelID;
	struct Node_s* prev;
	struct Node_s* next;
} Node_t;

#endif
