#include "opcua_model.h"
#include "w5x00_opcua_config.h"

#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

#include "pico/stdlib.h"

static SemaphoreHandle_t g_valueMutex;
static int32_t g_testValue = OPCUA_TEST_VALUE_DEFAULT;
static bool g_ledState = false;
static uint16_t g_nsIndex = 1;
static UA_NodeId g_testValueNodeId;
static UA_NodeId g_uptimeNodeId;
static UA_NodeId g_ledStateNodeId;

void opcua_model_init_runtime(void) {
    if(g_valueMutex == NULL)
        g_valueMutex = xSemaphoreCreateMutex();

    g_testValue = OPCUA_TEST_VALUE_DEFAULT;
    g_ledState = false;

#ifdef PICO_DEFAULT_LED_PIN
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, g_ledState ? 1 : 0);
#endif
}

void opcua_model_set_test_value(int32_t value) {
    if(g_valueMutex && xSemaphoreTake(g_valueMutex, portMAX_DELAY) == pdTRUE) {
        g_testValue = value;
        xSemaphoreGive(g_valueMutex);
    }
}

int32_t opcua_model_get_test_value(void) {
    int32_t value = 0;
    if(g_valueMutex && xSemaphoreTake(g_valueMutex, portMAX_DELAY) == pdTRUE) {
        value = g_testValue;
        xSemaphoreGive(g_valueMutex);
    }
    return value;
}

void opcua_model_set_led_state(bool state) {
    if(g_valueMutex && xSemaphoreTake(g_valueMutex, portMAX_DELAY) == pdTRUE) {
        g_ledState = state;
        xSemaphoreGive(g_valueMutex);
    }

#ifdef PICO_DEFAULT_LED_PIN
    gpio_put(PICO_DEFAULT_LED_PIN, state ? 1 : 0);
#endif
}

bool opcua_model_get_led_state(void) {
    bool state = false;
    if(g_valueMutex && xSemaphoreTake(g_valueMutex, portMAX_DELAY) == pdTRUE) {
        state = g_ledState;
        xSemaphoreGive(g_valueMutex);
    }
    return state;
}

static UA_StatusCode readTestValue(UA_Server *server,
                                   const UA_NodeId *sessionId, void *sessionContext,
                                   const UA_NodeId *nodeId, void *nodeContext,
                                   UA_Boolean sourceTimeStamp,
                                   const UA_NumericRange *range,
                                   UA_DataValue *dataValue) {
    (void)server;
    (void)sessionId;
    (void)sessionContext;
    (void)nodeId;
    (void)nodeContext;
    (void)sourceTimeStamp;
    (void)range;

    UA_Int32 value = (UA_Int32)opcua_model_get_test_value();
    UA_Variant_setScalarCopy(&dataValue->value, &value, &UA_TYPES[UA_TYPES_INT32]);
    dataValue->hasValue = true;
    dataValue->hasSourceTimestamp = true;
    dataValue->sourceTimestamp = UA_DateTime_now();
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode writeTestValue(UA_Server *server,
                                    const UA_NodeId *sessionId, void *sessionContext,
                                    const UA_NodeId *nodeId, void *nodeContext,
                                    const UA_NumericRange *range,
                                    const UA_DataValue *data) {
    (void)server;
    (void)sessionId;
    (void)sessionContext;
    (void)nodeId;
    (void)nodeContext;
    (void)range;

    if(!data || !data->hasValue || !UA_Variant_hasScalarType(&data->value, &UA_TYPES[UA_TYPES_INT32]))
        return UA_STATUSCODE_BADTYPEMISMATCH;

    UA_Int32 value = *(UA_Int32 *)data->value.data;
    opcua_model_set_test_value((int32_t)value);
    printf("OPC UA: TestValue written = %ld\n", (long)value);
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode readUptime(UA_Server *server,
                                const UA_NodeId *sessionId, void *sessionContext,
                                const UA_NodeId *nodeId, void *nodeContext,
                                UA_Boolean sourceTimeStamp,
                                const UA_NumericRange *range,
                                UA_DataValue *dataValue) {
    (void)server;
    (void)sessionId;
    (void)sessionContext;
    (void)nodeId;
    (void)nodeContext;
    (void)sourceTimeStamp;
    (void)range;

    UA_UInt64 value = (UA_UInt64)(xTaskGetTickCount() * portTICK_PERIOD_MS);
    UA_Variant_setScalarCopy(&dataValue->value, &value, &UA_TYPES[UA_TYPES_UINT64]);
    dataValue->hasValue = true;
    dataValue->hasSourceTimestamp = true;
    dataValue->sourceTimestamp = UA_DateTime_now();
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode readLedState(UA_Server *server,
                                  const UA_NodeId *sessionId, void *sessionContext,
                                  const UA_NodeId *nodeId, void *nodeContext,
                                  UA_Boolean sourceTimeStamp,
                                  const UA_NumericRange *range,
                                  UA_DataValue *dataValue) {
    (void)server;
    (void)sessionId;
    (void)sessionContext;
    (void)nodeId;
    (void)nodeContext;
    (void)sourceTimeStamp;
    (void)range;

    UA_Boolean value = opcua_model_get_led_state() ? true : false;
    UA_Variant_setScalarCopy(&dataValue->value, &value, &UA_TYPES[UA_TYPES_BOOLEAN]);
    dataValue->hasValue = true;
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode writeLedState(UA_Server *server,
                                   const UA_NodeId *sessionId, void *sessionContext,
                                   const UA_NodeId *nodeId, void *nodeContext,
                                   const UA_NumericRange *range,
                                   const UA_DataValue *data) {
    (void)server;
    (void)sessionId;
    (void)sessionContext;
    (void)nodeId;
    (void)nodeContext;
    (void)range;

    if(!data || !data->hasValue || !UA_Variant_hasScalarType(&data->value, &UA_TYPES[UA_TYPES_BOOLEAN]))
        return UA_STATUSCODE_BADTYPEMISMATCH;

    UA_Boolean value = *(UA_Boolean *)data->value.data;
    opcua_model_set_led_state(value ? true : false);
    printf("OPC UA: LedState written = %u\n", value ? 1u : 0u);
    return UA_STATUSCODE_GOOD;
}

#ifdef UA_ENABLE_METHODCALLS
static UA_StatusCode resetTestValueMethod(UA_Server *server,
                                          const UA_NodeId *sessionId, void *sessionContext,
                                          const UA_NodeId *methodId, void *methodContext,
                                          const UA_NodeId *objectId, void *objectContext,
                                          size_t inputSize, const UA_Variant *input,
                                          size_t outputSize, UA_Variant *output) {
    (void)server;
    (void)sessionId;
    (void)sessionContext;
    (void)methodId;
    (void)methodContext;
    (void)objectId;
    (void)objectContext;
    (void)inputSize;
    (void)input;

    opcua_model_set_test_value(OPCUA_TEST_VALUE_DEFAULT);
    UA_Int32 value = OPCUA_TEST_VALUE_DEFAULT;
    if(outputSize > 0)
        UA_Variant_setScalarCopy(&output[0], &value, &UA_TYPES[UA_TYPES_INT32]);
    printf("OPC UA: ResetTestValue method called\n");
    return UA_STATUSCODE_GOOD;
}
#endif

static UA_StatusCode addDataSourceVariable(UA_Server *server,
                                           UA_NodeId nodeId,
                                           const char *browseName,
                                           const char *description,
                                           const UA_DataType *dataType,
                                           UA_Byte accessLevel,
                                           UA_DataSource dataSource,
                                           UA_NodeId parentNodeId) {
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT("en-US", (char *)browseName);
    attr.description = UA_LOCALIZEDTEXT("en-US", (char *)description);
    attr.dataType = dataType->typeId;
    attr.accessLevel = accessLevel;
    attr.userAccessLevel = accessLevel;

    UA_QualifiedName qn = UA_QUALIFIEDNAME(g_nsIndex, (char *)browseName);
    return UA_Server_addDataSourceVariableNode(server,
                                               nodeId,
                                               parentNodeId,
                                               UA_NS0ID(ORGANIZES),
                                               qn,
                                               UA_NS0ID(BASEDATAVARIABLETYPE),
                                               attr,
                                               dataSource,
                                               NULL,
                                               NULL);
}

UA_StatusCode opcua_model_add_nodes(UA_Server *server) {
    if(!server)
        return UA_STATUSCODE_BADINTERNALERROR;

    g_nsIndex = UA_Server_addNamespace(server, OPCUA_APP_NAMESPACE_URI);

    UA_ObjectAttributes objectAttr = UA_ObjectAttributes_default;
    objectAttr.displayName = UA_LOCALIZEDTEXT("en-US", "RP2040");
    objectAttr.description = UA_LOCALIZEDTEXT("en-US", "RP2040 WIZnet HAT demo object");

    UA_NodeId rp2040ObjectId = UA_NODEID_STRING(g_nsIndex, "RP2040");
    UA_StatusCode rc = UA_Server_addObjectNode(server,
                                               rp2040ObjectId,
                                               UA_NS0ID(OBJECTSFOLDER),
                                               UA_NS0ID(ORGANIZES),
                                               UA_QUALIFIEDNAME(g_nsIndex, "RP2040"),
                                               UA_NS0ID(BASEOBJECTTYPE),
                                               objectAttr,
                                               NULL,
                                               NULL);
    if(rc != UA_STATUSCODE_GOOD)
        return rc;

    g_testValueNodeId = UA_NODEID_STRING_ALLOC(g_nsIndex, OPCUA_NODE_TEST_VALUE_STRING);
    g_uptimeNodeId = UA_NODEID_STRING_ALLOC(g_nsIndex, OPCUA_NODE_UPTIME_STRING);
    g_ledStateNodeId = UA_NODEID_STRING_ALLOC(g_nsIndex, OPCUA_NODE_LED_STATE_STRING);

    UA_DataSource testValueDs;
    memset(&testValueDs, 0, sizeof(testValueDs));
    testValueDs.read = readTestValue;
    testValueDs.write = writeTestValue;
    rc = addDataSourceVariable(server,
                               g_testValueNodeId,
                               "TestValue",
                               "Read/write Int32 test value protected by a FreeRTOS mutex",
                               &UA_TYPES[UA_TYPES_INT32],
                               UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE,
                               testValueDs,
                               rp2040ObjectId);
    if(rc != UA_STATUSCODE_GOOD)
        return rc;

    UA_DataSource uptimeDs;
    memset(&uptimeDs, 0, sizeof(uptimeDs));
    uptimeDs.read = readUptime;
    rc = addDataSourceVariable(server,
                               g_uptimeNodeId,
                               "UptimeMs",
                               "FreeRTOS tick-based uptime in milliseconds",
                               &UA_TYPES[UA_TYPES_UINT64],
                               UA_ACCESSLEVELMASK_READ,
                               uptimeDs,
                               rp2040ObjectId);
    if(rc != UA_STATUSCODE_GOOD)
        return rc;

    UA_DataSource ledDs;
    memset(&ledDs, 0, sizeof(ledDs));
    ledDs.read = readLedState;
    ledDs.write = writeLedState;
    rc = addDataSourceVariable(server,
                               g_ledStateNodeId,
                               "LedState",
                               "Read/write onboard LED state if PICO_DEFAULT_LED_PIN is available",
                               &UA_TYPES[UA_TYPES_BOOLEAN],
                               UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE,
                               ledDs,
                               rp2040ObjectId);
    if(rc != UA_STATUSCODE_GOOD)
        return rc;

#ifdef UA_ENABLE_METHODCALLS
    UA_Argument outputArgument;
    UA_Argument_init(&outputArgument);
    outputArgument.description = UA_LOCALIZEDTEXT("en-US", "Reset value");
    outputArgument.name = UA_STRING("value");
    outputArgument.dataType = UA_TYPES[UA_TYPES_INT32].typeId;
    outputArgument.valueRank = UA_VALUERANK_SCALAR;

    UA_MethodAttributes methodAttr = UA_MethodAttributes_default;
    methodAttr.displayName = UA_LOCALIZEDTEXT("en-US", "ResetTestValue");
    methodAttr.description = UA_LOCALIZEDTEXT("en-US", "Reset TestValue to the default value 42");
    methodAttr.executable = true;
    methodAttr.userExecutable = true;

    rc = UA_Server_addMethodNode(server,
                                 UA_NODEID_STRING(g_nsIndex, OPCUA_NODE_RESET_METHOD),
                                 rp2040ObjectId,
                                 UA_NS0ID(HASCOMPONENT),
                                 UA_QUALIFIEDNAME(g_nsIndex, "ResetTestValue"),
                                 methodAttr,
                                 resetTestValueMethod,
                                 0, NULL,
                                 1, &outputArgument,
                                 NULL,
                                 NULL);
    if(rc != UA_STATUSCODE_GOOD)
        return rc;
#endif

    printf("OPC UA: namespace index = %u\n", (unsigned)g_nsIndex);
    printf("OPC UA: browse path Objects/RP2040/TestValue\n");
    printf("OPC UA: NodeIds: ns=%u;s=TestValue, ns=%u;s=UptimeMs, ns=%u;s=LedState\n",
           (unsigned)g_nsIndex, (unsigned)g_nsIndex, (unsigned)g_nsIndex);

    return UA_STATUSCODE_GOOD;
}

void opcua_model_periodic_update(UA_Server *server) {
    (void)server;
    /*
     * Values are exposed through DataSource callbacks, so there is no polling
     * write required here. Keep this hook for future sensor updates.
     */
}
