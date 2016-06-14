#include "simpletools.h"                      
//#include "talk.h" 
#include "ping.h" 
#include "servo.h"

//#define sound_port 26 

int main(void)                                  
{
  //talk_Start(sound_port);
  servo_speed(16,0); //initialize at stationary speed=0
  servo_speed(17,0);
  servo_angle(19, 900);
  pause(5000);
  int br = 1;
  while(br){
  servo_speed(16,50); 
  servo_speed(17,-50);
  int cmDist = ping_cm(13);
  print("cmDist = %d\n", cmDist);           // Display distance
  if(cmDist < 10) {
    //freqout(4 , 1000 , 3000);
    //talk_Spell("turn left");
    //pause(2000);
    servo_speed(16 , -100); 
    servo_speed(16 , -100);
    pause(600);
    servo_angle(19, 0);
    br = 0;
  }}
  servo_speed(16,0); //initialize at stationary speed=0
  servo_speed(17,0);
  pause(2000);
  while(1){
  int cmDist = ping_cm(13);
  print("cmDist = %d\n", cmDist);           // Display distance
//int wL = input(7);                        // Left whisker -> wL variable
  int wL = input(8);                        // Right whisker -> wR variable
  print("wL = %d\n", wL);        // Display whisker variables
  if(cmDist < 20 && wL == 1){
    servo_speed(16,50); 
    servo_speed(17,-50);
  }      
  else if(cmDist < 20 && wL == 0){
    //freqout(4 , 1000 , 3000);
    //talk_Spell("turn left");
    //pause(2000);
    servo_speed(16,-50); 
    servo_speed(17,50);
    pause(300);
    servo_speed(16 , -100); 
    servo_speed(16 , -100);
    pause(600);
    servo_speed(16,50); 
    servo_speed(17,-50);
  }      
  else if(cmDist > 20 && wL == 1){
    //freqout(4 , 1000 , 3000);
    //talk_Say("turn right");
    //pause(2000);
    servo_speed(16,50); 
    servo_speed(17,-50);
    pause(900);
    servo_speed(17 , 100); 
    servo_speed(17 , 100);
    pause(600);
    servo_speed(16,50); 
    servo_speed(17,-50);
    pause(900);
    
  }      
  else{
    servo_speed(16,50); 
    servo_speed(17,-50);     
  }  
  }    
    return 0;
}

 /*
int main(void)                                  
{
//  talk_Start(sound_port);
  servo_speed(16,0); //initialize at stationary speed=0
  servo_speed(17,0);
  pause(3000);

  while(1){
    servo_speed(16,50); 
    servo_speed(17,-50);
    int cmDist = ping_cm(15);
    print("cmDist = %d\n", cmDist);           // Display distance
    pause(200);                               // Wait 1/5 second
    if(cmDist < 10) {
      servo_speed(16,0); 
      servo_speed(17,0);
      servo_angle(19, 0);                       // P19 servo to 90 degrees(right)
      int cmD_90 = ping_cm(15);
      print("cmD_90 = %d\n", cmD_90);           // Display distance                   
      pause(3000);  
      servo_angle(19, 1800);                      // P19 servo to 180 degrees(left)
      pause(3000); 
      int cmD_180 = ping_cm(15);
      pause(3000); 
      print("cmD_180 = %d\n", cmD_180);           // Display distance
      pause(3000);  
      servo_angle(19, 900);
      pause(3000);  
      if(cmD_90 > cmD_180){                       //turn right
        //freqout(4 , 1000 , 3000);
        //talk_Say("turn right");
        //pause(2000);
        servo_speed(17 , 100); 
        servo_speed(17 , 100);
        pause(1200);}        
      else{                                       //turn left
        //freqout(4 , 1000 , 3000);
        //talk_Spell("turn left");
        //pause(2000);
        servo_speed(16 , -100); 
        servo_speed(16 , -100);
        pause(1200);}                                     
    }  
    else continue;
    }
    return 0;
}*/