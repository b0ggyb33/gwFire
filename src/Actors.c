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

