/*
Life counter
June 1, 2017
Copyright (C) 2017 Adam Bielinski
All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.
*/

#include "Arduboy.h"
#include "counter_bitmaps.h"

// Make an instance of arduboy used for many functions
Arduboy arduboy;

// Random horribly organized variables here
char text[] = "asdf";
signed int score;
signed int diff;
int frame_counter;
bool pressing;
bool timer_pressed = false;
bool lost_game;
bool mute = false;
const int num_sub_scores = 6;
int cur_subscore = 0;
int subscores[num_sub_scores] = { 0 }; // all elements 0
byte ALL_DIR_BUTTONS = RIGHT_BUTTON + LEFT_BUTTON + UP_BUTTON + DOWN_BUTTON;
byte ALL_BUTTONS = RIGHT_BUTTON + LEFT_BUTTON + UP_BUTTON + DOWN_BUTTON + A_BUTTON + B_BUTTON;
static uint32_t StartTime; // Start time when timer is running
static uint32_t EndTime; // End time when timer stops
int TimerState = 0;

//Start Screen
bool intro=true;

//Pointers to Bitmaps
unsigned char* bitmap = title;
unsigned char* bitmap2 = backgroundimage;
unsigned char* bitmap3 = fontsheet;
unsigned char* bitmap4 = skull;


// not sure how correct this is
const byte start_game_sound [] = {
   0,166, 0x80, 0x81, 0x82, 0x90,0x53, 0x91,0x3F, 0,166, 0x81, 0x91,0x40, 0,166, 0x81, 0x91,0x3F, 0x92,0x42
};

void setup() {
  //initiate arduboy instance
  arduboy.beginNoLogo();
  
  // here we set the framerate to 30, we do not need to run at default 60 and
  // it saves us battery life.
  arduboy.setFrameRate(30);
  score = 40;
  pressing = false;
  lost_game = false;
  frame_counter = 0;
  diff = 0;
}


void splash(){
//Simple Splash Screen
  while(intro == true)
  {    
    //Check for Button Press
    //Standard
    if (arduboy.pressed(B_BUTTON)){
      score=20;
      intro = false;
    }
    //Commander
    if (arduboy.pressed(A_BUTTON)){
      score=40;
      intro = false;
    }
    
    //Intro Screen Draw
    arduboy.clear();
    arduboy.setCursor(0, HEIGHT-12);
    arduboy.print("Press A or B to Start");
    //Title Image
    arduboy.drawSlowXYBitmap(0,HEIGHT/2-20, title,128,20,1);
    arduboy.display();
  }
}

void loop() {

  splash();
  //Back to Splash
  if (arduboy.pressed(LEFT_BUTTON) & arduboy.pressed(RIGHT_BUTTON)){
    intro=true;
  }
  
  if (!(arduboy.nextFrame()))
    return;
    
  if (arduboy.notPressed(ALL_DIR_BUTTONS)){
    pressing = false;
  }else{
    if (!pressing ) {
      pressing = true;
      int local_diff = 0;
      // if the right button is pressed move 1 pixel to the right every frame
      if(arduboy.pressed(RIGHT_BUTTON)) {
        local_diff+=5;
      }
    
      // if the left button is pressed move 1 pixel to the left every frame
      if(arduboy.pressed(LEFT_BUTTON)) {
        local_diff-=5;
      }
    
      // if the up button or B button is pressed move 1 pixel up every frame
      if(arduboy.pressed(UP_BUTTON)) {
        local_diff++;
      }
    
      // if the down button or A button is pressed move 1 pixel down every frame
      if (arduboy.pressed(DOWN_BUTTON)) {
        local_diff--;
      }
      
      if (arduboy.pressed(B_BUTTON) && abs(local_diff) == 1){
        if (local_diff < 0){
          cur_subscore += 1;
        }else{
          cur_subscore -= 1;
        }
        cur_subscore = (cur_subscore + num_sub_scores) % num_sub_scores;
        arduboy.tunes.tone(330, 50);
        local_diff = 0;
      }else if (arduboy.pressed(B_BUTTON) && abs(local_diff) > 1){
        local_diff /= 5;
        subscores[cur_subscore] += local_diff;
      }else{
        if (frame_counter <= 0){
          diff = 0;
        }
        frame_counter = 40;//Time to Clear Tracker (approx 4 seconds)
        diff += local_diff;
        
        score += local_diff;
      }
      if (local_diff != 0){
        int temp_tone = 220;
        if (local_diff > 0){
          temp_tone = 440;
        }
        temp_tone += diff * 10;
        if (temp_tone < 50) temp_tone = 50;
        
        arduboy.tunes.tone(temp_tone, 100 + abs(local_diff * 20));
      }
    }
  }

  // we clear our screen to black
  arduboy.clear();

  //Background
  arduboy.drawSlowXYBitmap(0,0, backgroundimage,128,64,1);
   
  //Score Display
   if(frame_counter >= 0 && frame_counter != 5 && frame_counter != 10 && frame_counter != 15)
  {
    sprintf(text, "%i", score);
    if(score>=100){
      arduboy.setCursor(22,8);
      arduboy.setTextSize(5);
    }else if(score>=10){
      arduboy.setCursor(38,8);
      arduboy.setTextSize(5);
    }else if(score<10 && score>0){
      arduboy.setCursor(53,8);
      arduboy.setTextSize(5);
    }else if(score==0){
      //Dead. Paint me a jolly roger!
      arduboy.drawSlowXYBitmap(WIDTH/2-24,HEIGHT/2-32, skull,48,58,1);
    }else{
      //If we need negative life totals
      arduboy.setCursor(22,8);
      arduboy.setTextSize(5);
    }
    arduboy.print(text);  
  }else{
    arduboy.fillRect(10,10,WIDTH-20,HEIGHT-30,BLACK);
  }
  

  
  //Difference Tracker
  if(frame_counter>0)
  {
    sprintf(text, "%+i", diff);
    //arduboy.setCursor(8, 50);
    arduboy.setCursor(WIDTH-8-20, 50);
    arduboy.setTextSize(1);
    arduboy.print(text);
  }
  int box_w = 20;
  int box_h = 10;

  //Turn Timer
  if (arduboy.pressed(A_BUTTON)){
    if (timer_pressed == false){
      TimerState++;
      timer_pressed = true;
  
      if (TimerState == 1){
        StartTime = millis();
      }else if (TimerState == 2){
        EndTime = millis();
      }
    }
  }else{
    timer_pressed = false;
  }

  //Timer Logic
  if (TimerState == 0){
    // don't do anything
  }else if ((TimerState == 1) || (TimerState == 2)){
      long t;
      if (TimerState == 1){
        t = millis() - StartTime;
      }else if (TimerState == 2){
        t = EndTime  - StartTime;
      }
      int seconds = t/1000;
      int tens = (t/100)%10;
      //Draw Timer
      sprintf(text, "%d.%d", seconds, tens);
      //arduboy.setCursor(WIDTH-8-14, 50);
      arduboy.setCursor(12, 50);
      arduboy.setTextSize(1);
      arduboy.print(text);
  }else{
    TimerState = 0;
  }
  
  if (arduboy.pressed(A_BUTTON)){
    //arduboy.drawRect(WIDTH-box_w, cur_subscore * box_h, box_w, box_h, WHITE);
  }
  
  if (arduboy.pressed(B_BUTTON)){
    // draw an arrow at what we are selecting
    int arrow_width = 18;
    arduboy.fillRect(WIDTH - box_w - arrow_width, cur_subscore * box_h , arrow_width, box_h, BLACK);
    arduboy.drawLine(WIDTH - box_w - arrow_width, cur_subscore * box_h + box_h / 2, WIDTH - box_w - 2, cur_subscore * box_h + box_h / 2, WHITE);
    arduboy.fillTriangle (
      WIDTH - box_w - 1,               cur_subscore * box_h + box_h / 2,
      WIDTH - box_w - arrow_width * 2 / 3, cur_subscore * box_h,
      WIDTH - box_w - arrow_width * 2 / 3, cur_subscore * box_h + box_h / 2,
      WHITE);
    arduboy.fillTriangle (
      WIDTH - box_w -2,                cur_subscore * box_h + box_h / 2 ,
      WIDTH - box_w - arrow_width * 2 / 3, cur_subscore * box_h + box_h,
      WIDTH - box_w - arrow_width * 2 / 3, cur_subscore * box_h + box_h / 2,
      WHITE);
  }
  for (int i = 0; i < num_sub_scores; i++){
    if (subscores[i] != 0){
      sprintf(text, "%i", subscores[i]);
      arduboy.setCursor(WIDTH-box_w + 1, box_h * i + 1);
      arduboy.setTextSize(1);
      arduboy.print(text);
    }
     
  }
  
  if (frame_counter > 0){
    frame_counter--;
  }

  // then we finaly we tell the arduboy to display what we just wrote to the display.
  arduboy.display();
}
