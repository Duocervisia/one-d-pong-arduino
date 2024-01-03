#include <Adafruit_NeoPixel.h>
#include <ezButton.h>

#define PIN 2
#define NUMPIXELS 100
#define GAME_PIXEL_WIDTH 10
#define ONULL -1


Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

ezButton playerOneButton(14);
ezButton playerTwoButton(12);

int ballPosition;
int prevBallPosition;
int ballDirection;
bool gameRunning;

unsigned long lastUpdateTime;
int gameDelay;
int minDelay = 60;
int maxDelay = 200;

void setup() {
  Serial.begin(9600);
  Serial.println("init");

  playerOneButton.setDebounceTime(50);
  playerTwoButton.setDebounceTime(50);

  pixels.begin();
  setupGame();
  pixels.show();
}

void loop() {
  playerOneButton.loop();
  playerTwoButton.loop();

  unsigned long currentTime = millis();
  
  if(playerOneButton.isPressed()){
    if(!gameRunning){
      startGame(1);
      updateBallVisual();
    }else if(ballDirection == -1){
      shootBack(1);
      updateBallPosition();
   	  updateBallVisual();
    }
  }else if(playerTwoButton.isPressed()){
    if(!gameRunning){
      startGame(2);
      updateBallVisual();
    }else if(ballDirection == 1){
      shootBack(2);
      updateBallPosition();
      updateBallVisual();
    }
  }else if (gameRunning && currentTime - lastUpdateTime >= gameDelay) {
    //Serial.println(currentTime - lastUpdateTime);
  	//Serial.println(gameDelay);
    updateBallPosition();
    updateBallVisual();
    lastUpdateTime = currentTime;
  }
}

void setupGame() {
  for (int i = 0; i < GAME_PIXEL_WIDTH; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 0, 0));
    pixels.setPixelColor(NUMPIXELS - 1 - i, pixels.Color(0, 255, 0));
  }
}

void startGame(int player) {
  if (player == 1) {
    ballPosition = GAME_PIXEL_WIDTH;
    ballDirection = 1;
  } else {
    ballPosition = NUMPIXELS - 1 - GAME_PIXEL_WIDTH;
    ballDirection = -1;
  }
  gameDelay = (maxDelay - minDelay) / 2;
  gameRunning = true;
}
void shootBack(int player) {
  if(
  	(player == 1 && ballPosition < GAME_PIXEL_WIDTH) ||
    (player == 2 && ballPosition >= NUMPIXELS - GAME_PIXEL_WIDTH)
  ){
    ballDirection *= -1;
    
    if(ballPosition < GAME_PIXEL_WIDTH){
      gameDelay = minDelay + (ballPosition/(GAME_PIXEL_WIDTH-1)) * (maxDelay - minDelay);
    }else{
      gameDelay = minDelay + ((NUMPIXELS - 1 - ballPosition)/ (GAME_PIXEL_WIDTH-1)) * (maxDelay - minDelay);
    }
  }else{
  	endGame();
  }
}
void endGame(){
  gameRunning = false;
  prevBallPosition = ONULL;
  pixels.setPixelColor(ballPosition, pixels.Color(0, 0, 0));
  for(int j = 0; j <= 6; j++){
  	for (int i = 0; i < GAME_PIXEL_WIDTH; i++) {
      pixels.setPixelColor(i, pixels.Color(j%2 == 0 ? 255 : 0, 0, 0));
      pixels.setPixelColor(NUMPIXELS - 1 - i, pixels.Color(0, j%2 == 0 ? 255 : 0, 0));
      pixels.show();
      delay(20);
  	}
    delay(100);
  }
}

void updateBallPosition() {
  prevBallPosition = ballPosition;
  ballPosition += ballDirection;
  if(ballPosition < 0 || ballPosition > NUMPIXELS - 1){
    endGame();
  }
}

void updateBallVisual() {
  pixels.setPixelColor(ballPosition, pixels.Color(0, 0, 255));
  Serial.println(ballPosition);

  if (prevBallPosition != ONULL){
    pixels.setPixelColor(prevBallPosition, pixels.Color(prevBallPosition < GAME_PIXEL_WIDTH && ballDirection == 1 ? 255 : 0, prevBallPosition >= NUMPIXELS - GAME_PIXEL_WIDTH && ballDirection == -1 ? 255 : 0, 0));
  }
  pixels.show();
}
