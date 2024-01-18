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
int scorePlayerOne = 0;
int scorePlayerTwo = 0;
int scoreNeeded = 5;
bool scoreBoardShown = false;
bool animationSet = false;
bool gameRunning = false;
bool gameJustEnded = false;
bool playerOneWins = false;
bool playerOneStarts = false;
bool lastGame = true;
bool initShootAnimation = false;
int bounceCounter = 0;
int difficulty = ONULL;
int difficultyLevels = 10;
bool difficultyBoardShown = false;
int shotStrengthPosition = ONULL;
bool shotStrengthReversed = false;


unsigned long lastUpdateTime;
int gameDelay;
int minDelay = 8;
int maxDelay = 40;
int minDelayAdapted;
int maxDelayAdapted;

CRGB CRGBA(int r, int g, int b, int brightness = BRIGHTNESS){
    CRGB color = CRGB(r,g,b);
    color.fadeLightBy(255 - brightness);
    return color;
}

void showScoreBoard(int brightness = BRIGHTNESS){
  scoreBoardShown = true;
  CRGB borderColor = CRGBA(0, 0, 0, brightness);
  CRGB playerColor = CRGBA(0, 255, 0, brightness);
  CRGB backgroundColor = CRGBA(255, 0, 0, brightness);

  leds[int(NUM_LEDS*0.5)] = borderColor;
  // leds[int(NUM_LEDS*0.5) + scoreNeeded + 1] = borderColor;
  leds[int(NUM_LEDS*0.5)-1] = borderColor;
  // leds[int(NUM_LEDS*0.5)-1 - scoreNeeded - 1] = borderColor;

  for(int i = 0; i < scoreNeeded; i++){
    if(i < scorePlayerOne){
      leds[int(NUM_LEDS*0.5) - 2 - i] = playerColor;
    }else{
      leds[int(NUM_LEDS*0.5) - 2 - i] = backgroundColor;
    }
  }
  for(int i = 0; i < scoreNeeded; i++){
    if(i < scorePlayerTwo){
      leds[int(NUM_LEDS*0.5) + 1 + i] = playerColor;
    }else{
      leds[int(NUM_LEDS*0.5) + 1 + i] = backgroundColor;
    }
  }
  FastLED.show();
}

void showDifficultyBoard(int brightness = BRIGHTNESS){
  difficultyBoardShown = true;
  scoreBoardShown = false;
  boardAnimation(
    CRGBA(255, 255, 0, brightness),
    CRGBA(255, 0, 0, brightness),
    int(NUM_LEDS*0.5)-difficultyLevels/2, 
    int(NUM_LEDS*0.5)+difficultyLevels/2 -1, 
    1, 
    int(NUM_LEDS*0.5)-difficultyLevels/2 + difficulty);
}
void boardAnimation(CRGB backgroundColor, CRGB foregroundColor, int startPosition, int endPosition, int direction, int position){
  if(direction == 1){
    for(int i = startPosition; i <= endPosition; i++){
      if(i < position){
        leds[i] = foregroundColor;
      }else{
        leds[i] = backgroundColor;
      }
    }
  }else{
    for(int i = startPosition; i >= endPosition; i--){
      if(i > position){
        leds[i] = foregroundColor;
      }else{
        leds[i] = backgroundColor;
      }
    }
  }
 
  FastLED.show();
}

void setupGame() {
  for (int i = 0; i < GAME_LED_WIDTH; i++) {
    leds[i] = CRGBA(0, 255, 0);
    leds[NUM_LEDS - 1 - i] = CRGBA(0, 255, 0);
  }
  
  initShootAnimation = true;

  if(!lastGame){
    ballDirection = 1;
    ballPosition = GAME_LED_WIDTH - 1;
    playerOneStarts = true;

    if(playerOneWins){
      ballDirection *= -1;
      ballPosition = NUM_LEDS - GAME_LED_WIDTH;
      playerOneStarts = false;
    }
    
    // if(playerOneWins){
    //   leds[NUM_LEDS - GAME_LED_WIDTH -1] = CRGBA(255, 255, 255);
    // }else{
    //   leds[GAME_LED_WIDTH] = CRGBA(255, 255, 255);
    // }
  }else{
    scorePlayerOne = 0;
    scorePlayerTwo = 0;
   
    ballDirection = 1;
    ballPosition = GAME_LED_WIDTH - 1;
    playerOneStarts = true;

    if(random(2) == 1){
      ballDirection *= -1;
      ballPosition = NUM_LEDS - GAME_LED_WIDTH;
      playerOneStarts = false;
    }
    showScoreBoard();
  }
  FastLED.show();
}

void setup() {
  Serial.begin(115200);
  Serial.println("init");

  playerOneButton.setDebounceTime(50);
  playerTwoButton.setDebounceTime(50);

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  setupGame();
}

void loop() {
  for(int i = 0; i < 100; i++){
    Serial.println(random(2));
  }
  playerOneButton.loop();
  playerTwoButton.loop();

  unsigned long currentTime = millis();

  if(gameJustEnded && (playerOneButton.isPressed() || playerTwoButton.isPressed())){
    return;
  }else{
    gameJustEnded = false;
  }
  
  if(playerOneButton.isPressed()){
    if(!gameRunning){
      if(!lastGame){
        if(!playerOneWins){
          startGame(1);
        }
      }else if(playerOneStarts){
        startGame(1);
      }
    }else if(ballDirection == -1){
      shootBack(1);
    }
  }
  if(playerTwoButton.isPressed()){
    if(!gameRunning){
       if(!lastGame){
        if(playerOneWins){
          startGame(2);
        }
      }else if(!playerOneStarts){
        startGame(2);
      }
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
  if(!gameRunning){
    checkDifficultyPotentiometer();
    if(currentTime - lastUpdateTime >= (maxDelayAdapted - minDelayAdapted) / 3 * 2){

      int startPosition = 0;
      int endPosition = GAME_LED_WIDTH - 1;
      if(!playerOneStarts){
        startPosition = NUM_LEDS - 1;
        endPosition = NUM_LEDS - GAME_LED_WIDTH;
      }

      if(initShootAnimation){
        shotStrengthPosition = startPosition;
        initShootAnimation = false;
        shotStrengthReversed = false;
      }else{
        if(playerOneStarts){
          if(shotStrengthReversed){
            if(shotStrengthPosition == 0){
              shotStrengthReversed = !shotStrengthReversed;
              shotStrengthPosition++;
            }else{
              shotStrengthPosition--;
            }
          }else{
            if(shotStrengthPosition == GAME_LED_WIDTH){
              shotStrengthReversed = !shotStrengthReversed;
              shotStrengthPosition--;
            }else{
              shotStrengthPosition++;
            }
          }
        }else{
          if(shotStrengthReversed){
            if(shotStrengthPosition == NUM_LEDS - 1){
              shotStrengthReversed = !shotStrengthReversed;
              shotStrengthPosition--;
            }else{
              shotStrengthPosition++;
            }
          }else{
            if(shotStrengthPosition == NUM_LEDS - GAME_LED_WIDTH - 1){
              shotStrengthReversed = !shotStrengthReversed;
              shotStrengthPosition++;
            }else{
              shotStrengthPosition--;
            }
          }
        }
      }
      Serial.println("StrinshotStrengthPosition: " + String(shotStrengthPosition) + ", startPosition: " + String(startPosition) + ", endPosition: " + String(endPosition));
    
      boardAnimation(
        CRGBA(255, 0, 0, BRIGHTNESS),
        CRGBA(0, 255, 0, BRIGHTNESS),
        startPosition, 
        endPosition, 
        ballDirection, 
        shotStrengthPosition);

      lastUpdateTime = currentTime;
    }
  }
}

void checkDifficultyPotentiometer(){
  int analogValue = analogRead(A0);
  float voltage = floatMap(analogValue, 0, 1023, 1, difficultyLevels);
  if(voltage != difficulty){
    difficulty = voltage;
    double scalingFactor;
    if (difficulty < difficultyLevels / 2) {
      // Increase delay for difficulty levels below half of difficultyLevels
      scalingFactor = 1.0 + 0.5 * (difficultyLevels / 2 - difficulty) / (difficultyLevels / 2);
    } else {
      // Decrease delay for difficulty levels above half of difficultyLevels
      scalingFactor = 1.0 - 0.5 * (difficulty - difficultyLevels / 2) / (difficultyLevels / 2);
    }

    // Calculate the adapted minDelayAdapted
    minDelayAdapted = minDelay * scalingFactor;
    maxDelayAdapted = maxDelay * scalingFactor;

    Serial.println("Difficulty: " + String(difficulty) + ", minDelay: " + String(minDelayAdapted) + ", maxDelay: " + String(maxDelayAdapted));
    showDifficultyBoard();
  }
  // Serial.print("Analog: ");
  // Serial.print(analogValue);
  // Serial.print(", Voltage: ");
  // Serial.println(voltage);
}
float floatMap(float x, float in_min, float in_max, float out_min, float out_max) {
  return round((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}

void startGame(int player) {
  if(scoreBoardShown || difficultyBoardShown){
    removeBoards();
  }
  ballPosition = shotStrengthPosition;
  bounceCounter = 0;
  animatedPixels[0] = ballPosition;
  animationSet = true;
  setGameDelay();
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
    
    setGameDelay();
    
    animatedPixels[0] = ballPosition;
    animationSet = true;
  }else{
    endGame(ballDirection == 1);
    return false;
  }
  return true;
}
void setGameDelay(){
   if(ballPosition < GAME_LED_WIDTH){
      gameDelay = minDelayAdapted + (float(ballPosition)/ float(GAME_LED_WIDTH-1)) * (maxDelayAdapted - minDelayAdapted);
    }else{
      gameDelay = minDelayAdapted + (float(NUM_LEDS - 1 - ballPosition) / float(GAME_LED_WIDTH-1)) * (maxDelayAdapted - minDelayAdapted);
    }

    float speedMultiplier = 1.0 - (float) min(bounceCounter, 39) / 40.0; // Adjust this multiplier as needed
    gameDelay = int(gameDelay * speedMultiplier);
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


void endGame(bool bPlayerOneWins){
  playerOneWins = bPlayerOneWins;
  gameRunning = false;
  prevBallPosition = ONULL;
  leds[ballPosition] = CRGBA(0, 0, 0);
  showScoreBoard();
  int startPosition;
  if(playerOneWins){
    startPosition = NUM_LEDS - GAME_LED_WIDTH;
    scorePlayerOne++;
  }else{
    startPosition = GAME_LED_WIDTH - 1;
    scorePlayerTwo++;
  }
  ballDirection *= -1;


  for(int j = 0; j <= 3; j++){
    animatedPixels[0] = startPosition;
    int iteration = j == 3 ? GAME_LED_WIDTH + sizeof(animatedPixels) / sizeof(animatedPixels[0]) : GAME_LED_WIDTH;
    CRGB playerColor = CRGBA(0, 255, 0);

   
    for (int i = 0; i < iteration; i++) {
      if(j != 3){
        updateScoreWinnerVisual(playerOneWins, i);
      }
      updateAnimationPosition();
      updateAnimationVisual(true, j != 3);
      delay(40);
    }
  }

  if(scorePlayerOne == scoreNeeded || scorePlayerTwo == scoreNeeded){
   for (int i = -5; i < 56; i++) {
        // Calculate the angle in radians with a higher frequency
        double angle = static_cast<double>(i) * 2.0 * PI / 20.0;

        // Calculate sin value and scale to be between 0 and 1
        double sinValue = sin(angle);
        double scaledValue = (sinValue + 1.0) / 2.0; // Scale to be between 0 and 1
        // Serial.println(String(angle) + " " + String(scaledValue));
        showScoreBoard(BRIGHTNESS + 15 * scaledValue);
        delay(40);
    }
    lastGame = true;
  }else{
    lastGame = false;
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
    endGame(ballDirection == 1);
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

void removeBoards(){
  for(int j = 1; j <= 10; j++){
    if(scoreBoardShown){
      showScoreBoard(float(BRIGHTNESS)/10.0 * (10.0-j));
      delay(10);
    }
    if(difficultyBoardShown){
      showDifficultyBoard(float(BRIGHTNESS)/10.0 * (10.0-j));
      delay(10);
    } 
  }
  scoreBoardShown = false;
  difficultyBoardShown = false;
}
void updateScoreWinnerVisual(bool playerOneWins, int i){
  int position;
  if(playerOneWins){
    position = int(NUM_LEDS*0.5) - 1 - scorePlayerOne;
  }else{
    position = int(NUM_LEDS*0.5) + scorePlayerTwo;
  }
  // Calculate the angle in radians with a higher frequency
  double angle = static_cast<double>(i+1+(GAME_LED_WIDTH/4)) * 2.0 * PI / (GAME_LED_WIDTH);

  // Calculate sin value and scale to be between 0 and 1
  double sinValue = sin(angle);
  double scaledValue = (sinValue + 1.0) / 2.0; // Scale to be between 0 and 1
  // Serial.println(String(angle) + " " + String(scaledValue));
  leds[position] = CRGBA(
    0,  // Red value (decreasing from 255 to 0 as i increases)
    static_cast<uint8_t>(scaledValue * 255),           // Green value (increasing from 0 to 255 as i increases)
    0                                       // Blue value (set to 0)
  );
  FastLED.show();
}