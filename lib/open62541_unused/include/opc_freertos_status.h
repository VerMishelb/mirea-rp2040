#ifndef OPC_FREERTOS_STATUS_H
#define OPC_FREERTOS_STATUS_H

#include "open62541.h"
#include <FreeRTOS.h>

/**
 * ----------------------------------------------------------------------------------------------------
 * Declarations
 * ----------------------------------------------------------------------------------------------------
 */
static void addGetFreeHeapSizeVariable(UA_Server *server);
static void addValueCallbackToGetFreeHeapSizeVariable(UA_Server *server);
static void beforeWriteGetFreeHeapSize(UA_Server *server,
                            const UA_NodeId *sessionId, void *sessionContext,
                            const UA_NodeId *nodeId, void *nodeContext,
                            const UA_NumericRange *range, const UA_DataValue *data);
static void afterWriteGetFreeHeapSize(UA_Server *server,
                            const UA_NodeId *sessionId, void *sessionContext,
                            const UA_NodeId *nodeid, void *nodeContext,
                            const UA_NumericRange *range, const UA_DataValue *data);
static void updateGetFreeHeapSizeNode(UA_Server *server);

static void addGetMinimumEverFreeHeapSizeVariable(UA_Server *server);
static void addValueCallbackToGetMinimumEverFreeHeapSizeVariable(UA_Server *server);
static void beforeWriteGetMinimumEverFreeHeapSize(UA_Server *server,
                            const UA_NodeId *sessionId, void *sessionContext,
                            const UA_NodeId *nodeId, void *nodeContext,
                            const UA_NumericRange *range, const UA_DataValue *data);
static void afterWriteGetMinimumEverFreeHeapSize(UA_Server *server,
                            const UA_NodeId *sessionId, void *sessionContext,
                            const UA_NodeId *nodeid, void *nodeContext,
                            const UA_NumericRange *range, const UA_DataValue *data);
static void updateGetMinimumEverFreeHeapSizeNode(UA_Server *server);

static void addGetHeapStatsVariable(UA_Server *server);
static void addValueCallbackToGetHeapStatsVariable(UA_Server *server);
static void beforeWriteGetHeapStats(UA_Server *server,
                            const UA_NodeId *sessionId, void *sessionContext,
                            const UA_NodeId *nodeId, void *nodeContext,
                            const UA_NumericRange *range, const UA_DataValue *data);
static void afterWriteGetHeapStats(UA_Server *server,
                            const UA_NodeId *sessionId, void *sessionContext,
                            const UA_NodeId *nodeid, void *nodeContext,
                            const UA_NumericRange *range, const UA_DataValue *data);
static void updateGetHeapStatsNode(UA_Server *server);

/**
 * ----------------------------------------------------------------------------------------------------
 * xPortGetFreeHeapSize
 * ----------------------------------------------------------------------------------------------------
 */
static void addGetFreeHeapSizeVariable(UA_Server *server)
{
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    UA_UInt32 FreeHeapSize = xPortGetFreeHeapSize();
    UA_Variant_setScalarCopy(&attr.value, &FreeHeapSize, &UA_TYPES[UA_TYPES_UINT32]);
    attr.description = UA_LOCALIZEDTEXT_ALLOC("en-US", "Avaliable heap size from xPortGetFreeHeapSize() API-func");
    attr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", "Free Heap Size");
    UA_NodeId myTempNodeId = UA_NODEID_STRING_ALLOC(1, "FreeHeapSize");
    UA_QualifiedName myMemoryVarName = UA_QUALIFIEDNAME_ALLOC(1, "FreeHeapSize");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
    UA_StatusCode retval = UA_Server_addVariableNode(server, myTempNodeId, parentNodeId,
                                                    parentReferenceNodeId, myMemoryVarName,
                                                    variableTypeNodeId, attr, NULL, NULL);

    if (retval != UA_STATUSCODE_GOOD)
    {
        printf("[OPC UA]\tUA_Server_addVariableNode() Status: 0x%x (%s)\n", retval, UA_StatusCode_name(retval));
        while (1)
        {
        }
    }

    updateGetFreeHeapSizeNode(server);

    /* allocations on the heap need to be freed */
    UA_VariableAttributes_clear(&attr);
    UA_NodeId_clear(&myTempNodeId);
    UA_QualifiedName_clear(&myMemoryVarName);

    addValueCallbackToGetFreeHeapSizeVariable(server);
}

static void updateGetFreeHeapSizeNode(UA_Server *server)
{
    UA_Variant value;
    UA_UInt32 newMemory = xPortGetFreeHeapSize();
    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "FreeHeapSize");
    UA_Variant_setScalar(&value, &newMemory, &UA_TYPES[UA_TYPES_UINT32]);
    UA_Server_writeValue(server, currentNodeId, value);
}

static void
beforeWriteGetFreeHeapSize(UA_Server *server,
               const UA_NodeId *sessionId, void *sessionContext,
               const UA_NodeId *nodeid, void *nodeContext,
               const UA_NumericRange *range, const UA_DataValue *data) {
    updateGetFreeHeapSizeNode(server);
}

static void
afterWriteGetFreeHeapSize(UA_Server *server,
               const UA_NodeId *sessionId, void *sessionContext,
               const UA_NodeId *nodeId, void *nodeContext,
               const UA_NumericRange *range, const UA_DataValue *data)
{

}

static void
addValueCallbackToGetFreeHeapSizeVariable(UA_Server *server) {
    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "FreeHeapSize");
    UA_ValueCallback callback;
    callback.onRead = beforeWriteGetFreeHeapSize;
    callback.onWrite = afterWriteGetFreeHeapSize;
    UA_Server_setVariableNode_valueCallback(server, currentNodeId, callback);
}

/**
 * ----------------------------------------------------------------------------------------------------
 * xPortGetMinimumEverFreeHeapSize
 * ----------------------------------------------------------------------------------------------------
 */
static void addGetMinimumEverFreeHeapSizeVariable(UA_Server *server)
{
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    UA_UInt32 FreeHeapSize = xPortGetMinimumEverFreeHeapSize();
    UA_Variant_setScalarCopy(&attr.value, &FreeHeapSize, &UA_TYPES[UA_TYPES_UINT32]);
    attr.description = UA_LOCALIZEDTEXT_ALLOC("en-US", "Minimal heap size , which was available. xPortGetMinimumEverFreeHeapSize() API-func");
    attr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", "Minimum Ever Free Heap Size");
    UA_NodeId myTempNodeId = UA_NODEID_STRING_ALLOC(1, "MinimumEverFreeHeapSize");
    UA_QualifiedName myMemoryVarName = UA_QUALIFIEDNAME_ALLOC(1, "MinimumEverFreeHeapSize");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
    UA_StatusCode retval = UA_Server_addVariableNode(server, myTempNodeId, parentNodeId,
                                                    parentReferenceNodeId, myMemoryVarName,
                                                    variableTypeNodeId, attr, NULL, NULL);

    if (retval != UA_STATUSCODE_GOOD)
    {
        printf("[OPC UA]\tUA_Server_addVariableNode() Status: 0x%x (%s)\n", retval, UA_StatusCode_name(retval));
        while (1)
        {
        }
    }

    updateGetMinimumEverFreeHeapSizeNode(server);

    /* allocations on the heap need to be freed */
    UA_VariableAttributes_clear(&attr);
    UA_NodeId_clear(&myTempNodeId);
    UA_QualifiedName_clear(&myMemoryVarName);

    addValueCallbackToGetMinimumEverFreeHeapSizeVariable(server);
}

static void updateGetMinimumEverFreeHeapSizeNode(UA_Server *server)
{
    UA_Variant value;
    UA_UInt32 newMemory = xPortGetMinimumEverFreeHeapSize();
    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "MinimumEverFreeHeapSize");
    UA_Variant_setScalar(&value, &newMemory, &UA_TYPES[UA_TYPES_UINT32]);
    UA_Server_writeValue(server, currentNodeId, value);
}

static void
beforeWriteGetMinimumEverFreeHeapSize(UA_Server *server,
               const UA_NodeId *sessionId, void *sessionContext,
               const UA_NodeId *nodeid, void *nodeContext,
               const UA_NumericRange *range, const UA_DataValue *data) {
    updateGetMinimumEverFreeHeapSizeNode(server);
}

static void
afterWriteGetMinimumEverFreeHeapSize(UA_Server *server,
               const UA_NodeId *sessionId, void *sessionContext,
               const UA_NodeId *nodeId, void *nodeContext,
               const UA_NumericRange *range, const UA_DataValue *data)
{

}

static void
addValueCallbackToGetMinimumEverFreeHeapSizeVariable(UA_Server *server) {
    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "MinimumEverFreeHeapSize");
    UA_ValueCallback callback;
    callback.onRead = beforeWriteGetMinimumEverFreeHeapSize;
    callback.onWrite = afterWriteGetMinimumEverFreeHeapSize;
    UA_Server_setVariableNode_valueCallback(server, currentNodeId, callback);
}

/**
 * ----------------------------------------------------------------------------------------------------
 * vPortGetHeapStats
 * ----------------------------------------------------------------------------------------------------
 */
static void addGetHeapStatsVariable(UA_Server *server)
{
    HeapStats_t pxHeapStats;
    vPortGetHeapStats(&pxHeapStats);
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    UA_UInt32 *HeapStats = (UA_UInt32 *) UA_Array_new(7, &UA_TYPES[UA_TYPES_UINT32]);
    HeapStats[0] = pxHeapStats.xAvailableHeapSpaceInBytes;
    HeapStats[1] = pxHeapStats.xMinimumEverFreeBytesRemaining;
    HeapStats[2] = pxHeapStats.xNumberOfFreeBlocks;
    HeapStats[3] = pxHeapStats.xNumberOfSuccessfulAllocations;
    HeapStats[4] = pxHeapStats.xNumberOfSuccessfulFrees;
    HeapStats[5] = pxHeapStats.xSizeOfLargestFreeBlockInBytes;
    HeapStats[6] = pxHeapStats.xSizeOfSmallestFreeBlockInBytes;
    UA_Variant_setArray(&attr.value, HeapStats, 7, &UA_TYPES[UA_TYPES_UINT32]);
    attr.valueRank = UA_VALUERANK_ANY;
    UA_UInt32 HeapStatsDimensions[1] = {7};
    attr.value.arrayDimensions = HeapStatsDimensions;
    attr.value.arrayDimensionsSize = 1;
    attr.description = UA_LOCALIZEDTEXT_ALLOC("en-US", "Heap stats from vPortGetHeapStats() API-func");
    attr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", "Heap Stats");
    UA_NodeId myTempNodeId = UA_NODEID_STRING_ALLOC(1, "HeapStats");
    UA_QualifiedName myMemoryVarName = UA_QUALIFIEDNAME_ALLOC(1, "HeapStats");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
    UA_StatusCode retval = UA_Server_addVariableNode(server, myTempNodeId, parentNodeId,
                                                    parentReferenceNodeId, myMemoryVarName,
                                                    variableTypeNodeId, attr, NULL, NULL);

    if (retval != UA_STATUSCODE_GOOD)
    {
        printf("[OPC UA]\tUA_Server_addVariableNode() Status: 0x%x (%s)\n", retval, UA_StatusCode_name(retval));
        while (1)
        {
        }
    }

    updateGetHeapStatsNode(server);

    /* allocations on the heap need to be freed */
    UA_Array_delete(HeapStats, 7, &UA_TYPES[UA_TYPES_UINT32]);
    //UA_VariableAttributes_clear(&attr);
    UA_NodeId_clear(&myTempNodeId);
    UA_QualifiedName_clear(&myMemoryVarName);

    addValueCallbackToGetHeapStatsVariable(server);
}

static void updateGetHeapStatsNode(UA_Server *server)
{
    UA_Variant value;
    HeapStats_t pxHeapStats;
    vPortGetHeapStats(&pxHeapStats);
    UA_UInt32 *HeapStats = (UA_UInt32 *) UA_Array_new(7, &UA_TYPES[UA_TYPES_UINT32]);
    HeapStats[0] = pxHeapStats.xAvailableHeapSpaceInBytes;
    HeapStats[1] = pxHeapStats.xMinimumEverFreeBytesRemaining;
    HeapStats[2] = pxHeapStats.xNumberOfFreeBlocks;
    HeapStats[3] = pxHeapStats.xNumberOfSuccessfulAllocations;
    HeapStats[4] = pxHeapStats.xNumberOfSuccessfulFrees;
    HeapStats[5] = pxHeapStats.xSizeOfLargestFreeBlockInBytes;
    HeapStats[6] = pxHeapStats.xSizeOfSmallestFreeBlockInBytes;
    UA_Variant_setArray(&value, HeapStats, 7, &UA_TYPES[UA_TYPES_UINT32]);
    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "HeapStats");
    UA_Variant_setScalar(&value, &HeapStats, &UA_TYPES[UA_TYPES_UINT32]);
    UA_Server_writeValue(server, currentNodeId, value);

    UA_Array_delete(HeapStats, 7, &UA_TYPES[UA_TYPES_UINT32]);
}

static void
beforeWriteGetHeapStats(UA_Server *server,
               const UA_NodeId *sessionId, void *sessionContext,
               const UA_NodeId *nodeid, void *nodeContext,
               const UA_NumericRange *range, const UA_DataValue *data) {
    updateGetHeapStatsNode(server);
}

static void
afterWriteGetHeapStats(UA_Server *server,
               const UA_NodeId *sessionId, void *sessionContext,
               const UA_NodeId *nodeId, void *nodeContext,
               const UA_NumericRange *range, const UA_DataValue *data)
{

}

static void
addValueCallbackToGetHeapStatsVariable(UA_Server *server) {
    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "HeapStats");
    UA_ValueCallback callback;
    callback.onRead = beforeWriteGetHeapStats;
    callback.onWrite = afterWriteGetHeapStats;
    UA_Server_setVariableNode_valueCallback(server, currentNodeId, callback);
}

#endif // !OPC_FREERTOS_STATUS_H