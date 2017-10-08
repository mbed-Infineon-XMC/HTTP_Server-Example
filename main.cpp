/* mbed Example Program
 * Copyright (c) 2006-2014 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/******************************************************************* Includes */
#include "mbed.h"
#include "rtos.h"

/******************************************************************** Globals */
AnalogIn input(P14_0);
DigitalOut led_flash(LED1);
Serial device(P1_5, P1_4); // tx, rx

Thread thread1;

/****************************************************************** Functions */

/**
 * ADC Thread
 */
void adc_thread() {

    float vin;

    /* Initialize Serial */
    device.baud(9600);
    device.printf("ADC-Example\n");

    /* Print input voltage */
    while(1){
        wait(1);
        vin = (input.read() * 3.3);
        device.printf("Vin = %.01f V\n", vin);
    }
}

/**
 * Main Function
 */
int main() {

    /* Start ADC-Sample Thread */
    thread1.start(adc_thread);

    /* Toggle LED1  */
    while (1) {
        led_flash = !led_flash;
        wait(1);
    }
}

/*EOF*/
