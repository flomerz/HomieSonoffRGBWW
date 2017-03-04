#include <Homie.h>
#include "RGBJsonController.hpp"

#define PIN_LED     5
#define PIN_R       15//14
#define PIN_G       13//12
#define PIN_B       12//13

HomieNode lightNode("light", "light");
#define PROPERTY_RGB "rgb"

RGBJsonController rgbJsonController{PIN_R, PIN_G, PIN_B};

bool lightRGBHandler(HomieRange range, String value) {
  rgbJsonController.set(value);
  lightNode.setProperty(PROPERTY_RGB).send(rgbJsonController.get());
  return true;
}

void setup() {
  Homie_setFirmware("sonoff-rgbww", "1.0.0");

  Serial.begin(115200);
  Homie.setLoggingPrinter(&Serial1);
  Homie.getLogger() << endl << endl;

  Homie.setLedPin(PIN_LED, LOW);
  Homie.disableResetTrigger();

  rgbJsonController.setup();

  lightNode.advertise(PROPERTY_RGB).settable(lightRGBHandler);

  Homie.setup();
}

void loop() {
  Homie.loop();
  rgbJsonController.loop();
}
