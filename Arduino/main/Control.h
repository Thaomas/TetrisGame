#ifndef CONTROL_H
#define CONTROL_H

#include <Arduino.h>
#include "types.h"


class Controller {
public:
  Controller(int leftButtonPin, int rightButtonPin, int stickButtonPin, int stickXPin, int stickYPin, float stickMaxValue = 4095.0f, float percentThreshold = 0.15f);

  void begin();
  void update();
  bool isLeftPressed();
  bool isRightPressed();
  bool isStickPressed();
  int getStickX() const { return stickX; }
  int getStickY() const { return stickY; }
  StickDirection getDirection() const { return direction; }

private:
  StickDirection decodeStickDirection(int x, int y) const;

  const int leftButtonPin;
  const int rightButtonPin;
  const int stickButtonPin;
  const int stickXPin;
  const int stickYPin;

  const float stickMaxValue;
  const float percentThreshold;

  int stickCenterX;
  int stickCenterY;

  bool leftPressed;
  bool rightPressed;
  bool stickPressed;
  unsigned long lastLeftPressMs;
  unsigned long lastRightPressMs;
  unsigned long lastStickPressMs;
  const unsigned long debounceMs = 500; // cooldown per button press
  int stickX;
  int stickY;
  StickDirection direction;
};

#endif // CONTROL_H


