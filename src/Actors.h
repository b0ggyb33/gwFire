#ifndef ACTOR_H
#define ACTOR_H

#include <pebble.h>

#define DIRECTION_LEFT -1
#define DIRECTION_RIGHT 1

typedef struct
{
  int8_t live;
  int8_t position;
  int8_t upperLimit;
  int8_t lowerLimit;
} MrGameAndWatch;

void initialise_MisterGameAndWatch(MrGameAndWatch* object);
int8_t move_MisterGameAndWatch(MrGameAndWatch* object,int8_t direction);

#endif