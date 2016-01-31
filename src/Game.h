#include <pebble.h>
#pragma once


typedef struct
{
  bool gameInPlay;
  int game_time;
  uint16_t score;
  uint16_t highScore;
  int8_t speed;
  int8_t crash;
  int timeOfLastUpdate;
  int timeOfLastSpeedIncrease;
  int8_t delay;
  int updateSpeedFrequency; //controls 'difficulty'
  
}GameState;

void initialiseGameState(GameState* state);
void printGameState(GameState* state);