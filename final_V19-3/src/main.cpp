// Final Project Fall 2023: Need for Speed
//Group Number: 39
// Auther Name: Hongyu Guo (Hg2860), Zhenyuan Dong(zd2362)
// youtube link: https://youtu.be/Oi5I4e3ZiLI

#include "mbed.h"
#include "interface.h"
#include <queue>
#include "drivers/LCD_DISCO_F429ZI.h"
#include "drivers/TS_DISCO_F429ZI.h"
using namespace std;

#define SCALING_FACTOR (17.5f * 0.017453292519943295769236907684886f / 1000.0f)
float speed = 0;
float avg_speed = 0;
float gyroX1;
float instant_velocity;
float average_velocity;
float total_velocity = 0;
float Distance = 0;
float walking_time = 0;
float Height_Term = 0.8;
bool debug_flag = false;
queue<float>Velocity;
queue<float>all_time_velocity;

Ticker tick;
int16_t xyz_data[3];
SPI spi(PF_9, PF_8, PF_7, PC_1, use_gpio_ssel); // mosi, miso, sclk, cs
uint8_t write_buf[32];
uint8_t read_buf[32];
float totalDist = 0;
volatile uint8_t Get_Gyro_Data_Flag = 0;
volatile uint8_t offset = -1;
DigitalOut led(LED1); 


#define SPI_FLAG 1
#define CTRL_REG1 0x20
#define CTRL_REG1_CONFIG 0b01'10'1'1'1'1
#define CTRL_REG4 0x23
#define CTRL_REG4_CONFIG 0b0'0'01'0'00'0
#define OUT_X_L 0x28

EventFlags flags;



void spi_cb(int event) { flags.set(SPI_FLAG); };

void Gyro_cb() {
    Get_Gyro_Data_Flag = 1;
    offset++;
}

int16_t raw_gx, raw_gy, raw_gz;
float gx, gy, gz;

void get_gyro_value(){

    // reading the status register. bit 4 of the status register
    // is 1 when a new set of samples is ready
    write_buf[0] = 0x27 | 0x80;

    do {
        spi.transfer(write_buf, 2, read_buf, 2, spi_cb, SPI_EVENT_COMPLETE);
        flags.wait_all(SPI_FLAG);

    } while ((read_buf[1] & 0b0000'1000) == 0);

    // prepare the write buffer to trigger a sequential read
    write_buf[0] = OUT_X_L | 0x80 | 0x40;

    // start sequential sample reading
    spi.transfer(write_buf, 7, read_buf, 8, spi_cb, SPI_EVENT_COMPLETE);
    flags.wait_all(SPI_FLAG);

    //Process raw data
    raw_gx = (((uint16_t)read_buf[2]) << 8) | ((uint16_t)read_buf[1]);
    raw_gy = (((uint16_t)read_buf[4]) << 8) | ((uint16_t)read_buf[3]);
    raw_gz = (((uint16_t)read_buf[6]) << 8) | ((uint16_t)read_buf[5]);

    // SCALING_FACTOR is 17.5f * pi/180 / 1000
    gx = ((float)raw_gx) * SCALING_FACTOR;
    gy = ((float)raw_gy) * SCALING_FACTOR;
    gz = ((float)raw_gz) * SCALING_FACTOR;

}

void filter_to_compute_instant_velocity(){

    //use a filter to reduce the effect of noise
    if (Velocity.size() < 3){
        Velocity.push(gz);
        instant_velocity = gz;
    }

    else{
        // Get the last three speeds from the queue
        float second_last_speed = Velocity.front();
        Velocity.pop();
        float last_speed = Velocity.front();
        Velocity.push(gz);
        float now_speed = gz;

        // use filter(V) = (now_speed + last_speed + second_last_speed)/3
        instant_velocity = (now_speed + last_speed + second_last_speed)/3;
        if (instant_velocity < 0){
            instant_velocity = 0;
        }
    }

}


void compute_walkingtime(){
    
    //use the queue's size to allocate the tick time and finally compute the time
    all_time_velocity.push(instant_velocity);
    total_velocity += all_time_velocity.back();
    walking_time = all_time_velocity.size()/2;

}

void compute_distance(){

    //Convert those measurements of angular velocities to linear forward velocity
    speed = instant_velocity * Height_Term;

    //Calculate the overall distance traveled during the 20 seconds of measurements.
    Distance += speed * 0.5;

}

void compute_final_average_velocity(){

    //Calculate the overall average speed traveled during the 20 seconds of measurements.
    avg_speed = Distance / 20.0f;

}

void set_speed_event(){
    tick.attach(&Gyro_cb, 500ms);
    spi.format(8, 3);
    spi.frequency(1'000'000);


    write_buf[0] = CTRL_REG1;
    write_buf[1] = CTRL_REG1_CONFIG;
    spi.transfer(write_buf, 2, read_buf, 2, spi_cb, SPI_EVENT_COMPLETE);
    flags.wait_all(SPI_FLAG);

    write_buf[0] = CTRL_REG4;
    write_buf[1] = CTRL_REG4_CONFIG;
    spi.transfer(write_buf, 2, read_buf, 2, spi_cb, SPI_EVENT_COMPLETE);
    flags.wait_all(SPI_FLAG);

}

// this is a float convert function, because we find out board does not print float,alwasy print %0.2f
void PrintFloat(float value, char * Print_OUT) 
{
    int tmp,tmp1;
    tmp = (int)value;
    tmp1=(int)((value-tmp)*10)%10;
    printf("%d.%d\r\n",tmp,tmp1);
    snprintf(Print_OUT,40,"%d.%d",tmp,tmp1);
}


// this function show three things on the screen, instant_velocity, Distance, times
void show_result(){ 
 
    lcd.Clear(LCD_COLOR_LIGHTGREEN);
    lcd.SetBackColor(LCD_COLOR_LIGHTGREEN);
    lcd.SetTextColor(LCD_COLOR_LIGHTRED);
    lcd.SetFont(&Font20);

    lcd.DisplayStringAt(0, LINE(1), (uint8_t *)"Real Time Data", CENTER_MODE);

    char printout[3][40];
    char sentence[3][40];
    printf("instant_velocity is "); PrintFloat(instant_velocity, printout[0]);
    printf("distance is "); PrintFloat(Distance, printout[1]);
    printf("time is "); PrintFloat(all_time_velocity.size()/2.0, printout[2]);

    snprintf(sentence[0],40,"InstantVelocity");
    snprintf(sentence[1],40,"Distance(m)");
    snprintf(sentence[2],40,"Time(s)");


    //display the buffered string on the screen
    lcd.DisplayStringAt(0, LINE(3), (uint8_t *)sentence[0], CENTER_MODE);
    lcd.DisplayStringAt(0, LINE(5), (uint8_t *)printout[0], CENTER_MODE);
    lcd.DisplayStringAt(0, LINE(6), (uint8_t *)sentence[1], CENTER_MODE);
    lcd.DisplayStringAt(0, LINE(8), (uint8_t *)printout[1], CENTER_MODE);
    lcd.DisplayStringAt(0, LINE(10), (uint8_t *)sentence[2], CENTER_MODE);
    lcd.DisplayStringAt(0, LINE(12), (uint8_t *)printout[2], CENTER_MODE);

    // here is a button to go to plot screen, its sensiable area is top-middle, rest code for this button is in interface.cpp
    int buttonWidth = 200;  
    int buttonHeight = 50;
    int buttonX = (lcd.GetXSize() - buttonWidth) / 2; // Position at the center
    int buttonY = lcd.GetYSize() - buttonHeight - 10; // Position at the bottom

    int textX = buttonX;  
    int textY = buttonY + (buttonHeight / 2) -15;  // Position of text

    lcd.FillRect(buttonX, buttonY, buttonWidth, buttonHeight);
    lcd.SetBackColor(LCD_COLOR_LIGHTRED);
    lcd.SetTextColor(LCD_COLOR_WHITE);
    lcd.SetFont(&Font16);
    lcd.DisplayStringAt(textX -20 , textY, (uint8_t *)"Tap top-middle", CENTER_MODE);
    lcd.DisplayStringAt(textX-20 , textY + 17, (uint8_t *)"to go to plot", CENTER_MODE);
}

// this function show our first page, which is a start screen with a button can go to real time data screen
void set_lcd_event(){

    // start screen setup
    display_start_screen();
    bool startPressed = false;

    // Wait for the start button to be pressed
    while (!startPressed) {
        startPressed = check_start_button_pressed(TS_State);
        thread_sleep_for(100); // Check every 100ms
    }
    printf("Start button pressed, go to 'Real Time Data' screen\r\n");

    // draw plot screen setup
    show_result();
    //draw_thread.start(draw_thread_proc);

}

// this is function that we use to draw the plot screen, this is method3 from recition.
void draw_thread_proc(){

  static float next_value=0.0;
  setup_background_layer();
  setup_foreground_layer();
  draw_graph_window(10);
  lcd.SelectLayer(FOREGROUND);
  for(int i=0;i<graph_width;i++){
    graph_pixels[i]= GRAPH_PADDING+graph_height;
  }
    // this finish button is sensiable area is top-middle, rest code for this button is in interface.cpp
  display_finish_button();  
  while(true){

    if (check_finish_button_pressed(TS_State)) { // check if the finish button is pressed
        printf("Finish in plot pressed, go to end page\r\n");
        break; // if pressed, exit the loop
    }
    


    //wait for main thread to release a semaphore,
    //to indicate a new sample is ready to be graphed
    new_values_semaphore.acquire();
    new_values.pop(next_value);

    //update the graph
    char buffer[1024];


    snprintf(buffer, sizeof(buffer), "Value: %d", int(Distance)); // change here !!!!! 


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
    graph_pixels[next_write_index]=mapPixelY(next_value,0,30,GRAPH_PADDING,GRAPH_PADDING+graph_height);
    lcd.DrawPixel(GRAPH_PADDING+graph_width-1,graph_pixels[next_write_index],LCD_COLOR_BLUE);
    next_write_index=(next_write_index+1)%graph_width;
  }

  display_end_screen(walking_time, Distance, avg_speed); // once exit loop, we have to display the end screen, this screen shows the total result

}



// this is "main" function of the plot screen, some of code is method3 from recition.
void lcd_interface(){
    

    //  draw the plat screen
    draw_thread.start(draw_thread_proc);

    thread_sleep_for(1000);
    uint32_t index=0;
    
    printf("Distance is %d\n", int(Distance)); // we use this to show the distance on the plot screen
    while(1){

        if(!new_values.full()){
        float current_val= int(Distance); // distance is the value we want to plot
        index++;
        //push the next value into the circular buffer
        new_values.push(current_val);
        if(new_values_semaphore.release()!=osOK){
            MBED_ERROR(MBED_MAKE_ERROR(MBED_MODULE_APPLICATION,MBED_ERROR_CODE_OUT_OF_MEMORY),"semaphore overflow\r\n");
        }
        }
        else{
        MBED_ERROR(MBED_MAKE_ERROR(MBED_MODULE_APPLICATION,MBED_ERROR_CODE_OUT_OF_MEMORY),"circular buffer is full\r\n");
        }

        thread_sleep_for(100);
    }

}




int main() {
    // Initial configuration
    set_speed_event(); 
    set_lcd_event();
    float Height;

    //set the height who use this device
    Height = 1.85;

    //compute the Height_Term
    Height_Term = Height / 2;

    wait_us(5'000'000);

    while (true) {
        if (Get_Gyro_Data_Flag && !(walking_time > 19)) {
            // Continuously measure gyro values from the angular velocity sensor
            get_gyro_value();

            
            // Use a filter to reduce the effect of noise
            filter_to_compute_instant_velocity();
            
            // Compute the walking time
            compute_walkingtime();

            // Compute the distance walked
            compute_distance();

            // Compute the average velocity
            compute_final_average_velocity();

            // Show results
            show_result();


            Get_Gyro_Data_Flag = 0;
        }

        // Check if the back button is pressed
        if (check_back_button_pressed(TS_State)) {
            printf("Back button pressed, go to plot screen\r\n");

            // Start the plot page drawing thread
           lcd_interface();

            // Wait until the back button is pressed again to return to the main loop
            while (!check_back_button_pressed(TS_State)) {
                thread_sleep_for(100); // Check every 100ms
            }
            printf("Returning to the main screen\r\n");
        }

    }

    tick.detach();
}