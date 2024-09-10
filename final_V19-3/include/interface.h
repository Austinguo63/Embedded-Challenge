
// Final Project Fall 2023: Need for Speed
//Group Number: 39
// Auther Name: Hongyu Guo (Hg2860), Zhenyuan Dong(zd2362)
// youtube link: https://youtu.be/Oi5I4e3ZiLI


#ifndef INTERFACE_H
#define INTERFACE_H
#include <queue>
#include <mbed.h>
#include "drivers/LCD_DISCO_F429ZI.h"
#include "drivers/TS_DISCO_F429ZI.h"

#define BACKGROUND 1
#define FOREGROUND 0
#define GRAPH_PADDING 5

extern LCD_DISCO_F429ZI lcd;
extern TS_DISCO_F429ZI ts;

extern char display_buf[2][60];
extern uint32_t graph_width;
extern uint32_t graph_height;
extern TS_StateTypeDef TS_State;

extern void setup_background_layer();
extern void setup_foreground_layer();
extern void draw_graph_window(uint32_t horiz_tick_spacing);
extern uint16_t mapPixelY(float inputY, float minVal, float maxVal, int32_t minPixelY, int32_t maxPixelY);
extern void display_start_screen();
extern void display_end_screen();
extern bool check_start_button_pressed(TS_StateTypeDef &TS_State);
extern void display_end_screen(float totalTime, float totalWalkingDistance, float speed);
extern bool check_back_button_pressed(TS_StateTypeDef &TS_State);
extern void display_finish_button();
extern bool check_finish_button_pressed(TS_StateTypeDef &TS_State);

///////////
void filter_to_compute_instant_velocity();
extern float instant_velocity;
extern std::queue<float> Velocity;
extern float gz;
///////////


// Circular Buffer and Draw Thread
#define BUFFER_SIZE 230
extern CircularBuffer<float, BUFFER_SIZE> new_values;
extern Semaphore new_values_semaphore;
extern uint32_t graph_pixels[BUFFER_SIZE];
extern uint32_t next_write_index;
extern unsigned char draw_thread_stack[4096];
extern Thread draw_thread;
extern void draw_thread_proc();

#endif // INTERFACE_H



