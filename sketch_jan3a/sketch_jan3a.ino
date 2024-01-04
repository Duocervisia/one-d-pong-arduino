#include <Adafruit_NeoPixel.h>
#include <ezButton.h>

#define PIN 13
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
int minDelay = 20;
int maxDelay = 100;

void setup() {
  Serial.begin(115200);
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
      if(updateBallPosition()){
        updateBallVisual();
      }
    }
  }else if(playerTwoButton.isPressed()){
    if(!gameRunning){
      startGame(2);
      updateBallVisual();
    }else if(ballDirection == 1){
      shootBack(2);
      if(updateBallPosition()){
        updateBallVisual();
      }
    }
  }else if (gameRunning && currentTime - lastUpdateTime >= gameDelay) {
    //Serial.println(currentTime - lastUpdateTime);
  	//Serial.println(gameDelay);
    if(updateBallPosition()){
      updateBallVisual();
    }
    lastUpdateTime = currentTime;
  }
}


void setupGame() {
  for (int i = 0; i < GAME_PIXEL_WIDTH; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 255, 0));
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
    pixels.setPixelColor(0, pixels.Color(j%2 == 0 ? 255 : 0, 0, 0));
    pixels.setPixelColor(NUMPIXELS - 1, pixels.Color(j%2 == 0 ? 255 : 0, 0, 0));

  	for (int i = j==6 ? 0 : 1; i < GAME_PIXEL_WIDTH; i++) {
      pixels.setPixelColor(i, pixels.Color(0, j%2 == 0 ? 255 : 0, 0));
      pixels.setPixelColor(NUMPIXELS - 1 - i, pixels.Color(0, j%2 == 0 ? 255 : 0, 0));
      pixels.show();
      delay(100);
  	}
  }
}

bool updateBallPosition() {
  prevBallPosition = ballPosition;
  ballPosition += ballDirection;
  if(ballPosition < 0 || ballPosition > NUMPIXELS - 1){
    endGame();
    return false;
  }
  return true;
}

void updateBallVisual() {
  pixels.setPixelColor(ballPosition, pixels.Color(0, 0, 255));
  Serial.println(ballPosition);

  if (prevBallPosition != ONULL){
    if((prevBallPosition < GAME_PIXEL_WIDTH && ballDirection == 1) ||(prevBallPosition >= NUMPIXELS - GAME_PIXEL_WIDTH && ballDirection == -1)){
      pixels.setPixelColor(prevBallPosition, pixels.Color(0, 255, 0));
    }else{
      pixels.setPixelColor(prevBallPosition, pixels.Color(0, 0, 0));
	}
  }
  pixels.show();
}
