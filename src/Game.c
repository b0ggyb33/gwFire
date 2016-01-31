#include <pebble.h>
#include "Game.h"

void initialiseGameState(GameState* state)
{
  state->gameInPlay = 1;
  state->game_time = 0;
  state->score=0;
  state->speed=30;
  state->crash=0;
  state->timeOfLastUpdate=0;
  state->timeOfLastSpeedIncrease=0;
  state->delay = 50;
  state->updateSpeedFrequency=150; //controls 'difficulty'
  state->update=false;
  
}

void printGameState(GameState* state)
{
  APP_LOG(APP_LOG_LEVEL_INFO, "======\nCurrent game state:\n======");
  APP_LOG(APP_LOG_LEVEL_INFO, "Game in play: %u", (unsigned int)state->gameInPlay);
  APP_LOG(APP_LOG_LEVEL_INFO, "Game time: %d", state->game_time);
  APP_LOG(APP_LOG_LEVEL_INFO, "Score: %u", (unsigned int)state->score);
  APP_LOG(APP_LOG_LEVEL_INFO, "Speed: %d", (int)state->speed);
  APP_LOG(APP_LOG_LEVEL_INFO, "Crash: %d", (int)state->crash);
  APP_LOG(APP_LOG_LEVEL_INFO, "Time of Last Update: %d", state->timeOfLastUpdate);
  APP_LOG(APP_LOG_LEVEL_INFO, "Time of Last Speed Increase: %d", state->timeOfLastSpeedIncrease);
  APP_LOG(APP_LOG_LEVEL_INFO, "Delay: %d", (int)state->delay);
  APP_LOG(APP_LOG_LEVEL_INFO, "Update Speed Frequency: %d", state->updateSpeedFrequency);
}