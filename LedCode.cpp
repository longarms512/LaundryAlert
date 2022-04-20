#include "mbed.h"
#include "PinDetect.h"
#include "rtos.h"

PwmOut red(p21); // led out on pin 21 - blue
PwmOut green(p22); // led out on pin 22 - green
PwmOut blue(p23); // led out on pin 23 - red

PinDetect change_state_but(p20); //push button in on pin 20

Serial pc(USBTX, USBRX);

//---------------------------------------------------------------------------------------------------

Thread led_thread;
Thread button_thread;
Mutex led_mutex;
DigitalOut led1(LED1);

int state = 0;
/*
state 0 = ready to use (no clothes inside)
state 1 = user input (setting up notification system)
state 2 = in progress (washing machine running)
state 3 = clothes done (waiting for user to return)
*/
float red_v = 0.0f;
float green_v = 1.0f;
float blue_v = 0.0f;

float p = 0.0f; 


void set_led_color(int state_in) {
    if (state_in == 0) {
        led_mutex.lock();
        red_v = 0.0f;
        green_v = 1.0f;
        blue_v = 0.0f;
        led_mutex.unlock();
    }
    
    else if (state_in == 1) {
        led_mutex.lock();
        red_v = 0.0f;
        green_v = 0.0f;
        blue_v = 1.0f;
        led_mutex.unlock();
    }
    
    else if (state_in == 2) {
        led_mutex.lock();
        red_v = 1.0f;
        green_v = 0.0f;
        blue_v = 0.0f;
        led_mutex.unlock();
    }
    
    else if (state_in == 3) {
        led_mutex.lock();
        red_v = 1.0f;
        green_v = 0.8f;
        blue_v = 0.0f;
        led_mutex.unlock();
    }
    
    else {
        led_mutex.lock();
        red_v = 1.0f;
        green_v = 0.0f;
        blue_v = 0.0f;
        led_mutex.unlock();
    }
    pc.printf("changing led color \n\rstate: %d\n\r", state_in);
    pc.printf("red_v = %f\n\r", red_v);
    pc.printf("green_v = %f\n\r", green_v);
    pc.printf("blue_v = %f\n\r", blue_v);
}

void change_state (void) {
    
    //state = new_state;
    
           
    while(1) {
        Thread::wait(4000);
        if (state < 3) {
            state++;
        }
        else {
            state = 0;
        }
        
        set_led_color(state);
    }
    
}

void led_thread_func (void) {
    
    while (1) {
        
        led_mutex.lock();
        red = red_v;
        green = green_v;
        blue = blue_v;
        led_mutex.unlock();
        
        Thread::wait(1000);
        
        red = 0;
        green = 0;
        blue = 0;
        
        Thread::wait(1000);
        
    }
    
}

int main(){ 
    //setup push buttons    
    change_state_but.mode(PullUp); 
        
    // Delay for initial pullup to take effect    
    wait(.01);
    
    //change_state_but.attach_deasserted(&change_state_callback);
    // Start sampling pb input using interrupts
    //change_state_but.setSampleFrequency();
  
    led_thread.start(led_thread_func);
    button_thread.start(change_state);
    
    while (true) {
        //led1 = !led1;
        continue;
        Thread::wait(500);
    
    }     
    
    
} //end main