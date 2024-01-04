#include <FastLED.h>
#include <ezButton.h>

#define DATA_PIN 13
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
int animatedPixels[5] = {ONULL,ONULL,ONULL,ONULL,ONULL};
bool animationSet = false;
bool gameRunning;

unsigned long lastUpdateTime;
int gameDelay;
int minDelay = 10;
int maxDelay = 50;

void setup() {
  Serial.begin(115200);
  Serial.println("init");

  playerOneButton.setDebounceTime(50);
  playerTwoButton.setDebounceTime(50);

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  // FastLED.setBrightness(BRIGHTNESS);

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

CRGB CRGBA(int r, int g, int b, int brightness = BRIGHTNESS){
    CRGB color = CRGB(r,g,b);
    color.fadeLightBy(255 - brightness);
    return color;
}

void setupGame() {
  for (int i = 0; i < GAME_LED_WIDTH; i++) {
    leds[i] = CRGBA(0, 255, 0);
    leds[NUM_LEDS - 1 - i] = CRGBA(0, 255, 0);
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
  animatedPixels[0] = ballPosition;
  animationSet = true;
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
    animatedPixels[0] = ballPosition;
    animationSet = true;
  }else{
    endGame();
    return false;
  }
  return true;
}

void endGame(){
  gameRunning = false;
  prevBallPosition = ONULL;
  leds[ballPosition] = CRGBA(0, 0, 0);
  for(int j = 0; j <= 6; j++){
    if(ballDirection == 1){
      leds[0] = CRGBA(0 , j%2 == 0 ? 255 : 0, 0);
      leds[NUM_LEDS - 1] = CRGBA(j%2 == 0 ? 255 : 0, 0, 0);
    }else{
      leds[0] = CRGBA(j%2 == 0 ? 255 : 0, 0, 0);
      leds[NUM_LEDS - 1] =  CRGBA(0, j%2 == 0 ? 255 : 0, 0);
    }

    for (int i = j==6 ? 0 : 1; i < GAME_LED_WIDTH; i++) {
      leds[i] = CRGBA(0, j%2 == 0 ? 255 : 0, 0);
      leds[NUM_LEDS - 1 - i] = CRGBA(0, j%2 == 0 ? 255 : 0, 0);
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
  if(!animationSet){
    for(int i = sizeof(animatedPixels) / sizeof(animatedPixels[0]) - 1; i >= 0; i--) {
      if(i != 0){
        animatedPixels[i] = animatedPixels[i-1];
      }else if(ballDirection == 1){
        if(animatedPixels[i+1] >=1){
          animatedPixels[i] = animatedPixels[i+1]-1;
        }else{
          animatedPixels[i] = ONULL;
        }
      }else if(ballDirection == -1){
        if(animatedPixels[i+1] < NUM_LEDS - 1 && animatedPixels[i+1] > ONULL){
          animatedPixels[i] = animatedPixels[i+1]+1;
        }else{
          animatedPixels[i] = ONULL;
        }
      }
    }
  }else{
    animationSet = false;
  }
  // for(int i = sizeof(animatedPixels) / sizeof(animatedPixels[0]) - 1; i >= 0; i--) {
  //   Serial.print(animatedPixels[i]);
  //   Serial.print(" ");
  // }
  // Serial.println();

  return true;
}

void updateBallVisual() {
  leds[ballPosition] = CRGBA(0, 0, 255);
  if (prevBallPosition != ONULL){
    if((prevBallPosition < GAME_LED_WIDTH && ballDirection == 1) ||(prevBallPosition >= NUM_LEDS - GAME_LED_WIDTH && ballDirection == -1)){
      leds[prevBallPosition] = CRGBA(0, 255, 0);
    }else{
      leds[prevBallPosition] = CRGBA(0, 0, 0);
    }
  }
   for(int i = sizeof(animatedPixels) / sizeof(animatedPixels[0]) - 1; i >= 0; i--) {
    if(animatedPixels[i] != ONULL){
      leds[animatedPixels[i]] = CRGBA(0, 255, 20 * (sizeof(animatedPixels) / sizeof(animatedPixels[0]) - i - 1), BRIGHTNESS + 10 * (sizeof(animatedPixels) / sizeof(animatedPixels[0]) - i - 1));
    }
  }
  FastLED.show();
}