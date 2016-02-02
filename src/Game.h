#include <pebble.h>
#pragma once


typedef struct
{
  bool gameInPlay;
  int game_time;
  uint16_t score;
  int8_t speed;
  int8_t maximumSpeed;
  int8_t crash;
  int timeOfLastUpdate;
  int timeOfLastSpeedIncrease;
  int updateReleaseFrequency;
  int timeOfLastJumperReleaseSpeedIncrease;
  int maximumReleaseFrequency;
  int8_t delay;
  int updateSpeedFrequency; //controls 'difficulty'
  bool update;
  
}GameState;

void initialiseGameState(GameState* state);
void printGameState(GameState* state);