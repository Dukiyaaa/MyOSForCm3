#include "tEvent.h"

void tEventInit(tEvent * event,tEventType type) {
	event->type = tEventTypeUnknow;
	tListInit(&(event->waitList));
}
