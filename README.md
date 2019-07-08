# libstomp-cpp

stomp protocol version 1.0/1.1 implementation by C++11.



## Requirement

- C++11
- libwebsockets (Optional, Preprocessing Definition HAS_LIBWEBSOCKETS Required)



## namespace & classe

| namespace/class | description |
| --------------- | ----------- |
| stomp | default namespace |
| stomp::Client | stomp client interface |
| stomp::Frame | stomp protocol frame class |
| stomp::FrameReader | websocket stream reader class for stomp protocol frame |
| stomp::LibwebsocketsClient | stomp client for libwebsockets |
| stomp::command | stomp commands namespace |


## Support feature and command

| type    | name                                  | support |
| ------- | ------------------------------------- | ------- |
| Feature | Heart-beat send                       | yes     |
| Feature | Heart-beat receive                    | yes     |
| Feature | Auto disconnect by Heart-beat timeout | not yet |
| Command | CONNECT / CONNECTED                   | yes     |
| Command | BEGIN                                 | yes     |
| Command | COMMIT                                | yes     |
| Command | ABORT                                 | yes     |
| Command | ACK                                   | yes     |
| Command | NACK                                  | yes     |
| Command | SEND                                  | yes     |
| Command | SUBSCRIBE                             | yes     |
| Command | UNSUBSCRIBE                           | yes     |
| Command | DISCONNECT                            | yes     |
| Command | MESSAGE                               | yes     |



## Example

sendCommand example:

```c++
stomp::command::Subscribe subscribe(this);
subscribe.destination("/topic/greetings");
this->sendCommand(&subscribe);
```
