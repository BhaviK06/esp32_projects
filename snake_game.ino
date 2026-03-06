#include <TFT_eSPI.h>

// Create display instance
TFT_eSPI tft = TFT_eSPI();

// Game constants
#define GRID_SIZE 10  // Size of each grid cell in pixels
#define WIDTH_CELLS 24  // Number of grid cells horizontally (240 / 10)
#define HEIGHT_CELLS 24  // Number of grid cells vertically (240 / 10)
#define GAME_SPEED_INITIAL 150  // Time between updates in ms (lower = faster)

// Rotary encoder pins
#define ROTARY_CLK_PIN 32
#define ROTARY_DT_PIN  33

// Colors
#define SNAKE_COLOR TFT_GREEN
#define FOOD_COLOR TFT_RED
#define BACKGROUND_COLOR TFT_BLACK
#define BORDER_COLOR TFT_BLUE

// Direction definitions
enum Direction {
  UP,
  RIGHT,
  DOWN,
  LEFT
};

// Game variables
Direction currentDirection = RIGHT;
Direction nextDirection = RIGHT;
int lastCLKState;
unsigned long lastFrameTime = 0;
unsigned long gameSpeed = GAME_SPEED_INITIAL;
int score = 0;
bool gameOver = false;

// Snake structure
const int MAX_SNAKE_LENGTH = 100;
int snakeX[MAX_SNAKE_LENGTH];
int snakeY[MAX_SNAKE_LENGTH];
int snakeLength = 3;

// Food position
int foodX;
int foodY;

void setup() {
  Serial.begin(115200);
  
  // Initialize the display
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(BACKGROUND_COLOR);
  
  // Rotary Encoder setup
  pinMode(ROTARY_CLK_PIN, INPUT_PULLUP);
  pinMode(ROTARY_DT_PIN, INPUT_PULLUP);
  lastCLKState = digitalRead(ROTARY_CLK_PIN);
  
  // Initialize game
  initGame();
}

void initGame() {
  // Clear screen
  tft.fillScreen(BACKGROUND_COLOR);
  
  // Draw border
  tft.drawRect(0, 0, WIDTH_CELLS * GRID_SIZE, HEIGHT_CELLS * GRID_SIZE, BORDER_COLOR);
  
  // Initialize snake in the middle of the screen
  for (int i = 0; i < snakeLength; i++) {
    snakeX[i] = WIDTH_CELLS / 2 - i;
    snakeY[i] = HEIGHT_CELLS / 2;
  }
  
  // Place initial food
  placeFood();
  
  // Reset game variables
  currentDirection = RIGHT;
  nextDirection = RIGHT;
  score = 0;
  snakeLength = 3;
  gameSpeed = GAME_SPEED_INITIAL;
  gameOver = false;
  
  // Display score
  updateScore();
}

void placeFood() {
  bool validPosition;
  
  do {
    validPosition = true;
    foodX = random(1, WIDTH_CELLS - 1);
    foodY = random(1, HEIGHT_CELLS - 1);
    
    // Make sure food doesn't appear on the snake
    for (int i = 0; i < snakeLength; i++) {
      if (snakeX[i] == foodX && snakeY[i] == foodY) {
        validPosition = false;
        break;
      }
    }
  } while (!validPosition);
  
  // Draw food
  tft.fillRect(foodX * GRID_SIZE, foodY * GRID_SIZE, GRID_SIZE, GRID_SIZE, FOOD_COLOR);
}

void handleRotaryEncoder() {
  // Read CLK pin
  int currentCLKState = digitalRead(ROTARY_CLK_PIN);

  // Detect rotation
  if (currentCLKState != lastCLKState) {
    // If DT and CLK are different, rotate clockwise
    if (digitalRead(ROTARY_DT_PIN) != currentCLKState) {
      // Turn right (clockwise rotation)
      switch (currentDirection) {
        case UP: nextDirection = RIGHT; break;
        case RIGHT: nextDirection = DOWN; break;
        case DOWN: nextDirection = LEFT; break;
        case LEFT: nextDirection = UP; break;
      }
    } else {
      // Turn left (counter-clockwise rotation)
      switch (currentDirection) {
        case UP: nextDirection = LEFT; break;
        case LEFT: nextDirection = DOWN; break;
        case DOWN: nextDirection = RIGHT; break;
        case RIGHT: nextDirection = UP; break;
      }
    }
  }
  lastCLKState = currentCLKState;
}

void updateGame() {
  if (gameOver) {
    return;
  }
  
  // Update direction (ensure we can't move directly opposite of current direction)
  if ((currentDirection == UP && nextDirection != DOWN) ||
      (currentDirection == DOWN && nextDirection != UP) ||
      (currentDirection == LEFT && nextDirection != RIGHT) ||
      (currentDirection == RIGHT && nextDirection != LEFT)) {
    currentDirection = nextDirection;
  }
  
  // Calculate new head position
  int newHeadX = snakeX[0];
  int newHeadY = snakeY[0];
  
  // Move head in current direction
  switch (currentDirection) {
    case UP:    newHeadY--; break;
    case RIGHT: newHeadX++; break;
    case DOWN:  newHeadY++; break;
    case LEFT:  newHeadX--; break;
  }
  
  // Check for collision with walls
  if (newHeadX < 0 || newHeadX >= WIDTH_CELLS || newHeadY < 0 || newHeadY >= HEIGHT_CELLS) {
    gameOver = true;
    displayGameOver();
    return;
  }
  
  // Check for collision with self
  for (int i = 0; i < snakeLength; i++) {
    if (snakeX[i] == newHeadX && snakeY[i] == newHeadY) {
      gameOver = true;
      displayGameOver();
      return;
    }
  }
  
  // Check if food was eaten
  bool ateFood = (newHeadX == foodX && newHeadY == foodY);
  
  // If food was eaten, increase snake length and don't erase tail
  if (ateFood) {
    if (snakeLength < MAX_SNAKE_LENGTH) {
      snakeLength++;
    }
    score += 10;
    updateScore();
    
    // Speed up the game slightly with each food eaten
    if (gameSpeed > 50) {
      gameSpeed -= 5;
    }
    
    // Place new food
    placeFood();
  } else {
    // Erase tail
    tft.fillRect(snakeX[snakeLength-1] * GRID_SIZE, snakeY[snakeLength-1] * GRID_SIZE, 
                 GRID_SIZE, GRID_SIZE, BACKGROUND_COLOR);
  }
  
  // Move body segments
  for (int i = snakeLength - 1; i > 0; i--) {
    snakeX[i] = snakeX[i-1];
    snakeY[i] = snakeY[i-1];
  }
  
  // Update head position
  snakeX[0] = newHeadX;
  snakeY[0] = newHeadY;
  
  // Draw new head
  tft.fillRect(snakeX[0] * GRID_SIZE, snakeY[0] * GRID_SIZE, 
               GRID_SIZE, GRID_SIZE, SNAKE_COLOR);
}

void updateScore() {
  // Clear score area
  tft.fillRect(0, HEIGHT_CELLS * GRID_SIZE + 2, WIDTH_CELLS * GRID_SIZE, 20, BACKGROUND_COLOR);
  
  // Display score
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(5, HEIGHT_CELLS * GRID_SIZE + 5);
  tft.print("Score: ");
  tft.print(score);
}

void displayGameOver() {
  tft.setTextSize(2);
  tft.setTextColor(TFT_RED);
  tft.setCursor(70, 100);
  tft.print("GAME OVER");
  
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(50, 130);
  tft.print("Final Score: ");
  tft.print(score);
  
  tft.setCursor(40, 150);
  tft.print("Rotate to play again");
}

void loop() {
  // Handle rotary encoder input
  handleRotaryEncoder();
  
  // If game over and rotary encoder moved, restart game
  if (gameOver && currentDirection != nextDirection) {
    initGame();
    delay(500); // Small delay to avoid immediate restart
    return;
  }
  
  // Control game speed
  if (millis() - lastFrameTime > gameSpeed) {
    updateGame();
    lastFrameTime = millis();
  }
}
