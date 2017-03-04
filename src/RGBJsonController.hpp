#ifndef RGBJSONCONTROLLER_H
#define RGBJSONCONTROLLER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "RGBController.hpp"

class RGBJsonController {

  RGBController rgbController;

  String state = "off";
  byte brightness = 255;
  byte red = 255;
  byte green = 255;
  byte blue = 255;
  String effect = "none";

  word effectTransitionSeconds = 10;


  void setEffect(String const & newEffect, word const & transitionSeconds) {
    effect = newEffect;
    effectTransitionSeconds = transitionSeconds;
    if (effect.equals("rainbow")) {
      rgbController.setRainbow(128, 127, transitionSeconds);
    } else if (effect.equals("rainbow-light")) {
      rgbController.setRainbow(172, 83, transitionSeconds);
    }
  }

public:

  RGBJsonController(uint8_t const & pinRed, uint8_t const & pinGreen, uint8_t const & pinBlue) : rgbController{pinRed, pinGreen, pinBlue} {}

  void setup() const {
    rgbController.setup();
  }

  void loop() {
    rgbController.loop();
  }

  bool set(String const & json) {
    StaticJsonBuffer<512> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(json.c_str());

    if (!root.success()) {
      return false;
    }

    if (root.containsKey("state")) {
      if (strcmp(root["state"], "off") == 0) {
        state = "off";
        rgbController.setColor(0, 0, 0, 10);
        rgbController.setBrightness(0, 10);
      } else if (state.equals("off")) {
        state = "on";
        rgbController.setColor(red, green, blue, 2);
        rgbController.setBrightness(brightness, 2);
        if (!effect.equals("none")) {
          setEffect(effect, effectTransitionSeconds);
        }
      }
    }

    if (root.containsKey("brightness")) {
      brightness = root["brightness"];
      auto transitionSeconds = root.containsKey("transition") ? root["transition"] : 1;
      rgbController.setBrightness(brightness, transitionSeconds);
    }

    if (root.containsKey("color")) {
      effect = "none";
      red = root["color"][0];
      green = root["color"][1];
      blue = root["color"][2];
      auto transitionSeconds = root.containsKey("transition") ? root["transition"] : 1;
      rgbController.setColor(red, green, blue, transitionSeconds);
    }

    if (root.containsKey("effect")) {
      auto transitionSeconds = root.containsKey("transition") ? root["transition"] : 60;
      setEffect(root["effect"].asString(), transitionSeconds);
    }

    return true;
  }

  String get() const {
    StaticJsonBuffer<512> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();

    root["state"] = state;
    root["brightness"] = brightness;
    root["effect"] = effect;

    JsonArray& color = root.createNestedArray("color");
    color.add(red);
    color.add(green);
    color.add(blue);

    String message;
    root.printTo(message);
    return message;
  }
};

#endif
