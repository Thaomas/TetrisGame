#ifndef CONTROL_H
#define CONTROL_H

#include <Arduino.h>
#include "types.h"

class Controller {
public:
  Controller(int leftButtonPin, int rightButtonPin, int stickButtonPin, int stickXPin, int stickYPin, float stickMaxValue = 4095.0f, float percentThreshold = 0.15f);

  void begin();
  void update();

  bool isLeftPressed() const { return leftPressed; }
  bool isRightPressed() const { return rightPressed; }
  bool isStickPressed() const { return stickPressed; }
  int getStickX() const { return stickX; }
  int getStickY() const { return stickY; }
  StickDirection getDirection() const { return direction; }

  int stickCenterX;
  int stickCenterY;
private:
  StickDirection decodeStickDirection(int x, int y) const;

  const int leftButtonPin;
  const int rightButtonPin;
  const int stickButtonPin;
  const int stickXPin;
  const int stickYPin;

  const float stickMaxValue;
  const float percentThreshold;


  bool leftPressed;
  bool rightPressed;
  bool stickPressed;
  int stickX;
  int stickY;
  StickDirection direction;
};

#endif // CONTROL_H


