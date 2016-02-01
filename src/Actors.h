#ifndef ACTOR_H
#define ACTOR_H

#include <pebble.h>
#include "Game.h"

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

//code for jumpers

typedef struct
{
  bool live;
  int8_t position;
  int8_t upperLimit;
  int8_t lowerLimit;
  int8_t upperCheck;
  int8_t middleCheck;
  int8_t lowerCheck;  
  int8_t velocity;
  bool hasBeenScored;
} Jumper;

void initialise_Jumper(Jumper* object, int8_t initialPosition);
int8_t atCheckpoint(Jumper* object);
bool update(Jumper* object);

#endif