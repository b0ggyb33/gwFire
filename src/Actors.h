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

//BALL CODE
typedef struct 
{
  int8_t live;
  int8_t position;
  int8_t upperLimit;
  int8_t lowerLimit;
  int8_t track;
  int8_t velocity;
  int8_t hasBeenScored;
} Ball;

void initialise_Ball(Ball* object,int8_t lowerLimit,int8_t upperLimit, int8_t initialPosition, int8_t track);
int8_t move_Ball(Ball* object,int8_t direction);
int8_t atLimit(Ball* object);
void update(Ball* object);
void changeDirection(Ball* object);

#endif