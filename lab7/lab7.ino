/*
 * CSCI 1600 lab 7 starter code
 * Author: Milda Zizyte
 */

#include <SPI.h>
#include <WiFi101.h>

WiFiClient client;
char buffer[200];

const int buf_size = 80;

byte s_buf[buf_size];
int s_buf_start = 0;
int s_buf_end = 0;

byte r_buf[buf_size];
int r_buf_start = 0;
int r_buf_end = 0;

const int uart_period_micros = 104;

const int in_pin = 5;
const int out_pin = 3;

const int r_pin = 0;
const int g_pin = 1;
const int b_pin = 2;
const int led_pins[3] = {r_pin, g_pin, b_pin};

/*
 * LAB STEP 4c/g
 * Change this to false for receiver!!
 */
const bool SENDER = false;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial);
  Serial1.begin(9600);
  
  /*
   * Uncomment for LAB STEP 4a
   * Initialize software UART pins and behaviors
   */
  pinMode(in_pin, INPUT);
  pinMode(out_pin, OUTPUT);
  digitalWrite(out_pin, HIGH);
  attachInterrupt(in_pin, uart_receive, CHANGE);

  // initialize pins
  for (int i = 0; i < 3; i++) {
    pinMode(led_pins[i], OUTPUT);
    digitalWrite(led_pins[i], LOW);
  }
  
  if (SENDER) { setup_wifi(); }
  interrupts();
}

void loop() {
  if (SENDER) {
    read_webpage();
    /*
     * LAB STEP 4e:
     * Check if s_buf is not empty
     * Send bytes via UART
     */
    noInterrupts();
    if (!(s_buf_start == s_buf_end)) {
      uart_send(s_buf[s_buf_start]);
      s_buf_start = (s_buf_start + 1) % buf_size;
    }
    interrupts();




    // Connection ended / no more bytes to read
    if (!client.connected()) {
      delay(500);
      // Try to reconnect
      connect_to_webpage();
    }
  } else { // not SENDER
    Serial.println("loop");
    /*
     * LAB STEP 4f:
     * Check if r_buf is not empty
     * Print to serial monitor (for debugging) and/or light up corresponding LED
     */
     noInterrupts();
     if (r_buf_start != r_buf_end) {
       byte b = r_buf[r_buf_start];
       r_buf_start = (r_buf_start + 1) % buf_size;
      
       Serial.print("Lighting LED: ");
       Serial.println(b);
      
       if (b == 'R') {
         digitalWrite(led_pins[0], HIGH);
         digitalWrite(led_pins[1], LOW);
         digitalWrite(led_pins[2], LOW);
       } else if (b == 'G') {
         digitalWrite(led_pins[0], LOW);
         digitalWrite(led_pins[1], LOW);
         digitalWrite(led_pins[2], HIGH);
       } else if (b == 'B') {
         digitalWrite(led_pins[0], LOW);
         digitalWrite(led_pins[1], HIGH);
         digitalWrite(led_pins[2], LOW);
       }
     }
     interrupts();
  }
}
