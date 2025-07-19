#pragma once
#define GL_SILENCE_DEPRECATION

#if defined(__APPLE__)
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include <array>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
using namespace std;
#include <unistd.h>

#include "logging.h"

const int RATIO = 4;
const int VIDEO_WIDTH = 320, VIDEO_HEIGHT = 200;
int WINDOW_WIDTH = VIDEO_WIDTH * RATIO;
int WINDOW_HEIGHT = VIDEO_HEIGHT * RATIO;
const int MAX_FPS = 60;
const int UPDATE_GRAPH_INTERVAL = int(1000.0 / MAX_FPS);
struct RGB {
  uint8_t rgb[3];
};
RGB VIDEO_BUFFER[VIDEO_HEIGHT][VIDEO_WIDTH];
uint16_t KEY_BOARD_MAP[256];

class GUI;
GUI *GUI_P = nullptr;

void gui_main_func(GUI *);

class GUI {
 public:
  GUI() {
    memset(KEY_BOARD_MAP, 0, sizeof(KEY_BOARD_MAP));
    KEY_BOARD_MAP[' '] = 0x3920;
    KEY_BOARD_MAP['w'] = 0x4800;
    KEY_BOARD_MAP['s'] = 0x5000;
    KEY_BOARD_MAP['a'] = 0x4b00;
    KEY_BOARD_MAP['d'] = 0x4d00;
    cur_key = 0;
  }
  void set_video_addr(void *addr) {
    video_addr = reinterpret_cast<uint8_t *>(addr);
  }
  uint8_t *get_video_addr() { return video_addr; }
  void set_palette_index(int index) {
    CHECK_GE(index, 0);
    CHECK_LT(index, 256);
    palette_index = index;
    palette_sub_index = 0;
  }
  void set_palette_value(uint8_t value) {
    palette[palette_index].rgb[palette_sub_index++] = value << 2;
  }
  uint16_t get_key() {
    unique_lock<mutex> lock(key_mutex);
    key_cv.wait(lock, [&]() { return cur_key != 0; });
    uint16_t key = cur_key;
    cur_key = 0;
    return key;
  }
  void set_key(uint16_t key) {
    cur_key = key;
    key_cv.notify_one();
  }
  bool check_key() { return cur_key != 0; }

 public:
  RGB palette[256];

 private:
  int palette_index = 0;
  int palette_sub_index = 0;
  uint8_t *video_addr = nullptr;
  uint16_t cur_key;
  mutex key_mutex;
  condition_variable key_cv;
};

void Display() {
  glClear(GL_COLOR_BUFFER_BIT);

  uint8_t *video_addr = GUI_P->get_video_addr();
  if (video_addr) {
    int i = 0;
    for (int y = VIDEO_HEIGHT - 1; y >= 0; --y) {
      for (int x = 0; x < VIDEO_WIDTH; ++x) {
        VIDEO_BUFFER[y][x] = GUI_P->palette[video_addr[i++]];
      }
    }
  }
  glDrawPixels(VIDEO_WIDTH, VIDEO_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE,
               (uint8_t *)VIDEO_BUFFER);
  float ratio = min(WINDOW_WIDTH / (float)VIDEO_WIDTH,
                    WINDOW_HEIGHT / (float)VIDEO_HEIGHT);
  glPixelZoom(ratio, ratio);
  glutSwapBuffers();
}

void Idle() {
  usleep(UPDATE_GRAPH_INTERVAL);
  Display();
}

void Reshape(int w, int h) {
  WINDOW_WIDTH = w;
  WINDOW_HEIGHT = h;
}

void Keyboard(unsigned char key, int x, int y) {
  if (key == 27) {
    exit(0);  // Exit on ESC key
  }
  uint16_t code = KEY_BOARD_MAP[key];
  if (code) {
    GUI_P->set_key(code);
  }
}

void SpecialKeys(int key, int x, int y) {
  uint16_t code = 0;
  switch (key) {
    case GLUT_KEY_UP:
      code = 0x4800;  // Up arrow
      break;
    case GLUT_KEY_DOWN:
      code = 0x5000;  // Down arrow
      break;
    case GLUT_KEY_LEFT:
      code = 0x4b00;  // Left arrow
      break;
    case GLUT_KEY_RIGHT:
      code = 0x4d00;  // Right arrow
      break;
    default:
      return;  // Ignore other special keys
  }
  GUI_P->set_key(code);
}

void gui_main_func(GUI *gui) {
  CHECK(GUI_P == nullptr);
  GUI_P = gui;
  int argc = 0;
  char **argv = nullptr;
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
  glutInitWindowPosition(0, 0);
  glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
  int globalWindow = glutCreateWindow("Happy QQT");
  glutDisplayFunc(&Display);
  glutReshapeFunc(Reshape);
  glutKeyboardFunc(&Keyboard);
  glutSpecialFunc(SpecialKeys);
  glutIdleFunc(&Idle);
  glutMainLoop();
}
