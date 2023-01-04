#include "lab5.h"

// TODO ask TAs:
// are comments good enough

// FSM variables
byte x, y;
orientation o;
int lxb, uxb;
int level, time_step, saved_clock, countdown;

state CURRENT_STATE;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Initialized Serial!");

  initialize_system();

  /*
   * LAB STEP 4b
   */
  //  calibrate();
  // Change to 7, 8, 9, 10 based on calibration
  cap_sensors[UP] = 8; // 540 -> 1700
  cap_sensors[RIGHT] = 7; // 2400 -> 3160
  cap_sensors[DOWN] = 10; // 290 -> 670
  cap_sensors[LEFT] = 9; // 430-> 700
  // Change based on calibration
  thresholds[UP] = 1000;
  thresholds[RIGHT] = 2700;
  thresholds[DOWN] = 500;
  thresholds[LEFT] = 600;

  /*
   * LAB STEP 4c
   */
//    test_calibration();
  
  /*
   * LAB STEP 6
   * Initialize all variables
   */
  CURRENT_STATE = (state) 1;
  lxb = 0;
  uxb = 16;
  xyo random_xyo = random_location_orientation(lxb, uxb);
  x = random_xyo.x;
  y = random_xyo.y;
  o = random_xyo.o;
  level = 0;
  time_step = 1000;
  saved_clock = millis();
  countdown = 3;
  // display something on the LCD

//  test_all_tests();
}

void loop() {
  update_inputs();
  CURRENT_STATE = update_fsm(CURRENT_STATE, millis(), num_buttons_pressed, last_button_pressed);
  delay(10);
}

/*
 * LAB STEP 5
 * determines whether the cursor is about to run into the wall
 * returns: true if the cursor is about to run into the wall, false otherwise
 */
bool facing_wall(byte x, byte y, byte o, int lxb, int uxb) {
  switch (o) {
    case UP:
      return y == 0;
    case DOWN:
      return y == 3;
    case LEFT:
      return x == lxb;
    case RIGHT:
      return x == (uxb - 1);
  }
}

/*
 * LAB STEP 5
 * calculates the new position of the cursor based on the input orientation
 * returns: a pair of bytes representing the new (x,y) coordinates
 */
xy move(byte x, byte y, byte o) {
  switch (o) {
    case UP:
      return {x, y-1};
    case DOWN:
      return {x, y+1};
    case LEFT:
      return {x-1, y};
    case RIGHT:
      return {x+1, y};
  }
}

/*
 * LAB STEP 7
 */
state update_fsm(state cur_state, long mils, int num_buttons, int last_button) {
  state next_state;
  switch(cur_state) {
  case sDISP_COUNTDOWN:
    if ((mils - saved_clock) >= 500 and countdown >= 0) { // transition 1-1
      display_level(level, countdown);
      countdown -= 1;
      saved_clock = mils;
      next_state = sDISP_COUNTDOWN;
    } else if ((mils - saved_clock > 500) && (countdown < 0) && !(facing_wall(x, y, o, lxb, uxb))) { // transition 1-2
      display_cursor(x, y, o, false, lxb, uxb);
      saved_clock = mils;
      reset_buttons();
      next_state = sWAIT_AFTER_ROT;
    } else if ((mils - saved_clock > 500) and (countdown < 0) and (facing_wall(x, y, o, lxb, uxb))) { // transition 1-6
      display_cursor(x, y, o, false, lxb, uxb);
      saved_clock = mils;
      reset_buttons();
      next_state = sWAIT_FOR_BUT;
    } else {
      next_state = sDISP_COUNTDOWN;
    }
    break;
  case sWAIT_AFTER_ROT:
    if ((mils - saved_clock) >= time_step and num_buttons > 0) { // transition 2-3 (a)
      display_cursor(x, y, o, false, lxb, uxb);
      lu shrunken = shrink_bounds(x, o, lxb, uxb);
      lxb = shrunken.l;
      uxb = shrunken.u;
      xy new_xy = move(x, y, o);
      x = new_xy.x;
      y = new_xy.y;
      next_state = sMOV;
    } else if ((mils - saved_clock) >= time_step and num_buttons == 0) { // transition 2-3 (b)
      display_cursor(x, y, o, false, lxb, uxb);
      xy new_xy = move(x, y, o);
      x = new_xy.x;
      y = new_xy.y;
      next_state = sMOV;
    } else {
      next_state = sWAIT_AFTER_ROT;
    }
    break;
  case sMOV:
    if (lxb < uxb) { // transition 3-4
      display_cursor(x, y, o, true, lxb, uxb);
      saved_clock = mils;
      reset_buttons();
      next_state = sWAIT_AFTER_MOV;
    } else if (lxb = uxb) { // transition 3-7
      display_game_over(level);
      next_state = sGAME_OVER;
    } else {
      next_state = sMOV;
    }
    break;
  case sWAIT_AFTER_MOV:
    if ((mils - saved_clock >= time_step) && (num_buttons > 0)) { // transition 4-5 (a)
      display_cursor(x, y, o, true, lxb, uxb);
      lu new_lu = shrink_bounds(x, o, lxb, uxb);
      uxb = new_lu.u;
      lxb = new_lu.l;
      o = random_turn(o, lxb, uxb);
      next_state = sROT;
    } else if ((mils - saved_clock >= time_step) && (num_buttons == 0)) { // transition 4-5 (b)
      display_cursor(x, y, o, true, lxb, uxb);
      o = random_turn(o, lxb, uxb);
      next_state = sROT;
    } else {
      next_state = sWAIT_AFTER_MOV;
    }
    break;
  case sROT:
    if ((lxb < uxb) && !(facing_wall(x, y, o, lxb, uxb))) { // transition 5-2
      display_cursor(x, y, o, false, lxb, uxb);
      saved_clock = mils;
      reset_buttons();
      next_state = sWAIT_AFTER_ROT;
    } else if ((lxb < uxb) && (facing_wall(x, y, o, lxb, uxb))) { // transition 5-6
      display_cursor(x, y, o, false, lxb, uxb);
      saved_clock = mils;
      reset_buttons();
      next_state = sWAIT_FOR_BUT;
    } else if (lxb = uxb) { // transition 5-7
      display_game_over(level);
      next_state = sGAME_OVER;
    } else {
      next_state = sROT;
    }
    break;
  case sWAIT_FOR_BUT:
    if ((mils - saved_clock >= time_step) && (num_buttons == 1) && (last_button == o)) { // transition 6-1
      display_level(level + 1, 3);
      countdown = 2;
      saved_clock = mils;
      level += 1;
      time_step *= 0.95;
      xyo random_xyo = random_location_orientation(lxb, uxb);
      x = random_xyo.x;
      y = random_xyo.y;
      o = random_xyo.o;
      next_state = sDISP_COUNTDOWN;
    } else if ((mils - saved_clock >= time_step) && !((num_buttons == 1) && (last_button == o))) { // transition 6-7
      display_game_over(level);
      next_state = sGAME_OVER;
    } else {
      next_state = sWAIT_FOR_BUT;
    }
    break;
  case sGAME_OVER: // no transitions from state 7
    next_state = sGAME_OVER;
    break;
  }
  return next_state;
}
