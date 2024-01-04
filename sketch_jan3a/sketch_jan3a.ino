#include <FastLED.h>
#include <ezButton.h>

#define DATA_PIN 5
#define BRIGHTNESS 5
#define NUM_LEDS 100
#define GAME_LED_WIDTH 10
#define ONULL -1

CRGB leds[NUM_LEDS];

ezButton playerOneButton(14);
ezButton playerTwoButton(12);

int ballPosition;
int prevBallPosition;
int ballDirection;
bool gameRunning;

unsigned long lastUpdateTime;
int gameDelay;
int minDelay = 10;
int maxDelay = 2000;

void setup() {
  Serial.begin(115200);
  Serial.println("init");

  playerOneButton.setDebounceTime(50);
  playerTwoButton.setDebounceTime(50);

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  setupGame();
}

void loop() {
  playerOneButton.loop();
  playerTwoButton.loop();

  unsigned long currentTime = millis();
  
  if(playerOneButton.isReleased()){
    Serial.println("press player 1");
    if(!gameRunning){
      startGame(1);
    }else if(ballDirection == -1){
      shootBack(1);
    }
  }
  if(playerTwoButton.isReleased()){
    if(!gameRunning){
      startGame(2);
    }else if(ballDirection == 1){
      shootBack(2);
    }
  }
  if (gameRunning && currentTime - lastUpdateTime >= gameDelay) {
    if(updateBallPosition()){
      updateBallVisual();
    }
    lastUpdateTime = currentTime;
  }
}

void setupGame() {
  for (int i = 0; i < GAME_LED_WIDTH; i++) {
    leds[i] = CRGB(0, 255, 0);
    leds[NUM_LEDS - 1 - i] = CRGB(0, 255, 0);
    FastLED.show();
  }
}

void startGame(int player) {
  if (player == 1) {
    ballPosition = GAME_LED_WIDTH - 1;
    ballDirection = 1;
  } else {
    ballPosition = NUM_LEDS - GAME_LED_WIDTH;
    ballDirection = -1;
  }
  gameDelay = (maxDelay - minDelay) / 2;
  lastUpdateTime = gameDelay;
  Serial.println(gameDelay);
  gameRunning = true;
}

bool shootBack(int player) {
  if(
    (player == 1 && ballPosition < GAME_LED_WIDTH) ||
    (player == 2 && ballPosition >= NUM_LEDS - GAME_LED_WIDTH)
  ){
    ballDirection *= -1;
    
    if(ballPosition < GAME_LED_WIDTH){
      gameDelay = minDelay + (float(ballPosition)/ float(GAME_LED_WIDTH-1)) * (maxDelay - minDelay);
    }else{
      gameDelay = minDelay + (float(NUM_LEDS - 1 - ballPosition) / float(GAME_LED_WIDTH-1)) * (maxDelay - minDelay);
    }
    Serial.println(String(minDelay) + " + " +  String(ballPosition) + " / " + String(GAME_LED_WIDTH-1) + " * " + String(maxDelay - minDelay) + " = " + String(gameDelay));

  }else{
    endGame();
    return false;
  }
  return true;
}

void endGame(){
  gameRunning = false;
  prevBallPosition = ONULL;
  leds[ballPosition] = CRGB(0, 0, 0);
  for(int j = 0; j <= 6; j++){
    leds[0] = CRGB(j%2 == 0 ? 255 : 0, 0, 0);
    leds[NUM_LEDS - 1] = CRGB(j%2 == 0 ? 255 : 0, 0, 0);

    for (int i = j==6 ? 0 : 1; i < GAME_LED_WIDTH; i++) {
      leds[i] = CRGB(0, j%2 == 0 ? 255 : 0, 0);
      leds[NUM_LEDS - 1 - i] = CRGB(0, j%2 == 0 ? 255 : 0, 0);
      FastLED.show();
      delay(20);
    }
  }
  playerOneButton.loop();
  playerTwoButton.loop();
}

bool updateBallPosition() {
  prevBallPosition = ballPosition;
  ballPosition += ballDirection;
  if(ballPosition < 0 || ballPosition > NUM_LEDS - 1){
    endGame();
    return false;
  }
  return true;
}

void updateBallVisual() {
  leds[ballPosition] = CRGB(0, 0, 255);
  // Serial.println(ballPosition);

  if (prevBallPosition != ONULL){
    if((prevBallPosition < GAME_LED_WIDTH && ballDirection == 1) ||(prevBallPosition >= NUM_LEDS - GAME_LED_WIDTH && ballDirection == -1)){
      leds[prevBallPosition] = CRGB(0, 255, 0);
    }else{
      leds[prevBallPosition] = CRGB(0, 0, 0);
    }
  }
  FastLED.show();
}