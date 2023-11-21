#include <vector>
#include <set>
#define pii pair<int, int>
using namespace std;

#include "object.h"
#include "miscsymbols.h"
#include "blockgroup.h"
#include "util.h"
#include "grid.h"
#include "solver.h"
#include "randgrid.h"
#include "witnessgame.h"

// Original library and code base - https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
using namespace std;
 
#include "matrices.h" // Standard matrix template

SET_LOOP_TASK_STACK_SIZE(64 * 1024); 

// Functions and shit

void render(Solver solver, Color bg, Color line, Color path);

void example();

Solver puzzleSolver = Solver();
WitnessGame game;

bool gameActive = false;
const bool DEBUG = false;

RandGrid gridgen;

int dx[4] = {01, 00, -1, 00};
int dy[4] = {00, 01, 00, -1};

// Flash memory

#include <EEPROM.h>

#define EEPROM_SIZE 1
uint8_t flashbyte = 0;

void eeprom_setup() {
  EEPROM.begin(EEPROM_SIZE);
  flashbyte = EEPROM.read(0);
  EEPROM.write(0, (flashbyte != 0) ? 0 : 127);
  EEPROM.commit();
}

// ANIMATION (Occurs every other bootup)

#include "panels.h"

int blinkdelay = 0;
int blinkthreshold = 0;

vector<vector<vector<bool>>> animation{grid0, grid1, grid2, grid3, grid4, grid5, grid6, grid7, grid8};

void anim_loop() {
  uint16_t color = protogenfullhue();
  color = matrixpanel->color565(255, 255, 255);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < m * chain; j++) col[i][j] = color;
  }
  delay(100);
  // Serial.println(rand() % 1024);
  disp(grid0);

  blinkdelay++;
  if (blinkdelay >= blinkthreshold) {
    blinkdelay = 0;
    blinkthreshold = (int)(runif(16, 64));
    playAnimation(animation, 20, false);
    playAnimation(animation, 20, true);
    dispstats();
    Serial.printf("BLINK THRESHOLD %ld\n", blinkthreshold);
  }
}

// WIRELESS

#include <esp_now.h>
#include <WiFi.h>

uint8_t direction = 0;
uint8_t prevdir = 0;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  Serial.print("Bytes received: ");
  Serial.println(*incomingData);
  direction = *incomingData;
}

void WirelessSetup() {
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
}

// THE WITNESS (Occurs every other bootup)

// Random generation

void randChallenge() {
  if (!gameActive) {
  gridgen.pathfind();
  int index = gridgen.randint(7);
  Grid grid;
  if (index == 0) grid = (gridgen.randMaze()); // Panels 1 + 2
  else if (index == 1) grid = (gridgen.randBlobs(8, 2, 4 + gridgen.randint(4))); // Panel 3
  else if (index == 2) grid = (gridgen.randDots(2, 12 + gridgen.randint(4))); // Panel 4
  else if (index == 3) grid = (gridgen.randChallengeBlocks(4 + gridgen.randint(6))); // Blocks and stars panel
  else if (index == 4) grid = (gridgen.randChallengeStars(4 + gridgen.randint(6))); // Stars and dots
  else if (index == 5) grid = (gridgen.randBlobs(9, 3, 0));
  else if (index == 6) grid = (gridgen.randTriangles(6 + gridgen.randint(4), 0));
  puzzleSolver.set(grid);
  game.reset(grid);

  render(game, BLACK, WHITE, GREEN);

  Serial.printf("Total heap: %d", ESP.getHeapSize());
  Serial.printf("Free heap: %d", ESP.getFreeHeap());
  Serial.printf("Total PSRAM: %d", ESP.getPsramSize());
  Serial.printf("Free PSRAM: %d", ESP.getFreePsram());
  Serial.printf("\n");

  gameActive = true;
  return;
  }
}

// Setup

const int MODE = 2; // 0 FULL 1 TEST 2 CHALLENGE

void setup() {
  matrixsetup();
  eeprom_setup();
  WirelessSetup();
  gridgen.pathfind();

  prevdir = 0;
  direction = 0;

  // example();
}

void loop() {
  if (flashbyte != 0) {
    anim_loop();
    return;
  }
  // put your main code here, to run repeatedly:
  randChallenge();
  /*
  Serial.print(direction);
  Serial.print(" ");
  Serial.println(prevdir);
  */

  if ((prevdir == 0) && (direction != 0)) {

        cout << "DIRECTION " << direction << endl;
        game.processInput(direction);
        prevdir = direction;
        direction = 0;
        

        render(game, BLACK, WHITE, GREEN);
        
        if (game.reachedEnd()) {
            if (game.grid.ver(game.origin.first, game.origin.second)) {
                cout << "YOU WIN!!!!\n";
                game.grid.disp();
                render(game, BLACK, WHITE, CYAN);
                delay(1500);
                gameActive = false;
                return;
            }
            else {
                cout << "YOU LOSE!!!!\n";
                game.grid.disp();
                render(game, BLACK, WHITE, RED);
                game.clear();
                delay(1500);
                gameActive = false;
                return;
            }
        }

        render(game, BLACK, WHITE, GREEN);
        direction = 0;

  if (DEBUG) {

  Serial.printf("Total heap: %d", ESP.getHeapSize());
  Serial.printf("Free heap: %d", ESP.getFreeHeap());
  Serial.printf("Total PSRAM: %d", ESP.getPsramSize());
  Serial.printf("Free PSRAM: %d", ESP.getFreePsram());
  Serial.printf("\n");
  }

  direction = 0;

  }
  else if (prevdir != 0 && direction != 0) direction = 0;
  prevdir = direction;
}



uint16_t hex565(int x) {
  // cout << x << " = [" << (x>>16) % (1<<8) << " " << (x>>8) % (1<<8) << " " << x % (1<<8) << "]\n";
  return matrixpanel->color565((x>>16) % (1<<8), (x>>8) % (1<<8), x % (1<<8));
}

uint64_t scale565(int x, double y) {
  double r = (x>>16) % (1<<8);
  double g = (x>>8) % (1<<8);
  double b = (x % (1<<8));

  r *= y;
  g *= y;
  b *= y;
  return matrixpanel->color565((int)r, (int)g, (int)b);
}

uint64_t inv565(int x) {
  double r = 255 - (x>>16) % (1<<8);
  double g = 255 - (x>>8) % (1<<8);
  double b = 255 - (x % (1<<8));
  return matrixpanel->color565((int)r, (int)g, (int)b);
}

// NOTE TO SELF - drawRect is (top, left, width, height) while drawLine is endpoints.

double fade_scale = 1.0 / 3.0;

void render(WitnessGame solver, Color bg, Color line, Color path) {
    
  if (solver.grid.n > 9 || solver.grid.m > 9) {
    matrixpanel->fillScreenRGB888((bg>>16) % (1<<16), (bg>>8) % (1<<8), bg % (1<<8));
    return;
  }
  // cout << "RENDERING!!!!" << endl;

  const int rad = 3;

  int height = (solver.grid.n)>>1;
  int width = (solver.grid.m)>>1;

  // cout << width << " " << height << endl;

  int top = 15 - rad * height;
  int left = 20 - rad * width;

  matrixpanel->fillScreenRGB888((bg>>16) % (1<<16), (bg>>8) % (1<<8), bg % (1<<8));

  for (int yayayaya = 0; yayayaya < 2; yayayaya++) {
  // cout << top << " " << left << endl;
  // matrixpanel->fillRect(left, top, (rad<<1) * width + 2, (rad<<1) * height + 2, hex565(line));
  for (int i = 0; i <= height; i++) {
    matrixpanel->drawLine(left, top + (rad<<1) * i, left + (rad<<1) * width, top + (rad<<1) * i, hex565(line));
  }

  for (int i = 0; i <= width; i++) {
    matrixpanel->drawLine(left + (rad<<1) * i, top, left + (rad<<1) * i, top + (rad<<1) * height, hex565(line));
  }

  for (int i = 0; i < solver.grid.n; i++) { // y coordinate aka. vertical
    for (int j = 0; j < solver.grid.m; j++) { // x coordinate aka. horizontal
      int xcoord = left + rad * j;
      int ycoord = top + rad * i;
      shared_ptr<Object> o = solver.grid.board[i][j];
      bool mov = o->isPath;
      bool sol = o->isPathOccupied;
      if (!mov) {
        if (j % 2 == 1) matrixpanel->drawLine(xcoord - 2, ycoord, xcoord + 2, ycoord, hex565(bg));
        else if (i % 2 == 1) matrixpanel->drawLine(xcoord, ycoord - 2, xcoord, ycoord + 2, hex565(bg));
      }

      if (sol) {
        matrixpanel->drawPixel(xcoord, ycoord, hex565(path));
        if (j % 2 == 1) matrixpanel->drawLine(xcoord - 2, ycoord, xcoord + 2, ycoord, hex565(path));
        else if (i % 2 == 1) matrixpanel->drawLine(xcoord, ycoord - 2, xcoord, ycoord + 2, hex565(path));
      }
    }
  }

  // 2nd round

  for (int i = 0; i < solver.grid.n; i++) {
    for (int j = 0; j < solver.grid.m; j++) {
      int xcoord = left + rad * j;
      int ycoord = top + rad * i;
      shared_ptr<Object> o = solver.grid.board[i][j];
      bool mov = o->isPath;
      bool sol = o->isPathOccupied;
      if (isStartingPoint(o)) matrixpanel->drawRect(xcoord - 1, ycoord - 1, 3, 3, sol ? hex565(path) : hex565(line));
      if (isEndingPoint(o)) {
        uint16_t color = sol ? hex565(path) : hex565(line);
        if (i == 0 && j == 0) matrixpanel->drawLine(xcoord - 2, ycoord - 2, xcoord, ycoord, color);
        else if (i == solver.grid.n - 1 && j == 0) matrixpanel->drawLine(xcoord - 2, ycoord + 2, xcoord, ycoord, color);
        else if (i == 0 && j == solver.grid.n - 1) matrixpanel->drawLine(xcoord + 2, ycoord - 2, xcoord, ycoord, color);
        else if (i == solver.grid.n - 1 && j == solver.grid.m - 1) matrixpanel->drawLine(xcoord + 2, ycoord + 2, xcoord, ycoord, color);
        
        else if (i == 0) matrixpanel->drawLine(xcoord, ycoord - 2, xcoord, ycoord, color);
        else if (i == solver.grid.n - 1) matrixpanel->drawLine(xcoord, ycoord + 2, xcoord, ycoord, color);
        else if (j == 0) matrixpanel->drawLine(xcoord - 2, ycoord, xcoord, ycoord, color);
        else if (j == solver.grid.m - 1) matrixpanel->drawLine(xcoord + 2, ycoord, xcoord, ycoord, color);
        
        else matrixpanel->drawRect(xcoord - 2, ycoord - 2, 5, 5, sol ? hex565(path) : hex565(line));
      }
    }
  }

  // 3rd round

  for (int i = 0; i < solver.grid.n; i++) {
    for (int j = 0; j < solver.grid.m; j++) {
      int xcoord = left + rad * j;
      int ycoord = top + rad * i;
      shared_ptr<Object> o = solver.grid.board[i][j];
      bool mov = o->isPath;
      bool sol = o->isPathOccupied;

      if (instanceof<Dot>(o)) matrixpanel->drawPixel(xcoord, ycoord, hex565(RED));
      if (instanceof<Blob>(o)) matrixpanel->fillRect(xcoord - 1, ycoord - 1, 3, 3, hex565(o->color));
      if (instanceof<Star>(o)) {
        matrixpanel->drawLine(xcoord, ycoord - 1, xcoord, ycoord + 1, hex565(o->color));
        matrixpanel->drawLine(xcoord + 1, ycoord, xcoord - 1, ycoord, hex565(o->color));
      }
      if (instanceof<Cancel>(o)) {
        matrixpanel->drawPixel(xcoord, ycoord, hex565(WHITE));
        matrixpanel->drawPixel(xcoord, ycoord - 1, hex565(WHITE));
        matrixpanel->drawPixel(xcoord - 1, ycoord + 1, hex565(WHITE));
        matrixpanel->drawPixel(xcoord + 1, ycoord + 1, hex565(WHITE));
      }

      if (instanceof<Triangle>(o)) {
        // Draw 4 dots to differentiate between what's not.
        for (int d = 0; d < 4; d++) matrixpanel->drawPixel(xcoord + 2 * dx[d], ycoord + 2 * dy[d], scale565(o->color, fade_scale));

        int num = (dynamic_pointer_cast<Triangle>(o))->x;
        if (num == 1) matrixpanel->drawPixel(xcoord, ycoord, hex565(o->color));
        else if (num == 2) {
          matrixpanel->drawPixel(xcoord - 1, ycoord, hex565(o->color));
          matrixpanel->drawPixel(xcoord + 1, ycoord, hex565(o->color));
        }
        else if (num == 3) {
          matrixpanel->drawPixel(xcoord - 1, ycoord + 1, hex565(o->color));
          matrixpanel->drawPixel(xcoord + 1, ycoord + 1, hex565(o->color));
          matrixpanel->drawPixel(xcoord, ycoord - 1, hex565(o->color));
        }
      }

      if (instanceof<BlockGroup>(o)) {  
        if (dynamic_pointer_cast<BlockGroup>(o)->n <= 0) continue;
        BlockGroup group = (dynamic_pointer_cast<BlockGroup>(o))->clone();
        group.normalize();
        set<pair<int, int>> blocks = group.pairs;
        bool orientation = group.oriented;
        pair<int, int> sides = group.boundingbox;

        matrixpanel->drawRect(xcoord - 2, ycoord - 2, 5, 5, scale565(o->color, fade_scale)); // draw a rect to differentiate

        if (!orientation) {
          for (int d = 0; d < 4; d++) matrixpanel->drawPixel(xcoord + 2 * dx[d], ycoord + 2 * dy[d], hex565(bg));
        }
        for (auto i : blocks) matrixpanel->drawPixel(xcoord - ((sides.first)>>1) + i.first, ycoord + ((sides.second)>>1) - i.second, hex565(o->color));
      }
    }
  }

    left += (127 - (left<<1) - (rad<<1) * width);
  }
}

void example() {
  vector<vector<shared_ptr<Object>>> v = vector<vector<shared_ptr<Object>>>(9, vector<shared_ptr<Object>>(9));
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
          v[i][j] = shared_ptr<Object>(new Object());
          if (i % 2 == 0 || j % 2 == 0) v[i][j]->isPath = true;
        }
    }
    
    v[8][0] = shared_ptr<Endpoint>(new Endpoint(true));
    v[0][8] = shared_ptr<Endpoint>(new Endpoint(false));
    v[1][1] = shared_ptr<Star>(new Star(RED));
    v[3][7] = shared_ptr<Star>(new Star(RED));
    v[5][5] = shared_ptr<BlockGroup>(new BlockGroup(0, 0, vector<pii>({{0, 0}, {0, 1}, {-1, 1}})));
    v[5][7] = shared_ptr<BlockGroup>(new BlockGroup(1, 0, vector<pii>({{0, 0}, {1, 0}, {2, 0}})));
    
    v[0][7]->isPath = false;
    v[1][6]->isPath = false;
    v[3][2]->isPath = false;
    v[3][4]->isPath = false;
    v[5][0]->isPath = false;
    v[6][5]->isPath = false;
    v[8][1]->isPath = false;
    v[8][7]->isPath = false;
    

    Grid grid2 = Grid(v);
    
    Solver sx = Solver(grid2);
    delay(1000);
    cout << "\n\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << (long long)(&(sx.grid)) << "\n";
    render(sx, BLACK, WHITE, GREEN);
    cout << "XXXXXXXXXXXXXXXXXXXXXXXXXXXX\n";
    sx.solve();
    cout << "############################\n";
    sx.disp();

    render(sx, BLACK, WHITE, GREEN);
}