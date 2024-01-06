#include <FastLED.h>
#include <ezButton.h>

#define DATA_PIN 13
#define BRIGHTNESS 5
#define NUM_LEDS 100
#define GAME_LED_WIDTH 12
#define ONULL -1

CRGB leds[NUM_LEDS];

ezButton playerOneButton(14);
ezButton playerTwoButton(12);

int ballPosition;
int prevBallPosition;
int ballDirection;
int animatedPixels[5] = {ONULL,ONULL,ONULL,ONULL,ONULL};
bool animationSet = false;
bool gameRunning = false;
bool gameJustEnded = false;
int bounceCounter = 0;

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
  setupGame();
}

void loop() {
  playerOneButton.loop();
  playerTwoButton.loop();

  unsigned long currentTime = millis();

  if(gameJustEnded && (playerOneButton.isPressed() || playerTwoButton.isPressed())){
    return;
  }else{
    gameJustEnded = false;
  }
  
  if(playerOneButton.isPressed()){
    Serial.println("press player 1");
    if(!gameRunning){
      startGame(1);
    }else if(ballDirection == -1){
      shootBack(1);
    }
  }
  if(playerTwoButton.isPressed()){
    Serial.println(playerTwoButton.isReleased());

    if(!gameRunning){
      startGame(2);
    }else if(ballDirection == 1){
      shootBack(2);
    }
  }
  if (gameRunning && currentTime - lastUpdateTime >= gameDelay) {
    if(updatePositions()){
      updateVisuals();
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
  bounceCounter = 0;
  animatedPixels[0] = ballPosition;
  animationSet = true;
  gameDelay = (maxDelay - minDelay) / 2;
  lastUpdateTime = gameDelay;
  gameRunning = true;
}

bool shootBack(int player) {
  if(
    (player == 1 && ballPosition < GAME_LED_WIDTH) ||
    (player == 2 && ballPosition >= NUM_LEDS - GAME_LED_WIDTH)
  ){
    ballDirection *= -1;
    bounceCounter++;
    
    if(ballPosition < GAME_LED_WIDTH){
      gameDelay = minDelay + (float(ballPosition)/ float(GAME_LED_WIDTH-1)) * (maxDelay - minDelay);
    }else{
      gameDelay = minDelay + (float(NUM_LEDS - 1 - ballPosition) / float(GAME_LED_WIDTH-1)) * (maxDelay - minDelay);
    }

    float speedMultiplier = 1.0 - (float) min(bounceCounter, 49) / 50.0; // Adjust this multiplier as needed
    gameDelay = int(gameDelay * speedMultiplier);
    
    animatedPixels[0] = ballPosition;
    animationSet = true;
  }else{
    endGame();
    return false;
  }
  return true;
}
void updateAnimationPosition(){
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
}
void updateAnimationVisual(bool show = false, bool end = false){
  for(int i = sizeof(animatedPixels) / sizeof(animatedPixels[0]) - 1; i >= 0; i--) {
    if(animatedPixels[i] != ONULL){
      if(!end){
        leds[animatedPixels[i]] = CRGBA(0, 255, 20 * (sizeof(animatedPixels) / sizeof(animatedPixels[0]) - i - 1), BRIGHTNESS + 10 * (sizeof(animatedPixels) / sizeof(animatedPixels[0]) - i - 1));
      }else{
        float t = static_cast<float>(i) / (sizeof(animatedPixels) / sizeof(animatedPixels[0]) - 1);

        leds[animatedPixels[i]] = CRGBA(
            static_cast<uint8_t>((1.0 - t) * 255),  // Red value (decreasing from 255 to 0 as i increases)
            static_cast<uint8_t>(t * 255),           // Green value (increasing from 0 to 255 as i increases)
            0,                                       // Blue value (set to 0)
            BRIGHTNESS + 10 * (sizeof(animatedPixels) / sizeof(animatedPixels[0]) - i - 1)
        );
      }
    }
  }
  if(show){
    FastLED.show();
  }
}

void endGame(){
  gameRunning = false;
  prevBallPosition = ONULL;
  leds[ballPosition] = CRGBA(0, 0, 0);
  int startPosition = NUM_LEDS - GAME_LED_WIDTH;
  if(ballDirection == -1){
    startPosition = GAME_LED_WIDTH - 1;
  }
  ballDirection *= -1;


  for(int j = 0; j <= 3; j++){
    animatedPixels[0] = startPosition;
    int iteration = j == 3 ? GAME_LED_WIDTH + sizeof(animatedPixels) / sizeof(animatedPixels[0]) : GAME_LED_WIDTH;

    for (int i = 0; i < iteration; i++) {
      updateAnimationPosition();
      updateAnimationVisual(true, j != 3);
      delay(40);
    }
  }
  setupGame();
  //Neccesary because of isPressed bug
  gameJustEnded = true;
}
bool updatePositions(){
  bool stillRunning = updateBallPosition();
  if(!animationSet){
    updateAnimationPosition();
  }else{
    animationSet = false;
  }
  return stillRunning;
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

void updateVisuals(){
  updateBallVisual();
  updateAnimationVisual();
  FastLED.show();
}

void updateBallVisual() {
  leds[ballPosition] = CRGBA(255, 255, 255);
  if (prevBallPosition != ONULL){
    if((prevBallPosition < GAME_LED_WIDTH && ballDirection == 1) ||(prevBallPosition >= NUM_LEDS - GAME_LED_WIDTH && ballDirection == -1)){
      leds[prevBallPosition] = CRGBA(0, 255, 0);
    }else{
      leds[prevBallPosition] = CRGBA(0, 0, 0);
    }
  }
}