#ifndef STACKITEM_H
#define STACKITEM_H

typedef union
{
    int integer;
    void *pointer;
} StackItem;

#endif