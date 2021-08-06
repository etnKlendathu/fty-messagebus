# fty-common-messagebus2

## Dependencies
* [fty-cmake](https://github.com/42ity/fty-cmake/)
* [fty_common_mlm](https://github.com/42ity/fty-common-mlm)
* [fty_common_logging](https://github.com/42ity/fty-common-logging)
* [PahoMqttC](https://github.com/eclipse/paho.mqtt.c)
* [PahoMqttCpp](https://github.com/eclipse/paho.mqtt.cpp)
* [googletest](https://github.com/google/googletest)

## Description
This project is developped on the purpose to provide somme common methods to address Request/Reply, and Publish/Subscribe patterns above AMQP, Malamute or Mqtt message bus.

## How to build

To build fty-```cmake common-messagebus2 project run:

```cmake
cmake -B build -DBUILD_ALL=ON
cmake --build build
```

#### Build options

| Option                       | description                                  | acceptable value      | default value           |
|------------------------------|----------------------------------------------|-----------------------|-------------------------|
| BUILD_ALL_ADD_ON             | Build all addons                             | ON\|OFF               | ON                      |
| BUILD_AMQP                   | Enable AMQP addon                            | ON\|OFF               | ON                      |
| BUILD_MALAMUTE               | Enable Malamute addon                        | ON\|OFF               | ON                      |
| BUILD_MQTT                   | Enable Mqtt addon                            | ON\|OFF               | ON                      |
| BUILD_SAMPLES                | Enable samples build                         | ON\|OFF               | OFF                     |
| BUILD_TESTING                | Add test compilation                         | ON\|OFF               | ON                      |
| BUILD_DOC                    | Build documentation                          | ON\|OFF               | OFF                     |


## How to use the dependency in your project

Add the dependency in CMakeList.txt:

```cmake
etn_target(${PROJECT_NAME}
  SOURCES
    .....
  USES_PUBLIC
    .....
    fty_common_messagebus2
    .....
)
```

## Howto

See all samples in samples folder.

### Mqtt samples

* [PubSub](samples/mqtt/src/FtyCommonMessagebusMqttSamplePubSub.cpp)
* [WaitRequest](samples/mqtt/src/FtyCommonMessagebusMqttSampleAsyncReply.cpp)
* [SendRequest](samples/mqtt/src/FtyCommonMessagebusMqttSampleSendRequest.cpp)

### Amqp samples

* [PubSub](samples/amqp/src/FtyCommonMessagebusAmqpSamplePubSub.cpp)
* [WaitRequest](samples/amqp/src/FtyCommonMessagebusAmqpSampleAsyncReply.cpp)
* [SendRequest](samples/amqp/src/FtyCommonMessagebusAmqpSampleSendRequest.cpp)

### Malamute samples

* [PubSub](samples/malamute/src/FtyCommonMessagebusMlmSamplePubSub.cpp)
* [WaitRequest](samples/malamute/src/FtyCommonMessagebusMlmSampleReply.cpp)
* [SendRequest](samples/malamute/src/FtyCommonMessagebusMlmSampleRequest.cpp)

## Change log

[Change log](CHANGELOG.md) provides informations about bug fixing, improvement, etc.
