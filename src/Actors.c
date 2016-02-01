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
  object->lowerLimit = initialPosition;
  object->upperLimit = 21;
  object->upperCheck = 17;
  object->lowerCheck = 3;
  object->middleCheck = 11;
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
bool update(Jumper* object)
{
  //check to see if the jumper was at a checkpoint and was missed
  //if they were missed, crash the jumper and return
  if (atCheckpoint(object)>=0 && !object->hasBeenScored)
  {
    object->live=false;
    return false;
  }
  else
  {
    ++object->position;
    object->hasBeenScored = false; //reset scored status in preparation for next checkpoint
    if (object->position > object->upperLimit)
    {
      object->live=false;
    }
    return true;
  }  
}
