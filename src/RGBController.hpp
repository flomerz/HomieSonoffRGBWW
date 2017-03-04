#ifndef RGBCONTROLLER_H
#define RGBCONTROLLER_H

#include <Arduino.h>

#define RGBRANGE 255

class RGBController {

  const uint8_t pinRed;
  const uint8_t pinGreen;
  const uint8_t pinBlue;

  unsigned short currentRed;
  unsigned short currentGreen;
  unsigned short currentBlue;
  unsigned short currentBrightness;

  unsigned short targetRed;
  unsigned short targetGreen;
  unsigned short targetBlue;
  unsigned short targetBrightness;

  unsigned long lastTransitionColorMicros;
  unsigned long transitionColorMicrosOffset;

  unsigned long lastTransitionBrightnessMicros;
  unsigned long transitionBrightnessMicrosOffset;

  bool rainbowOn = false;
  unsigned short rainbowColorOffset;
  unsigned short rainbowColorFactor;
  unsigned long transitionRainbowMillis;


  unsigned short mapPWM(byte const & color) const {
    return map(color, 0, RGBRANGE, 0, PWMRANGE);
  }

  unsigned short mapPWMColor(byte const & color) const {
    auto pwm = mapPWM(color);
    return pow(2, (pwm + 1) / (PWMRANGE / 10)) - 1;
  }

  long offsetLong(long const & x, long const & y) const {
    return x > y ? x - y : y - x;
  }

  long maxLong(long const & x, long const & y) const {
    return x > y ? x : y;
  }

  unsigned long calcTransition(word const & transitionSeconds) const {
    if (transitionSeconds > 0) {
      auto steps = maxLong(
        maxLong(
          offsetLong(currentRed, targetRed),
          offsetLong(currentGreen, targetGreen)
        ), maxLong(
          offsetLong(currentBlue, targetBlue),
          offsetLong(currentBrightness, targetBrightness)
        )
      );

      if (steps > 0) {
        return (transitionSeconds * 1000000) / steps;
      }
    }

    return 0;
  }

  unsigned short calcRainbowColor(double const & colorOffset) const {
    return sin((millis() * 2 * PI / transitionRainbowMillis) + colorOffset) * rainbowColorFactor + rainbowColorOffset;
  }

  void updateRGB() {
    analogWrite(pinRed, currentRed * currentBrightness / PWMRANGE);
    analogWrite(pinGreen, currentGreen * currentBrightness / PWMRANGE);
    analogWrite(pinBlue, currentBlue * currentBrightness / PWMRANGE);
  }

  void updateColor() {
    if (micros() - lastTransitionColorMicros > transitionColorMicrosOffset) {
      lastTransitionColorMicros = micros();

      bool needUpdate = false;
      if (currentRed != targetRed) {
        needUpdate = true;
        currentRed -= constrain(currentRed - targetRed, -1, 1);
      }
      if (currentGreen != targetGreen) {
        needUpdate = true;
        currentGreen -= constrain(currentGreen - targetGreen, -1, 1);
      }
      if (currentBlue != targetBlue) {
        needUpdate = true;
        currentBlue -= constrain(currentBlue - targetBlue, -1, 1);
      }

      if (needUpdate) updateRGB();
    }
  }

  void updateBrightness() {
    if (micros() - lastTransitionBrightnessMicros > transitionBrightnessMicrosOffset) {
      lastTransitionBrightnessMicros = micros();

      bool needUpdate = false;
      if (currentBrightness != targetBrightness) {
        needUpdate = true;
        currentBrightness -= constrain(currentBrightness - targetBrightness, -1, 1);
      }

      if (needUpdate) updateRGB();
    }
  }

  void updateRainbow() {
    currentRed = calcRainbowColor(0);
    currentGreen = calcRainbowColor(2 * PI / 3);
    currentBlue = calcRainbowColor(4 * PI / 3);

    updateRGB();
  }


public:

  RGBController(uint8_t const & pinRed, uint8_t const & pinGreen, uint8_t const & pinBlue) :
    pinRed{pinRed},
    pinGreen{pinGreen},
    pinBlue{pinBlue} {}

	void setup() const {
    pinMode(pinRed, OUTPUT);
    pinMode(pinGreen, OUTPUT);
    pinMode(pinBlue, OUTPUT);
	}

  void loop() {
    updateBrightness();
    if (rainbowOn) {
      updateRainbow();
    } else {
      updateColor();
    }
  }

  void setColor(byte const & red, byte const & green, byte const & blue, word const & transitionSeconds) {
    disableRainbow();
    targetRed = mapPWMColor(red);
    targetGreen = mapPWMColor(green);
    targetBlue = mapPWMColor(blue);
    transitionColorMicrosOffset = calcTransition(transitionSeconds);
  }

  void setBrightness(byte const & brightness, word const & transitionSeconds) {
    targetBrightness = mapPWM(brightness);
    transitionBrightnessMicrosOffset = calcTransition(transitionSeconds);
  }

  void setRainbow(byte const & colorOffset, byte const & colorFactor, word const & transitionSeconds) {
    rainbowOn = true;
    rainbowColorOffset = mapPWM(colorOffset);
    rainbowColorFactor = mapPWM(colorFactor);
    transitionRainbowMillis = transitionSeconds * 1000;
  }

  void disableRainbow() {
    rainbowOn = false;
  }
};

#endif
