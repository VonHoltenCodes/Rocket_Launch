/*
 * Matrix Portal M4 SpaceX-Style Rocket Animation - ENHANCED VERSION
 * 64x32 RGB LED Matrix
 * 
 * Features:
 * - Countdown from 5 with BLACK numbers for better visibility (6 seconds)
 * - Rocket launch with ground descending and space revealing (3 seconds)
 * - Space travel with stars (5 seconds)
 * - Moon landing fully visible on screen (4-5 seconds)
 * - Astronauts on the moon (8 seconds)
 * - Flag planting in top corner of moon (5 seconds)
 * - Rainbow text on black background for final message (7-8 seconds)
 */

#include <Adafruit_Protomatter.h>

// Matrix configuration
#define MATRIX_WIDTH 64
#define MATRIX_HEIGHT 32
#define MATRIX_BIT_DEPTH 4

// Debug mode (set to true for serial debugging info)
#define DEBUG_MODE false

// Matrix pins for Adafruit Matrix Portal M4
uint8_t rgbPins[]  = {7, 8, 9, 10, 11, 12};
uint8_t addrPins[] = {17, 18, 19, 20};
uint8_t clockPin   = 14;
uint8_t latchPin   = 15;
uint8_t oePin      = 16;

// Create matrix instance
Adafruit_Protomatter matrix(
  MATRIX_WIDTH,      // Width of matrix (in pixels)
  4,                 // Bit depth (1-6)
  1, rgbPins,        // # of matrix chains, array of RGB pins
  4, addrPins,       // # of address pins, array of pins
  clockPin, latchPin, oePin,
  false,             // No double-buffering
  1);                // Default matrix height

// Colors
#define BLACK    matrix.color565(0, 0, 0)
#define WHITE    matrix.color565(255, 255, 255)
#define RED      matrix.color565(255, 0, 0)
#define GREEN    matrix.color565(0, 255, 0)
#define BLUE     matrix.color565(0, 0, 255)
#define YELLOW   matrix.color565(255, 255, 0)
#define ORANGE   matrix.color565(255, 140, 0)
#define LIGHT_BLUE matrix.color565(135, 206, 250)
#define SKY_BLUE matrix.color565(100, 180, 255)
#define GROUND_GREEN matrix.color565(40, 160, 40)
#define PINK     matrix.color565(255, 105, 180)
#define GRAY     matrix.color565(128, 128, 128)
#define DARK_GRAY matrix.color565(80, 80, 80)
#define MOON_COLOR matrix.color565(220, 220, 180)
#define PURPLE  matrix.color565(180, 0, 255)

// Animation phases
enum AnimPhase {
  COUNTDOWN,
  LAUNCH,
  SPACE_TRAVEL,
  LANDING,
  ASTRONAUTS,
  PLANT_FLAG,
  TEXT_MESSAGE
};

AnimPhase currentPhase = COUNTDOWN;
unsigned long phaseStartTime = 0;
int countdownNumber = 5;
int rocketY = MATRIX_HEIGHT - 10;
int rocketX = 32;
int moonY = 5; // Keep the moon at a fixed position, visible on the screen
int groundLevel = 5; // Ground level for launch phase
bool astronautsOnMoon = false;
int astronaut1X = 20;
int astronaut2X = 40;
int astronautY = 0;
int flagX = 42; // Flag moved to edge of moon
int flagY = 0;
bool flagPlanted = false;
int messageScroll = MATRIX_WIDTH;

void setup() {
  Serial.begin(9600);
  
  // Initialize matrix
  ProtomatterStatus status = matrix.begin();
  Serial.print("Matrix status: ");
  Serial.println((int)status);
  if (status != PROTOMATTER_OK) {
    // Matrix setup failed, halt
    Serial.println("Matrix initialization failed!");
    for(;;);
  }
  
  matrix.fillScreen(BLACK);
  phaseStartTime = millis();
  
  Serial.println("Animation starting...");
}

void loop() {
  // Clear screen
  matrix.fillScreen(BLACK);
  
  // Update animation based on current phase
  switch (currentPhase) {
    case COUNTDOWN:
      updateCountdown();
      break;
    case LAUNCH:
      updateLaunch();
      break;
    case SPACE_TRAVEL:
      updateSpaceTravel();
      break;
    case LANDING:
      updateLanding();
      break;
    case ASTRONAUTS:
      updateAstronauts();
      break;
    case PLANT_FLAG:
      updatePlantFlag();
      break;
    case TEXT_MESSAGE:
      updateTextMessage();
      break;
  }
  
  // Debug output
  if (DEBUG_MODE) {
    Serial.print("Phase: ");
    Serial.print(currentPhase);
    Serial.print(", Time in phase: ");
    Serial.print(millis() - phaseStartTime);
    Serial.print("ms, RocketY: ");
    Serial.print(rocketY);
    Serial.print(", MoonY: ");
    Serial.println(moonY);
  }
  
  // Update the display
  matrix.show();
  delay(50); // Standard frame rate (20 FPS)
}

// Draw rocket at current position
void drawRocket() {
  // Rocket body (red and white)
  matrix.fillRect(rocketX - 2, rocketY - 8, 5, 8, WHITE);
  matrix.fillRect(rocketX - 2, rocketY - 4, 5, 4, RED);
  
  // Rocket nose cone
  matrix.fillTriangle(rocketX, rocketY - 10, rocketX - 2, rocketY - 8, rocketX + 2, rocketY - 8, RED);
  
  // Rocket fins
  matrix.fillTriangle(rocketX - 2, rocketY, rocketX - 4, rocketY, rocketX - 2, rocketY - 3, RED);
  matrix.fillTriangle(rocketX + 2, rocketY, rocketX + 4, rocketY, rocketX + 2, rocketY - 3, RED);
  
  // Rocket engine fire (when launching or landing)
  if (currentPhase == LAUNCH || currentPhase == LANDING) {
    int fireLength = random(3, 6);
    
    // Orange center flame
    matrix.fillTriangle(
      rocketX, rocketY + fireLength,
      rocketX - 1, rocketY,
      rocketX + 1, rocketY,
      ORANGE
    );
    
    // Blue outer flame
    matrix.drawLine(rocketX - 2, rocketY + 1, rocketX - 1, rocketY + 3, LIGHT_BLUE);
    matrix.drawLine(rocketX + 2, rocketY + 1, rocketX + 1, rocketY + 3, LIGHT_BLUE);
  }
}

// Draw astronauts
void drawAstronauts(int x1, int x2, int y) {
  // Astronaut 1 (blue suit)
  matrix.fillCircle(x1, y - 2, 1, WHITE);  // Helmet
  matrix.fillRect(x1 - 1, y, 3, 3, BLUE);  // Body
  
  // Astronaut 2 (pink suit)
  matrix.fillCircle(x2, y - 2, 1, WHITE);  // Helmet
  matrix.fillRect(x2 - 1, y, 3, 3, PINK);  // Body
}

// Draw the moon - REPOSITIONED TO BE FULLY VISIBLE
void drawMoon() {
  // Fixed position moon that's fully visible on screen
  matrix.fillCircle(32, moonY + 10, 8, MOON_COLOR);
  
  // Craters - adjusted positions
  matrix.fillCircle(28, moonY + 7, 2, DARK_GRAY);
  matrix.fillCircle(38, moonY + 12, 1, DARK_GRAY);
  matrix.fillCircle(32, moonY + 14, 2, DARK_GRAY);
}

// Draw Earth (blue sky, green ground)
void drawEarth(int groundPosition) {
  // Sky (always fill whole screen first)
  matrix.fillRect(0, 0, MATRIX_WIDTH, MATRIX_HEIGHT, SKY_BLUE);
  
  // Ground at bottom portion
  if (groundPosition > 0) {
    matrix.fillRect(0, MATRIX_HEIGHT - groundPosition, MATRIX_WIDTH, groundPosition, GROUND_GREEN);
  }
  
  // Simple launch pad
  if (groundPosition > 2) {
    matrix.fillRect(rocketX - 8, MATRIX_HEIGHT - groundPosition, 16, 2, GRAY);
  }
}

// Draw flag - REPOSITIONED to top corner of moon
void drawFlag(int x, int y) {
  matrix.drawLine(x, y, x, y - 5, WHITE);  // Flagpole
  matrix.fillRect(x + 1, y - 5, 3, 2, RED);  // Flag
}

// Draw stars in the background
void drawStars(int density) {
  for (int i = 0; i < density; i++) {
    int x = random(MATRIX_WIDTH);
    int y = random(MATRIX_HEIGHT);
    
    // 20% chance of a brighter star
    if (random(100) < 20) {
      matrix.drawPixel(x, y, WHITE);
      
      // Occasionally draw a twinkle effect
      if (random(100) < 30) {
        if (x+1 < MATRIX_WIDTH) matrix.drawPixel(x+1, y, DARK_GRAY);
        if (x-1 >= 0) matrix.drawPixel(x-1, y, DARK_GRAY);
        if (y+1 < MATRIX_HEIGHT) matrix.drawPixel(x, y+1, DARK_GRAY);
        if (y-1 >= 0) matrix.drawPixel(x, y-1, DARK_GRAY);
      }
    } else {
      matrix.drawPixel(x, y, GRAY);
    }
  }
}

// Get a rainbow color based on position/time
uint16_t rainbowColor(int position) {
  position = position % 256;
  
  if (position < 85) {
    return matrix.color565(255 - position * 3, 0, position * 3);
  } else if (position < 170) {
    position -= 85;
    return matrix.color565(0, position * 3, 255 - position * 3);
  } else {
    position -= 170;
    return matrix.color565(position * 3, 255 - position * 3, 0);
  }
}

// Phase 1: Countdown from 5 (6 seconds total)
void updateCountdown() {
  unsigned long currentTime = millis();
  unsigned long phaseDuration = currentTime - phaseStartTime;
  
  // Draw Earth background with launch pad
  drawEarth(5);
  
  if (phaseDuration > 1000) {  // 1 second per number
    countdownNumber--;
    phaseStartTime = currentTime;
    
    if (countdownNumber < 0) {
      // Move to launch phase
      currentPhase = LAUNCH;
      phaseStartTime = currentTime;
      groundLevel = 5;  // Set initial ground level
      Serial.println("Starting LAUNCH phase");
      return;
    }
  }
  
  // Draw countdown number - CENTERED in screen with BLACK numbers
  int numWidth = 5;
  int x = (MATRIX_WIDTH - numWidth) / 2;
  int y = (MATRIX_HEIGHT - 7) / 2;
  
  switch (countdownNumber) {
    case 5:
      matrix.fillRect(x, y, numWidth, 1, BLACK);
      matrix.fillRect(x, y, 1, 4, BLACK);
      matrix.fillRect(x, y + 3, numWidth, 1, BLACK);
      matrix.fillRect(x + numWidth - 1, y + 3, 1, 3, BLACK);
      matrix.fillRect(x, y + 6, numWidth, 1, BLACK);
      break;
    case 4:
      matrix.fillRect(x, y, 1, 4, BLACK);
      matrix.fillRect(x + numWidth - 1, y, 1, 7, BLACK);
      matrix.fillRect(x, y + 3, numWidth, 1, BLACK);
      break;
    case 3:
      matrix.fillRect(x, y, numWidth, 1, BLACK);
      matrix.fillRect(x + numWidth - 1, y, 1, 3, BLACK);
      matrix.fillRect(x, y + 3, numWidth, 1, BLACK);
      matrix.fillRect(x + numWidth - 1, y + 3, 1, 3, BLACK);
      matrix.fillRect(x, y + 6, numWidth, 1, BLACK);
      break;
    case 2:
      matrix.fillRect(x, y, numWidth, 1, BLACK);
      matrix.fillRect(x + numWidth - 1, y, 1, 3, BLACK);
      matrix.fillRect(x, y + 3, numWidth, 1, BLACK);
      matrix.fillRect(x, y + 3, 1, 3, BLACK);
      matrix.fillRect(x, y + 6, numWidth, 1, BLACK);
      break;
    case 1:
      matrix.fillRect(x + numWidth - 1, y, 1, 7, BLACK);
      break;
    case 0:
      matrix.fillCircle(MATRIX_WIDTH / 2, MATRIX_HEIGHT / 2, 3, BLACK);
      break;
  }
  
  // Only draw rocket at bottom if we're not displaying the countdown number
  if (countdownNumber <= 0) {
    drawRocket();
  }
}

// Phase 2: Launch the rocket with ground descending and space revealing (3 seconds)
void updateLaunch() {
  unsigned long currentTime = millis();
  unsigned long phaseDuration = currentTime - phaseStartTime;
  
  // Calculate the ground level - GROUND DESCENDS as rocket rises
  float progress = phaseDuration / 3000.0; // 0.0 to 1.0 over 3 seconds
  if (progress > 1.0) progress = 1.0;
  
  // Ground level starts at 5 and descends to 0
  groundLevel = 5 * (1.0 - progress);
  
  // Calculate how much of sky is visible vs. space
  // Sky recedes from top as rocket rises
  int skyHeight = MATRIX_HEIGHT * (1.0 - progress);
  
  // First draw all black (space)
  matrix.fillScreen(BLACK);
  
  // Draw stars in space (only in the space part)
  int starHeight = MATRIX_HEIGHT - skyHeight;
  if (starHeight > 0) {
    for (int i = 0; i < starHeight/2; i++) {
      int x = random(MATRIX_WIDTH);
      int y = random(starHeight);
      matrix.drawPixel(x, y, WHITE);
    }
  }
  
  // Then draw sky at bottom portion
  if (skyHeight > 0) {
    matrix.fillRect(0, starHeight, MATRIX_WIDTH, skyHeight, SKY_BLUE);
  }
  
  // Draw ground at the very bottom
  if (groundLevel > 0) {
    matrix.fillRect(0, MATRIX_HEIGHT - groundLevel, MATRIX_WIDTH, groundLevel, GROUND_GREEN);
    matrix.fillRect(rocketX - 8, MATRIX_HEIGHT - groundLevel, 16, 2, GRAY); // launch pad
  }
  
  // Move rocket up at a moderate pace
  rocketY -= 1;
  
  // Draw rocket
  drawRocket();
  
  // Transition to space travel
  if (rocketY <= -10 || phaseDuration > 3000) {
    currentPhase = SPACE_TRAVEL;
    phaseStartTime = currentTime;
    Serial.println("Starting SPACE_TRAVEL phase");
  }
}

// Phase 3: Space travel (5 seconds)
void updateSpaceTravel() {
  unsigned long currentTime = millis();
  unsigned long phaseDuration = currentTime - phaseStartTime;
  
  // Draw stars in the background
  drawStars(30);
  
  // Rocket moves across screen
  rocketX = 10 + (phaseDuration / 200) % (MATRIX_WIDTH - 20);
  rocketY = 10;
  
  // Show rocket traveling
  drawRocket();
  
  // Transition to landing
  if (phaseDuration > 5000) {
    currentPhase = LANDING;
    phaseStartTime = currentTime;
    rocketY = -5;  // Start off screen
    rocketX = 32;  // Center horizontally
    Serial.println("Starting LANDING phase");
  }
}

// Phase 4: Landing on the moon (4-5 seconds) - REPOSITIONED TO BE VISIBLE
void updateLanding() {
  unsigned long currentTime = millis();
  unsigned long phaseDuration = currentTime - phaseStartTime;
  
  // Draw stars in background
  drawStars(15);
  
  // Draw moon (fixed position that's visible on screen)
  drawMoon();
  
  // Move rocket down for landing - destination is the moon surface
  int moonSurface = moonY + 5; // The surface of the moon where rocket will land
  
  // Calculate rocket position to land exactly at the end of the phase
  float landingProgress = (float)phaseDuration / 4500.0; // 4.5 seconds total
  if (landingProgress > 1.0) landingProgress = 1.0;
  
  // Start at -5 (offscreen) and move to moonSurface
  rocketY = -5 + (moonSurface - (-5)) * landingProgress;
  
  // Draw rocket
  drawRocket();
  
  // Transition to astronauts
  if (phaseDuration > 4500) {
    currentPhase = ASTRONAUTS;
    phaseStartTime = currentTime;
    astronautsOnMoon = true;
    astronautY = moonY + 5;  // On moon surface
    astronaut1X = rocketX - 4;
    astronaut2X = rocketX + 4;
    Serial.println("Starting ASTRONAUTS phase");
  }
}

// Phase 5: Astronauts on the moon (8 seconds)
void updateAstronauts() {
  unsigned long currentTime = millis();
  unsigned long phaseDuration = currentTime - phaseStartTime;
  
  // Draw stars in background
  drawStars(15);
  
  // Draw moon
  drawMoon();
  
  // Draw landed rocket
  drawRocket();
  
  // Position and animate astronauts
  if (phaseDuration < 3000) {
    // Astronauts getting out of rocket
    astronaut1X = rocketX - 4 - (phaseDuration / 500);
    astronaut2X = rocketX + 4 + (phaseDuration / 500);
  } else {
    // Astronauts hopping on the moon
    int hopHeight = sin((float)phaseDuration / 300.0) * 2;
    astronautY = moonY + 5 + hopHeight;
    
    // Also move side to side a bit
    astronaut1X = rocketX - 8 + sin((float)phaseDuration / 500.0) * 3;
    astronaut2X = rocketX + 8 + cos((float)phaseDuration / 500.0) * 3;
  }
  
  // Keep astronauts on screen
  astronaut1X = constrain(astronaut1X, 2, MATRIX_WIDTH - 3);
  astronaut2X = constrain(astronaut2X, 2, MATRIX_WIDTH - 3);
  
  // Draw astronauts
  drawAstronauts(astronaut1X, astronaut2X, astronautY);
  
  // Transition to plant flag
  if (phaseDuration > 8000) {
    currentPhase = PLANT_FLAG;
    phaseStartTime = currentTime;
    // Move flag to top edge of moon (42, moonY + 3)
    flagX = 42;
    flagY = moonY + 3;
    Serial.println("Starting PLANT_FLAG phase");
  }
}

// Phase 6: Planting the flag (5 seconds) - REPOSITIONED TO CORNER OF MOON
void updatePlantFlag() {
  unsigned long currentTime = millis();
  unsigned long phaseDuration = currentTime - phaseStartTime;
  
  // Draw stars in background
  drawStars(15);
  
  // Draw moon
  drawMoon();
  
  // Draw landed rocket
  drawRocket();
  
  // Draw astronauts moving toward flag position
  int astronaut1TargetX = flagX - 3;
  int astronaut2TargetX = flagX - 1;
  
  // Gradually move astronauts to flag position
  if (phaseDuration < 2000) {
    astronaut1X = rocketX - 8 + (astronaut1TargetX - (rocketX - 8)) * (phaseDuration / 2000.0);
    astronaut2X = rocketX + 8 + (astronaut2TargetX - (rocketX + 8)) * (phaseDuration / 2000.0);
  } else {
    astronaut1X = astronaut1TargetX;
    astronaut2X = astronaut2TargetX;
    
    // Small hopping motion when at flag
    int hopHeight = sin((float)(phaseDuration - 2000) / 300.0) * 1;
    astronautY = moonY + 5 + hopHeight;
  }
  
  // Draw astronauts
  drawAstronauts(astronaut1X, astronaut2X, astronautY);
  
  // Animate flag planting
  if (phaseDuration > 1000 && !flagPlanted) {
    flagPlanted = true;
  }
  
  // Draw flag at top corner of moon
  drawFlag(flagX, flagY);
  
  // Transition to text message
  if (phaseDuration > 5000) {
    currentPhase = TEXT_MESSAGE;
    phaseStartTime = currentTime;
    messageScroll = MATRIX_WIDTH;
    Serial.println("Starting TEXT_MESSAGE phase");
  }
}

// Phase 7: Text message with rainbow colors on black background
void updateTextMessage() {
  unsigned long currentTime = millis();
  unsigned long phaseDuration = currentTime - phaseStartTime;
  
  // Black background (already set by fillScreen at start of loop)
  
  // Scroll message
  messageScroll -= 1;
  
  // Display scrolling message with RAINBOW colors
  int charWidth = 4;
  int spacing = 1;
  const char* text = "ONE SMALL STEP FOR VON HOLTEN";
  
  for (int i = 0; text[i] != '\0'; i++) {
    int charX = messageScroll + i * (charWidth + spacing);
    
    // Only draw if character is visible on screen
    if (charX + charWidth >= 0 && charX < MATRIX_WIDTH) {
      // Calculate rainbow color based on character position and time
      uint16_t color = rainbowColor((i * 10) + (phaseDuration / 50));
      drawChar(text[i], charX, MATRIX_HEIGHT / 2 - 2, color);
    }
  }
  
  // Reset animation after message scrolls off screen
  if (messageScroll < -250 || phaseDuration > 8000) {
    currentPhase = COUNTDOWN;
    phaseStartTime = currentTime;
    countdownNumber = 5;
    rocketY = MATRIX_HEIGHT - 10;
    rocketX = 32;
    astronautsOnMoon = false;
    flagPlanted = false;
    messageScroll = MATRIX_WIDTH;
    Serial.println("Restarting animation");
  }
}

// Helper to draw text
void drawText(const char* text, int x, int y, uint16_t color) {
  int charWidth = 4;
  int spacing = 1;
  
  for (int i = 0; text[i] != '\0'; i++) {
    int charX = x + i * (charWidth + spacing);
    
    // Only draw if character is visible on screen
    if (charX + charWidth >= 0 && charX < MATRIX_WIDTH) {
      drawChar(text[i], charX, y, color);
    }
  }
}

// Draw a simple 3x5 character
void drawChar(char c, int x, int y, uint16_t color) {
  switch (c) {
    case 'A':
      matrix.drawLine(x, y+1, x, y+4, color);
      matrix.drawLine(x+2, y+1, x+2, y+4, color);
      matrix.drawLine(x, y, x+2, y, color);
      matrix.drawLine(x, y+2, x+2, y+2, color);
      break;
    case 'B':
      matrix.drawLine(x, y, x, y+4, color);
      matrix.drawLine(x, y, x+1, y, color);
      matrix.drawLine(x+2, y+1, x+2, y+1, color);
      matrix.drawLine(x, y+2, x+1, y+2, color);
      matrix.drawLine(x+2, y+3, x+2, y+3, color);
      matrix.drawLine(x, y+4, x+1, y+4, color);
      break;
    case 'C':
      matrix.drawLine(x+1, y, x+2, y, color);
      matrix.drawLine(x, y+1, x, y+3, color);
      matrix.drawLine(x+1, y+4, x+2, y+4, color);
      break;
    case 'D':
      matrix.drawLine(x, y, x, y+4, color);
      matrix.drawLine(x, y, x+1, y, color);
      matrix.drawLine(x+2, y+1, x+2, y+3, color);
      matrix.drawLine(x, y+4, x+1, y+4, color);
      break;
    case 'E':
      matrix.drawLine(x, y, x, y+4, color);
      matrix.drawLine(x, y, x+2, y, color);
      matrix.drawLine(x, y+2, x+2, y+2, color);
      matrix.drawLine(x, y+4, x+2, y+4, color);
      break;
    case 'F':
      matrix.drawLine(x, y, x, y+4, color);
      matrix.drawLine(x, y, x+2, y, color);
      matrix.drawLine(x, y+2, x+2, y+2, color);
      break;
    case 'G':
      matrix.drawLine(x+1, y, x+2, y, color);
      matrix.drawLine(x, y+1, x, y+3, color);
      matrix.drawLine(x+1, y+4, x+2, y+4, color);
      matrix.drawLine(x+2, y+2, x+2, y+3, color);
      break;
    case 'H':
      matrix.drawLine(x, y, x, y+4, color);
      matrix.drawLine(x+2, y, x+2, y+4, color);
      matrix.drawLine(x, y+2, x+2, y+2, color);
      break;
    case 'I':
      matrix.drawLine(x, y, x+2, y, color);
      matrix.drawLine(x+1, y, x+1, y+4, color);
      matrix.drawLine(x, y+4, x+2, y+4, color);
      break;
    case 'J':
      matrix.drawLine(x+2, y, x+2, y+3, color);
      matrix.drawLine(x, y+3, x, y+3, color);
      matrix.drawLine(x+1, y+4, x+1, y+4, color);
      break;
    case 'K':
      matrix.drawLine(x, y, x, y+4, color);
      matrix.drawLine(x, y+2, x+1, y+2, color);
      matrix.drawLine(x+2, y, x+2, y+1, color);
      matrix.drawLine(x+2, y+3, x+2, y+4, color);
      break;
    case 'L':
      matrix.drawLine(x, y, x, y+4, color);
      matrix.drawLine(x, y+4, x+2, y+4, color);
      break;
    case 'M':
      matrix.drawLine(x, y, x, y+4, color);
      matrix.drawLine(x+2, y, x+2, y+4, color);
      matrix.drawPixel(x+1, y+1, color);
      break;
    case 'N':
      matrix.drawLine(x, y, x, y+4, color);
      matrix.drawLine(x+2, y, x+2, y+4, color);
      matrix.drawLine(x, y, x+2, y+4, color);
      break;
    case 'O':
      matrix.drawLine(x, y+1, x, y+3, color);
      matrix.drawLine(x+2, y+1, x+2, y+3, color);
      matrix.drawLine(x+1, y, x+1, y, color);
      matrix.drawLine(x+1, y+4, x+1, y+4, color);
      break;
    case 'P':
      matrix.drawLine(x, y, x, y+4, color);
      matrix.drawLine(x, y, x+2, y, color);
      matrix.drawLine(x+2, y, x+2, y+2, color);
      matrix.drawLine(x, y+2, x+2, y+2, color);
      break;
    case 'Q':
      matrix.drawLine(x, y+1, x, y+3, color);
      matrix.drawLine(x+2, y+1, x+2, y+3, color);
      matrix.drawLine(x+1, y, x+1, y, color);
      matrix.drawLine(x+1, y+4, x+1, y+4, color);
      matrix.drawPixel(x+2, y+4, color);
      break;
    case 'R':
      matrix.drawLine(x, y, x, y+4, color);
      matrix.drawLine(x, y, x+2, y, color);
      matrix.drawLine(x+2, y, x+2, y+2, color);
      matrix.drawLine(x, y+2, x+2, y+2, color);
      matrix.drawLine(x+1, y+2, x+2, y+4, color);
      break;
    case 'S':
      matrix.drawLine(x, y, x+2, y, color);
      matrix.drawLine(x, y, x, y+2, color);
      matrix.drawLine(x, y+2, x+2, y+2, color);
      matrix.drawLine(x+2, y+2, x+2, y+4, color);
      matrix.drawLine(x, y+4, x+2, y+4, color);
      break;
    case 'T':
      matrix.drawLine(x, y, x+2, y, color);
      matrix.drawLine(x+1, y, x+1, y+4, color);
      break;
    case 'U':
      matrix.drawLine(x, y, x, y+3, color);
      matrix.drawLine(x+2, y, x+2, y+3, color);
      matrix.drawLine(x+1, y+4, x+1, y+4, color);
      break;
    case 'V':
      matrix.drawLine(x, y, x+1, y+4, color);
      matrix.drawLine(x+2, y, x+1, y+4, color);
      break;
    case 'W':
      matrix.drawLine(x, y, x, y+4, color);
      matrix.drawLine(x+2, y, x+2, y+4, color);
      matrix.drawPixel(x+1, y+3, color);
      break;
    case 'X':
      matrix.drawLine(x, y, x+2, y+4, color);
      matrix.drawLine(x+2, y, x, y+4, color);
      break;
    case 'Y':
      matrix.drawLine(x, y, x+1, y+2, color);
      matrix.drawLine(x+2, y, x+1, y+2, color);
      matrix.drawLine(x+1, y+2, x+1, y+4, color);
      break;
    case 'Z':
      matrix.drawLine(x, y, x+2, y, color);
      matrix.drawLine(x+2, y, x, y+4, color);
      matrix.drawLine(x, y+4, x+2, y+4, color);
      break;
    case '0':
      matrix.drawLine(x, y+1, x, y+3, color);
      matrix.drawLine(x+2, y+1, x+2, y+3, color);
      matrix.drawLine(x+1, y, x+1, y, color);
      matrix.drawLine(x+1, y+4, x+1, y+4, color);
      matrix.drawLine(x, y+1, x+2, y+3, color);
      break;
    case '1':
      matrix.drawLine(x+1, y, x+1, y+4, color);
      matrix.drawPixel(x, y+1, color);
      matrix.drawLine(x, y+4, x+2, y+4, color);
      break;
    case '2':
      matrix.drawLine(x, y+1, x, y+1, color);
      matrix.drawLine(x+1, y, x+1, y, color);
      matrix.drawLine(x+2, y+1, x+2, y+1, color);
      matrix.drawLine(x+1, y+2, x+1, y+2, color);
      matrix.drawLine(x, y+3, x, y+3, color);
      matrix.drawLine(x, y+4, x+2, y+4, color);
      break;
    case '3':
      matrix.drawLine(x, y, x+2, y, color);
      matrix.drawLine(x+2, y+1, x+2, y+1, color);
      matrix.drawLine(x+1, y+2, x+1, y+2, color);
      matrix.drawLine(x+2, y+3, x+2, y+3, color);
      matrix.drawLine(x, y+4, x+2, y+4, color);
      break;
    case '4':
      matrix.drawLine(x, y, x, y+2, color);
      matrix.drawLine(x+2, y, x+2, y+4, color);
      matrix.drawLine(x, y+2, x+2, y+2, color);
      break;
    case '5':
      matrix.drawLine(x, y, x+2, y, color);
      matrix.drawLine(x, y+1, x, y+2, color);
      matrix.drawLine(x, y+2, x+2, y+2, color);
      matrix.drawLine(x+2, y+3, x+2, y+3, color);
      matrix.drawLine(x, y+4, x+2, y+4, color);
      break;
    case '6':
      matrix.drawLine(x+1, y, x+1, y, color);
      matrix.drawLine(x, y+1, x, y+3, color);
      matrix.drawLine(x, y+2, x+2, y+2, color);
      matrix.drawLine(x+2, y+3, x+2, y+3, color);
      matrix.drawLine(x, y+4, x+2, y+4, color);
      break;
    case '7':
      matrix.drawLine(x, y, x+2, y, color);
      matrix.drawLine(x+2, y+1, x+2, y+2, color);
      matrix.drawLine(x+1, y+3, x+1, y+4, color);
      break;
    case '8':
      matrix.drawLine(x, y+1, x, y+1, color);
      matrix.drawLine(x+2, y+1, x+2, y+1, color);
      matrix.drawLine(x+1, y, x+1, y, color);
      matrix.drawLine(x+1, y+2, x+1, y+2, color);
      matrix.drawLine(x+1, y+4, x+1, y+4, color);
      matrix.drawLine(x, y+3, x, y+3, color);
      matrix.drawLine(x+2, y+3, x+2, y+3, color);
      break;
    case '9':
      matrix.drawLine(x, y, x+2, y, color);
      matrix.drawLine(x, y+1, x, y+1, color);
      matrix.drawLine(x+2, y+1, x+2, y+3, color);
      matrix.drawLine(x, y+2, x+2, y+2, color);
      matrix.drawLine(x+1, y+4, x+1, y+4, color);
      break;
    case ' ':
      break;
    case '-':
      matrix.drawLine(x, y+2, x+2, y+2, color);
      break;
    case '.':
      matrix.drawPixel(x+1, y+4, color);
      break;
    case ',':
      matrix.drawPixel(x+1, y+4, color);
      matrix.drawPixel(x, y+5, color);
      break;
    case '!':
      matrix.drawLine(x+1, y, x+1, y+3, color);
      matrix.drawPixel(x+1, y+5, color);
      break;
    default:
      matrix.drawRect(x, y, 3, 5, color);
      break;
  }
}