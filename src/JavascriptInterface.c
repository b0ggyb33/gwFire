#include <pebble.h>
#include "JavascriptInterface.h"

void outbox_sent_callback(DictionaryIterator *iterator, void *context) 
{
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) 
{
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

void sendScore(uint16_t score)
{
  // Begin dictionary
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  
  // Add a key-value pair
  DictionaryResult result = dict_write_uint16(iter, 1, score);
  switch(result)
  {
    case DICT_OK:
      APP_LOG(APP_LOG_LEVEL_INFO, "nothing went wrong in message construction");
      break;
    case DICT_INVALID_ARGS:
    case DICT_MALLOC_FAILED:
    case DICT_INTERNAL_INCONSISTENCY:
    case DICT_NOT_ENOUGH_STORAGE:
    default:
      APP_LOG(APP_LOG_LEVEL_INFO, "something went wrong in message construction");
      break;
  }
  uint32_t size = dict_write_end(iter);
  APP_LOG(APP_LOG_LEVEL_INFO, "size of dictionary: %u", (unsigned int)size);
  
  // Send the message!
  app_message_outbox_send();
  
}

