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
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
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

  // Profiling data (written by machine thread, read by GUI thread)
  std::atomic<uint64_t> profile_ips{0};           // Instructions per second
  std::atomic<uint64_t> profile_inst_count{0};    // Total instructions executed
  std::atomic<double>   profile_frame_ms{0.0};    // Time per INT 0x08 frame (ms)
  std::atomic<int>      profile_int08_fps{0};     // INT 0x08 calls per second

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

void DrawText(float x, float y, const char *text) {
  glRasterPos2f(x, y);
  for (const char *c = text; *c != '\0'; ++c) {
    glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *c);
  }
}

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

  // Compute aspect-ratio-preserving viewport
  float scale_x = WINDOW_WIDTH / (float)VIDEO_WIDTH;
  float scale_y = WINDOW_HEIGHT / (float)VIDEO_HEIGHT;
  float scale = min(scale_x, scale_y);
  int vp_w = (int)(VIDEO_WIDTH * scale);
  int vp_h = (int)(VIDEO_HEIGHT * scale);
  int vp_x = (WINDOW_WIDTH - vp_w) / 2;
  int vp_y = (WINDOW_HEIGHT - vp_h) / 2;

  // Draw game image scaled to fit window with correct aspect ratio
  glViewport(vp_x, vp_y, vp_w, vp_h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, VIDEO_WIDTH, 0, VIDEO_HEIGHT, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glPixelZoom(1.0f, 1.0f);
  glRasterPos2i(0, 0);
  glPixelZoom((float)vp_w / VIDEO_WIDTH, (float)vp_h / VIDEO_HEIGHT);
  glDrawPixels(VIDEO_WIDTH, VIDEO_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE,
               (uint8_t *)VIDEO_BUFFER);

  // Draw profiling overlay in full window coordinates
  glPixelZoom(1.0f, 1.0f);
  glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glColor3f(0.0f, 1.0f, 0.0f);
  char buf[128];
  uint64_t ips = GUI_P->profile_ips.load(std::memory_order_relaxed);
  int int08_fps = GUI_P->profile_int08_fps.load(std::memory_order_relaxed);
  double frame_ms = GUI_P->profile_frame_ms.load(std::memory_order_relaxed);
  uint64_t inst_count = GUI_P->profile_inst_count.load(std::memory_order_relaxed);

  float text_y = WINDOW_HEIGHT - 18;
  snprintf(buf, sizeof(buf), "IPS: %llu", (unsigned long long)ips);
  DrawText(10, text_y, buf);
  text_y -= 18;
  snprintf(buf, sizeof(buf), "INT08 FPS: %d", int08_fps);
  DrawText(10, text_y, buf);
  text_y -= 18;
  snprintf(buf, sizeof(buf), "Frame: %.2f ms", frame_ms);
  DrawText(10, text_y, buf);
  text_y -= 18;
  snprintf(buf, sizeof(buf), "Total: %llu", (unsigned long long)inst_count);
  DrawText(10, text_y, buf);
  text_y -= 18;
  snprintf(buf, sizeof(buf), "Render: %dx%d  Window: %dx%d", VIDEO_WIDTH, VIDEO_HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT);
  DrawText(10, text_y, buf);

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
