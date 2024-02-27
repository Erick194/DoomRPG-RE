
#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include "DoomRPG.h"
#include "MenuItem.h"
#include "SDL_Video.h"

void MenuItem_Set(MenuItem_t* menuItem, char* textField, int flags, int action)
{
	MenuItem_Set2(menuItem, textField, NULL, flags, action);
}

void MenuItem_Set2(MenuItem_t* menuItem, char* textField, char* textField2, int flags, int action)
{
    if (textField == NULL) {
        menuItem->textField[0] = '\0';
    }
    else {
        strncpy(menuItem->textField, textField, sizeof(menuItem->textField));
    }
    if (textField2 == NULL) {
        menuItem->textField2[0] = '\0';
    }
    else {
        strncpy(menuItem->textField2, textField2, sizeof(menuItem->textField2));
    }
    menuItem->flags = flags;
    menuItem->action = action;
}