#include <Arduino.h>

class ReadSerial{
private:
    String type;
    String message_value;
    String lat;
    String lon;
    String depth; 
    String infoGPS;
    String message_received; 

public:
    bool readSerial();
    String getType();
    String getValue();
    String getMessageReceived();
};