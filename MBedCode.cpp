#include "LSM9DS1.h"
#include "PinDetect.h"
#include "mbed.h"
#include "rtos.h"
#include "uLCD_4DGL.h"

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
Serial pc(USBTX, USBRX);
PinDetect pb1(p7);
PinDetect pb2(p8);
LSM9DS1 lol(p9, p10, 0xD6, 0x3C);
uLCD_4DGL uLCD(p13, p14, p15);
Thread thread1;
Thread thread2;
Thread led_thread;
PwmOut red(p21); // led out on pin 21 - blue
PwmOut green(p22); // led out on pin 22 - green
PwmOut blue(p23); // led out on pin 23 - red

DigitalOut Email(p20);

bool still = false;
bool armed = false;
int counter = 0;

int running = 0;
int fresh = 1;
time_t rawtime;
struct tm* timeinfo;
char buffer[80];
Mutex lcdmut;


Mutex led_mutex;

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

void pb1_hit_callback(void) {
	armed = true;
	led1 = 1;
	running = 1;
	set_time(0);

	lcdmut.lock();
	red_v = 1.0f;
	green_v = 0.0f;
	blue_v = 0.0f;
	lcdmut.unlock();
}

void pb2_hit_callback(void) {
	armed = false;
	still = false;
	led1 = 0;
	led2 = 0;
	counter = 0;
	Email = 0;

	lcdmut.lock();
	running = 0;
	fresh = 0;
	red_v = 0.0f;
	green_v = 1.0f;
	blue_v = 0.0f;
	lcdmut.unlock();
}

void checkFinished() {
	while (1) {
		if (armed == true) {
			counter = 0;
			Thread::wait(1000);
			lol.readGyro();
			//pc.printf("X: %d Y:%d\n\r", lol.gx, lol.gy);
			int x = abs(lol.gx);
			int y = abs(lol.gy);
			pc.printf("X: %d Y:%d\n\r", x, y);

			if (x <= 50 && armed == true) { //&& y <= 50
				while (counter < 15) {
					if (x <= 50) {
						pc.printf("still \n\r");
						Thread::wait(1000);
						//still = true;
						counter++;
						pc.printf("Counter: %d \n\r", counter);

						lol.readGyro();
						x = abs(lol.gx);
					}
					if (counter == 15) {
						still = true;
						counter = 16;
					}
					if (x > 50) {
						counter = 16;
						pc.printf("Counter: %d \n\r", counter);
					}
				}
			}

			//lol.readGyro();
//            x = abs(lol.gx);
//            y = abs(lol.gy);
//            pc.printf("X: %d Y:%d\n\r", x, y);

			if (still == true) { //&& x <= 50 ){ //&& y <=50
				led2 = 1;
				still = false;
				armed = false;
				Email = 1;
				pc.printf("email sent \n\r");
				Thread::wait(500);
				Email = 0;
				lcdmut.lock();
				running = 0;
				fresh = 0;
				//set_led_color(1);
				//change_state(1);
				red_v = 0.0f;
				green_v = 0.0f;
				blue_v = 1.0f;
				lcdmut.unlock();


			}
		}
	}
}
void lcdupdate() {
	while (1) {
		if (running == 0) {
			lcdmut.lock();
			uLCD.text_string("Waiting    ", 9, 2, FONT_7X8, WHITE);
			uLCD.text_string("              ", 2, 5, FONT_7X8, WHITE);
			uLCD.text_string("                ", 2, 7, FONT_7X8, WHITE);
			if (fresh == 0) {
				uLCD.text_string("Last Wash Took:", 2, 10, FONT_7X8, WHITE);
				uLCD.text_string(buffer, 5, 12, FONT_7X8, WHITE);
			}
			lcdmut.unlock();
			Thread::wait(500);
		}
		else {
			lcdmut.lock();
			uLCD.text_string("Running  ", 9, 2, FONT_7X8, GREEN);
			uLCD.text_string("Washing...", 4, 5, FONT_7X8, BLUE);
			time(&rawtime);
			timeinfo = localtime(&rawtime);
			strftime(buffer, 80, "%H:%M:%S", timeinfo);
			uLCD.text_string(buffer, 5, 7, FONT_7X8, WHITE);
			lcdmut.unlock();
			Thread::wait(200);
		}
	}
}




void led_thread_func(void) {

	while (1) {

		//lcdmut.lock();
		red = red_v;
		green = green_v;
		blue = blue_v;
		//lcdmut.lock();

		Thread::wait(1000);

		red = 0;
		green = 0;
		blue = 0;

		Thread::wait(1000);

	}
}

int main() {

	lol.begin();
	if (!lol.begin()) {
		pc.printf("Failed to communicate with LSM9DS1.\n");
	}
	lol.calibrate();

	Email = 0;

	pb1.mode(PullUp);
	pb2.mode(PullUp);
	wait(.001);

	pb1.attach_deasserted(&pb1_hit_callback);
	pb2.attach_deasserted(&pb2_hit_callback);

	pb1.setSampleFrequency();
	pb2.setSampleFrequency();
	wait(.001);

	uLCD.text_height(1);
	uLCD.text_width(1);
	wait(0.01);

	uLCD.text_string("Status:", 2, 2, FONT_7X8, WHITE);

	thread1.start(checkFinished);
	thread2.start(lcdupdate);
	led_thread.start(led_thread_func);


}