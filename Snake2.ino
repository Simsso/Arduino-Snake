#include <ShiftRegister74HC595.h> // library download: http://shiftregister.simsso.de

class Coord {
public:
  Coord () {
    x = 255;
    y = 255; 
  }
  Coord (byte pX, byte pY) {
    x = pX;
    y = pY; 
  }
  byte x;
  byte y;
};

const byte topDirPin = 8, rightDirPin = 9, bottomDirPin = 10, leftDirPin = 11, mspmPin = A1;

ShiftRegister74HC595 col (1,0,1,2); // low to enable
ShiftRegister74HC595 row (1,3,4,5); // high to enable

uint8_t led[8][8];

byte snakeDirection = 0, snakeLength = 1; // 0 = up, 1 = right, 2 = bottom, 3 = left
Coord snakePosition (0,7), foodPosition;
Coord * snakeSegments = new Coord[64];
int totalRefreshes = 0;
long mspm = 1000, lastMove = 0; // milliseconds per move
boolean gameRunning = true; 

void setup() {
  //Serial.begin(9600);

  // set direction pins as input
  pinMode(topDirPin, INPUT);
  pinMode(rightDirPin, INPUT);
  pinMode(bottomDirPin, INPUT);
  pinMode(leftDirPin, INPUT);

  // enable pull-up resistor
  digitalWrite(topDirPin, HIGH);
  digitalWrite(rightDirPin, HIGH);
  digitalWrite(bottomDirPin, HIGH);
  digitalWrite(leftDirPin, HIGH);

  randomSeed(analogRead(0));
  
  snakeSegments[0].x = snakePosition.x;
  snakeSegments[0].y = snakePosition.y;

  zeroLedArray(); 

  spawnFood();
}

void loop() {
  // check for new direction
  if (digitalRead(topDirPin) == LOW && digitalRead(rightDirPin) == HIGH && digitalRead(bottomDirPin) == HIGH && digitalRead(leftDirPin) == HIGH && snakeDirection != 2)
    snakeDirection = 0;
  else if (digitalRead(topDirPin) == HIGH && digitalRead(rightDirPin) == LOW && digitalRead(bottomDirPin) == HIGH && digitalRead(leftDirPin) == HIGH && snakeDirection != 3)
    snakeDirection = 1;
  else if (digitalRead(topDirPin) == HIGH && digitalRead(rightDirPin) == HIGH && digitalRead(bottomDirPin) == LOW && digitalRead(leftDirPin) == HIGH && snakeDirection != 0)
    snakeDirection = 2;
  else if (digitalRead(topDirPin) == HIGH && digitalRead(rightDirPin) == HIGH && digitalRead(bottomDirPin) == HIGH && digitalRead(leftDirPin) == LOW && snakeDirection != 1)
    snakeDirection = 3;

  // mspm * totalRefreshes < millis() && gameRunning
  if (mspm + lastMove < millis() && gameRunning) { // time to move the snake
    lastMove = millis();
    mspm = analogRead(mspmPin);
    totalRefreshes++;

    if (snakeDirection == 0)
      snakePosition.y++;
    else if (snakeDirection == 1)
      snakePosition.x++;
    else if (snakeDirection == 2)
      snakePosition.y--;
    else if (snakeDirection == 3)
      snakePosition.x--;

    // boundary conditions
    if (snakePosition.x == 255)
      snakePosition.x = 7;
    if (snakePosition.x == 8)
      snakePosition.x = 0;
    if (snakePosition.y == 255)
      snakePosition.y = 7;
    if (snakePosition.y == 8)
      snakePosition.y = 0;

    // check if snake eats itself
    if (segmentExists(snakePosition.x, snakePosition.y))
      gameRunning = false; 

    // check if snake eats food
    if (snakePosition.x != foodPosition.x || snakePosition.y != foodPosition.y) 
      removeLastSegment();
    else 
      spawnFood();

    addNewSegment();

    zeroLedArray();
    refreshLedArray();

    // Serial.println(String(snakeDirection) + " (" + snakePosition.x + "|" + snakePosition.y + ")");
  }

  if (gameRunning) {  
    // refresh led matrix
    for (int i = 0; i < 8; i++) {
      row.setAll(led[i]);
      col.set(i, LOW);
      delayMicroseconds(50);
      col.setAllHigh();
    }
  }

  // check if game is over
  if (!gameRunning) {
    col.setAllLow();
    row.setAllHigh();
  }
}

boolean segmentExists(byte x, byte y) {
  for (int i = 0; i < snakeLength; i++) 
    if (snakeSegments[i].x == x && snakeSegments[i].y == y)
      return true;
  return false;
}

void addNewSegment() {
  snakeSegments[snakeLength].x = snakePosition.x;
  snakeSegments[snakeLength].y = snakePosition.y;

  snakeLength++;
}

void removeLastSegment() {
  int i = -1;
  for (i = 1; i < snakeLength; i++) {
    snakeSegments[i-1].x = snakeSegments[i].x; 
    snakeSegments[i-1].y = snakeSegments[i].y;
  }
  if (i != -1) {
    snakeSegments[i].x = 255;
    snakeSegments[i].y = 255;
  }

  snakeLength--; 
}

void refreshLedArray() {
  int i = 0;
  while (snakeSegments[i].x != 255) {
    led[snakeSegments[i].x][snakeSegments[i].y] = 1;
    i++;
  }

  led[foodPosition.x][foodPosition.y] = 1;
}

void zeroLedArray() {
  for (int i = 0; i < 8; i++) 
    for (int j = 0; j < 8; j++) 
      led[i][j] = 0; 
}

void spawnFood() {
  byte x, y;

  // Yes! This is the first time I actually use a do {} while (); loop!
  do {
    x = random(8);
    y = random(8);
  } 
  while (segmentExists(x, y));
  foodPosition.x = x;
  foodPosition.y = y;
}
