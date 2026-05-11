# RP2040 WIZnet OPC UA demo server

Этот пример сделан на базе структуры `examples/mqtt`, но вместо MQTT поднимает минимальный OPC UA Binary server на WIZnet Ethernet HAT.

## Что реализовано

- TCP endpoint: `opc.tcp://192.168.11.2:4840` по умолчанию.
- SecurityPolicy: `None`.
- UserTokenPolicy: anonymous.
- Тестовая переменная: `ns=2;s=TestValue`.
- Значение переменной: `Int32 42`.
- Поддержанные OPC UA сервисы: `FindServers`, `GetEndpoints`, `OpenSecureChannel`, `CreateSession`, `ActivateSession`, `Read`, `CloseSession`.

Это демонстрационный минимальный сервер для проверки чтения одного значения. Browsing, subscriptions, write, full address space и сертификационная совместимость OPC UA здесь намеренно не реализованы.

## Установка в репозиторий

1. Скопируйте папку `opcua` в:

```text
RP-2040-Hat-FREERTOS-C/examples/opcua
```

2. В `RP-2040-Hat-FREERTOS-C/examples/CMakeLists.txt` добавьте:

```cmake
add_subdirectory(opcua)
```

3. Проверьте SPI-пины в:

```text
RP-2040-HAT-FREERTOS-C/port/ioLibrary_Driver/w5x00_spi.h
```

Пример из MQTT-проекта:

```c
#define SPI_PORT spi0
#define PIN_SCK 18
#define PIN_MOSI 19
#define PIN_MISO 16
#define PIN_CS 17
#define PIN_RST 20
```

4. При необходимости поменяйте IP-адрес в `w5x00_opcua.c`:

```c
static wiz_NetInfo g_net_info =
{
    .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56},
    .ip = {192, 168, 11, 2},
    .sn = {255, 255, 255, 0},
    .gw = {192, 168, 11, 1},
    .dns = {8, 8, 8, 8},
    .dhcp = NETINFO_STATIC
};
```

## Сборка и загрузка

После сборки должен появиться файл:

```text
build/examples/opcua/w5x00_opcua.uf2
```

Загрузите его на RP2040 через BOOTSEL как обычный `.uf2` файл.

## Проверка через Python OPC UA client

На ПК установите клиентскую библиотеку, если она ещё не установлена:

```bash
pip install opcua
```

Тест чтения:

```python
from opcua import Client

client = Client("opc.tcp://192.168.11.2:4840")
client.connect()
try:
    node = client.get_node("ns=2;s=TestValue")
    print(node.get_value())  # ожидается: 42
finally:
    client.disconnect()
```

## Проверка через UaExpert

1. Создайте endpoint `opc.tcp://192.168.11.2:4840`.
2. Выберите `SecurityPolicy=None`, `MessageSecurityMode=None`, anonymous.
3. Если browsing не покажет узел, прочитайте NodeId напрямую: `ns=2;s=TestValue`.

## Как заменить тестовое значение

В `w5x00_opcua.c` поменяйте:

```c
#define UA_TEST_VALUE 42
```

Либо замените `ua_put_datavalue_int32(&b, UA_TEST_VALUE);` на чтение из вашего датчика/регистра.
