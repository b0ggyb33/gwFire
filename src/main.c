#include <pebble.h>
#include "Actors.h"
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
static GBitmap *s_crash_left, *s_crash_middle, *s_crash_right;

static GBitmap *BMPfire0, *BMPfire1, *BMPfire2, *BMPfire3;

static BitmapLayer *s_background_layer;
static BitmapLayer *s_mgw_layer;
static BitmapLayer *s_crash_layer;
static BitmapLayer *s_fire_layer0, *s_fire_layer1;
static MrGameAndWatch* mgw;

static TextLayer *scoreLayer;
static TextLayer *nameLayer;
static TextLayer *restartTextLayer;

static char scoreString[10];
static char friendlyNameString[256];

static BitmapLayer* jumperBitmapLayers[10];
static GBitmap* jumperBitmaps[21];
static Jumper* jumpers[10];

static GameState *game;


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

void renderCrash(int8_t position)
{
  if (position == 2)
  {
    bitmap_layer_set_bitmap(s_crash_layer, s_crash_right);
  }
  if (position == 0)
  {
    bitmap_layer_set_bitmap(s_crash_layer, s_crash_left);
  }
  else
  {
    bitmap_layer_set_bitmap(s_crash_layer, s_crash_middle);
  }
}

void renderScores()
{
  snprintf(scoreString, 10,"%u", (unsigned int)game->score);
  text_layer_set_text(scoreLayer, scoreString);
}

void updateScore()
{
  game->score += 10;
  renderScores();
}


void triggerEndGame()
{
  game->gameInPlay=0;
  renderCrash(game->crash); 
  sendScore(game->score);
  text_layer_set_text(nameLayer, friendlyNameString);
  text_layer_set_text(restartTextLayer, "Press Up to Restart ->");
}

void updateJumpers()
{
  for (int i=0;i<10;++i)
  {
    if (jumpers[i]->live)
    {
      if (!update(jumpers[i]))
      {//returned false, so jumper has fallen
        game->crash = atCheckpoint(jumpers[i]); //get their position in checkpoint co-ordinates
        triggerEndGame();
      }
    }
  }
}

void render(Jumper* object, int i)
{
  //need to pass in i to update the correct layer
  bitmap_layer_set_bitmap(jumperBitmapLayers[i], jumperBitmaps[object->position]);
}

void renderJumpers()
{
  for (int i=0;i<10;++i)
  {
    if (jumpers[i]->live)
      render(jumpers[i], i);
  }
}
void handleCollisionsWithFiremen()
{
  for (int i=0;i<10;++i)
  {
    if (!jumpers[i]->hasBeenScored)
    {
      if (mgw->position == atCheckpoint(jumpers[i]))
      {
        jumpers[i]->hasBeenScored=true;
        updateScore();
      }
    }
  }
}

void spawnNewJumper(void)
{
  for (int i=0;i<10;++i)
  {
    if (!jumpers[i]->live)
    {
      initialise_Jumper(jumpers[i],0);
      jumpers[i]->live=true;
      break;
    }
  }
}

void updateWorld()
{
  if (!game->gameInPlay)
    return;
  
  //update time
  game->game_time += 1;
  
  //check if sprites need to be updated
  if (game->timeOfLastUpdate - game->game_time >= game->speed)
  {
    game->update = true;
    game->timeOfLastUpdate = game->game_time;
  }
  
  //this needs to happen outside the jumper update 
  handleCollisionsWithFiremen(); 
  
  if (game->update)
  {
    updateJumpers();
    renderJumpers();
  }
  
  renderFire(game);  
    
  //handle speed increases
  if ( (game->game_time - game->timeOfLastSpeedIncrease >= game->updateSpeedFrequency)  && game->speed >= 1 )
  {
    spawnNewJumper();
    game->speed -= 1;
    game->timeOfLastSpeedIncrease = game->game_time;
  } 
  
  //handle game end
  
  
  //reinitialise loop
  app_timer_register(game->delay, updateWorld, NULL); 
  
}

void renderFire(GameState* game)
{
  int localGameTime = game->game_time%60;
  if (localGameTime < 15)
  {
    bitmap_layer_set_bitmap(s_fire_layer0, BMPfire0);
    bitmap_layer_set_bitmap(s_fire_layer1, BMPfire1);
  }
  else if (localGameTime < 30)
  {
    bitmap_layer_set_bitmap(s_fire_layer0, BMPfire1);
    bitmap_layer_set_bitmap(s_fire_layer1, BMPfire2);
  }
  else if (localGameTime < 45)
  {
    bitmap_layer_set_bitmap(s_fire_layer0, BMPfire2);
    bitmap_layer_set_bitmap(s_fire_layer1, BMPfire3);
  }
  else
  {
    bitmap_layer_set_bitmap(s_fire_layer0, BMPfire3);
    bitmap_layer_set_bitmap(s_fire_layer1, BMPfire0);
  }
  
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

void initFire(void)
{
  BMPfire0 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_FIRE0);
  BMPfire1 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_FIRE1);
  BMPfire2 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_FIRE2);
  BMPfire3 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_FIRE3);
  s_fire_layer0 = bitmap_layer_create(GRect(0, 0, 144, 168));
  s_fire_layer1 = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_compositing_mode(s_fire_layer0, GCompOpSet);
  bitmap_layer_set_compositing_mode(s_fire_layer1, GCompOpSet);
}

void handle_init(void) 
{ 
  //app_message_register_outbox_failed(outbox_failed_callback);
  //app_message_register_outbox_sent(outbox_sent_callback);
  //app_message_register_inbox_received(inbox_received_callback);
  
  my_window = window_create();

  game = malloc(sizeof(GameState));
  initialiseGameState(game);
  
  // initialise score layers
  scoreLayer = text_layer_create(GRect(0,0,60,20));
  nameLayer = text_layer_create(GRect(0,40,160,60));
  restartTextLayer = text_layer_create(GRect(0,20,160,20));
  text_layer_set_background_color(scoreLayer, GColorClear);
  text_layer_set_background_color(nameLayer, GColorClear);
  text_layer_set_background_color(restartTextLayer, GColorClear);
  
  // Load the images
  s_background = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BG);
  s_mgw_left = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MGW_LEFT);
  s_mgw_middle = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MGW_MIDDLE);
  s_mgw_right = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MGW_RIGHT);
  
  //s_crash_left = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CRASH_LEFT);
  //s_crash_right = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CRASH_RIGHT);
  
  // Create the BitmapLayers
  window_set_background_color(my_window,BACKGROUND_COLOUR);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  s_mgw_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  s_crash_layer = bitmap_layer_create(GRect(0, 168-51, 144, 51));
  
    
  // Set the correct compositing mode
  bitmap_layer_set_compositing_mode(s_background_layer, GCompOpSet);
  bitmap_layer_set_compositing_mode(s_mgw_layer, GCompOpSet);
  bitmap_layer_set_compositing_mode(s_crash_layer, GCompOpSet);
  
  bitmap_layer_set_bitmap(s_background_layer, s_background);
  
  initFire();

  for (int i=0;i<10;++i)
  {
    jumpers[i] = malloc(sizeof(Jumper));
    initialise_Jumper(jumpers[i],0);
    jumperBitmapLayers[i] = bitmap_layer_create(GRect(0, 0, 144, 168));
  }
  
  
  //set mgw based on keys
  bitmap_layer_set_bitmap(s_mgw_layer, s_mgw_middle);
  
  renderScores();
  
  // Add to the Window
  layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(s_background_layer));
  layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(s_mgw_layer));
  layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(s_crash_layer));
  layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(s_fire_layer0));
  layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(s_fire_layer1));
  
  layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(scoreLayer));
  layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(nameLayer));
  layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(restartTextLayer));
  
  window_stack_push(my_window, true);
  
  
  //initialise game objects
  mgw = malloc(sizeof(MrGameAndWatch));
  initialise_MisterGameAndWatch(mgw);
    
  light_enable(true);
  
}

void handle_deinit(void) 
{
  window_destroy(my_window);
  gbitmap_destroy(s_background);
  gbitmap_destroy(s_mgw_left);
  gbitmap_destroy(s_mgw_middle);
  gbitmap_destroy(s_mgw_right);
  gbitmap_destroy(s_crash_left);
  gbitmap_destroy(s_crash_middle);
  gbitmap_destroy(s_crash_right);
  gbitmap_destroy(BMPfire0);
  gbitmap_destroy(BMPfire1);
  gbitmap_destroy(BMPfire2);
  gbitmap_destroy(BMPfire3);
  bitmap_layer_destroy(s_background_layer);
  bitmap_layer_destroy(s_mgw_layer);
  bitmap_layer_destroy(s_crash_layer);
  bitmap_layer_destroy(s_fire_layer0);
  bitmap_layer_destroy(s_fire_layer1);
  text_layer_destroy(scoreLayer);
  text_layer_destroy(nameLayer);
  
  for (int i=0;i<10;++i)
  {  
    free(jumpers[i]);
    free(jumperBitmapLayers[i]);
  }
  
  free(mgw);
  free(game);
  
  light_enable(false);
}

int main(void) 
{  
  //app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  handle_init();
  window_set_click_config_provider(my_window, click_config_provider);
  app_timer_register(game->delay, updateWorld, NULL); 
  app_event_loop();
  handle_deinit();
}
