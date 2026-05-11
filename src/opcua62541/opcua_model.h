#ifndef OPCUA_MODEL_H
#define OPCUA_MODEL_H

#include <stdint.h>
#include <stdbool.h>
#include "open62541.h"

#ifdef __cplusplus
extern "C" {
#endif

void opcua_model_init_runtime(void);
void opcua_model_set_test_value(int32_t value);
int32_t opcua_model_get_test_value(void);
void opcua_model_set_led_state(bool state);
bool opcua_model_get_led_state(void);
UA_StatusCode opcua_model_add_nodes(UA_Server *server);
void opcua_model_periodic_update(UA_Server *server);

#ifdef __cplusplus
}
#endif

#endif /* OPCUA_MODEL_H */
