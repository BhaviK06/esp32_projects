#include <TFT_eSPI.h>

// Create display instance
TFT_eSPI tft = TFT_eSPI();

// Game variables
int playerY = 100;        // Player paddle position
int computerY = 100;      // Computer paddle position
int ballSize = 2;         // Smaller ball size (radius)
int lastBallX, lastBallY; // Track previous ball position
int ballX = 120;          // Ball X position
int ballY = 120;          // Ball Y position
int ballSpeedX = 2;       // Ball X speed
int ballSpeedY = 2;       // Ball Y speed
int paddleWidth = 5;      // Paddle width
int paddleHeight = 30;    // Paddle height
int playerX = 10;         // Player paddle X position
int computerX = 225;      // Computer paddle X position
int playerScore = 0;      // Player score
int computerScore = 0;    // Computer score
unsigned long lastFrameTime = 0;

// Use the rotary encoder for paddle movement
#define ROTARY_CLK_PIN 32
#define ROTARY_DT_PIN  33 
int lastCLKState;
int paddleSpeed = 6;      // Paddle movement speed
// Add variable for smoother paddle movement
int targetPlayerY = 100;  // Target position for player paddle
float currentPlayerY = 100; // Floating point for smoother movement
float paddleSmoothness = 0.3; // Lower = smoother but slower response (0.1-0.5 is good range)

void setup() {
  Serial.begin(115200);
  
  // Initialize the display
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  
  // Rotary Encoder setup
  pinMode(ROTARY_CLK_PIN, INPUT_PULLUP);
  pinMode(ROTARY_DT_PIN, INPUT_PULLUP);
  lastCLKState = digitalRead(ROTARY_CLK_PIN);
  
  // Initial game screen
  drawGameScreen();

    lastBallX = ballX;
  lastBallY = ballY;
}


void drawGameScreen() {
  // Clear the screen
  tft.fillScreen(TFT_BLACK);
  
  // Draw center line
  for (int i = 0; i < 240; i += 10) {
    tft.drawLine(120, i, 120, i + 5, TFT_DARKGREY);
  }
  
  // Draw scores
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(90, 10);
  tft.print(playerScore);
  tft.setCursor(140, 10);
  tft.print(computerScore);
}

void drawGame() {
 // Clear only the previous ball position instead of a large area
  tft.fillCircle(lastBallX, lastBallY, ballSize, TFT_BLACK);
  
  // Clear previous paddle positions
  tft.fillRect(playerX, int(currentPlayerY) - paddleSpeed - 5, paddleWidth, paddleHeight + paddleSpeed*2 + 10, TFT_BLACK);
  tft.fillRect(computerX, computerY - paddleSpeed - 5, paddleWidth, paddleHeight + paddleSpeed*2 + 10, TFT_BLACK);
  
  // Draw paddles with current positions
  tft.fillRect(playerX, int(currentPlayerY), paddleWidth, paddleHeight, TFT_WHITE);
  tft.fillRect(computerX, computerY, paddleWidth, paddleHeight, TFT_WHITE);
  
  // Draw ball
  tft.fillCircle(ballX, ballY, ballSize, TFT_WHITE);
  
  // Save current ball position
  lastBallX = ballX;
  lastBallY = ballY;
}



void handleRotaryEncoder() {

  // Read CLK pin
  int currentCLKState = digitalRead(ROTARY_CLK_PIN);

  // Detect rotation
  if (currentCLKState != lastCLKState) {
    // If DT and CLK are different, rotate clockwise
    if (digitalRead(ROTARY_DT_PIN) != currentCLKState) {
      // Rotate right (move paddle down)
      targetPlayerY = min(targetPlayerY + paddleSpeed, 240 - paddleHeight);
    } else {
      // Rotate left (move paddle up)
      targetPlayerY = max(targetPlayerY - paddleSpeed, 0);
    }
  }
  lastCLKState = currentCLKState;
}

void updateGame() {

   // Apply smoothing to player paddle movement
  currentPlayerY = currentPlayerY + (targetPlayerY - currentPlayerY) * paddleSmoothness;
  playerY = int(currentPlayerY);
  // Move ball
  ballX += ballSpeedX;
  ballY += ballSpeedY;
  
  // Ball collision with top/bottom
  if (ballY <= 0 || ballY >= 240) {
    ballSpeedY = -ballSpeedY;
  }
  
  // Ball collision with paddles
  if ((ballX <= playerX + paddleWidth) && 
      (ballY >= playerY) && 
      (ballY <= playerY + paddleHeight) && 
      (ballSpeedX < 0)) {
    ballSpeedX = -ballSpeedX;
    // Slight randomization of return angle
    ballSpeedY += random(-1, 2);
  }
  
  if ((ballX >= computerX - 3) && 
      (ballY >= computerY) && 
      (ballY <= computerY + paddleHeight) && 
      (ballSpeedX > 0)) {
    ballSpeedX = -ballSpeedX;
    // Slight randomization of return angle
    ballSpeedY += random(-1, 2);
  }
  
  // Scoring
  if (ballX <= 0) {
    // Computer scores
    computerScore++;
    resetBall();
    drawGameScreen();
  }
  
  if (ballX >= 240) {
    // Player scores
    playerScore++;
    resetBall();
    drawGameScreen();
  }
  
  // Very simple AI for computer paddle
  if (ballSpeedX > 0) {  // Only move if ball is moving toward computer
    // Move toward ball with slight delay
    if (computerY + (paddleHeight / 2) < ballY) {
      computerY = min(computerY + 3, 240 - paddleHeight);
    }
    if (computerY + (paddleHeight / 2) > ballY) {
      computerY = max(computerY - 3, 0);
    }
  }
  
  // Keep ball speed in reasonable range
  ballSpeedY = constrain(ballSpeedY, -4, 4);
}

void resetBall() {
  // Reset ball to center
  ballX = 120;
  ballY = 120;
  // Randomize initial direction
  ballSpeedX = random(0, 2) == 0 ? -2 : 2;
  ballSpeedY = random(-2, 3);
}



void loop() {
  // Control game speed
  if (millis() - lastFrameTime > 16) {  // ~60fps
    handleRotaryEncoder();
    updateGame();
    drawGame();
    lastFrameTime = millis();
  }
}
