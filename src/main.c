#include <pebble.h>
#include "Actors.h"
#include "main.h"
#include "JavascriptInterface.h"
#include "Game.h"
#include "jumperOffsets.h"
#ifdef PBL_COLOR
  #define BACKGROUND_COLOUR GColorLimerick
#else
  #define BACKGROUND_COLOUR GColorWhite  
#endif

#define NUMBER_OF_JUMPER_IMAGES 21
#define NUMBER_OF_JUMPERS 10

Window *my_window;
static GBitmap *s_background;
static GBitmap *mgwBitmaps[3];
static GBitmap *crashBitmaps[3];
static GBitmap *startingImage0,*startingImage1;
static GBitmap *fireBitmaps[4];

static BitmapLayer *s_background_layer;
static BitmapLayer *s_mgw_layer;
static BitmapLayer *s_crash_layer;
static BitmapLayer *fireLayers[2];
static MrGameAndWatch* mgw;

static TextLayer *scoreLayer;
static TextLayer *nameLayer;
static TextLayer *restartTextLayer;

static char scoreString[10];
static char friendlyNameString[256];

static BitmapLayer* jumperBitmapLayers[NUMBER_OF_JUMPERS];
static GBitmap* jumperBitmaps[NUMBER_OF_JUMPER_IMAGES];
static Jumper* jumpers[NUMBER_OF_JUMPERS];

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
    bitmap_layer_set_bitmap(s_crash_layer, crashBitmaps[position]);
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
  for (int i=0;i<NUMBER_OF_JUMPERS;++i)
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
{//need to pass in i to update the correct layer
  if (object->position == object->lowerLimit)
  {//always true when initialised
    GRect newFrame = GRect(0,0,144,168);
    
    APP_LOG(APP_LOG_LEVEL_INFO, "Render Jumper in initialPosition");
    if (object->lowerLimit==-1)
    {
      layer_set_frame(bitmap_layer_get_layer(jumperBitmapLayers[i]), newFrame);
      APP_LOG(APP_LOG_LEVEL_INFO, "initialPosition0");
      bitmap_layer_set_bitmap(jumperBitmapLayers[i], startingImage0);
    }
    else
    {
      layer_set_frame(bitmap_layer_get_layer(jumperBitmapLayers[i]), newFrame);
      APP_LOG(APP_LOG_LEVEL_INFO, "initialPosition1");
      bitmap_layer_set_bitmap(jumperBitmapLayers[i], startingImage1);
    }
  }
  else
  {
    APP_LOG(APP_LOG_LEVEL_INFO, "Render Jumper in position: %d",object->position);
    GRect newFrame = GRect(offsets[object->position][1],
						               offsets[object->position][0], 
                           offsets[object->position][3]-offsets[object->position][1],
						               offsets[object->position][2]-offsets[object->position][0]);
    
    layer_set_frame(bitmap_layer_get_layer(jumperBitmapLayers[i]), newFrame);

    bitmap_layer_set_bitmap(jumperBitmapLayers[i], jumperBitmaps[object->position]);  
  }
}

void renderJumpers()
{
  for (int i=0;i<NUMBER_OF_JUMPERS;++i)
  {
    if (jumpers[i]->live)
    {
      APP_LOG(APP_LOG_LEVEL_INFO, "Rendering Live Jumper!");
      render(jumpers[i], i);
    }
  }
}
void handleCollisionsWithFiremen()
{
  for (int i=0;i<NUMBER_OF_JUMPERS;++i)
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
  for (int i=0;i<NUMBER_OF_JUMPERS;++i)
  {
    if (!jumpers[i]->live)
    {
      APP_LOG(APP_LOG_LEVEL_INFO, "New Jumper!");
      if (i==0 || i%4!=0)
      {
        initialise_Jumper(jumpers[i],-1); //means that first "position" will be 0
      }
      else
      {
        initialise_Jumper(jumpers[i],1); //means that first "position" will be 2
      }
      jumpers[i]->live=true;
      render(jumpers[i], i);
      break;
    }
  }
}

void updateWorld()
{
  if (!game->gameInPlay)
    return;
  
  //update time
  ++game->game_time;
      
  //check if sprites need to be updated
  if (game->game_time - game->timeOfLastUpdate >= game->speed)
  {
    APP_LOG(APP_LOG_LEVEL_INFO, "Game Update");
    game->update = true;
    game->timeOfLastUpdate = game->game_time;
  }
  
  //this needs to happen outside the jumper update 
  handleCollisionsWithFiremen(); 
  
  if (game->update)
  {
    int liveJumpers=0;
    for (int i=0;i<NUMBER_OF_JUMPERS;++i)
    {
      if (jumpers[i]->live)
      {
        ++liveJumpers;
      }
    }
    APP_LOG(APP_LOG_LEVEL_INFO, "Number of Jumpers: %d", liveJumpers);
    updateJumpers();
    renderJumpers();
    
    game->update = false;
  }

  renderFire(game);  

  if ( game->game_time - game->timeOfLastJumperReleaseSpeedIncrease >= game->updateReleaseFrequency)
  {
    spawnNewJumper();
    if(game->updateReleaseFrequency >= game->maximumReleaseFrequency)
    {
      game->updateReleaseFrequency -= 5;
    }  
    game->timeOfLastJumperReleaseSpeedIncrease = game->game_time;
  }
  
  
  //handle speed increases
  if ( game->game_time - game->timeOfLastSpeedIncrease >= game->updateSpeedFrequency  )
  {
    
    if(game->speed >= game->maximumSpeed)
    {
      game->speed -= 2;
    }  
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
    bitmap_layer_set_bitmap(fireLayers[0], fireBitmaps[0]);
    bitmap_layer_set_bitmap(fireLayers[1], fireBitmaps[1]);
  }
  else if (localGameTime < 30)
  {
    bitmap_layer_set_bitmap(fireLayers[0], fireBitmaps[1]);
    bitmap_layer_set_bitmap(fireLayers[1], fireBitmaps[2]);
  }
  else if (localGameTime < 45)
  {
    bitmap_layer_set_bitmap(fireLayers[0], fireBitmaps[2]);
    bitmap_layer_set_bitmap(fireLayers[1], fireBitmaps[3]);
  }
  else
  {
    bitmap_layer_set_bitmap(fireLayers[0], fireBitmaps[3]);
    bitmap_layer_set_bitmap(fireLayers[1], fireBitmaps[0]);
  }
  
}

void render_MisterGameAndWatch(MrGameAndWatch* object)
{
    bitmap_layer_set_bitmap(s_mgw_layer, mgwBitmaps[object->position]);
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
    
    for (int i=0;i<NUMBER_OF_JUMPERS;++i)
    {
      layer_set_frame(bitmap_layer_get_layer(jumperBitmapLayers[i]), GRect(0, 0, 1, 1));
      initialise_Jumper(jumpers[i],0);
      
    }
    spawnNewJumper();
    renderJumpers();
    
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
  fireBitmaps[0] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_FIRE0);
  fireBitmaps[1] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_FIRE1);
  fireBitmaps[2] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_FIRE2);
  fireBitmaps[3] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_FIRE3);
  
  for (int i=0;i<2;++i)
  {
    fireLayers[i] =  bitmap_layer_create(GRect(0, 0, 60, 30));
    bitmap_layer_set_compositing_mode(fireLayers[i], GCompOpSet);
  }
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
  scoreLayer = text_layer_create(GRect(144-60,0,60,20));
  nameLayer = text_layer_create(GRect(0,40,160,60));
  restartTextLayer = text_layer_create(GRect(0,20,160,20));
  text_layer_set_background_color(scoreLayer, GColorClear);
  text_layer_set_background_color(nameLayer, GColorClear);
  text_layer_set_background_color(restartTextLayer, GColorClear);
  
  // Load the images
  s_background = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BG);
  mgwBitmaps[0] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MGW_LEFT);
  mgwBitmaps[1] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MGW_MIDDLE);
  mgwBitmaps[2] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MGW_RIGHT);
  
  crashBitmaps[0] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CRASH_LEFT);
  crashBitmaps[1] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CRASH_MIDDLE);
  crashBitmaps[2] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CRASH_RIGHT);
  
  // Create the BitmapLayers
  window_set_background_color(my_window,BACKGROUND_COLOUR);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  s_mgw_layer = bitmap_layer_create(GRect(0, 118, 144, 50));
  s_crash_layer = bitmap_layer_create(GRect(0, 138, 144, 30));
  
    
  // Set the correct compositing mode
  bitmap_layer_set_compositing_mode(s_background_layer, GCompOpSet);
  bitmap_layer_set_compositing_mode(s_mgw_layer, GCompOpSet);
  bitmap_layer_set_compositing_mode(s_crash_layer, GCompOpSet);
  
  bitmap_layer_set_bitmap(s_background_layer, s_background);
  
  initFire();
  
  startingImage0 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_START0);
  startingImage1 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_START1);
  
  
  //set mgw based on keys
  bitmap_layer_set_bitmap(s_mgw_layer, mgwBitmaps[1]);
  
  renderScores();
  
  // Add to the Window
  layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(s_background_layer));
  layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(s_mgw_layer));
  layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(s_crash_layer));
  for (int i=0;i<2;++i)
  {
    layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(fireLayers[i]));
  }
  
  layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(scoreLayer));
  layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(nameLayer));
  layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(restartTextLayer));
  
  jumperBitmaps[0] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_JUMP0);
  jumperBitmaps[1] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_JUMP1);
  jumperBitmaps[2] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_JUMP2);
  jumperBitmaps[3] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_JUMP3);
  jumperBitmaps[4] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_JUMP4);
  jumperBitmaps[5] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_JUMP5);
  jumperBitmaps[6] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_JUMP6);
  jumperBitmaps[7] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_JUMP7);
  jumperBitmaps[8] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_JUMP8);
  jumperBitmaps[9] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_JUMP9);
  jumperBitmaps[10] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_JUMP10);
  jumperBitmaps[11] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_JUMP11);
  jumperBitmaps[12] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_JUMP12);
  jumperBitmaps[13] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_JUMP13);
  jumperBitmaps[14] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_JUMP14);
  jumperBitmaps[15] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_JUMP15);
  jumperBitmaps[16] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_JUMP16);
  jumperBitmaps[17] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_JUMP17);
  jumperBitmaps[18] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_JUMP18);
  jumperBitmaps[19] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_JUMP19);
  jumperBitmaps[20] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_JUMP20);
  
  for (int i=0;i<NUMBER_OF_JUMPERS;++i)
  {
    jumpers[i] = malloc(sizeof(Jumper));
    initialise_Jumper(jumpers[i],0);
    jumperBitmapLayers[i] = bitmap_layer_create(GRect(0, 0, 144, 168));
    bitmap_layer_set_compositing_mode(jumperBitmapLayers[i], GCompOpSet);
    layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(jumperBitmapLayers[i]));
  }
  
  window_stack_push(my_window, true);
  
  
  //initialise game objects
  mgw = malloc(sizeof(MrGameAndWatch));
  initialise_MisterGameAndWatch(mgw);
  
  spawnNewJumper();
  
  light_enable(true);
  
}

void handle_deinit(void) 
{
  window_destroy(my_window);
  gbitmap_destroy(s_background);
  for (int i=0;i<3;++i)
  {
    gbitmap_destroy(mgwBitmaps[i]);
    gbitmap_destroy(crashBitmaps[i]);
  }
  for (int i=0;i<4;++i)
  {
    gbitmap_destroy(fireBitmaps[i]);
  }
  bitmap_layer_destroy(s_background_layer);
  bitmap_layer_destroy(s_mgw_layer);
  bitmap_layer_destroy(s_crash_layer);
  for (int i=0;i<2;++i)
  {
    bitmap_layer_destroy(fireLayers[i]);
  }
  
  text_layer_destroy(scoreLayer);
  text_layer_destroy(nameLayer);
  
  for (int i=0;i<NUMBER_OF_JUMPERS;++i)
  {  
    free(jumpers[i]);
    free(jumperBitmapLayers[i]);
  }
  for (int i=0;i<NUMBER_OF_JUMPER_IMAGES;++i)
  {  
    free(jumperBitmaps[i]);
  }
  
  free(startingImage0);
  free(startingImage1);

  free(mgw);
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
