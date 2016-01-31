#pragma once
#include <pebble.h>

void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context);
void outbox_sent_callback(DictionaryIterator *iterator, void *context);

void sendScore(uint16_t score);