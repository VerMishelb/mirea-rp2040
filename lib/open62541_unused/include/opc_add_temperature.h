#ifndef OPC_ADD_TEMPERATURE_H
#define OPC_ADD_TEMPERATURE_H

#include "open62541.h"

/**
 * ----------------------------------------------------------------------------------------------------
 * Declarations
 * ----------------------------------------------------------------------------------------------------
 */
extern float g_current_temperature;

extern float read_temperature();

static void addTempVariable(UA_Server *server);

static void addTempExternalDataSource(UA_Server *server);

static void addTempDataSourceVariable(UA_Server *server);

static UA_StatusCode writeTemp(UA_Server *server,
                                    const UA_NodeId *sessionId, void *sessionContext,
                                    const UA_NodeId *nodeId, void *nodeContext,
                                    const UA_NumericRange *range, const UA_DataValue *data);

static UA_StatusCode readTemp(UA_Server *server,
                                    const UA_NodeId *sessionId, void *sessionContext,
                                    const UA_NodeId *nodeId, void *nodeContext,
                                    UA_Boolean TempSource, const UA_NumericRange *range,
                                    UA_DataValue *dataValue);

static void addValueCallbackToTempVariable(UA_Server *server);

static void afterWriteTemp(UA_Server *server,
                            const UA_NodeId *sessionId, void *sessionContext,
                            const UA_NodeId *nodeId, void *nodeContext,
                            const UA_NumericRange *range, const UA_DataValue *data);

static void beforeReadTemp(UA_Server *server,
                            const UA_NodeId *sessionId, void *sessionContext,
                            const UA_NodeId *nodeid, void *nodeContext,
                            const UA_NumericRange *range, const UA_DataValue *data);

static void updateTempNode(UA_Server *server);

/**
 * ----------------------------------------------------------------------------------------------------
 * Definitions
 * ----------------------------------------------------------------------------------------------------
 */
static void addTempVariable(UA_Server *server)
{
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    UA_Float myTemp = g_current_temperature;
    UA_Variant_setScalarCopy(&attr.value, &myTemp, &UA_TYPES[UA_TYPES_FLOAT]);
    attr.description = UA_LOCALIZEDTEXT_ALLOC("en-US", "Temperature from Pico internal sensor");
    attr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", "Internal Temperature");
    UA_NodeId myTempNodeId = UA_NODEID_STRING_ALLOC(1, "InternalTemp");
    UA_QualifiedName myTempName = UA_QUALIFIEDNAME_ALLOC(1, "InternalTemperature");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);
    UA_StatusCode retval = UA_Server_addVariableNode(server, myTempNodeId, parentNodeId,
                                                    parentReferenceNodeId, myTempName,
                                                    variableTypeNodeId, attr, NULL, NULL);

    if (retval != UA_STATUSCODE_GOOD)
    {
        printf("[OPC UA]\tUA_Server_addVariableNode() Status: 0x%x (%s)\n", retval, UA_StatusCode_name(retval));
        while (1)
        {
        }
    }

    //updateTempNode(server);

    /* allocations on the heap need to be freed */
    UA_VariableAttributes_clear(&attr);
    UA_NodeId_clear(&myTempNodeId);
    UA_QualifiedName_clear(&myTempName);

    addValueCallbackToTempVariable(server);
}

static void updateTempNode(UA_Server *server)
{
    UA_Variant value;
    UA_Float newTemp = g_current_temperature;
    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "InternalTemp");
    UA_Variant_setScalar(&value, &newTemp, &UA_TYPES[UA_TYPES_FLOAT]);
    UA_Server_writeValue(server, currentNodeId, value);
}

static void
beforeReadTemp(UA_Server *server,
               const UA_NodeId *sessionId, void *sessionContext,
               const UA_NodeId *nodeid, void *nodeContext,
               const UA_NumericRange *range, const UA_DataValue *data) {
    updateTempNode(server);
}

static void
afterWriteTemp(UA_Server *server,
               const UA_NodeId *sessionId, void *sessionContext,
               const UA_NodeId *nodeId, void *nodeContext,
               const UA_NumericRange *range, const UA_DataValue *data)
{

}

static void
addValueCallbackToTempVariable(UA_Server *server) {
    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "InternalTemp");
    UA_ValueCallback callback;
    callback.onRead = beforeReadTemp;
    callback.onWrite = afterWriteTemp;
    UA_Server_setVariableNode_valueCallback(server, currentNodeId, callback);
}

static UA_StatusCode
readTemp(UA_Server *server,
                const UA_NodeId *sessionId, void *sessionContext,
                const UA_NodeId *nodeId, void *nodeContext,
                UA_Boolean TempSource, const UA_NumericRange *range,
                UA_DataValue *dataValue) {
    UA_Float newTemp = g_current_temperature;
    UA_Variant_setScalarCopy(&dataValue->value, &newTemp,
                             &UA_TYPES[UA_TYPES_FLOAT]);
    dataValue->hasValue = true;
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
writeTemp(UA_Server *server,
                 const UA_NodeId *sessionId, void *sessionContext,
                 const UA_NodeId *nodeId, void *nodeContext,
                 const UA_NumericRange *range, const UA_DataValue *data) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "Changing the internal temp is not implemented");
    return UA_STATUSCODE_BADINTERNALERROR;
}


static void
addTempDataSourceVariable(UA_Server *server) {
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT("en-US", "Internal-Temperature-datasource");
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "internal-Temperature-datasource");
    UA_QualifiedName currentName = UA_QUALIFIEDNAME(1, "internal-Temperature-datasource");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableTypeNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE);

    UA_DataSource TempSource;
    TempSource.read = readTemp;
    TempSource.write = writeTemp;
    UA_Server_addDataSourceVariableNode(server, currentNodeId, parentNodeId,
                                        parentReferenceNodeId, currentName,
                                        variableTypeNodeId, attr,
                                        TempSource, NULL, NULL);
}

static UA_DataValue *externalValue;

static void
addTempExternalDataSource(UA_Server *server) {
    UA_NodeId currentNodeId = UA_NODEID_STRING(1, "internal-Temperature-datasource");

    UA_ValueBackend valueBackend;
    valueBackend.backendType = UA_VALUEBACKENDTYPE_EXTERNAL;
    valueBackend.backend.external.value = &externalValue;

    UA_Server_setVariableNode_valueBackend(server, currentNodeId, valueBackend);
}

#endif