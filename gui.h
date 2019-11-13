#pragma once
#include <GL/glut.h>
#include <GL/glu.h>
#include <array>
#include <iostream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <thread>
#include <mutex>
#include <condition_variable>
using namespace std;
#include "logging.h"
#include <unistd.h>

const int WINDOW_WIDTH = 320, WINDOW_HEIGHT = 200;
const int WINDOW_SIZE = WINDOW_WIDTH * WINDOW_HEIGHT;
const int MAX_FPS = 60;
const int UPDATE_GRAPH_INTERVAL = int(1000.0 / MAX_FPS);
array<uint8_t, WINDOW_WIDTH * WINDOW_HEIGHT * 3> VIDEO_BUFFER;
uint16_t KEY_BOARD_MAP[256];

class GUI;
GUI *GUI_P = nullptr;

void gui_main_func(GUI*);

class GUI {
public:
  GUI() : gui_thread(gui_main_func, this) {
    memset(KEY_BOARD_MAP, 0, sizeof(KEY_BOARD_MAP));
    KEY_BOARD_MAP[' '] = 0x3920;
    KEY_BOARD_MAP['w'] = 0x4800;
    KEY_BOARD_MAP['s'] = 0x5000;
    KEY_BOARD_MAP['a'] = 0x4b00;
    KEY_BOARD_MAP['d'] = 0x4d00;
    cur_key = 0;
  }
  void set_video_addr(void *addr) {
    video_addr = reinterpret_cast<uint8_t*>(addr);
  }
  uint8_t* get_video_addr() {
    return video_addr;
  }
  void set_palette_index(int index) {
    CHECK_GE(index, 0);
    CHECK_LT(index, palette.size());
    palette_index = index;
    palette_sub_index = 0;
  }
  void set_palette_value(uint8_t value) {
    palette[palette_index][palette_sub_index++] = value;
  }
  uint16_t get_key() {
    unique_lock<mutex> lock(key_mutex);
    key_cv.wait(lock, [&](){return cur_key != 0;});
    uint16_t key = cur_key;
    cur_key = 0;
    return key;
  }
  void set_key(uint16_t key) {
    cur_key = key;
    key_cv.notify_one();
  }
  bool check_key() {
    return cur_key != 0;
  }
public:
  array<array<uint8_t, 3>, 256> palette;
private:
  int palette_index = 0;
  int palette_sub_index = 0;
  uint8_t *video_addr = nullptr;
  thread gui_thread;
  uint16_t cur_key;
  mutex key_mutex;
  condition_variable key_cv;
};

void Display() {
	glClear(GL_COLOR_BUFFER_BIT);

  uint8_t *video_addr = GUI_P->get_video_addr();
  if (video_addr) {
    _Pragma("omp parallel for")
    for (int i = 0; i < WINDOW_SIZE; ++i) {
      const int y = WINDOW_HEIGHT - 1 - (i / WINDOW_WIDTH);
      const int x = i % WINDOW_WIDTH;
      const int k = y * WINDOW_WIDTH + x;
      VIDEO_BUFFER[k*3+0] = GUI_P->palette[video_addr[i]][0] << 2;
      VIDEO_BUFFER[k*3+1] = GUI_P->palette[video_addr[i]][1] << 2;
      VIDEO_BUFFER[k*3+2] = GUI_P->palette[video_addr[i]][2] << 2;
    }
  }
  glDrawPixels(WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, VIDEO_BUFFER.data());
	glutSwapBuffers();
}

void Idle(){
	usleep(UPDATE_GRAPH_INTERVAL);
	Display();
}

void Reshape(int w, int h){
	glViewport (0, 0, (GLsizei) WINDOW_WIDTH, (GLsizei) WINDOW_HEIGHT);   
}

void Keyboard(unsigned char key, int x,int y){
  uint16_t code = KEY_BOARD_MAP[key];
  if (code) {
    GUI_P->set_key(code);
  }
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
	glutIdleFunc(&Idle);
  glutMainLoop();
}

