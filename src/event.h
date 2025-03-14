#pragma once

#include "ecs.h"
#include <stdint.h>

#define MAX_LISTENERS 16

typedef enum {
    EVENT_SHOOT,
    EVENT_COUNT // Must be last
} EventType;

typedef struct {
    Entity shooter;
    vec3 position;
    vec3 direction;
} ShootEvent;

typedef void (*EventListener)(void* data);

void event_init(void);
void event_register(EventType type, EventListener listener);
void event_send(EventType type, void* data);
