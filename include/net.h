#ifndef net_h
#define net_h

#include <SDL_net.h>

int initNetwork(UDPsocket *sd, IPaddress *srvadd, UDPpacket **sendPacket, UDPpacket **receivePacket,
                int *is_server, int argc, char *argv[], int *pMyId);
void cleanUpNetwork(UDPsocket *sd, UDPpacket **sendPacket, UDPpacket **receivePacket);
void connectToServer(Player *pPlayer[MAX_NROFPLAYERS], bool is_server, int *pMy_id, IPaddress srvadd, int *pNrOfPlayers,
                     UDPpacket *pSendPacket, UDPpacket *pReceivePacket, UDPsocket sd);
#endif