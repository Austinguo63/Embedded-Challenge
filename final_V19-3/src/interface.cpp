// Final Project Fall 2023: Need for Speed
//Group Number: 39
// Auther Name: Hongyu Guo (Hg2860), Zhenyuan Dong(zd2362)
// youtube link: https://youtu.be/Oi5I4e3ZiLI

#include <mbed.h>
#include "interface.h"
//this file has all the functions for interacting
//with the screen
// some functions are from example code from recition.
#include "drivers/LCD_DISCO_F429ZI.h"
#include "drivers/TS_DISCO_F429ZI.h"


extern float instant_velocity;


LCD_DISCO_F429ZI lcd;
TS_DISCO_F429ZI ts;
//buffer for holding displayed text strings
char display_buf[2][60];
uint32_t graph_width=lcd.GetXSize()-2*GRAPH_PADDING;
uint32_t graph_height=graph_width;
TS_StateTypeDef TS_State; // global define for touch screen state

//sets the background layer 
//to be visible, transparent, and
//resets its colors to all black
extern void setup_background_layer(){
  lcd.SelectLayer(BACKGROUND);
  lcd.Clear(LCD_COLOR_BLACK);
  lcd.SetBackColor(LCD_COLOR_BLACK);
  lcd.SetTextColor(LCD_COLOR_GREEN);
  lcd.SetLayerVisible(BACKGROUND,ENABLE);
  lcd.SetTransparency(BACKGROUND,0xFFu);
}

//resets the foreground layer to
//all black
extern void setup_foreground_layer(){
    lcd.SelectLayer(FOREGROUND);
    lcd.Clear(LCD_COLOR_BLACK);
    lcd.SetBackColor(LCD_COLOR_BLACK);
    lcd.SetTextColor(LCD_COLOR_LIGHTGREEN);
}

//draws a rectangle with horizontal tick marks
//on the background layer. The spacing between tick
//marks in pixels is taken as a parameter
extern void draw_graph_window(uint32_t horiz_tick_spacing){
  lcd.SelectLayer(BACKGROUND);
  
  lcd.DrawRect(GRAPH_PADDING,GRAPH_PADDING,graph_width,graph_width);
  //draw the x-axis tick marks
  for (int32_t i = 0 ; i < graph_width;i+=horiz_tick_spacing){
    lcd.DrawVLine(GRAPH_PADDING+i,graph_height,GRAPH_PADDING);
  }
}

//maps inputY in the range minVal to maxVal, to a y-axis value pixel in the range
//minPixelY to MaxPixelY
uint16_t mapPixelY(float inputY,float minVal, float maxVal, int32_t minPixelY, int32_t maxPixelY){
  const float mapped_pixel_y=(float)maxPixelY-(inputY)/(maxVal-minVal)*((float)maxPixelY-(float)minPixelY);
  return mapped_pixel_y;
}


//this is start_screen setup function
extern void display_start_screen() {
    lcd.Clear(LCD_COLOR_LIGHTRED);
    lcd.SetBackColor(LCD_COLOR_LIGHTRED);

    // all the text displeyed on the start screen, with Xmas sepical color. 
    lcd.SetFont(&Font20);
    lcd.SetTextColor(LCD_COLOR_LIGHTGREEN);
    lcd.DisplayStringAt(10, LINE(1), (uint8_t *) "ECE-6483 Final", CENTER_MODE);
    lcd.DisplayStringAt(10, LINE(2), (uint8_t *) "Embedded", CENTER_MODE);
    lcd.DisplayStringAt(10, LINE(3), (uint8_t *) "Challenge 2023", CENTER_MODE);
    lcd.DisplayStringAt(8, LINE(11), (uint8_t *) "MERRY XMAS", CENTER_MODE);
    lcd.DisplayStringAt(8, LINE(13), (uint8_t *) "Hongyu Guo", CENTER_MODE);
    lcd.DisplayStringAt(7, LINE(14), (uint8_t *) "Zhenyuan Dong", CENTER_MODE);

    // Display a "Start" button
    lcd.SetTextColor(LCD_COLOR_LIGHTGREEN);
    int buttonWidth = 100;
    int buttonHeight = 50;
    int buttonX = (lcd.GetXSize() - buttonWidth) / 2;
    int buttonY = (lcd.GetYSize() - buttonHeight) / 2;

    lcd.FillRect(buttonX, buttonY, buttonWidth, buttonHeight);

    int textX = buttonX -70;
    int textY = buttonY +15;
    lcd.SetFont(&Font24);
    lcd.SetBackColor(LCD_COLOR_LIGHTGREEN);
    lcd.SetTextColor(LCD_COLOR_LIGHTRED);
    lcd.DisplayStringAt(textX, textY, (uint8_t *) "Start", CENTER_MODE);
}





//function for check if the start button is pressed, if yes, return true
extern bool check_start_button_pressed(TS_StateTypeDef &TS_State) {
    int touchX, touchY;

    // Coordinates where the start button is displayed
    int buttonX = (lcd.GetXSize() - 100) / 2;
    int buttonY = (lcd.GetYSize() - 50) / 2;

    ts.GetState(&TS_State); // get the touch screen state
    if (TS_State.TouchDetected) { // if the touch screen is touched
        touchX = TS_State.X;
        touchY = TS_State.Y;

        // Check if the touch is within the button area
        if (touchX >= buttonX && touchX <= (buttonX + 100) && touchY >= buttonY && touchY <= (buttonY + 50)) {
            return true;
        }
    }
    return false;
}

// this is the function to covert float to char, so we can display it on the screen
extern void snprintfloat(float value, char * Print_OUT)
{
    int tmp,tmp1;
    tmp = (int)value;
    tmp1=(int)((value-tmp)*10)%10;
    snprintf(Print_OUT,40,"%d.%d",tmp,tmp1);
}


// here is setup for our end screen, with all the final data we want to display
extern void display_end_screen(float totalTime, float totalWalkingDistance, float speed) {
    lcd.Clear(LCD_COLOR_LIGHTGREEN);
    lcd.SetBackColor(LCD_COLOR_LIGHTGREEN); //set the  color, font, and size
    lcd.SetTextColor(LCD_COLOR_LIGHTRED);
    lcd.SetFont(&Font20);

    // Display "End" title
    lcd.DisplayStringAt(0, LINE(1), (uint8_t *)"Total Data", CENTER_MODE);

    // Display information
    char printout[3][40];
    char sentence[3][40];
    snprintfloat(totalTime, printout[0]);
    snprintfloat(totalWalkingDistance, printout[1]);
    snprintfloat(speed, printout[2]);
    snprintf(sentence[0],40,"TotalTime(s): ");
    snprintf(sentence[1],40,"Distance(m): ");
    snprintf(sentence[2],40,"Average(m/s): ");


    //display the buffered string on the screen
    lcd.DisplayStringAt(0, LINE(3), (uint8_t *)sentence[0], CENTER_MODE);
    lcd.DisplayStringAt(0, LINE(5), (uint8_t *)printout[0], CENTER_MODE);
    lcd.DisplayStringAt(0, LINE(6), (uint8_t *)sentence[1], CENTER_MODE);
    lcd.DisplayStringAt(0, LINE(8), (uint8_t *)printout[1], CENTER_MODE);
    lcd.DisplayStringAt(0, LINE(10), (uint8_t *)sentence[2], CENTER_MODE);
    lcd.DisplayStringAt(0, LINE(12), (uint8_t *)printout[2], CENTER_MODE);

}


// this is the function to check if the restart button is pressed, if yes, return true. 
extern bool check_back_button_pressed(TS_StateTypeDef &TS_State) {
    int touchX, touchY;

    // Coordinates where the restart button is displayed
    int buttonX = (lcd.GetXSize() - 100) / 2;
    int buttonY = lcd.GetYSize() - 50 - 10;

    ts.GetState(&TS_State); // Sensible touchscreen area for the restart button
    if (TS_State.TouchDetected) { // if the touch screen is touched
        touchX = TS_State.X;
        touchY = TS_State.Y;

        // Check if the touch is within the button area
        if (touchX >= buttonX && touchX <= (buttonX + 100) && touchY >= buttonY && touchY <= (buttonY + 50)) {
            return true;
        }
    }
    return false;
}


unsigned char draw_thread_stack[4096]; //stack used by the draw_thread to run the draw_thread_proc function
Thread draw_thread(osPriorityBelowNormal1,4096,draw_thread_stack);


// this is setup for finish button, this will lead to the end screen
extern void display_finish_button() {
    // Set the layer for drawing the button
    lcd.SelectLayer(BACKGROUND);

    // Button dimensions and position
    int buttonWidth = 300;
    int buttonHeight = 50;
    int buttonX = lcd.GetXSize() - buttonWidth - 10; // 10 pixels from the right edge
    int buttonY = lcd.GetYSize() - buttonHeight - 10; // 10 pixels from the bottom edge

    int textX = buttonX + 70;  
    int textY = buttonY + (buttonHeight / 2) - 8; 
    // Draw the button
    lcd.FillRect(buttonX, buttonY, buttonWidth, buttonHeight);
    lcd.SetFont(&Font16);
    lcd.SetTextColor(LCD_COLOR_WHITE);
    lcd.SetBackColor(LCD_COLOR_BLACK);
    lcd.DisplayStringAt( textX , textY ,(uint8_t *)"Press top-middle part", CENTER_MODE);
    lcd.DisplayStringAt( textX , textY + 20 ,(uint8_t *)"of graph to Finish", CENTER_MODE);
}

// this is the function to check if the finish button is pressed, if yes, return true.
extern bool check_finish_button_pressed(TS_StateTypeDef &TS_State) {
    int touchX, touchY;

    // Coordinates where the 'Finish' button is displayed
    int buttonWidth = 100;                    // Same as used in display_finish_button
    int buttonHeight = 50;
    int buttonX = lcd.GetXSize() - buttonWidth - 10;
    int buttonY = lcd.GetYSize() - buttonHeight - 10; 

    ts.GetState(&TS_State);  // get the touch screen Sensible area state
    if (TS_State.TouchDetected) { // if the touch screen is touched
        touchX = TS_State.X;
        touchY = TS_State.Y;

        // Check if the touch is within the button area
        if (touchX >= buttonX && touchX <= (buttonX + buttonWidth) && touchY >= buttonY && touchY <= (buttonY + buttonHeight)) {
            return true;
        }
    }
    return false;
}



//*//START OF EXAMPLE 3-------------------------------------------------
#define BUFFER_SIZE 230
//stack used by the draw_thread to run the draw_thread_proc function
// unsigned char draw_thread_stack[4096];
// Thread draw_thread(osPriorityBelowNormal1,4096,draw_thread_stack);
//circular buffer is used like a queue. The main thread pushes
//new data into the buffer, and the draw thread pops them out
//and updates the graph
CircularBuffer<float, BUFFER_SIZE> new_values;
//semaphore used to protect the new_values buffer
Semaphore new_values_semaphore(0,BUFFER_SIZE);


uint32_t graph_pixels[BUFFER_SIZE];
uint32_t next_write_index=0;


//Pleasse ignore rest 
// we moved rest of main into our main.cpp, this rest is just our recoed.

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
extern void draw_thread_proc(){

  static float next_value=0.0;
  setup_background_layer();
  setup_foreground_layer();
  draw_graph_window(10);
  lcd.SelectLayer(FOREGROUND);
  for(int i=0;i<graph_width;i++){
    graph_pixels[i]= GRAPH_PADDING+graph_height;
  }

  display_finish_button();  
  while(true){

    if (check_finish_button_pressed(TS_State)) {
        printf("Finish in plot pressed, go to end page\r\n");
        break; // Exit the loop to finish drawing
    }
    


    //wait for main thread to release a semaphore,
    //to indicate a new sample is ready to be graphed
    new_values_semaphore.acquire();
    new_values.pop(next_value);

    //update the graph
    char buffer[1024];
    filter_to_compute_instant_velocity();
    int new_instant_velocity = (int)instant_velocity;
    snprintf(buffer, sizeof(buffer), "Value: %d", new_instant_velocity); // change here !!!!! 
    printf ("buffer is %s\n", buffer);
    printf ("instant_velocity is %f\n", instant_velocity);
    printf("velocity in plot is %d\n", new_instant_velocity);
    lcd.SetTextColor(LCD_COLOR_LIGHTRED);
    lcd.DisplayStringAt(0, LINE(20), (uint8_t *)buffer, LEFT_MODE);


    for(int i = 0; i<(graph_width-1);i++){
        //the x coordinate of the graph value being updated.
        //think about it like a vertical line
        //that sweeps across the graph from left to right,
        //updating each point in the graph as it travels across.
        const uint32_t target_x_coord=GRAPH_PADDING+i;
         //y coordinate of the previous function value at the target x coordinate
         const uint32_t old_pixelY=graph_pixels[(i+next_write_index)%graph_width];
        //y coordinate of the current function value at the target x coordinate
        const uint32_t new_pixelY=graph_pixels[(i+next_write_index+1)%graph_width];
         //remove (set to black) the old pixel for the current x coordinate
         //from the screen
         lcd.DrawPixel(target_x_coord,old_pixelY,LCD_COLOR_BLACK);
        //draw the new pixel for the current x coordinate on the screen
        lcd.DrawPixel(target_x_coord,new_pixelY,LCD_COLOR_BLUE);
    }
    //retrieve and erase the right most(last) pixel in the graph
    const uint32_t last_old_pixelY= graph_pixels[(graph_width-1+next_write_index)%graph_width]; 
    lcd.DrawPixel(GRAPH_PADDING+graph_width-1,last_old_pixelY,LCD_COLOR_BLACK);
    //map, draw and store the newest value
    graph_pixels[next_write_index]=mapPixelY(next_value,0,2,GRAPH_PADDING,GRAPH_PADDING+graph_height);
    lcd.DrawPixel(GRAPH_PADDING+graph_width-1,graph_pixels[next_write_index],LCD_COLOR_BLUE);
    next_write_index=(next_write_index+1)%graph_width;
  }

  display_end_screen(10,10,10); // display_end_screen(totalTime, totalWalkingDistance, speed);

}
*/


