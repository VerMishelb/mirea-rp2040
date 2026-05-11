#ifndef W5X00_OPEN62541_NET_H
#define W5X00_OPEN62541_NET_H

#include <stdint.h>
#include "open62541.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Create an open62541 1.3.x UA_ServerNetworkLayer backed by the WIZnet ioLibrary.
 *
 * This intentionally targets the open62541 1.3 network plugin API. In 1.4/1.5 the
 * networking architecture moved to EventLoop/ConnectionManager, so the port has to
 * be adapted before switching major open62541 branches.
 */
UA_ServerNetworkLayer UA_ServerNetworkLayerW5x00(uint8_t socketNumber,
                                                 uint16_t port,
                                                 const char *discoveryUrl,
                                                 uint32_t recvBufferSize,
                                                 uint32_t sendBufferSize);

#ifdef __cplusplus
}
#endif

#endif /* W5X00_OPEN62541_NET_H */
