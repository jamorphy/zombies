#include "event.h"
#include <string.h>

typedef struct {
    EventListener listeners[MAX_LISTENERS];
    uint32_t listener_count;
} EventListeners;

static EventListeners event_listeners[EVENT_COUNT];

void event_init(void)
{
    memset(event_listeners, 0, sizeof(event_listeners));
}

void event_register(EventType type, EventListener listener)
{
    if (type >= EVENT_COUNT || event_listeners[type].listener_count >= MAX_LISTENERS) {
        return; // Silent fail TODO: add logging/assert later
    }
    event_listeners[type].listeners[event_listeners[type].listener_count++] = listener;
}

void event_send(EventType type, void* data)
{
    if (type >= EVENT_COUNT) return;

    EventListeners* listeners = &event_listeners[type];
    for (uint32_t i = 0; i < listeners->listener_count; i++) {
        if (listeners->listeners[i]) {
            listeners->listeners[i](data);
        }
    }
}
