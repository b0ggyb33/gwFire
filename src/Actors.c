#include <pebble.h>
#include "src/Actors.h"
#include "Game.h"

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

void initialise_Jumper(Jumper* object, int8_t initialPosition)
{
  object->live = false;
  object->position = initialPosition;
  object->upperCheck = 20;
  object->lowerCheck = 14;
  object->middleCheck = 7;
  object->hasBeenScored=false;
}

int8_t atCheckpoint(Jumper* object)
{
  if (object->position == object->lowerCheck)
  {
    return 0;
  }
  if (object->position == object->middleCheck)
  {
    return 1;
  }
  if(object->position == object->upperCheck)
  {
    return 2;
  }
  else
  {
    return -1;
  }
}
void update(Jumper* object)
{
    ++object->position;
}
