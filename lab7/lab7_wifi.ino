char ssid[] = "Brown-Guest";  // network SSID (name)
char pass[] = ""; // for networks that require a password
int status = WL_IDLE_STATUS;  // the WiFi radio's status

void setup_wifi() {
  // attempt to connect to WiFi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid); // WiFi.begin(ssid, pass) for password
    delay(10000);
  }
  Serial.println("Connected!");
  if (connect_to_webpage()) {
    Serial.println("fetched webpage");
  } else {
    Serial.println("failed to fetch webpage");
  }
  Serial.println();
}

bool connect_to_webpage() {
  if (client.connect("cs1600-lab7.herokuapp.com", 80)) {
    client.println("GET /color HTTP/1.1");
    client.println("Host: cs1600-lab7.herokuapp.com");
    client.println("Connection: close");
    client.println();
    return true;
  } else {
    Serial.println("Failed to fetch webpage");
    return false;
  }
}void read_webpage() {
    // Check for bytes to read
  int len = client.available();
  if (len == 0){
    return;
  }
  memset(buffer, 0x0, sizeof(buffer));
  int index = 0;
  while (client.available()) {
    char c = client.read();
    buffer[index] = c;
    index++;
  }
  if (index > 0) {
    // Serial.println();
    // Serial.write(buffer);
  }
  /*
   * LAB STEP 3b: change above code to only print RED, GREEN, or BLUE as it is read */

  char *color;
  const char lineBreak = '\n';
  char * token1 = strtok(buffer, &lineBreak);;

  token1 = strtok(NULL, &lineBreak);
  token1 = strtok(NULL, &lineBreak);
  token1 = strtok(NULL, &lineBreak);
  token1 = strtok(NULL, &lineBreak);
  token1 = strtok(NULL, &lineBreak);
  token1 = strtok(NULL, &lineBreak);
  token1 = strtok(NULL, &lineBreak);
  token1 = strtok(NULL, &lineBreak);




  const char firstToken = token1[0];

  
   /** LAB STEP 3c: change above code to send 'r', 'g', or 'b' via Serial1 */
  //Serial.println(firstToken);
  Serial1.write(firstToken);
  delay(500);


   /*
   * LAB STEP 4e: change above code to put 'r', 'b', or 'g' in s_buf for sending on UART
   */
  noInterrupts();
  s_buf[s_buf_end] = firstToken;
  s_buf_end = (s_buf_end + 1) % buf_size;
  interrupts();

}
