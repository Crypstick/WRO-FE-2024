#include "EV3ColorSensor.h"

const static char *TAG = "EV3ColorSensor";

void EV3ColorSensor::messageHandler(uint8_t mode, uint8_t *message, int length)
{
    switch (mode)
    {
    case 0:
        if (onColReflect)
            onColReflect(message[0]);
        break;
    case 1:
        if (onColAmbient)
            onColAmbient(message[0]);
        break;
    case 2:
        if (onColColor)
            onColColor(static_cast<EV3ColorSensorColor>(message[0]));
        break;
    default:
        ESP_LOGE(TAG, "Currently not supported EV3 color sensor mode %d", mode);
    }
}

void writeEV3ColorSensorColorToStream(EV3ColorSensorColor col, Stream *stream)
{
    switch (col)
    {
    case EV3ColorSensorColor::NONE:
        stream->print("none");
        break;
    case EV3ColorSensorColor::COLOR_BLACK:
        stream->print("black");
        break;
    case EV3ColorSensorColor::COLOR_BLUE:
        stream->print("blue");
        break;
    case EV3ColorSensorColor::COLOR_GREEN:
        stream->print("green");
        break;
    case EV3ColorSensorColor::COLOR_YELLOW:
        stream->print("yellow");
        break;
    case EV3ColorSensorColor::COLOR_RED:
        stream->print("red");
        break;
    case EV3ColorSensorColor::COLOR_WHITE:
        stream->print("white");
        break;
    case EV3ColorSensorColor::COLOR_BROWN:
        stream->print("brown");
        break;
    default:
        ESP_LOGE(TAG, "Unknown color %d", static_cast<uint8_t>(col));
        stream->print("unknown");
    }
}