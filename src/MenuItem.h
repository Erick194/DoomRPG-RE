#ifndef MENUITEM_H__
#define MENUITEM_H__


struct DoomRPG_s;
typedef struct MenuItem_s
{
	char textField[32]; // original 18
	char textField2[16]; // original 8
	byte flags;
	int action;
} MenuItem_t;

void MenuItem_Set(MenuItem_t* menuItem, char* textField, int flags, int action);
void MenuItem_Set2(MenuItem_t* menuItem, char* textField, char* textField2, int flags, int action);

#endif
