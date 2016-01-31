#include <pebble.h>
#include "src/Actors.h"

void initialise_MisterGameAndWatch(MrGameAndWatch* object)
{
  object->lowerLimit = 0;
  object->upperLimit = 2;
  object->live=1;
  
  object->position = 1; //centre position
}

int8_t move_MisterGameAndWatch(MrGameAndWatch* object,int8_t direction)
{
  int8_t newPosition = object->position + direction;
  if ( (object->lowerLimit <= newPosition) && (newPosition <= object->upperLimit))
  {
    object->position = newPosition;
    return 1;
  }
  return 0;
}

//BALL CODE

void initialise_Ball(Ball* object,int8_t lowerLimit,int8_t upperLimit, int8_t initialPosition, int8_t track)
{
  object->lowerLimit = lowerLimit;
  object->upperLimit = upperLimit;
  object->live=1;
  object->hasBeenScored = 1;
  object->track = track;
  
  if (initialPosition == DIRECTION_LEFT)
  {
    object->position = object->lowerLimit;
  }
  else if (initialPosition == DIRECTION_RIGHT)
  {
    object->position = object->upperLimit;
  }
  else
  {
    object->position = 1;
  }
  
  if (initialPosition == DIRECTION_RIGHT)
  {
    object->velocity = DIRECTION_LEFT;
  }
  else if (initialPosition == DIRECTION_LEFT)
  {
    object->velocity = DIRECTION_RIGHT;
  }
  else
  {
    object->velocity = 0;
  }
}

int8_t move_Ball(Ball* object,int8_t direction)
{
  char newPosition = object->position + direction;
  if ( (object->lowerLimit <= newPosition) && (newPosition <= object->upperLimit))
  {
    object->position = newPosition;
    return 1;
  }
  return 0;
}

int8_t atLimit(Ball* object)
{
  if (object->position == object->lowerLimit || object->position == object->upperLimit)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

void update(Ball* object)
{
  object->hasBeenScored=0;
  object->live = move_Ball(object, object->velocity);
}

void changeDirection(Ball* object)
{
  if (object->velocity == DIRECTION_LEFT)
  {
    object->velocity = DIRECTION_RIGHT;
  }
  else
    object->velocity = DIRECTION_LEFT;
}
