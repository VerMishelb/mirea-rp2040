#include "w5x00_open62541_net.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>

#include "socket.h"

#ifndef SF_TCP_NODELAY
#define SF_TCP_NODELAY 0x20
#endif

#ifndef SOCK_INIT
#define SOCK_INIT 0x13
#endif

#ifndef SOCK_LISTEN
#define SOCK_LISTEN 0x14
#endif

#ifndef SOCK_ESTABLISHED
#define SOCK_ESTABLISHED 0x17
#endif

#ifndef SOCK_CLOSE_WAIT
#define SOCK_CLOSE_WAIT 0x1C
#endif

#ifndef SOCK_CLOSED
#define SOCK_CLOSED 0x00
#endif

typedef struct {
    uint8_t socketNumber;
    uint16_t port;
    bool connectionActive;
    UA_Connection connection;
    UA_Logger logger;
} W5x00NetworkLayerContext;

static W5x00NetworkLayerContext *connectionToContext(UA_Connection *connection) {
    return (W5x00NetworkLayerContext *)connection->handle;
}

static UA_StatusCode w5x00GetSendBuffer(UA_Connection *connection,
                                         size_t length,
                                         UA_ByteString *buf) {
    (void)connection;
    return UA_ByteString_allocBuffer(buf, length);
}

static void w5x00ReleaseSendBuffer(UA_Connection *connection, UA_ByteString *buf) {
    (void)connection;
    UA_ByteString_clear(buf);
}

static void w5x00ReleaseRecvBuffer(UA_Connection *connection, UA_ByteString *buf) {
    (void)connection;
    UA_ByteString_clear(buf);
}

static UA_StatusCode w5x00Send(UA_Connection *connection, UA_ByteString *buf) {
    W5x00NetworkLayerContext *ctx = connectionToContext(connection);
    if(!ctx || !buf || !buf->data) {
        if(buf)
            UA_ByteString_clear(buf);
        return UA_STATUSCODE_BADINTERNALERROR;
    }

    size_t offset = 0;
    while(offset < buf->length) {
        const uint8_t sr = getSn_SR(ctx->socketNumber);
        if(sr != SOCK_ESTABLISHED && sr != SOCK_CLOSE_WAIT) {
            UA_ByteString_clear(buf);
            return UA_STATUSCODE_BADCONNECTIONCLOSED;
        }

        uint16_t chunk = (uint16_t)(buf->length - offset);
        if(chunk > 2048u)
            chunk = 2048u;

        int32_t ret = send(ctx->socketNumber, (uint8_t *)&buf->data[offset], chunk);
        if(ret > 0) {
            offset += (size_t)ret;
        } else {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }

    UA_ByteString_clear(buf);
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode w5x00Recv(UA_Connection *connection,
                                UA_ByteString *response,
                                UA_UInt32 timeout) {
    W5x00NetworkLayerContext *ctx = connectionToContext(connection);
    if(!ctx || !response)
        return UA_STATUSCODE_BADINTERNALERROR;

    uint32_t waited = 0;
    while(true) {
        uint8_t sr = getSn_SR(ctx->socketNumber);
        if(sr == SOCK_CLOSED)
            return UA_STATUSCODE_BADCONNECTIONCLOSED;

        uint16_t available = getSn_RX_RSR(ctx->socketNumber);
        if(available > 0u) {
            size_t length = available;
            if(length > connection->localConf.recvBufferSize)
                length = connection->localConf.recvBufferSize;

            if(response->data == NULL || response->length == 0u) {
                UA_StatusCode rc = UA_ByteString_allocBuffer(response, length);
                if(rc != UA_STATUSCODE_GOOD)
                    return rc;
            } else if(response->length < length) {
                length = response->length;
            }

            int32_t ret = recv(ctx->socketNumber, response->data, (uint16_t)length);
            if(ret > 0) {
                response->length = (size_t)ret;
                return UA_STATUSCODE_GOOD;
            }
        }

        if(timeout == 0u || waited >= timeout)
            return UA_STATUSCODE_BADCOMMUNICATIONERROR;

        vTaskDelay(pdMS_TO_TICKS(1));
        waited++;
    }
}

static void w5x00CloseConnection(UA_Connection *connection) {
    W5x00NetworkLayerContext *ctx = connectionToContext(connection);
    if(!ctx)
        return;

    disconnect(ctx->socketNumber);
    close(ctx->socketNumber);
    connection->state = UA_CONNECTIONSTATE_CLOSED;
}

static void w5x00FreeConnection(UA_Connection *connection) {
    W5x00NetworkLayerContext *ctx = connectionToContext(connection);
    if(!ctx)
        return;

    ctx->connectionActive = false;
    memset(connection, 0, sizeof(*connection));
}

static void w5x00InitConnection(W5x00NetworkLayerContext *ctx,
                                const UA_ConnectionConfig *localConfig) {
    UA_Connection *c = &ctx->connection;
    memset(c, 0, sizeof(*c));

    c->state = UA_CONNECTIONSTATE_OPENING;
    c->localConf = *localConfig;
    c->remoteConf = *localConfig;
    c->sockfd = (UA_SOCKET)ctx->socketNumber;
    c->handle = ctx;
    c->openingDate = UA_DateTime_now();

    c->getSendBuffer = w5x00GetSendBuffer;
    c->releaseSendBuffer = w5x00ReleaseSendBuffer;
    c->send = w5x00Send;
    c->recv = w5x00Recv;
    c->releaseRecvBuffer = w5x00ReleaseRecvBuffer;
    c->close = w5x00CloseConnection;
    c->free = w5x00FreeConnection;

    ctx->connectionActive = true;
}

static UA_StatusCode w5x00OpenListenSocket(W5x00NetworkLayerContext *ctx) {
    close(ctx->socketNumber);

    int8_t ret = socket(ctx->socketNumber, Sn_MR_TCP, ctx->port, SF_TCP_NODELAY);
    if(ret != ctx->socketNumber) {
        printf("open62541/W5x00: socket(%u) failed: %d\n", ctx->socketNumber, ret);
        return UA_STATUSCODE_BADINTERNALERROR;
    }

    ret = listen(ctx->socketNumber);
    if(ret != SOCK_OK) {
        printf("open62541/W5x00: listen(%u) failed: %d\n", ctx->socketNumber, ret);
        close(ctx->socketNumber);
        return UA_STATUSCODE_BADINTERNALERROR;
    }

    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode w5x00Start(UA_ServerNetworkLayer *nl,
                                const UA_Logger *logger,
                                const UA_String *customHostname) {
    (void)customHostname;

    W5x00NetworkLayerContext *ctx = (W5x00NetworkLayerContext *)nl->handle;
    if(!ctx)
        return UA_STATUSCODE_BADINTERNALERROR;

    if(logger)
        ctx->logger = *logger;

    UA_StatusCode rc = w5x00OpenListenSocket(ctx);
    if(rc == UA_STATUSCODE_GOOD)
        printf("open62541/W5x00: listening on socket %u, port %u\n", ctx->socketNumber, ctx->port);
    return rc;
}

static UA_StatusCode w5x00Listen(UA_ServerNetworkLayer *nl,
                                 UA_Server *server,
                                 UA_UInt16 timeout) {
    W5x00NetworkLayerContext *ctx = (W5x00NetworkLayerContext *)nl->handle;
    if(!ctx)
        return UA_STATUSCODE_BADINTERNALERROR;

    const uint8_t sr = getSn_SR(ctx->socketNumber);

    if(sr == SOCK_ESTABLISHED) {
        if(!ctx->connectionActive) {
            w5x00InitConnection(ctx, &nl->localConnectionConfig);
            printf("open62541/W5x00: OPC UA client connected\n");
        }

        uint16_t available = getSn_RX_RSR(ctx->socketNumber);
        if(available > 0u) {
            size_t length = available;
            if(length > nl->localConnectionConfig.recvBufferSize)
                length = nl->localConnectionConfig.recvBufferSize;

            UA_ByteString message = UA_BYTESTRING_NULL;
            UA_StatusCode rc = UA_ByteString_allocBuffer(&message, length);
            if(rc != UA_STATUSCODE_GOOD)
                return rc;

            int32_t ret = recv(ctx->socketNumber, message.data, (uint16_t)length);
            if(ret > 0) {
                message.length = (size_t)ret;
                UA_Server_processBinaryMessage(server, &ctx->connection, &message);
                /* UA_Server_processBinaryMessage releases the buffer via connection->releaseRecvBuffer. */
            } else {
                UA_ByteString_clear(&message);
            }
        }
    } else if(sr == SOCK_CLOSE_WAIT || sr == SOCK_CLOSED) {
        if(ctx->connectionActive) {
            printf("open62541/W5x00: OPC UA client disconnected\n");
            UA_Server_removeConnection(server, &ctx->connection);
        }
        disconnect(ctx->socketNumber);
        close(ctx->socketNumber);
        return w5x00OpenListenSocket(ctx);
    } else if(sr != SOCK_INIT && sr != SOCK_LISTEN) {
        close(ctx->socketNumber);
        return w5x00OpenListenSocket(ctx);
    }

    if(timeout > 0u)
        vTaskDelay(pdMS_TO_TICKS(timeout));
    else
        vTaskDelay(pdMS_TO_TICKS(1));

    return UA_STATUSCODE_GOOD;
}

static void w5x00Stop(UA_ServerNetworkLayer *nl, UA_Server *server) {
    W5x00NetworkLayerContext *ctx = (W5x00NetworkLayerContext *)nl->handle;
    if(!ctx)
        return;

    if(ctx->connectionActive)
        UA_Server_removeConnection(server, &ctx->connection);

    disconnect(ctx->socketNumber);
    close(ctx->socketNumber);
}

static void w5x00Clear(UA_ServerNetworkLayer *nl) {
    if(!nl)
        return;

    W5x00NetworkLayerContext *ctx = (W5x00NetworkLayerContext *)nl->handle;
    if(ctx)
        UA_free(ctx);

    UA_String_clear(&nl->discoveryUrl);
    memset(nl, 0, sizeof(*nl));
}

UA_ServerNetworkLayer UA_ServerNetworkLayerW5x00(uint8_t socketNumber,
                                                 uint16_t port,
                                                 const char *discoveryUrl,
                                                 uint32_t recvBufferSize,
                                                 uint32_t sendBufferSize) {
    UA_ServerNetworkLayer nl;
    memset(&nl, 0, sizeof(nl));

    W5x00NetworkLayerContext *ctx = (W5x00NetworkLayerContext *)UA_calloc(1, sizeof(W5x00NetworkLayerContext));
    if(!ctx)
        return nl;

    ctx->socketNumber = socketNumber;
    ctx->port = port;

    nl.handle = ctx;
    nl.discoveryUrl = UA_STRING_ALLOC(discoveryUrl ? discoveryUrl : "opc.tcp://0.0.0.0:4840");
    nl.localConnectionConfig.protocolVersion = 0;
    nl.localConnectionConfig.recvBufferSize = recvBufferSize;
    nl.localConnectionConfig.sendBufferSize = sendBufferSize;
    nl.localConnectionConfig.localMaxMessageSize = 0;
    nl.localConnectionConfig.remoteMaxMessageSize = 0;
    nl.localConnectionConfig.localMaxChunkCount = 0;
    nl.localConnectionConfig.remoteMaxChunkCount = 0;

    nl.start = w5x00Start;
    nl.listen = w5x00Listen;
    nl.stop = w5x00Stop;
    nl.clear = w5x00Clear;

    return nl;
}
