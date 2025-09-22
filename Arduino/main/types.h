enum StickDirection {
    CENTER,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    UP_LEFT,
    UP_RIGHT,
    DOWN_LEFT,
    DOWN_RIGHT
  };
static const char* StickDirectionStr[] = {
    "CENTER",
    "UP",
    "DOWN",
    "LEFT",
    "RIGHT",
    "UP_LEFT",
    "UP_RIGHT",
    "DOWN_LEFT",
    "DOWN_RIGHT"
};
enum PacketType {
    GAME_STATE_UPDATE,
    GAME_SCORE_UPDATE,
    GAME_BUFFER_UPDATE,
    GAME_END,
    GAME_START
};
static const char* PacketTypeStr[] = {
    "GAME_STATE_UPDATE",
    "GAME_SCORE_UPDATE",
    "GAME_BUFFER_UPDATE",
    "GAME_END",
    "GAME_START"
};