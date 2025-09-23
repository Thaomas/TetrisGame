#include "Control.h"

Controller::Controller(int leftButtonPin, int rightButtonPin, int stickButtonPin, int stickXPin, int stickYPin, float stickMaxValue, float percentThreshold)
  : leftButtonPin(leftButtonPin),
    rightButtonPin(rightButtonPin),
    stickButtonPin(stickButtonPin),
    stickXPin(stickXPin),
    stickYPin(stickYPin),
    stickMaxValue(stickMaxValue),
    percentThreshold(percentThreshold),
    stickCenterX(0),
    stickCenterY(0),
    leftPressed(false),
    rightPressed(false),
    stickPressed(false),
    stickX(0),
    stickY(0),
    direction(CENTER) {}

void Controller::begin() {
  pinMode(leftButtonPin, INPUT_PULLUP);
  pinMode(rightButtonPin, INPUT_PULLUP);
  pinMode(stickButtonPin, INPUT_PULLUP);

  delay(100);
  stickCenterX = analogRead(stickXPin);
  stickCenterY = analogRead(stickYPin);
}

void Controller::update() {
  
  if (!leftPressed)
    leftPressed = digitalRead(leftButtonPin) == HIGH;
  
  if (!rightPressed)
    rightPressed = digitalRead(rightButtonPin) == HIGH;

  if (!stickPressed)
    stickPressed = digitalRead(stickButtonPin) == LOW;

  stickX = analogRead(stickXPin);
  stickY = analogRead(stickYPin);
  direction = decodeStickDirection(stickX, stickY);
}


bool Controller::isLeftPressed() {
  if (leftPressed) {
    leftPressed = false;
    return true;
  }
  return false;
}

bool Controller::isRightPressed() {
  if (rightPressed) {
    rightPressed = false;
    return true;
  }
  return false;
}

bool Controller::isStickPressed() {
  if (stickPressed) {
    stickPressed = false;
    return true;
  }
  return false;
}

inline StickDirection Controller::decodeStickDirection(int x, int y) const {
  int left_threshold = stickCenterX + (stickMaxValue - stickCenterX) * percentThreshold;
  int right_threshold = stickCenterX - (stickCenterX * percentThreshold);
  int up_threshold = stickCenterY - (stickCenterY * percentThreshold);
  int down_threshold = stickCenterY + (stickMaxValue - stickCenterY) * percentThreshold;

  bool left = x > left_threshold;
  bool right = x < right_threshold;
  bool up = y < up_threshold;
  bool down = y > down_threshold;

  if (up) {
    if (left) return UP_LEFT;
    if (right) return UP_RIGHT;
    return UP;
  }
  if (down) {
    if (left) return DOWN_LEFT;
    if (right) return DOWN_RIGHT;
    return DOWN;
  }
  if (left) return LEFT;
  if (right) return RIGHT;
  return CENTER;
}
