#include "ReadSerial.h"

bool ReadSerial::readSerial() {
    if (Serial.available()) {
        // Only do anything if the received string starts with a '|'
        if (Serial.read() == '|') {
            type = Serial.readStringUntil('|');

            // Handle the different message types
            if (type == "MSG") {
                message_value = Serial.readStringUntil('|');
                message_received = "$|" + type + "|" + message_value + "|";
            } else if (type == "EMR") {
                message_value = Serial.readStringUntil('|');
                message_received = "$|" + type + "|" + message_value + "|";
            } else if (type == "GPS") {
                type = "GPB";
                lat = Serial.readStringUntil('|');
                lon = Serial.readStringUntil('|');
                depth = Serial.readStringUntil('|');
                infoGPS = Serial.readStringUntil('|');
                message_value = lat + "," + lon + "," + depth;
                message_received = "$|" + type + "|" + message_value + "|";
            }
        }

        return true;
    }

    // Empty buffer
    while (Serial.available()) {
        Serial.read();
    }

    return false;
}

String ReadSerial::getType() {
    return type;
}

String ReadSerial::getValue() {
    return message_value;
}

String ReadSerial::getMessageReceived() {
    return message_received;
}