/*
 * Manually wait for UART cycle
 */
inline void uart_delay(unsigned long ltime) {
  delayMicroseconds(uart_period_micros - (micros() - ltime));
}

/*
 * LAB STEP 4e
 * Send byte via UART
 */
void uart_send(byte B) {
  Serial.println(B);
  unsigned long last_clock_time = micros();
  // send start bit
  digitalWrite(out_pin, LOW);
  int i = 0;
  byte parity = 0;
  while (i < 8) {
    if ((B & 0x1) == 0x1) {
      uart_delay(last_clock_time);
      digitalWrite(out_pin, HIGH);
      last_clock_time = micros();
      parity = parity ^ 0x01;
    } else {
      uart_delay(last_clock_time);
      digitalWrite(out_pin, LOW);
      last_clock_time = micros();
    }
    B = B >> 1;
    i += 1;
  }
  uart_delay(last_clock_time);
  // write parity bit;
  if (parity == 1) {
    digitalWrite(out_pin, HIGH);
  } else {
    digitalWrite(out_pin, LOW);
  }
  uart_delay(micros());
  // back to high
  digitalWrite(out_pin, HIGH);
  
}

/*
 * LAB STEP 4f
 * Read a byte from the UART and store in the r_buf
 */
void uart_receive() {
  // delay for 1/3 of the UART period just to get reads towards the middle of bits
  noInterrupts();
  delayMicroseconds(uart_period_micros / 3);
  unsigned long last_clock_time = micros();
  int i = 0;
  byte B = 0;
  byte parity = 0;
  while (i < 8) {
    B = B >> 1;
    uart_delay(last_clock_time);
    int in_pin_val = digitalRead(in_pin);
    last_clock_time = micros();
    if (in_pin_val == HIGH) {
      B = B | (0x1 << 7);
      parity = parity ^ 0x1;
    }
    i += 1;
  }
  // Receive parity bit
  uart_delay(last_clock_time);
  int in_pin_val = digitalRead(in_pin);
  last_clock_time = micros();
  // compare computed and received parity
  // if match, put value at end of r_buf
  if ((parity == in_pin_val) && !(r_buf_start == ((r_buf_end + 1) % buf_size))) {
    r_buf[r_buf_end] = B;
    r_buf_end = (r_buf_end + 1) % buf_size;
  }
  // get past this last bit so as not to trigger an early interrupt
  uart_delay(last_clock_time);
  interrupts();
}
