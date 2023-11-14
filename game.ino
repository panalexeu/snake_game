#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// BUTTONS INPUT PINS 
#define LEFT 2
#define UP 3
#define RIGHT 4
#define DOWN 5

// DISPLAY PARAMETERS 
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// SNAKE CONSTANTS, VARIABLES AND BITMAPS
#define TILE_SIZE 8 

typedef struct {
  int x;
  int y;
  char dir_x; 
  char dir_y;
  unsigned char tail; 
  char tail_x[105];
  char tail_y[105];
} SNAKE; 

typedef struct {
  int x;
  int y;
} FRUIT;

SNAKE snake; 
FRUIT fruit;
char game_over;
unsigned char score;

static const unsigned char PROGMEM BRICK[8] = {
  0b11011101,
  0b00000000,
  0b10111011,
  0b00000000,
  0b11011101,
  0b00000000,
  0b10111011,
  0b00000000,
};

static const unsigned char PROGMEM FRUIT_MAP[8] = {
  0b00001000,
  0b00010000,
  0b00111100,
  0b01110010,
  0b01111010,
  0b01111110,
  0b00111100,
  0b00000000,
};

static const unsigned char PROGMEM SNAKE_HEAD_UP[8] = {
  0b00000000,
  0b00111100,
  0b01111110,
  0b01011010,
  0b01111110,
  0b01111110,
  0b01111110,
  0b00000000
};

static const unsigned char PROGMEM SNAKE_HEAD_DOWN[8] = {
  0b00000000,
  0b01111110,
  0b01111110,
  0b01111110,
  0b01011010,
  0b01111110,
  0b00111100,
  0b00000000
};

static const unsigned char PROGMEM SNAKE_HEAD_LEFT[8] = {
  0b00000000,
  0b00111110,
  0b01101110,
  0b01111110,
  0b01111110,
  0b01101110,
  0b00111110,
  0b00000000
};

static const unsigned char PROGMEM SNAKE_HEAD_RIGHT[8] = {
  0b00000000,
  0b01111100,
  0b01110110,
  0b01111110,
  0b01111110,
  0b01110110,
  0b01111100,
  0b00000000
};

static const unsigned char PROGMEM SNAKE_BODY[8] = {
  0b00000000,
  0b01111110,
  0b01111110,
  0b01111110,
  0b01111110,
  0b01111110,
  0b01111110,
  0b00000000
};

static const unsigned char PROGMEM SNAKE_TAIL[8] = {
  0b00000000,
  0b00000000,
  0b00111100,
  0b00111100,
  0b00111100,
  0b00111100,
  0b00000000,
  0b00000000
};

// MAIN FUNCTIONS 
void setup() {
  // debug init
  Serial.begin(9600);

  // buttons pins init 
  pinMode(LEFT, INPUT);
  pinMode(UP, INPUT);
  pinMode(RIGHT, INPUT);
  pinMode(DOWN, INPUT);

  // display init 
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();

  // game setups  
  snake_setup();
}

void loop() {
  display.clearDisplay();
  
  if (!game_over) {
    snake_input();
    snake_logic();
    snake_draw();
  } else {
    game_over_screen();
    game_over = 0; 
    snake_setup(); // game restarts
  }
  
  // game time update in ms 
  delay(75);

  display.display();
}

// SNAKE GAME 
void snake_setup() {
  game_over = 0;
  score = 0;
  
  snake.x = SCREEN_WIDTH / TILE_SIZE / 2 - 1;
  snake.y = SCREEN_HEIGHT / TILE_SIZE / 2 - 1;
  snake.dir_x = 0;
  snake.dir_y = 0;
  snake.tail = 0;

  fruit.x = random(1, SCREEN_WIDTH / TILE_SIZE - 1);
  fruit.y = random(1, SCREEN_HEIGHT / TILE_SIZE - 1);
}

void snake_input() {
  if (snake.x != 0 && snake.x != 15 && snake.y != 0 && snake.y != 7) { // if we are not outside the map
    if (digitalRead(LEFT) == LOW && (snake.dir_x == 0 || snake.tail == 0)) { // button pressed and horizontal direction wasn't pressed or snake tail == 0
      snake.dir_x = -1;
      snake.dir_y = 0;
    } else if (digitalRead(UP) == LOW && (snake.dir_y == 0 || snake.tail == 0)) { // button pressed and vertical direction wasn't pressed or snake tail == 0
      snake.dir_y = -1;
      snake.dir_x = 0;
    } else if (digitalRead(RIGHT) == LOW && (snake.dir_x == 0 || snake.tail == 0)) {
      snake.dir_x = 1;
      snake.dir_y = 0;
    } else if (digitalRead(DOWN) == LOW && (snake.dir_y == 0 || snake.tail == 0)) {
      snake.dir_y = 1;
      snake.dir_x = 0;
    }
  }
}

int map_x, map_y;
void snake_draw() {
  for (int i = 0; i < SCREEN_WIDTH / TILE_SIZE; i++) {
    for (int j = 0; j < SCREEN_HEIGHT / TILE_SIZE; j++) {
      map_x = i * TILE_SIZE;
      map_y = j * TILE_SIZE; 

      // brick drawing 
      if ((i == 0 || i == 15) || (j == 0 || j == 7)) display.drawBitmap(map_x, map_y, BRICK, TILE_SIZE, TILE_SIZE, 1);

      // snake drawing
      // head rotation handling
      if (i == snake.x && j == snake.y) {
        if (snake.dir_y == -1 || (snake.dir_y == 0 && snake.dir_x == 0)) display.drawBitmap(map_x, map_y, SNAKE_HEAD_UP, TILE_SIZE, TILE_SIZE, 1);
        if (snake.dir_x == -1) display.drawBitmap(map_x, map_y, SNAKE_HEAD_LEFT, TILE_SIZE, TILE_SIZE, 1);
        if (snake.dir_x == 1) display.drawBitmap(map_x, map_y, SNAKE_HEAD_RIGHT, TILE_SIZE, TILE_SIZE, 1);
        if (snake.dir_y == 1) display.drawBitmap(map_x, map_y, SNAKE_HEAD_DOWN, TILE_SIZE, TILE_SIZE, 1);
      }

      // snake tail body handling
      for (int k = 0; k < snake.tail; k++) {
        if (i == snake.tail_x[k] && j == snake.tail_y[k]) {
          if (k != snake.tail - 1) {
            display.drawBitmap(map_x, map_y, SNAKE_BODY, TILE_SIZE, TILE_SIZE, 1);
          } else {
            display.drawBitmap(map_x, map_y, SNAKE_TAIL, TILE_SIZE, TILE_SIZE, 1);
          }
        }
      }

      // fruit drawing 
      if (i == fruit.x && j == fruit.y) display.drawBitmap(map_x, map_y, FRUIT_MAP, TILE_SIZE, TILE_SIZE, 1);
    }
  }
}

int prev_tail_x, prev_tail_y, prev_prev_tail_x, prev_prev_tail_y;
void snake_logic() {
  // fruit collision
  if (snake.x == fruit.x && snake.y == fruit.y) {
    score++;
    snake.tail++;  

    // setting coordinates to newly added tail fragments outside of the game window 
    snake.tail_x[snake.tail] = -1;
    snake.tail_y[snake.tail] = -1;

    fruit.x = random(1, SCREEN_WIDTH / TILE_SIZE - 1);
    fruit.y = random(1, SCREEN_HEIGHT / TILE_SIZE - 1);
  }

  // tail handling 
  prev_tail_x = snake.x;
  prev_tail_y = snake.y;

  for (int i = 0; i < snake.tail; i++) {
    // snake collided with itself 
    if (snake.x == snake.tail_x[i] && snake.y == snake.tail_y[i]) game_over = 1; 

    prev_prev_tail_x = snake.tail_x[i];
    prev_prev_tail_y= snake.tail_y[i];

    snake.tail_x[i] = prev_tail_x;
    snake.tail_y[i] = prev_tail_y;

    prev_tail_x = prev_prev_tail_x;
    prev_tail_y = prev_prev_tail_y;
  }
  
  // snake movement
  snake.x += snake.dir_x; 
  snake.y += snake.dir_y;

  // handling snake borders exiting 
  if (snake.x == 0) {
    snake.x = 14;
    snake.dir_x = -1;
  } else if (snake.x == 15) {
    snake.x = 1;
    snake.dir_x = 1;
  } else if (snake.y == 0) {
    snake.y = 7;
    snake.dir_y = -1;
  } else if (snake.y == 7) {
    snake.y = 1;
    snake.dir_y = 1;
  }
}

// GAME OVER 
void game_over_screen() {
  delay(2500);

  display.setCursor(40, 16);
  display.setTextColor(SSD1306_WHITE);
  display.println("GAME OVER\n");
  display.print("SCORE:");
  display.print(score);
  display.display();
  
  delay(2500);
}