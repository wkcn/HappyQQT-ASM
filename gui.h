#pragma once
#include <GL/glut.h>
#include <GL/glu.h>
#include <array>
#include <iostream>
#include <vector>
#include <cstdint>
#include <thread>
using namespace std;
#include "logging.h"
#include <unistd.h>

const int WINDOW_WIDTH = 320, WINDOW_HEIGHT = 200;
const int MAX_FPS = 60;
const int UPDATE_GRAPH_INTERVAL = int(1000.0 / MAX_FPS);

void Display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  char *buffer = new char[WINDOW_WIDTH * WINDOW_HEIGHT * 3]; // RGB
  for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; ++i) {
    buffer[i * 3+2] = 255;
  }
  glDrawPixels(WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, buffer);
	glutSwapBuffers();
  delete []buffer;
}

void Idle(){
	usleep(UPDATE_GRAPH_INTERVAL);
	Display();
}

void Reshape(int w, int h){
	glViewport (0, 0, (GLsizei) WINDOW_WIDTH, (GLsizei) WINDOW_HEIGHT);   
}

void gui_main_func() {
  int argc = 0;
  char **argv = nullptr;
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowPosition(0, 0);
  glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
  int globalWindow = glutCreateWindow("Happy QQT");
  glutDisplayFunc(&Display);
	glutReshapeFunc(Reshape);
	glutIdleFunc(&Idle);
  glutMainLoop();
}

class GUI {
public:
  GUI() : gui_thread(gui_main_func) {
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
private:
  array<array<uint8_t, 3>, 256> palette;
  int palette_index = 0;
  int palette_sub_index = 0;
  thread gui_thread;
};
