#include <bits/stdc++.h> 
#define pii pair<int, int>
using namespace std;

#include "object.h"
#include "miscsymbols.h"
#include "blockgroup.h"
#include "util.h"
#include "grid.h"
#include "solver.h"
#include "randgrid.h"

// Original library and code base - https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
using namespace std;
 
//MatrixPanel_I2S_DMA matrixpanel;
MatrixPanel_I2S_DMA *matrixpanel = nullptr;

const int n = 32;
const int m = 64;
const int chain = 2;

#define R1_PIN 25
#define G1_PIN 26
#define B1_PIN 27
#define R2_PIN 14
#define G2_PIN 12
#define B2_PIN 13
#define A_PIN 23
#define B_PIN 19
#define C_PIN 5
#define D_PIN 17
#define E_PIN 18 // required for 1/32 scan panels, like 64x64. Any available pin would do, i.e. IO32
#define LAT_PIN 4
#define OE_PIN 15
#define CLK_PIN 16

HUB75_I2S_CFG::i2s_pins _pins={R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN};

SET_LOOP_TASK_STACK_SIZE(64 * 1024); 

// Functions and shit

void render(Solver solver, Color bg, Color line, Color path);

void example();

Solver puzzleSolver = Solver();

RandGrid gridgen;

int dx[4] = {01, 00, -1, 00};
int dy[4] = {00, 01, 00, -1};

// Flash memory

#include <EEPROM.h>
#include "prog3.h"

#define EEPROM_SIZE 1
uint8_t flashbyte = 0;

void eeprom_setup() {
  EEPROM.begin(EEPROM_SIZE);
  flashbyte = EEPROM.read(0);
  EEPROM.write(0, (flashbyte != 0) ? 0 : 127);
  EEPROM.commit();
}

// Random generation

void randTests() {
  gridgen.pathfind();
  int index = gridgen.randint(7);
  Grid grid;
  if (index == 0) grid = (gridgen.randBlobs(9, 3, 2));
  else if (index == 1) grid = (gridgen.randMaze());
  else if (index == 2) grid = (gridgen.randStars());
  else if (index == 3) grid = (gridgen.randBlobs(12, 2, 0));
  else if (index == 4) grid = (gridgen.randTriangles(8, 0));
  else if (index == 5) grid = (gridgen.randBlocks(4, gridgen.randint(4)));
  else if (index == 6) grid = (gridgen.randDots(4 + gridgen.randint(4), gridgen.randint(8)));
  puzzleSolver.set(grid);
  render(puzzleSolver, BLACK, WHITE, GREEN);
  delay(1000);
  puzzleSolver.solve();
  if (puzzleSolver.solution.size() == 0) {
    cout << "NO SOLUTION :(\n";
    matrixpanel->fillScreenRGB888(255, 0, 0);
  }
  else {
    render(puzzleSolver, BLACK, WHITE, GREEN);
  }
}

void randChallenge() {
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

  render(puzzleSolver, BLACK, WHITE, GREEN);
  delay(1000);
  puzzleSolver.solve();
  if (puzzleSolver.solution.size() == 0) {
    cout << "NO SOLUTION :(\n";
    matrixpanel->fillScreenRGB888(255, 0, 0);
  }
  else {
    render(puzzleSolver, BLACK, WHITE, GREEN);
  }
}

void randFull() {
  gridgen.pathfind();
  int index = gridgen.randint(10);
  Grid grid;
  if (index == 0) grid = (gridgen.randMaze()); // Panels 1 + 2
  else if (index == 1) grid = (gridgen.randBlobs(8, 2, 4 + gridgen.randint(4))); // Panel 3
  else if (index == 2) grid = (gridgen.randDots(2, 12 + gridgen.randint(4))); // Panel 4
  else if (index == 3) grid = (gridgen.randChallengeBlocks(4 + gridgen.randint(6))); // Blocks and stars panel
  else if (index == 4) grid = (gridgen.randChallengeStars(4 + gridgen.randint(6))); // Stars and dots
  else if (index == 5) grid = (gridgen.randBlobs(9, 3, 0));
  else if (index == 6) grid = (gridgen.randTriangles(6 + gridgen.randint(4), 0));

  else if (index == 7) grid = (gridgen.randStars());
  else if (index == 8) grid = (gridgen.randBlocks(4, gridgen.randint(4)));
  else if (index == 9) grid = (gridgen.randDots(4 + gridgen.randint(4), gridgen.randint(8)));

  puzzleSolver.set(grid);

  render(puzzleSolver, BLACK, WHITE, GREEN);
  delay(1000);
  puzzleSolver.solve();
  if (puzzleSolver.solution.size() == 0) {
    cout << "NO SOLUTION :(\n";
    matrixpanel->fillScreenRGB888(255, 0, 0);
  }
  else {
    render(puzzleSolver, BLACK, WHITE, GREEN);
  }
}

void miniChallenge() {
  vector<Grid> grids;
  grids.push_back(gridgen.randMaze());
  grids.push_back(gridgen.randBlobs(8, 2, 4 + gridgen.randint(4)));
  grids.push_back(gridgen.randDots(2, 12 + gridgen.randint(4)));
  grids.push_back(gridgen.randChallengeBlocks(4 + gridgen.randint(6)));
  grids.push_back(gridgen.randChallengeStars(4 + gridgen.randint(6)));
  grids.push_back(gridgen.randBlobs(9, 3, 0));
  grids.push_back(gridgen.randTriangles(6 + gridgen.randint(4), 0));

  for (auto grid : grids) {
    puzzleSolver.set(grid);
    render(puzzleSolver, BLACK, WHITE, GREEN);
    delay(1000);
    puzzleSolver.solve();
    if (puzzleSolver.solution.size() == 0) {
      cout << "NO SOLUTION :(\n";
      matrixpanel->fillScreenRGB888(255, 0, 0);
    }
    else {
      render(puzzleSolver, BLACK, WHITE, GREEN);
    }
    delay(2000);
  }
}

// Setup

const int MODE = 2; // 0 FULL 1 TEST 2 CHALLENGE

void setup() {
  eeprom_setup();
  if (flashbyte != 0) {
    prog3_setup();
    return;
  }
  Serial.begin(9600);
  Serial.print("INITIALIZING...\n");
  Serial.printf("Arduino Stack was set to %d bytes\n", getArduinoLoopTaskStackSize());

  // put your setup code here, to run once:
  HUB75_I2S_CFG mxconfig(m, n, chain, _pins);

  mxconfig.gpio.e = 18;
  mxconfig.clkphase = false;
  mxconfig.driver = HUB75_I2S_CFG::FM6126A;

  // Display Setup
  matrixpanel = new MatrixPanel_I2S_DMA(mxconfig);
  matrixpanel->setLatBlanking(2);
  matrixpanel->begin();
  matrixpanel->clearScreen();
  matrixpanel->setBrightness8(64);

  matrixpanel->fillScreenRGB888(255, 255, 255);
  delay(100);
  matrixpanel->drawRect(0, 0, 10, 10, matrixpanel->color565(0, 0, 0));
  delay(100);

  Serial.printf("Total heap: %d", ESP.getHeapSize());
  Serial.printf("Free heap: %d", ESP.getFreeHeap());
  Serial.printf("Total PSRAM: %d", ESP.getPsramSize());
  Serial.printf("Free PSRAM: %d", ESP.getFreePsram());
  Serial.printf("\n");

  gridgen.pathfind();

  // example();
}

void loop() {
  if (flashbyte != 0) {
    prog3_loop();
    return;
  }
  // put your main code here, to run repeatedly:
  if (MODE == 0) randFull();
  else if (MODE == 1) randTests();
  else if (MODE == 2) randChallenge();
  else randTests(); // Default

  Serial.printf("Total heap: %d", ESP.getHeapSize());
  Serial.printf("Free heap: %d", ESP.getFreeHeap());
  Serial.printf("Total PSRAM: %d", ESP.getPsramSize());
  Serial.printf("Free PSRAM: %d", ESP.getFreePsram());
  Serial.printf("\n");
  delay(2000);
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

void render(Solver solver, Color bg, Color line, Color path) {
  solver.activate();
    
  if (solver.grid.n > 9 || solver.grid.m > 9) {
    matrixpanel->fillScreenRGB888((bg>>16) % (1<<16), (bg>>8) % (1<<8), bg % (1<<8));
    return;
  }
  cout << "RENDERING!!!!" << endl;

  const int rad = 3;

  int height = (solver.grid.n)>>1;
  int width = (solver.grid.m)>>1;

  cout << width << " " << height << endl;

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
  solver.deactivate();
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