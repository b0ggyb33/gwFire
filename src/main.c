#include <pebble.h>
#include <src/Actors.h>
#include "main.h"
#include "JavascriptInterface.h"
#include "Game.h"

#ifdef PBL_COLOR
  #define BACKGROUND_COLOUR GColorLimerick
#else
  #define BACKGROUND_COLOUR GColorWhite  
#endif

Window *my_window;
static GBitmap *s_background;
static GBitmap *s_mgw_left, *s_mgw_middle, *s_mgw_right;
static GBitmap *s_crash_left, *s_crash_right;

static BitmapLayer *s_background_layer;
static BitmapLayer *s_mgw_layer;
static Layer *s_ball_layer;
static BitmapLayer *s_crash_layer;
static MrGameAndWatch* mgw;

static TextLayer *scoreLayer;
static TextLayer *highScoreLayer;
static TextLayer *nameLayer;
static TextLayer *restartTextLayer;

static char scoreString[10];
static char highScoreString[10];
static char friendlyNameString[256];

static GameState *game;


  
static Ball *ball0,*ball1,*ball2;

static uint8_t positions0x[8] = {51,56,60,68,77,85,88,93};
static uint8_t positions0y[8] = {128,101,81,61,61,81,101,128};
static uint8_t positions1x[10] = {41,42,48,55,68,78,86,94,98,102};
static uint8_t positions1y[10] = {128,103,76,53,43,43,53,76,103,128};
static uint8_t positions2x[12] = {32,32,36,42,53,66,77,88,98,105,110,113};
static uint8_t positions2y[12] = {128,100,78,54,35,28,28,35,54,78,100,128};

static void inbox_received_callback(DictionaryIterator *iterator, void *context) 
{
  
  Tuple *data = dict_find(iterator, 0);
  
  if (data)
  {
    snprintf(friendlyNameString, 256, "Username:\n%s\nb0ggyb33.co.uk",data->value->cstring);
    APP_LOG(APP_LOG_LEVEL_INFO, "friendlyNameString set");
    APP_LOG(APP_LOG_LEVEL_INFO, "%s", data->value->cstring);
  }
  else
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "friendlyNameString not set");
  }
  
}

void renderBalls(Layer* layer,GContext* ctx)
{
  if (game->gameInPlay)
  {
    GPoint ball0position = GPoint(positions0x[ball0->position],
                                positions0y[ball0->position]);
    GPoint ball1position = GPoint(positions1x[ball1->position],
                                positions1y[ball1->position]);
    GPoint ball2position = GPoint(positions2x[ball2->position],
                                positions2y[ball2->position]);
  
    graphics_fill_circle(ctx, ball0position, 3);
    graphics_fill_circle(ctx, ball1position, 3);
    graphics_fill_circle(ctx, ball2position, 3);
  }
}

void renderCrash(int8_t direction)
{
  if (direction == DIRECTION_RIGHT)
  {
    bitmap_layer_set_bitmap(s_crash_layer, s_crash_right);
  }
  if (direction == DIRECTION_LEFT)
  {
    bitmap_layer_set_bitmap(s_crash_layer, s_crash_left);
  }
}

void renderScores()
{
  snprintf(scoreString, 10,"%u", (unsigned int)game->score);
  text_layer_set_text(scoreLayer, scoreString);
  snprintf(highScoreString, 10,"%u", (unsigned int)game->highScore);
  text_layer_set_text(highScoreLayer, highScoreString);
}

void updateScore()
{
  game->score += 10;
  if (game->score >= game->highScore)
  {
    game->highScore=game->score;
  }
  renderScores();
}

void collisionEvent(Ball* object)
{
  updateScore();
  changeDirection(object);
  object->hasBeenScored = 1;
}

void handleBallCollision(Ball* object)
{
  if (object->hasBeenScored == 0 && atLimit(object) == 1 )
  {
    switch(object->track)
    {
      case 0:
        if ( (mgw->position == mgw->lowerLimit && object->position == object->upperLimit)
          || (mgw->position == mgw->upperLimit && object->position == object->lowerLimit)
           )
        {
          collisionEvent(object);
        }
      break;
      
      case 1:
        if (mgw->position != mgw->lowerLimit && mgw->position != mgw->upperLimit)
        {
          collisionEvent(object);
        }
      break;
      
      case 2:
        if ( (mgw->position == mgw->upperLimit && object->position == object->upperLimit)
          || (mgw->position == mgw->lowerLimit && object->position == object->lowerLimit)
           )
        {
          collisionEvent(object);
        }
      break;
    }
  }
}

void triggerEndGame(Ball* object)
{
  game->crash=object->velocity;
  game->gameInPlay=0;
  persist_write_int(0, game->highScore);
  renderCrash(game->crash); 
  sendScore(game->score);
  text_layer_set_text(nameLayer, friendlyNameString);
  text_layer_set_text(restartTextLayer, "Press Up to Restart ->");
  
}

void updateWorld()
{
  if (!game->gameInPlay)
    return;
  
  //update time
  game->game_time += 1;
  
  //ignoring keypress as event driven
  
  //update ball updates
  if (game->game_time - game->timeOfLastUpdate >= game->speed)
  {
    update(ball0);
    update(ball1);
    update(ball2);
    layer_mark_dirty(s_ball_layer); //render changes
    game->timeOfLastUpdate = game->game_time;
  }
  
  //handle collisions
  handleBallCollision(ball0);
  handleBallCollision(ball1);
  handleBallCollision(ball2);
  
  //handle speed increases
  if ( (game->game_time - game->timeOfLastSpeedIncrease >= game->updateSpeedFrequency)  && game->speed >= 1 )
  {
    game->speed -= 1;
    game->timeOfLastSpeedIncrease = game->game_time;
  } 
  
  //handle game end
  if (!ball0->live)
  {
    triggerEndGame(ball0);
  }
  if (!ball1->live)
  {
    triggerEndGame(ball1);
  }
  if (!ball2->live)
  {
    triggerEndGame(ball2);
  }
  
  app_timer_register(game->delay, updateWorld, NULL); 
  
}

void render_MisterGameAndWatch(MrGameAndWatch* object)
{
  if (object->position==0)
  {
    bitmap_layer_set_bitmap(s_mgw_layer, s_mgw_left);
  }
  else if (object->position==1)
  {
    bitmap_layer_set_bitmap(s_mgw_layer, s_mgw_middle);
  }
  else if (object->position==2)
  {
    bitmap_layer_set_bitmap(s_mgw_layer, s_mgw_right);
  }
}

static void reset_game_handler(ClickRecognizerRef recognizer, void *context)
{
  if (!game->gameInPlay)
  {
    text_layer_set_text(restartTextLayer,"");
    text_layer_set_text(nameLayer,"");
    
    initialiseGameState(game);
    initialise_MisterGameAndWatch(mgw);
    render_MisterGameAndWatch(mgw);
    initialise_Ball(ball0, (int8_t)0, (int8_t)7, DIRECTION_RIGHT, 0);
    initialise_Ball(ball1, (int8_t)0, (int8_t)9, DIRECTION_LEFT, 1);
    initialise_Ball(ball2, (int8_t)0, (int8_t)11, DIRECTION_RIGHT, 2);
    layer_mark_dirty(s_ball_layer);
    
    renderScores();
    
    app_timer_register(game->delay, updateWorld, NULL); 
  }
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) 
{
  if (game->gameInPlay)
  {
    move_MisterGameAndWatch(mgw, DIRECTION_RIGHT);
    render_MisterGameAndWatch(mgw);
  }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) 
{
  if (game->gameInPlay)
  {  
    move_MisterGameAndWatch(mgw, DIRECTION_LEFT);
    render_MisterGameAndWatch(mgw);
  }
}

static void click_config_provider(void *context) 
{
  // Register the ClickHandlers
  window_single_click_subscribe(BUTTON_ID_SELECT, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, reset_game_handler);
}

void handle_init(void) 
{ 
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_register_inbox_received(inbox_received_callback);
  
  my_window = window_create();

  game = malloc(sizeof(GameState));
  initialiseGameState(game);
  
  // initialise score layers
  scoreLayer = text_layer_create(GRect(0,0,60,20));
  highScoreLayer = text_layer_create(GRect(144-30,0,30,20));  
  nameLayer = text_layer_create(GRect(0,40,160,60));
  restartTextLayer = text_layer_create(GRect(0,20,160,20));
  text_layer_set_background_color(scoreLayer, GColorClear);
  text_layer_set_background_color(highScoreLayer, GColorClear);
  text_layer_set_background_color(nameLayer, GColorClear);
  text_layer_set_background_color(restartTextLayer, GColorClear);
  
  // Load the images
  s_background = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BG);
  s_mgw_left = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MGW_LEFT);
  s_mgw_middle = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MGW_MIDDLE);
  s_mgw_right = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MGW_RIGHT);
  s_crash_left = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CRASH_LEFT);
  s_crash_right = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CRASH_RIGHT);
  
  // Create the BitmapLayer
  window_set_background_color(my_window,BACKGROUND_COLOUR);
  s_background_layer = bitmap_layer_create(GRect(0, 168-51, 144, 51));
  s_ball_layer = layer_create(GRect(0, 0, 144, 168)); 
  s_mgw_layer = bitmap_layer_create(GRect(0, 168-51, 144, 51));
  s_crash_layer = bitmap_layer_create(GRect(0, 168-51, 144, 51));
    
  // Set the correct compositing mode
  bitmap_layer_set_compositing_mode(s_background_layer, GCompOpSet);
  bitmap_layer_set_compositing_mode(s_mgw_layer, GCompOpSet);
  bitmap_layer_set_compositing_mode(s_crash_layer, GCompOpSet);
  
  bitmap_layer_set_bitmap(s_background_layer, s_background);
  
  //set mgw based on keys
  bitmap_layer_set_bitmap(s_mgw_layer, s_mgw_middle);
  
  renderScores();
  
  // Add to the Window
  layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(s_background_layer));
  layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(s_mgw_layer));
  layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(s_crash_layer));
  layer_add_child(window_get_root_layer(my_window), s_ball_layer);
  layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(scoreLayer));
  layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(highScoreLayer));
  layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(nameLayer));
  layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(restartTextLayer));
  layer_set_update_proc(s_ball_layer, renderBalls);

  window_stack_push(my_window, true);
  
  
  //initialise game objects
  mgw = malloc(sizeof(MrGameAndWatch));
  initialise_MisterGameAndWatch(mgw);
    
  ball0 = malloc(sizeof(Ball));
  ball1 = malloc(sizeof(Ball));
  ball2 = malloc(sizeof(Ball));
  
  initialise_Ball(ball0, (int8_t)0, (int8_t)7, DIRECTION_RIGHT, 0);
  initialise_Ball(ball1, (int8_t)0, (int8_t)9, DIRECTION_LEFT, 1);
  initialise_Ball(ball2, (int8_t)0, (int8_t)11, DIRECTION_RIGHT, 2);
  
  light_enable(true);
  
}

void handle_deinit(void) 
{
  window_destroy(my_window);
  gbitmap_destroy(s_background);
  gbitmap_destroy(s_mgw_middle);
  gbitmap_destroy(s_mgw_left);
  gbitmap_destroy(s_mgw_right);
  gbitmap_destroy(s_crash_left);
  gbitmap_destroy(s_crash_right);
  bitmap_layer_destroy(s_background_layer);
  bitmap_layer_destroy(s_mgw_layer);
  layer_destroy(s_ball_layer);
  bitmap_layer_destroy(s_crash_layer);
  text_layer_destroy(scoreLayer);
  text_layer_destroy(highScoreLayer);
  text_layer_destroy(nameLayer);
    
  free(mgw);
  free(ball0);
  free(ball1);
  free(ball2);
  free(game);
  
  light_enable(false);
}

int main(void) 
{  
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  handle_init();
  window_set_click_config_provider(my_window, click_config_provider);
  app_timer_register(game->delay, updateWorld, NULL); 
  app_event_loop();
  handle_deinit();
}
