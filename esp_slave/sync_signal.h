#ifndef SYNC_SIGNAL_H
#define SYNC_SIGNAL_H

void setupSync();
void IRAM_ATTR onSyncReceived();
void sendAck();

#endif
