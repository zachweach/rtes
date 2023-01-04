/*
 * Initialize LCD based on Lab 5 schematic
 */
const int rs = 0, en = 1, d4 = 2, d5 = 3, d6 = 4, d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

/*
 * Initialize system: set random, initialize arrow characters, set up LCD
 */
void initialize_system() {
  randomSeed(analogRead(6));
  for (int j = 0; j < 8; j++) {
    lcd.createChar(j, tail_arrows[(int)j]);
    arrow_is_tail[j] = true;
  }
  last_button_pressed = (orientation) 7;
  num_buttons_pressed = -1;
  lcd.begin(16,2);
}

/*
 * Display "CALIBRATING" as a scroll on the LCD
 * Display capacitive sensor readings from pins 7-10 in serial monitor
 */
void calibrate() {
  int i = 0;
  bool left = true;
  lcd.setCursor(0,0);
  lcd.print("CALIBRATING...");
  int scroll_len = ((String)"CALIBRATING...").length();
  delay(400);
  while(true) {
    Serial.println("Capacitive sensing:");
    Serial.print("7:  ");
    Serial.println(s7.capacitiveSensorRaw(10));
    Serial.print("8:  ");
    Serial.println(s8.capacitiveSensorRaw(10));
    Serial.print("9:  ");
    Serial.println(s9.capacitiveSensorRaw(10));
    Serial.print("10: " );
    Serial.println(s10.capacitiveSensorRaw(10));
    Serial.println();
    if (i == scroll_len) {
      i = 0;
      lcd.clear();
      if (left) {
        lcd.setCursor(16-scroll_len, 1);
        lcd.print("CALIBRATING...");
      } else {
        lcd.setCursor(0, 0);
        lcd.print("CALIBRATING...");
      }
      left = !left;
    } else {
      if (left) {
        lcd.scrollDisplayLeft();
      } else {
        lcd.scrollDisplayRight();
      }
    }
    i += 1;
    delay(300);
  }
}

/*
 * Display which sensor has been pressed on the gamepad, assuming labels[] and thresholds[]
 * have been set up correctly
 */
void test_calibration() {
  String labels[4];
  // A hack, because enum "orientation" defines values from 0 to 3
  labels[UP] = "UP pressed";
  labels[LEFT] = "LEFT pressed";
  labels[DOWN] = "DOWN pressed";
  labels[RIGHT] = "RIGHT pressed";

  while(true) {
    lcd.clear();
    // A hack, because enum "orientation" defines values from 0 to 3
    for(int i = 0; i < 4; i++) {
      int cap_reading;
      switch(cap_sensors[i]) {
        case 7:
          cap_reading = s7.capacitiveSensorRaw(10);
          break;
        case 8:
          cap_reading = s8.capacitiveSensorRaw(10);
          break;
        case 9:
          cap_reading = s9.capacitiveSensorRaw(10);
          break;
        case 10:
          cap_reading = s10.capacitiveSensorRaw(10);
          break;
        default:
          break;        
      }
      if (cap_reading > thresholds[i]) {
        lcd.setCursor(0,0);
        lcd.print(labels[i]);
      }
    }
    delay(50);
  }
}

/*
 * Read in gamepad inputs and update num_buttons_pressed and last_button_pressed
 * We only increase "num_buttons_pressed" if the current button pressed is greater than the previous button pressed
 */
void update_inputs() {
  // A hack, because enum "orientation" defines values from 0 to 3
  for(int i = 0; i < 4; i++) {
    int cap_reading;
    switch(cap_sensors[i]) {
      case 7:
        cap_reading = s7.capacitiveSensorRaw(10);
        break;
      case 8:
        cap_reading = s8.capacitiveSensorRaw(10);
        break;
      case 9:
        cap_reading = s9.capacitiveSensorRaw(10);
        break;
      case 10:
        cap_reading = s10.capacitiveSensorRaw(10);
        break;
      default:
        break;        
    }
    if (cap_reading > thresholds[i]) {
      if ((int) last_button_pressed != i) {
        num_buttons_pressed += 1;
      }
      last_button_pressed = (orientation) i;
    }
  }
}

/*
 * Reset num_buttons_pressed to 0 and last_button_pressed to an undefined value
 */
void reset_buttons() {
  last_button_pressed = (orientation) 7;
  num_buttons_pressed = 0;
}

/*
 * Display level l and countdown c on the LCD
 */
void display_level(int l, int c) {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Level: ");
  lcd.print(l);
  lcd.setCursor(0,1);
  lcd.print("...");
  lcd.print(c);
  lcd.print("...");
}

/*
 * Display cursor at (x,y) with orientation o
 * with or without tail
 * also draw black squares for any LCD squares outside of [lxb, uxb) (the valid area)
 */
void display_cursor(byte x, byte y, orientation o, bool with_tail, int lxb, int uxb) {
  // A hack, because enum "orientation" defines values from 0 to 3 and we defined arrow indexing in this way
  // we are converting LCD y-coordinates (0 and 1) to game y-coordinates (0, 1, 2, and 3)
  int arrow_ind = o + 4 * (y%2);
  // only 8 custom characters are allowed to be stored at once, so we have to swap them out sometimes
  if (with_tail and !arrow_is_tail[arrow_ind]) {
    lcd.createChar(arrow_ind, tail_arrows[arrow_ind]);
    arrow_is_tail[arrow_ind] = true;
  } else if (!with_tail and arrow_is_tail[arrow_ind]) {
    lcd.createChar(arrow_ind, arrows[arrow_ind]);
    arrow_is_tail[arrow_ind] = false;
  }
  lcd.clear();
  lcd.setCursor(x, y/2);
  lcd.write(byte(arrow_ind));
  // character 255 is a black square
  for (int i = 0; i <= lxb - 1; i++) {
    lcd.setCursor(i, 0);
    lcd.write(byte(255));
    lcd.setCursor(i, 1);
    lcd.write(byte(255));
  }
  for (int i = 15; i >= uxb; i--) {
    lcd.setCursor(i, 0);
    lcd.write(byte(255));
    lcd.setCursor(i, 1);
    lcd.write(byte(255));
  }
}

#ifndef TESTING
/*
 * Returns a random (x,y) location and orientation o within [lxb, uxb)
 */
xyo random_location_orientation(int lxb, int uxb) {
  xyo retval;
  retval.x = random(lxb, uxb);
  retval.y = random(0, 3);
  // A hack, because enum "orientation" defines values from 0 to 3
  retval.o = (orientation) random(0, 3);
  return retval;
}

/*
 * Returns a new orientation based on two rules:
 *   1) new orientation must be at most 90 degrees difference from o
 *   2) Probabilities of turning to a latitudinal vs longitudinal direction are based on the
 *      aspect ratio of the current valid area
 */
orientation random_turn(orientation o, int lxb, int uxb) {
  if (o == UP or o == DOWN) {
    int r = random(0, 4 + 2 * (uxb - lxb));
    if (r < 4) {
      return o;
    } else if (r < 4 + (uxb - lxb)) {
      return LEFT;
    } else {
      return RIGHT;
    }
  } else {
    int r = random(0, 8 + (uxb - lxb));
    if (r < 4) {
      return UP;
    } else if (r < 8) {
      return DOWN;
    } else {
      return o;
    }
  }
}

/*
 * Makes sure the wall is not spawned over the cursor or over where the cursor is about to be
 * (unless lxb and uxb will end up being equal)
 */
lu shrink_bounds(byte x, orientation o, int lxb, int uxb) {
  lu retval = {lxb, uxb};
  if (x == uxb - 1 or (o == RIGHT and x == uxb - 2)) {
    retval.l += 1;
  } else if (x == lxb or (o == LEFT and x == lxb + 1)) {
    retval.u -= 1;
  } else {
    if (random(0,2) == 0) {
      retval.l += 1;
    } else {
      retval.u -= 1;
    }
  }
  return retval;
}
#else
/*
 * MOCKED UP FUNCTIONS FOR TESTING
 */
xyo random_location_orientation(int lxb, int uxb) {
  return {lxb, 0, UP};
}

orientation random_turn(orientation o, int lxb, int uxb) {
  switch(o) {
    case UP:
    return RIGHT;
    case RIGHT:
    return DOWN;
    case DOWN:
    return LEFT;
    case LEFT:
    return UP;
  }
}

lu shrink_bounds(byte x, orientation o, int lxb, int uxb) {
  return {lxb + 1, uxb};
}
#endif

/*
 * Display GAME OVER and the level l achieved
 */
void display_game_over(int l) {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("GAME OVER");
  lcd.setCursor(0,1);
  lcd.print("Level: ");
  lcd.print(l);
}
