#include <Arduboy2.h>       // required to build for Arduboy
#include <ArduboyFX.h>      // required to access the FX external flash
#include "fxdata/fxdata.h"  // this file contains all references to FX data
#include <ArduboyTones.h>
#include "Tinyfont.h"


Arduboy2 arduboy;
Arduboy2Audio audio;
ArduboyTones sound(arduboy.audio.enabled);

Tinyfont tinyfont = Tinyfont(arduboy.sBuffer, Arduboy2::width(), Arduboy2::height());


uint8_t tilemapBuffer[32];

#define STAGE_GAME_TITLE    0
#define STAGE_PLAY_GAME     1
//#define STAGE_LEVEL_COMPLETED     2

byte game_stage = STAGE_GAME_TITLE;

bool enable_audio = true;
bool cheat_mode = false;
int Current_Level = 0;

uint24_t Level_Address = Level_0;

#define BOARD_GAME_WIDTH    255
#define BOARD_GAME_HEIGHT   176
#define SCREEN_GAME_WIDTH   127
#define CAMERA_RIGHT_OFFSET    20
#define CAMERA_LEFT_OFFSET     84
#define CAMERA_Y_NORMAL_OFFSET        20
#define CAMERA_Y_CROUNCHING_OFFSET  15
#define CAMERA_Y_LOOK_UP_OFFSET     40
#define PLAYER_WIDTH   24


#define FRAME_RATE 30
#define PLAYER_FRAME_RATE   2
#define PLAYER_SPEED        1

#define SOUND_NONE              0
#define SOUND_SHOOT             1
#define SOUND_GRENADE           2
#define SOUND_EXPLOSION         3
#define SOUND_TURRET            4
#define SOUND_LAUNCHER          5
#define SOUND_TELEPORT          6
#define SOUND_KILL_ENEMY        7
#define SOUND_PLAYER_IS_DEAD    8
#define SOUND_KILL_BULLET       9
#define SOUND_COLLISION         10

void play_sound(byte type){
    
    if (type == SOUND_SHOOT){sound.tone(50,50,50,20);}
    if (type == SOUND_EXPLOSION){sound.tone(100,500,50,200);}
    if (type == SOUND_GRENADE){sound.tone(200,300,300,50);}
    if (type == SOUND_TURRET){sound.tone(20,200,100,50);}
    if (type == SOUND_LAUNCHER){sound.tone(20,500,30,50);}
    if (type == SOUND_TELEPORT){sound.tone(50,500,50,200);}
    if (type == SOUND_KILL_ENEMY){sound.tone(75,50,75,200);}
    if (type == SOUND_PLAYER_IS_DEAD){sound.tone(200,50,50,200);}
    if (type == SOUND_KILL_BULLET){sound.tone(75,50,75,200);}
    if (type == SOUND_COLLISION){sound.tone(75,50,75,200);}
    
}




#define RIGHT   0
#define LEFT    1

#define FRAME_CROUNCHING    9
#define FRAME_WALKING       0
#define FRAME_JUMPING       7
#define FRAME_DEATH          10
#define JUMP_ITERATIONS     20
//#define AMMO                99  //ammo on the start
//#define LIVES               10
//#define GRENADES            10

struct Player {
   
    set(){
        is_crouching = false;
        is_directional_falling = false;
        is_directional_jumping = false;
        is_falling = false;
        is_jumping = false;
        is_walking = false;
        byte jump_iteration = 0;
        frame = 1;
        direction = RIGHT; 
        x = 0;
        
    }
    void is_dead(){
        
        if (!cheat_mode){
            frame = FRAME_DEATH;
            play_sound(SOUND_PLAYER_IS_DEAD);
            game_stage == STAGE_PLAY_GAME;
            prepare_level_parameters();
            //game_stage = STAGE_GAME_TITLE;
        }
    }
    float x = 0;
    float y = 176;
    float pos_y_1 = 0;  // startowa pozycja na podlodze 11
    float pos_y_2 = 0; // startowa pozycja na pietrze   12
    byte jump_iteration = 0;
    byte frame = 1; 

    byte direction = RIGHT;     //0 - right, 1 - left
    bool is_jumping = false;
    bool is_falling = false;
    bool is_walking = false; 
    bool is_crouching = false;
    bool is_directional_jumping = false; 
    bool is_directional_falling = false;
    bool is_super = false; 

    byte clear_parameters = true; //czy ma brac ustawienia pozycji z poprzedniego etapu czy od nowa je ustawi
   // float start_y;
     
} player;


#define MAX_BULLET_ON_SCREEN    5
struct Shoot {

    byte direction;     //0 - RIGHT or 1 - LEFT
    int x;
    byte y;
    int range;     //to calculate x +/- absolute_range
    byte speed = 5;
    byte absolute_range =      70;
    bool is_shooting = false;
    
} shoot[MAX_BULLET_ON_SCREEN];


struct Grenade {
    byte counter;
    byte direction;     //0 - RIGHT or 1 - LEFT
    int x;
    int y;
    int range;
    byte speed = 4;
    byte absolute_range = 120;
    bool is_shooting = false;
    
    
    
} grenade;


#define MAX_ENEMY   6
struct Enemy {
    
    bool is_alive;
    bool up;
    byte frame = 0;
    float angle = 0;
    float x;
    float y;
    float x2; //potrzebne jak robimy ruch po okregu
    float y2;
    
} Enemy[MAX_ENEMY];


#define MAX_PISTON 5
struct Piston {
    bool is_alive;
    int x;
    int y;
    byte frame = 0;
    byte counter = 0;
    byte timeout = 5;
} Piston[MAX_PISTON];


#define MAX_MINES 10
struct Mine {
    bool is_alive;
    int x;
    int y;

    
} Mine[MAX_MINES];


struct Camera {
    
    int offset = 0;
    int offset_y = CAMERA_Y_NORMAL_OFFSET;
    int x;
    int y;
    

}camera;

#define NO_OF_STATIC_ENEMIES    5
#define STATIC_ENEMIES_FIRST    150
#define STATIC_ENEMIES_LAST     160
#define COCOON      150
#define ROCKET      151
#define RADAR       152
#define FUNGUS      153
#define WAGGON      154
#define SHIP_LASER  155

#define PLAYER_START_POSITION   11
#define PLAYER_START_POSITION2  12 //gdy jest drugi poziom

#define PISTON_0    161
#define PISTON_1    162
#define PISTON_2    163
#define PISTON_3    164
#define PISTON_4    165

#define TURRET_0 169
#define TURRET_1 170
#define TURRET  170
#define DOUBLE_LAUNCHER  171
#define COMBINED_LANUCHER_TOP  172
#define COMBINED_LANUCHER_BOTTOM  173
#define INCUBATOR  174
#define EGGS    179 //egss without incubator


# define MINE_0     190
# define MINE_1     191
# define MINE_2     192
# define MINE_3     193
# define MINE_4     194
# define MINE_5     195
# define MINE_6     196
# define MINE_7     197 
# define MINE_8     198
# define MINE_9     199



#define MISSILE_TOWER  175
#define MISSILE_GUIDANCE  176

#define TELEPORT_0  181
#define TELEPORT_1  182

struct Enemies_Static {

    bool is_alive = false;
    bool explosion = false;
    byte explosion_iteration = 0;
    byte type;
    int x;
    int y;
    byte width = 0;
    byte height = 0;

    

}Enemies_Static[NO_OF_STATIC_ENEMIES];


struct Enemies_Incubator {

    bool is_alive = false;
    bool explosion = false;
    byte explosion_iteration = 0;
    int x;
    int y;
    

}Enemies_Incubator;

#define NO_OF_EGGS 8
struct Enemies_Egg {
    int x;
    int y;
    bool is_alive = false;
    
}Enemies_Eggs[NO_OF_EGGS];


byte teleport_frame =  0;
struct Teleport {

    bool is_alive = false;
    bool is_using = false;
    int x;
    int y;
    

}Teleport[2];

struct Enemies_Turret {
    void set(bool new_alive, int new_x, int new_y){
        is_alive = new_alive;
        x = new_x;
        y = new_y;
        bullet_x =  x;
        range = x - absolute_range;
        is_shooting == false;
    }
    bool is_alive = false;
    bool explosion = false;
    byte explosion_iteration = 0;  
    int x;
    int y;
    int bullet_x;
    
    int range;
    byte speed = 3;
    byte absolute_range = 150;
    bool is_shooting = false;
    
}Enemies_Turret[2];


struct Enemies_DoubleLauncher {
    bool is_alive = false;
    bool explosion = false;
    byte explosion_iteration = 0;  
    int x;
    int y;
    int bullet_1_x;
    int bullet_2_x;
    
    int range;
    byte speed = 3;
    byte absolute_range = 150;
    bool is_shooting_1 = false;
    bool is_shooting_2 = false;
    
}Enemies_DoubleLauncher;



struct Enemies_CombinedLauncher_Top {
    bool is_alive = false;
    bool explosion = false;
    byte explosion_iteration = 0;  
    int x;
    int y;
    int bullet_x;
    
    int range;
    byte speed = 3;
    byte absolute_range = 100;
    bool is_shooting = false;
    
}Enemies_CombinedLauncher_Top;


struct Enemies_CombinedLauncher_Bottom {
    bool is_alive = false;
    int x;
    int y;
    int bullet_x;
    
    int range;
    byte speed = 3;
    byte absolute_range = 100;
    bool is_shooting = false;

    
}Enemies_CombinedLauncher_Bottom;


struct Enemies_Missile_Guidance {
    bool is_alive = false;
    bool explosion = false;
    byte explosion_iteration = 0;  
    int x;
    int y;    
}Enemies_Missile_Guidance;


struct Enemies_Missle_Tower {
    bool is_alive = false;
    int x;
    int y;
}Enemies_Missle_Tower;


struct Missile {
    int x;
    int y;
    byte speed = 2;   
} Missile;


#define ENEMPY_TYPE_NONE        0
#define ENEMY_TYPE_BUBBLE       1
#define ENEMY_TYPE_INTERCEPTOR  2
#define ENEMY_TYPE_JELLYFISH    3
#define ENEMY_TYPE_LOUSE        4

// Parametry sinusoidy dla bubble
const float amplitude = 25.0;    // Amplituda
const float frequency = 0.1;    // Częstotliwość
const float phaseShift = 0.0;    // Przesunięcie fazowe

void draw_enemy(){
    
    uint8_t enemy_type_Buffer[1];

    FX::readDataArray(enemy_type_levels, Current_Level, 0, 1, enemy_type_Buffer, 1);
    
    if(enemy_type_Buffer[0] != 0){
        //set_enemy_y_position();
        
        
        for (byte x=0; x< MAX_ENEMY; x++){
            
            if (Enemy[x].is_alive){
                check_if_player_is_hit(Enemy[x].x+4, Enemy[x].y+4, 8, 8);
            }
            
                
                
                if (arduboy.everyXFrames(5)) {
                    Enemy[x].frame = Enemy[x].frame+1;
                    if (Enemy[x].frame>2) {Enemy[x].frame=0;}
                }

//////////////////                
                
                if (enemy_type_Buffer[0] == ENEMY_TYPE_BUBBLE){
                    if (Enemy[x].is_alive == true){
                        FX::drawBitmap(Enemy[x].x-camera.x, Enemy[x].y-camera.y, enemy_bubble_pic, Enemy[x].frame, 1);
                
                
                        if (arduboy.everyXFrames(1)) {
                            
                               // Enemy[x].x -= 1;
                               // Enemy[x].y = Enemy[x].y2 + amplitude * sin(frequency * (Enemy[x].x + 72) + phaseShift);
                            
                            if ((player.x > BOARD_GAME_WIDTH - PLAYER_WIDTH - 10) && (Enemy[x].x > BOARD_GAME_WIDTH)){
                                Enemy[x].x = Enemy[x].x;
                            }else{
                                Enemy[x].x -= 1;
                                Enemy[x].y = Enemy[x].y2 + amplitude * sin(frequency * (Enemy[x].x + 72) + phaseShift);
                            }
                        }
               
                        if (Enemy[x].x < -16.0) {
                            Enemy[x].is_alive = false;
                        }
                    
                    }
                     
                    if ((!Enemy[x].is_alive) && (player.x < BOARD_GAME_WIDTH - PLAYER_WIDTH - 10)){

                        Enemy[x].x = BOARD_GAME_WIDTH + (x*72);
                        Enemy[x].y2 = set_player_y_position(); //player.start_y;
                        Enemy[x].y = Enemy[x].y2;
                        Enemy[x].is_alive = true;
                    }
                                    
                }
 ////////////////////               
                
                if (enemy_type_Buffer[0] == ENEMY_TYPE_INTERCEPTOR){
                    if (Enemy[x].is_alive == true){
                        FX::drawBitmap(Enemy[x].x-camera.x, Enemy[x].y-camera.y, enemy_interceptor_pic, Enemy[x].frame, 1);
                    
                        if (arduboy.everyXFrames(1)) {
                            
                            if ((player.x > BOARD_GAME_WIDTH - PLAYER_WIDTH - 10) && (Enemy[x].x > BOARD_GAME_WIDTH)){
                                Enemy[x].x = Enemy[x].x;
                            }else{    
                                if (Enemy[x].x > BOARD_GAME_WIDTH / 2){ Enemy[x].x = Enemy[x].x - 1;}    
                                if (Enemy[x].x <= BOARD_GAME_WIDTH / 2){ Enemy[x].x = Enemy[x].x - 2;} 
                            }                                
                        }
                    }
                    if ((!Enemy[x].is_alive)  && (player.x < BOARD_GAME_WIDTH - PLAYER_WIDTH - 10)){
                        Enemy[x].x = BOARD_GAME_WIDTH + (x*120);
                       // Enemy[x].y = random(2) ? player.start_y-6 : player.start_y+16;
                        Enemy[x].y = random(2) ? set_player_y_position() -6 :set_player_y_position() +16;
                        
                        
                        Enemy[x].is_alive = true;
                    }
                
                
                
                }
//////////////////
                if (enemy_type_Buffer[0] == ENEMY_TYPE_JELLYFISH){
                    if (Enemy[x].is_alive == true){
                        FX::drawBitmap(Enemy[x].x-camera.x, Enemy[x].y-camera.y, enemy_jellyfish_pic, Enemy[x].frame, 1);
                    
                        if (arduboy.everyXFrames(1)) {
                             if ((player.x > BOARD_GAME_WIDTH - PLAYER_WIDTH - 10) && (Enemy[x].x > BOARD_GAME_WIDTH)){
                                Enemy[x].x = Enemy[x].x;
                            }else{  
                            
                                if (!Enemy[x].up) {
                                    Enemy[x].x = Enemy[x].x - 2;
                                
                                    if ((Enemy[x].x < BOARD_GAME_WIDTH / 3) && (Enemy[x].angle == PI/2)){
                                    Enemy[x].up = true;
                                    }   
                                }
                                                
                                if (Enemy[x].up){
                                    if (Enemy[x].x2 == 0){Enemy[x].x2 = Enemy[x].x;}
                                    if (Enemy[x].y2 == 0){Enemy[x].y2 = Enemy[x].y - 25;}
                                    Enemy[x].x = Enemy[x].x2 + 25 * cos(Enemy[x].angle);
                                    Enemy[x].y = Enemy[x].y2 + 25 * sin(Enemy[x].angle);
                                
                                    Enemy[x].angle += 0.1;
                                
                                    if (Enemy[x].angle >= 2.5*PI){Enemy[x].up = false;}
                                }
            
                            }
                        }
                    }
                    if ((!Enemy[x].is_alive)  && (player.x < BOARD_GAME_WIDTH - 75)){
                        //Enemy[x].x = BOARD_GAME_WIDTH + random(0,40)+(x*40);
                        Enemy[x].x = BOARD_GAME_WIDTH + (x*72);
                      //  Enemy[x].y = player.start_y+16;
                        Enemy[x].y =  set_player_y_position() + 16;
                        Enemy[x].up = false;
                        Enemy[x].x2 = 0;
                        Enemy[x].y2 = 0;
                        Enemy[x].angle = PI/2;
                        Enemy[x].is_alive = true;
                    }
                    
                }
///////////////////
                if (enemy_type_Buffer[0] == ENEMY_TYPE_LOUSE){
                    if (Enemy[x].is_alive == true){
                        FX::drawBitmap(Enemy[x].x-camera.x, Enemy[x].y-camera.y, enemy_louse_pic, Enemy[x].frame, 1);
                
                        if (arduboy.everyXFrames(1)) {
                            if ((player.x > BOARD_GAME_WIDTH - PLAYER_WIDTH - 10) && (Enemy[x].x > BOARD_GAME_WIDTH)){
                                Enemy[x].x = Enemy[x].x;
                            }else{  
                            
                            
                            
                                Enemy[x].x = Enemy[x].x - 1;
                                
                                if (Enemy[x].up == true){Enemy[x].y = Enemy[x].y - 1;}
                                if (Enemy[x].up == false){Enemy[x].y = Enemy[x].y + 1;}
                                
                             //   if (Enemy[x].y < player.start_y - 20){Enemy[x].up = false; }
                             //   if (Enemy[x].y > player.start_y + 15){Enemy[x].up = true; }
                               
                                if (Enemy[x].y < set_player_y_position() - 20){Enemy[x].up = false; }
                                if (Enemy[x].y > set_player_y_position() + 15){Enemy[x].up = true; }    
                                
                            }
                        }
                    }
                    if ((!Enemy[x].is_alive) && (player.x < BOARD_GAME_WIDTH - 75)){
                        Enemy[x].x = BOARD_GAME_WIDTH + (x*72);
                       // Enemy[x].y = random(player.start_y - 30, player.y + 20);
                        Enemy[x].y = random(set_player_y_position() - 30,set_player_y_position() + 20);
                         
                        Enemy[x].is_alive = true;
                    }
                    
                }
                    
                if (Enemy[x].x < 0) { Enemy[x].is_alive = false; }
            
       
        }
    }
    
}

    float pos_1 = 0;
    float pos_2 = 0; //pozycja na pietrze    

void initialize_enemies_static(){
    
    for (byte x=0; x<MAX_ENEMY; x++){
        Enemy[x].is_alive = false;
    }
    
    for (byte x=0; x< NO_OF_STATIC_ENEMIES; x++){
        Enemies_Static[x].is_alive = false;  
        Enemies_Static[x].explosion = false;  
    }
    
    for (byte x=0; x< MAX_PISTON; x++){
        Piston[x].is_alive = false;    
    }
    
    for (byte x=0; x< MAX_MINES; x++){
        Mine[x].is_alive = false;    
    }
    
    for(byte i=0; i< NO_OF_EGGS;i++){
        Enemies_Eggs[i].is_alive = false;
    }
    
    for (byte i=0; i<2; i++){
        Enemies_Turret[i].is_alive = false;
        Enemies_Turret[i].explosion = false;
        Teleport[i].is_alive = false;
    }
 
    Enemies_DoubleLauncher.is_alive = false;

    Enemies_CombinedLauncher_Top.is_alive = false;
    Enemies_CombinedLauncher_Top.explosion = false;

    Enemies_CombinedLauncher_Bottom.is_alive = false;

    Enemies_Incubator.is_alive = false;
    Enemies_Incubator.explosion = false;
        
    Enemies_Missle_Tower.is_alive = false;
       
    Enemies_Missile_Guidance.is_alive = false;
    Enemies_Missile_Guidance.explosion = false;
    

    
    

   // Serial.print("player.y: "); Serial.print(player.y);  
    
    for (int i=0; i<22; i++){
        FX::readDataArray(Level_Address + (32 * i), 0, 0, 32, tilemapBuffer, 32);
        for(int j=0; j<32; j++){ 
            if ((tilemapBuffer[j] >= STATIC_ENEMIES_FIRST) && (tilemapBuffer[j] <= STATIC_ENEMIES_LAST)){
                for (byte x=0; x< NO_OF_STATIC_ENEMIES; x++){
                    if ( Enemies_Static[x].is_alive == false ){
                        Enemies_Static[x].is_alive = true;
                        Enemies_Static[x].x = j*8;
                        Enemies_Static[x].y = i*8;
                        Enemies_Static[x].type = tilemapBuffer[j];
                        x = NO_OF_STATIC_ENEMIES; 
                    }
                }
                
            }
            if ((tilemapBuffer[j] == TURRET_0)){
                Enemies_Turret[0].set(true,j*8,i*8);                
            }
            
            if ((tilemapBuffer[j] == TURRET_1)){
                Enemies_Turret[1].set(true,j*8,i*8);
            }
            
            
            if ((tilemapBuffer[j] == DOUBLE_LAUNCHER)){
                Enemies_DoubleLauncher.is_alive = true;
                Enemies_DoubleLauncher.x = j*8;
                Enemies_DoubleLauncher.y = i*8; 
              
                Enemies_DoubleLauncher.bullet_1_x =  Enemies_DoubleLauncher.x;
                Enemies_DoubleLauncher.bullet_2_x =  Enemies_DoubleLauncher.x;
                Enemies_DoubleLauncher.range = Enemies_DoubleLauncher.x - Enemies_DoubleLauncher.absolute_range;
                Enemies_DoubleLauncher.is_shooting_1 == false;
                Enemies_DoubleLauncher.is_shooting_2 == false;

                
            }
            
            if ((tilemapBuffer[j] == COMBINED_LANUCHER_TOP)){
                Enemies_CombinedLauncher_Top.is_alive = true;
                Enemies_CombinedLauncher_Top.x = j*8;
                Enemies_CombinedLauncher_Top.y = i*8; 
                
                Enemies_CombinedLauncher_Top.bullet_x =  Enemies_CombinedLauncher_Top.x;
                Enemies_CombinedLauncher_Top.range = Enemies_CombinedLauncher_Top.x - Enemies_CombinedLauncher_Top.absolute_range;
                Enemies_CombinedLauncher_Top.is_shooting == false;
                
            }
            
            if ((tilemapBuffer[j] == COMBINED_LANUCHER_BOTTOM)){
                Enemies_CombinedLauncher_Bottom.is_alive = true;
                Enemies_CombinedLauncher_Bottom.x = j*8;
                Enemies_CombinedLauncher_Bottom.y = i*8; 
                
                Enemies_CombinedLauncher_Bottom.bullet_x =  Enemies_CombinedLauncher_Top.x;
                Enemies_CombinedLauncher_Bottom.range = Enemies_CombinedLauncher_Top.x - Enemies_CombinedLauncher_Top.absolute_range;
                Enemies_CombinedLauncher_Bottom.is_shooting == false;
                
            }
            
            if ((tilemapBuffer[j] == INCUBATOR) || (tilemapBuffer[j] == EGGS)) {
                Enemies_Incubator.is_alive = true;
                Enemies_Incubator.x = j*8;
                Enemies_Incubator.y = i*8; 
                
                for(byte i=0; i<NO_OF_EGGS;i++){
                    Enemies_Eggs[i].is_alive = true;
                    Enemies_Eggs[i].x = Enemies_Incubator.x+8;
                    Enemies_Eggs[i].y = Enemies_Incubator.y+8;
                }
                
                if (tilemapBuffer[j] == EGGS) {Enemies_Incubator.is_alive = false;}
            
            }
            
            if ((tilemapBuffer[j] == TELEPORT_0)){
                Teleport[0].is_alive = true;
                Teleport[0].x = j*8;
                Teleport[0].y = i*8;              
            }
            
            if ((tilemapBuffer[j] == TELEPORT_1)){
                Teleport[1].is_alive = true;
                Teleport[1].x = j*8;
                Teleport[1].y = i*8;                 
            }
            
            if ((tilemapBuffer[j] == MISSILE_TOWER)){
                Enemies_Missle_Tower.is_alive = true;
                Enemies_Missle_Tower.x = j*8;
                Enemies_Missle_Tower.y = i*8;                 
            }
            
            if ((tilemapBuffer[j] == MISSILE_GUIDANCE)){
                Enemies_Missile_Guidance.is_alive = true;
                Enemies_Missile_Guidance.x = j*8;
                Enemies_Missile_Guidance.y = i*8;   
                Missile.x = BOARD_GAME_WIDTH;
                Missile.y = BOARD_GAME_HEIGHT;  
            }
            
            
           
            for (byte x=0; x<MAX_PISTON; x++){
                if ((tilemapBuffer[j] == x + PISTON_0)){
                Piston[x].is_alive = true;
                Piston[x].x = j*8;
                Piston[x].y = i*8;      
                Piston[x].frame = random(0,8);
                         
                }
            }
            
            
            for (byte x=0; x<MAX_MINES; x++){
                if ((tilemapBuffer[j] == x + MINE_0)){
                Mine[x].is_alive = true;
                Mine[x].x = j*8;
                Mine[x].y = i*8;      

                         
                }
            }
                
                
           
            if (tilemapBuffer[j] == PLAYER_START_POSITION) {
                player.pos_y_1 = i*8;
              //  pos_1 = i*8;
            }
            if (tilemapBuffer[j] == PLAYER_START_POSITION2) {
                player.pos_y_2 = i*8;
               // pos_2 = i*8;
            }
            
        }
  
        
    }
          
        /*  
            if (player.y <= pos_2){
                    
                player.start_y = pos_2;
                
            }  else {
                player.start_y = pos_1;
                
            }
            */
            //player.y = player.start_y;
            player.y = set_player_y_position();
            player.x = 0;
            
           // Serial.print("| pos_1: "); Serial.print(pos_1);
         //   Serial.print("| pos_2: "); Serial.println(pos_2);
           // Serial.print("| player.y: "); Serial.println(player.y);  
}

//float set_enemy_y_position(){
    
float set_player_y_position(){
    if (player.y <= player.pos_y_2){
        return player.pos_y_2;
       // player.start_y = player.pos_y_2;
    }  else {
       // player.start_y = player.pos_y_1;
        return player.pos_y_1;
    }

}



void check_keys(){
    
        if (arduboy.pressed(LEFT_BUTTON)) {
            if ((!player.is_jumping) && (!player.is_falling) && (!player.is_directional_jumping) && (!player.is_directional_falling))
           {
            player.direction = LEFT;
            player.is_walking = true;
        }
        }            
                    
        if (arduboy.justReleased(LEFT_BUTTON)) {
            player.is_walking = false;        
        }  
        
   
        if (arduboy.pressed(RIGHT_BUTTON)) {
            if ((!player.is_jumping) && (!player.is_falling) && (!player.is_directional_jumping) && (!player.is_directional_falling))
            {
                player.direction = RIGHT;
                player.is_walking = true;
            } 
        }
                             
        if (arduboy.justReleased(RIGHT_BUTTON)) {
            player.is_walking = false;

        }
        
        if (arduboy.pressed(DOWN_BUTTON)) {
            player.frame = FRAME_CROUNCHING;
            player.is_walking = false;
            player.is_crouching = true;

            if (camera.offset_y > CAMERA_Y_CROUNCHING_OFFSET){
                camera.offset_y = camera.offset_y - 1;
            }
        
        }            
                    
        if (arduboy.justReleased(DOWN_BUTTON)) {
            player.frame = FRAME_WALKING;
            player.is_crouching = false;  
            camera.offset_y = CAMERA_Y_NORMAL_OFFSET;
        }  
        

        
        
        if (arduboy.justPressed(UP_BUTTON)) {
            player.frame = FRAME_JUMPING;
            if ((!player.is_walking) && (!player.is_jumping) && (!player.is_falling) && (!player.is_directional_jumping) && (!player.is_directional_falling)){
                player.is_jumping = true;
                player.is_walking = false;
                player.jump_iteration = 0;
            }
            
            if ((player.is_walking) && (!player.is_jumping) && (!player.is_falling) && (!player.is_directional_jumping) && (!player.is_directional_falling)){
                player.is_jumping = false;
                player.is_walking = false;
                player.is_directional_jumping = true;
                player.jump_iteration = 0;
            }
            



        
         
          }
        
        if (arduboy.justPressed(B_BUTTON)) {
            player.clear_parameters = false;
            game_stage = STAGE_GAME_TITLE;
        }     
          
    if (arduboy.justPressed(A_BUTTON)){
        grenade.counter = 0;
        for (byte i=0; i<MAX_BULLET_ON_SCREEN; i++){
                if (!shoot[i].is_shooting){    
                    shoot[i].is_shooting = true;
                    
                    play_sound(SOUND_SHOOT);
                    shoot[i].direction = player.direction;
                    if (shoot[i].direction == RIGHT){
                        shoot[i].x = player.x + 17;
                        shoot[i].range = shoot[i].x + shoot[i].absolute_range;
                    }
                    if (shoot[i].direction == LEFT){
                        shoot[i].x = player.x - 1;
                        shoot[i].range = shoot[i].x - shoot[i].absolute_range;
           
                    }
                    if (player.is_crouching){shoot[i].y = player.y + 19;}
                    else {shoot[i].y = player.y + 14;}
                    break;
                }
            }
    

          
        }
        
    if (arduboy.pressed(A_BUTTON)){   
        if (arduboy.everyXFrames(FRAME_RATE/2)) {

            grenade.counter = grenade.counter + 1;
        }
        if ((grenade.counter > 1) && (grenade.is_shooting == false)){
                
                grenade.direction = player.direction;
                if (grenade.direction == RIGHT){
                    grenade.x = player.x+5;
                    grenade.range = grenade.x + grenade.absolute_range;
                    if (player.is_crouching){grenade.y = player.y + 5;}
                    else {grenade.y = player.y;}
                    grenade.is_shooting = true;
                    play_sound(SOUND_GRENADE);
                }
                
                if (grenade.direction == LEFT){
                    grenade.x = player.x+11;
                    grenade.range = grenade.x - grenade.absolute_range;
                    if (player.is_crouching){grenade.y = player.y + 5;}
                    else {grenade.y = player.y;}
                    grenade.is_shooting = true;
                    play_sound(SOUND_GRENADE);
                }
            
        }
    }
      
    
     
    
}


void player_movement(){
    
    
     if ((!solid_below()) && (!player.is_jumping) && (!player.is_directional_jumping)){
        player.is_walking = false;
        player.is_falling = true;
     }
     

     

    if (arduboy.everyXFrames(PLAYER_FRAME_RATE)) {
        if(player.is_walking){
            player.frame = player.frame+1;
            if (player.frame>8) {player.frame=0;}
        }
        
        teleport_frame = teleport_frame + 1;
        if (teleport_frame>7){teleport_frame = 0;}
    
    }
    
    if (player.is_walking) {
        
            if ((player.direction == RIGHT) && (!player_colision_horizontal())) {
            if (arduboy.everyXFrames(PLAYER_SPEED)) { 
                    if ((player.x < 256)) {player.x = player.x + 1;
                       // camera.x = camera.x + 1;
                    //if ((player.x>60) && (player.x<=190)) {}}
                }
                    
            }

            }
            if ((player.direction == LEFT) && (!player_colision_horizontal())){
                if (arduboy.everyXFrames(PLAYER_SPEED)){ 
                    if (player.x > 0) {player.x = player.x - 1; 
                      //  camera.x = camera.x - 1;
                //    if ((player.x>=60) && (player.x<190)) {;}}
                }
            }
                
            }
    } 
    
    if (player.is_jumping) {
        if (player.is_jumping){
            player.is_walking = false;
        if ((player.jump_iteration < JUMP_ITERATIONS) && (!player_collision_vertical())) {
            if (arduboy.everyXFrames(PLAYER_SPEED)) { 
                player.y = player.y - 1;
                player.jump_iteration++;
            }
        }else {
            player.is_falling = true;
            player.is_jumping = false; 
            
            }
        }
    } 
    
    
    if (player.is_directional_jumping) {
         player.is_walking = false;
         if ((player.jump_iteration < JUMP_ITERATIONS) && (!player_collision_vertical())){       
            if ((arduboy.everyXFrames(PLAYER_SPEED))) { 
                player.y = player.y - 1;
                player.jump_iteration++;
            }
        
        
             if (arduboy.everyXFrames(PLAYER_SPEED)) { 
                if((player.direction == RIGHT) && (!player_colision_horizontal())){
                    if ((player.x < 256)) {player.x = player.x + 1.5;}
                } else if ((player.direction == LEFT) && (!player_colision_horizontal())){
                    if (player.x > 0) {player.x = player.x - 1.5;}
                }
            }
            
        }else {

            player.jump_iteration = 0;
            player.is_directional_falling = true;
            //player.is_falling = true;
            player.is_directional_jumping = false; 
            
        }
    }
    
    if (player.is_directional_falling){
        player.is_walking = false;
       // if ((player.jump_iteration < JUMP_ITERATIONS) && (!solid_below()) && (!player_collision_vertical())) { 
        if ((player.jump_iteration < JUMP_ITERATIONS) && (!solid_below())) {      
            
            if (arduboy.everyXFrames(PLAYER_SPEED)) {
                player.y = player.y + 1;
                player.jump_iteration++;
            }
            if (arduboy.everyXFrames(PLAYER_SPEED)) {
                if ((player.direction == RIGHT) && (!player_colision_horizontal())){
                    
                    if ((player.x < 256)) {player.x = player.x + 1.5;}
               
               
                } else if ((player.direction == LEFT) && (!player_colision_horizontal())){
                    if (player.x > 0) {player.x = player.x - 1.5;}
                }

            }
        }
        else {
            player.jump_iteration = 0;
            player.is_directional_falling = false;
            player.is_falling = true;
        }
    }
    
    
    
    if (player.is_crouching) {
        
        //    if (player.direction == RIGHT) {
            if (arduboy.everyXFrames(PLAYER_SPEED)) { 
                   // player.y = player.y + 1;
                    
                    //if ((player.x>60) && (player.x<=190)) {}}
                }
                    
            }     
            
            
       if (player.is_falling){
        if ((!solid_below()) ) {
            if (arduboy.everyXFrames(PLAYER_SPEED)) {
                player.y = player.y + 1;
                player.is_walking = false;

            }
        } else {
            player.is_falling = false;
        }
    }

}

void shooting(){


    for(byte i=0; i<MAX_BULLET_ON_SCREEN; i++){
        if (shoot[i].is_shooting){
            if(shoot[i].direction == RIGHT){
                shoot[i].x = shoot[i].x + shoot[i].speed;
                if (shoot[i].x > shoot[i].range) { 
                    shoot[i].is_shooting = false;
                    }
            }
            if (shoot[i].direction == LEFT){
                shoot[i].x = shoot[i].x - shoot[i].speed;
                if (shoot[i].x < shoot[i].range) { 
                    shoot[i].is_shooting = false;
                }
            }
            
            FX::drawBitmap(shoot[i].x-camera.x, shoot[i].y-camera.y, shoot_pic, 0, dbmWhite);
            if(player.is_super){
                FX::drawBitmap(shoot[i].x-camera.x, shoot[i].y-camera.y+6, shoot_pic, 0, dbmWhite);
                
            }
     
     
        
    //sprawdzenia czy strzal trafil w rozne obiekty
    
    byte super_shoot = 0;
    if (player.is_super){super_shoot = 6;}
    else  {super_shoot = 0;}
    
            for (byte x=0; x< MAX_ENEMY; x++){
                if (Enemy[x].is_alive){
                     if ((arduboy.collide(Rect(shoot[i].x, shoot[i].y-2, 6, 5), Rect(Enemy[x].x+2, Enemy[x].y-3, 12, 14))) || 
                     (arduboy.collide(Rect(shoot[i].x, shoot[i].y+super_shoot-2, 6, 5), Rect(Enemy[x].x+2, Enemy[x].y-3, 12, 14)))){
                        Enemy[x].is_alive = false;
                        shoot[i].is_shooting = false;
                        play_sound(SOUND_KILL_ENEMY);
                     }
                }
            }
            
             if (Enemies_DoubleLauncher.is_alive == true){
                 
                 if ((Enemies_DoubleLauncher.is_shooting_1) 
                 && ((arduboy.collide(Rect(shoot[i].x, shoot[i].y-3, 6, 3), Rect(Enemies_DoubleLauncher.bullet_1_x, Enemies_DoubleLauncher.y+2, 2, 3))) 
                 || (arduboy.collide(Rect(shoot[i].x, shoot[i].y+super_shoot, 6, 1), Rect(Enemies_DoubleLauncher.bullet_1_x, Enemies_DoubleLauncher.y+2, 2, 3))))){
                    Enemies_DoubleLauncher.is_shooting_1 = false;
                    shoot[i].is_shooting = false;
                    play_sound(SOUND_KILL_BULLET);
                 }
                 
                 if ((Enemies_DoubleLauncher.is_shooting_2) 
                 && ((arduboy.collide(Rect(shoot[i].x, shoot[i].y, 6, 3), Rect(Enemies_DoubleLauncher.bullet_2_x, Enemies_DoubleLauncher.y+10, 2, 3))) 
                 || (arduboy.collide(Rect(shoot[i].x, shoot[i].y+super_shoot, 6, 1), Rect(Enemies_DoubleLauncher.bullet_2_x, Enemies_DoubleLauncher.y+10, 2, 3))))){
                    Enemies_DoubleLauncher.is_shooting_2 = false;
                    shoot[i].is_shooting = false;
                    play_sound(SOUND_KILL_BULLET);
                 }
             }
             
            if (Enemies_CombinedLauncher_Bottom.is_alive == true){
                if ((Enemies_CombinedLauncher_Bottom.is_shooting)
                && ((arduboy.collide(Rect(shoot[i].x, shoot[i].y-3, 6, 3), Rect(Enemies_CombinedLauncher_Bottom.bullet_x, Enemies_CombinedLauncher_Bottom.y+2, 3, 3))) 
                || (arduboy.collide(Rect(shoot[i].x, shoot[i].y+super_shoot, 6, 1), Rect(Enemies_CombinedLauncher_Bottom.bullet_x, Enemies_CombinedLauncher_Bottom.y+2, 3, 3))))){
                    Enemies_CombinedLauncher_Bottom.is_shooting = false;
                    shoot[i].is_shooting = false;
                    play_sound(SOUND_KILL_BULLET);
                 } 
            
            }
            
            for(byte j=0; j< NO_OF_EGGS;j++){
                if (Enemies_Incubator.is_alive != true){
                    if (Enemies_Eggs[j].is_alive == true){
                        if ((arduboy.collide(Rect(shoot[i].x, shoot[i].y-3, 6, 3), Rect(Enemies_Eggs[j].x, Enemies_Eggs[j].y, 8, 8))) || 
                        (arduboy.collide(Rect(shoot[i].x, shoot[i].y+super_shoot, 6, 1), Rect(Enemies_Eggs[j].x, Enemies_Eggs[j].y, 8, 8)))){
                            Enemies_Eggs[j].is_alive = false;
                            shoot[i].is_shooting = false;
                             play_sound(SOUND_KILL_ENEMY);
                        }  
                    }
                }
           } 
            
            
            
                   
        }
        
        
        
    }
    
    
    
    

}

void  launch_grenade(){
    
    
    if (grenade.is_shooting == true) {
        if(grenade.direction == RIGHT){
        
            
            FX::drawBitmap(grenade.x - camera.x, grenade.y-camera.y, grenade_pic, grenade.direction, 1);
            
           
            
            grenade.x = grenade.x + grenade.speed;
            
            if (grenade.x < grenade.range - (grenade.absolute_range / 2)){
                FX::drawBitmap(grenade.x-8 - camera.x, grenade.y-camera.y, grenade_trace, 0, 0);
                
            }
            
            if (grenade.x < grenade.range - (grenade.absolute_range - 20)){
                grenade.y = grenade.y - 2;
            }
            
            if (grenade.x > grenade.range - 30){
                grenade.y = grenade.y + 2;
            }
            
            if (grenade.x > grenade.range) { 
                grenade.is_shooting = false;
            }
        
        }
        
        if(grenade.direction == LEFT){
        
            FX::drawBitmap(grenade.x - camera.x, grenade.y-camera.y, grenade_pic, grenade.direction, 1);
            grenade.x = grenade.x - grenade.speed;
            
             if (grenade.x > grenade.range + (grenade.absolute_range / 2)){
                FX::drawBitmap(grenade.x+8 - camera.x, grenade.y-camera.y, grenade_trace, 0, 0);
                
            }
            
            
            if (grenade.x > grenade.range + (grenade.absolute_range - 20)){
                grenade.y = grenade.y - 2;
            }
            
            if (grenade.x < grenade.range + 30){
                grenade.y = grenade.y + 2;
            }
            
            if (grenade.x < grenade.range) { 
                grenade.is_shooting = false;
            }
        
        }

    
    }
    
 
     for (byte x=0; x< NO_OF_STATIC_ENEMIES; x++){
        if ( Enemies_Static[x].is_alive == true){
        //    if (arduboy.collide(Rect(grenade.x, grenade.y, 8, 6), Rect( Enemies_Static[x].x,  Enemies_Static[x].y, 40, 60))){
            if (arduboy.collide(Rect(grenade.x, grenade.y, 8, 6), Rect( Enemies_Static[x].x,  Enemies_Static[x].y, Enemies_Static[x].width, Enemies_Static[x].height))){   
                play_sound(SOUND_EXPLOSION);
              
                Enemies_Static[x].is_alive = false;
                Enemies_Static[x].explosion = true;
                grenade.is_shooting = false;
            }
        }
    }
    
    for (byte i=0; i<2; i++){
    if ( Enemies_Turret[i].is_alive == true){
            if (arduboy.collide(Rect(grenade.x, grenade.y, 8, 6), Rect( Enemies_Turret[i].x+8,  Enemies_Turret[i].y, 40, 40))){
                play_sound(SOUND_EXPLOSION);
                Enemies_Turret[i].is_alive = false;
                Enemies_Turret[i].explosion = true;
                grenade.is_shooting = false;
            }
        }
    }
        
    if ( Enemies_CombinedLauncher_Top.is_alive == true){
            if (arduboy.collide(Rect(grenade.x, grenade.y, 8, 6), Rect( Enemies_CombinedLauncher_Top.x+8,  Enemies_CombinedLauncher_Top.y, 40, 40))){
                play_sound(SOUND_EXPLOSION);
                Enemies_CombinedLauncher_Top.is_alive = false;
                Enemies_CombinedLauncher_Top.explosion = true;
                grenade.is_shooting = false;
            }
        }
        
    if ( Enemies_Incubator.is_alive == true){
            if (arduboy.collide(Rect(grenade.x, grenade.y, 8, 6), Rect( Enemies_Incubator.x+8,  Enemies_Incubator.y, 40, 40))){
                play_sound(SOUND_EXPLOSION);
                Enemies_Incubator.is_alive = false;
                Enemies_Incubator.explosion = true;
                grenade.is_shooting = false;
            }
        }
        
        if ( Enemies_Missile_Guidance.is_alive == true){
            if (arduboy.collide(Rect(grenade.x, grenade.y, 8, 6), Rect( Enemies_Missile_Guidance.x+8,  Enemies_Missile_Guidance.y, 40, 40))){
                play_sound(SOUND_EXPLOSION);
                Enemies_Missile_Guidance.is_alive = false;
                Enemies_Missile_Guidance.explosion = true;
                grenade.is_shooting = false;
            }
        }






    
}

// formm 0 to 4 - for Explosion Enemies static
#define EXPLOSION_COMBINED_LAUNCHER     5
#define EXPLOSION_INCUBATOR             6
#define EXPLOSION_MISSILE_TOWER         7
#define EXPLOSION_TURRET                8
#define EXPLOSION_TURRET_0              8
#define EXPLOSION_TURRET_1              9

struct Explosion {   
   int x;
   int y;
   bool explosion = false;
   byte explosion_iteration = 0;   

}Explosion[10];

bool draw_explosion(byte no, int x, int y){

    if (Explosion[no].explosion == false){
        Explosion[no].x = x;
        Explosion[no].y = y-16;
        Explosion[no].explosion = true;
        Explosion[no].explosion_iteration = 0;
    }  
    


    for (byte i=0; i<4;i++){         
        
        FX::drawBitmap(Explosion[no].x + 20 - (i) - random(0,3) - Explosion[no].explosion_iteration - camera.x, Explosion[no].y + 8 +(i*10)+ random(0,3)-camera.y, explosion_pic, random(0,4), 1);
        FX::drawBitmap(Explosion[no].x + 20 + (i) + random(0,3) + Explosion[no].explosion_iteration - camera.x, Explosion[no].y + 8 +(i*10)+random(0,3)-camera.y, explosion_pic, random(0,4), 1);
    
    }
            
    
    Explosion[no].explosion_iteration = Explosion[no].explosion_iteration + 5;
    if (Explosion[no].explosion_iteration > 250) {
        Explosion[no].explosion = false;
        Explosion[no].explosion_iteration = 0;
    }
    
    
    return Explosion[no].explosion;
    
}


void draw_enemies_static(){
    for (byte x=0; x< NO_OF_STATIC_ENEMIES; x++){
        if ( Enemies_Static[x].is_alive == true){
        
            if (Enemies_Static[x].type == COCOON){ 
                FX::drawBitmap(Enemies_Static[x].x-camera.x, Enemies_Static[x].y-camera.y, cocon, 0, 1);
                Enemies_Static[x].width = 40;
                Enemies_Static[x].height = 64;
            }
            if (Enemies_Static[x].type == ROCKET){ 
                FX::drawBitmap(Enemies_Static[x].x-camera.x, Enemies_Static[x].y-camera.y, rocket, 0, 1);
                Enemies_Static[x].width = 32;
                Enemies_Static[x].height = 48;
            }
            if (Enemies_Static[x].type == RADAR){ 
                FX::drawBitmap(Enemies_Static[x].x-camera.x, Enemies_Static[x].y-camera.y, radar, 0, 1);
                Enemies_Static[x].width = 40;
                Enemies_Static[x].height = 64;
            }
            if (Enemies_Static[x].type == FUNGUS){ 
                FX::drawBitmap(Enemies_Static[x].x-camera.x, Enemies_Static[x].y-camera.y, fungus, 0, 1);
                Enemies_Static[x].width = 24;
                Enemies_Static[x].height = 32;
            }
            
            if (Enemies_Static[x].type == WAGGON){ 
                FX::drawBitmap(Enemies_Static[x].x-camera.x, Enemies_Static[x].y-camera.y, waggon, 0, 1);
                Enemies_Static[x].width = 32;
                Enemies_Static[x].height = 40;
            }
            
            if (Enemies_Static[x].type == SHIP_LASER){ 
                FX::drawBitmap(Enemies_Static[x].x-camera.x, Enemies_Static[x].y-camera.y, ship_laser, teleport_frame, 1);
                Enemies_Static[x].width = 8;
                Enemies_Static[x].height = 128;
            }
        
          
        }
    }
    
    
//}
//void draw_enemies_static_explosion(){
    
    
    for (byte x=0; x< NO_OF_STATIC_ENEMIES; x++){
        if (Enemies_Static[x].explosion == true){
            
            //Enemies_Static[x].explosion = draw_explosion(x, Enemies_Static[x].x, Enemies_Static[x].y);
            Enemies_Static[x].explosion = draw_explosion(x, grenade.x, grenade.y);
            
        }
    }
    
}


void check_if_player_is_hit(int x, int y, int size_x, int size_y){
    if (x < BOARD_GAME_WIDTH){
        if (!player.is_crouching){
            if (arduboy.collide(Rect(player.x, player.y, 20, 30), Rect(x, y, size_x, size_y))){
                player.is_dead();
            }
        }
        if (player.is_crouching){
            if (arduboy.collide(Rect(player.x, player.y+10, 20, 20), Rect(x, y, size_x, size_y))){
                player.is_dead();
            }
        }
    }
}

void draw_enemies_turret(){
    
    for (byte i=0; i<2; i++){
        if (Enemies_Turret[i].is_alive == true){
            FX::drawBitmap(Enemies_Turret[i].x-camera.x, Enemies_Turret[i].y-camera.y, turret_pic, 0, 1);
            
            if (arduboy.everyXFrames(random(30,240))) {
                if (Enemies_Turret[i].is_shooting == false){
                    Enemies_Turret[i].is_shooting = true;
                    play_sound(SOUND_TURRET);
                    }
            }

            if (Enemies_Turret[i].is_shooting == true){
                FX::drawBitmap(Enemies_Turret[i].bullet_x-camera.x, Enemies_Turret[i].y+11-camera.y, turret_bullet_pic, 0, 1);
                Enemies_Turret[i].bullet_x = Enemies_Turret[i].bullet_x - Enemies_Turret[i].speed;
                
                //czy turret trafil playera
                check_if_player_is_hit(Enemies_Turret[i].bullet_x, Enemies_Turret[i].y+11, 2, 2);
                /*if (!player.is_crouching){
                    if (arduboy.collide(Rect(player.x, player.y, 20, 30), Rect(Enemies_Turret[i].bullet_x, Enemies_Turret[i].y+11, 2, 2))){
                      player.is_dead();
                    }
                }
                if (player.is_crouching){
                    if (arduboy.collide(Rect(player.x, player.y+10, 20, 20), Rect(Enemies_Turret[i].bullet_x, Enemies_Turret[i].y+11, 2, 2))){
                       player.is_dead();
                    }
                }
                */
                
                
                if (Enemies_Turret[i].bullet_x < Enemies_Turret[i].range) { 
                    Enemies_Turret[i].bullet_x = Enemies_Turret[i].x;
                    Enemies_Turret[i].is_shooting = false;
                }
            }
        
        }
    }
//}

//void draw_enemies_turret_explosion(){
    
    
        if (Enemies_Turret[0].explosion == true){
         
         Enemies_Turret[0].explosion = draw_explosion(EXPLOSION_TURRET_0, Enemies_Turret[0].x, Enemies_Turret[0].y);
            }
            
        if (Enemies_Turret[1].explosion == true){
         
         Enemies_Turret[1].explosion = draw_explosion(EXPLOSION_TURRET_1, Enemies_Turret[1].x, Enemies_Turret[1].y);
            }
            
            
           
       
       
    
    
}




void draw_enemies_doublelauncher(){
    
    if (Enemies_DoubleLauncher.is_alive == true){
        FX::drawBitmap(Enemies_DoubleLauncher.x-camera.x, Enemies_DoubleLauncher.y-camera.y, double_launcher_pic, 0, 1);
        
        
        if (arduboy.everyXFrames(random(30,240))) {
            if ((Enemies_DoubleLauncher.is_shooting_1 == false) && (player.x < Enemies_DoubleLauncher.x - PLAYER_WIDTH - 5)){
                Enemies_DoubleLauncher.is_shooting_1 = true;
                Enemies_DoubleLauncher.bullet_1_x = Enemies_DoubleLauncher.x+8;
                play_sound(SOUND_LAUNCHER);
                }
            }
         if (arduboy.everyXFrames(random(30,240))) {       
                if ((Enemies_DoubleLauncher.is_shooting_2 == false) && (player.x < Enemies_DoubleLauncher.x - PLAYER_WIDTH - 5)){
                Enemies_DoubleLauncher.is_shooting_2 = true;
                Enemies_DoubleLauncher.bullet_2_x = Enemies_DoubleLauncher.x+8;
                play_sound(SOUND_LAUNCHER);
                }
        }
        
    
        if (Enemies_DoubleLauncher.is_shooting_1 == true){
            FX::drawBitmap(Enemies_DoubleLauncher.bullet_1_x-camera.x, Enemies_DoubleLauncher.y+2-camera.y, double_launcher_bullet_pic, 0, 1);            
            Enemies_DoubleLauncher.bullet_1_x = Enemies_DoubleLauncher.bullet_1_x - Enemies_DoubleLauncher.speed;
            
            //czy tuDoubleLauncher.is_shooting_1 trafil playera
            
            check_if_player_is_hit(Enemies_DoubleLauncher.bullet_1_x, Enemies_DoubleLauncher.y+2, 2, 3);
            
            /*    if (!player.is_crouching){
                    if (arduboy.collide(Rect(player.x, player.y, 20, 30), Rect(Enemies_DoubleLauncher.bullet_1_x, Enemies_DoubleLauncher.y+2, 2, 3))){
                      player.is_dead();
                    }
                }
                if (player.is_crouching){
                    if (arduboy.collide(Rect(player.x, player.y+10, 20, 20), Rect(Enemies_DoubleLauncher.bullet_1_x, Enemies_DoubleLauncher.y+2, 2, 3))){
                       player.is_dead();
                    }
                }
            */
            if (Enemies_DoubleLauncher.bullet_1_x < Enemies_DoubleLauncher.range) { 
                Enemies_DoubleLauncher.bullet_1_x = Enemies_DoubleLauncher.x+8;
                Enemies_DoubleLauncher.is_shooting_1 = false;
            }
        }

        if (Enemies_DoubleLauncher.is_shooting_2 == true){
            FX::drawBitmap(Enemies_DoubleLauncher.bullet_2_x-camera.x, Enemies_DoubleLauncher.y+10-camera.y, double_launcher_bullet_pic, 0, 1);   
            //czy tuDoubleLauncher.is_shooting_2 trafil playera
            
                check_if_player_is_hit(Enemies_DoubleLauncher.bullet_2_x, Enemies_DoubleLauncher.y+10, 2, 3);

              /*  if (!player.is_crouching){
                    if (arduboy.collide(Rect(player.x, player.y, 20, 30), Rect(Enemies_DoubleLauncher.bullet_2_x, Enemies_DoubleLauncher.y+10, 2, 3))){
                      player.is_dead();
                    }
                }
                if (player.is_crouching){
                    if (arduboy.collide(Rect(player.x, player.y+10, 20, 20), Rect(Enemies_DoubleLauncher.bullet_2_x, Enemies_DoubleLauncher.y+10, 2, 3))){
                       player.is_dead();
                    }
                }
                * */         
            Enemies_DoubleLauncher.bullet_2_x = Enemies_DoubleLauncher.bullet_2_x - Enemies_DoubleLauncher.speed;
            
            if (Enemies_DoubleLauncher.bullet_2_x < Enemies_DoubleLauncher.range) { 
                Enemies_DoubleLauncher.bullet_2_x = Enemies_DoubleLauncher.x+8;
                Enemies_DoubleLauncher.is_shooting_2 = false;
            }
        }
    
    
    
    
    }
}




void draw_enemies_combined_launcher(){
//void draw_enemies_combined_launcher_top(){
    
    if (Enemies_CombinedLauncher_Top.is_alive == true){
        FX::drawBitmap(Enemies_CombinedLauncher_Top.x-camera.x, Enemies_CombinedLauncher_Top.y-camera.y, combined_launcher_top_pic, 0, 1);
        
        if (arduboy.everyXFrames(random(30,240))) {
            if (Enemies_CombinedLauncher_Top.is_shooting == false){// && (player.x < Enemies_CombinedLauncher_Top.x - 40)){
                Enemies_CombinedLauncher_Top.is_shooting = true;
                play_sound(SOUND_TURRET);
                }
        }

        if (Enemies_CombinedLauncher_Top.is_shooting == true){
            FX::drawBitmap(Enemies_CombinedLauncher_Top.bullet_x-camera.x, Enemies_CombinedLauncher_Top.y+3-camera.y, turret_bullet_pic, 0, 1);
            check_if_player_is_hit(Enemies_CombinedLauncher_Top.bullet_x, Enemies_CombinedLauncher_Top.y+3, 2, 3);
            
            Enemies_CombinedLauncher_Top.bullet_x = Enemies_CombinedLauncher_Top.bullet_x - Enemies_CombinedLauncher_Top.speed;
            if (Enemies_CombinedLauncher_Top.bullet_x < Enemies_CombinedLauncher_Top.range) { 
                Enemies_CombinedLauncher_Top.bullet_x = Enemies_CombinedLauncher_Top.x;
                Enemies_CombinedLauncher_Top.is_shooting = false;
            }
        }
    
    }

    
    
        if (Enemies_CombinedLauncher_Top.explosion == true){
            Enemies_CombinedLauncher_Top.explosion = draw_explosion(EXPLOSION_COMBINED_LAUNCHER, Enemies_CombinedLauncher_Top.x, Enemies_CombinedLauncher_Top.y);
            
          
            
        }
    

    
    if (Enemies_CombinedLauncher_Bottom.is_alive == true){
        FX::drawBitmap(Enemies_CombinedLauncher_Bottom.x-camera.x, Enemies_CombinedLauncher_Bottom.y-camera.y, combined_launcher_bottom_pic, 0, 1);
        
        
        if (arduboy.everyXFrames(random(30,240))) {
            if ((Enemies_CombinedLauncher_Bottom.is_shooting == false) && (player.x < Enemies_CombinedLauncher_Bottom.x - PLAYER_WIDTH - 5)){
                Enemies_CombinedLauncher_Bottom.bullet_x = Enemies_CombinedLauncher_Bottom.x+8;
                Enemies_CombinedLauncher_Bottom.is_shooting = true;
                play_sound(SOUND_LAUNCHER);
                }
            }
       
    
        if (Enemies_CombinedLauncher_Bottom.is_shooting == true){
            FX::drawBitmap(Enemies_CombinedLauncher_Bottom.bullet_x-camera.x, Enemies_CombinedLauncher_Bottom.y+2-camera.y, double_launcher_bullet_pic, 0, 1);    
            check_if_player_is_hit(Enemies_CombinedLauncher_Bottom.bullet_x, Enemies_CombinedLauncher_Bottom.y+2, 3, 3);        
            Enemies_CombinedLauncher_Bottom.bullet_x = Enemies_CombinedLauncher_Bottom.bullet_x - Enemies_CombinedLauncher_Bottom.speed;
            
            if (Enemies_CombinedLauncher_Bottom.bullet_x < Enemies_CombinedLauncher_Bottom.range) { 
                Enemies_CombinedLauncher_Bottom.bullet_x = Enemies_CombinedLauncher_Bottom.x;
                Enemies_CombinedLauncher_Bottom.is_shooting = false;
            }
        }
    }
}


void draw_enemies_incubator(){
    
    if (Enemies_Incubator.is_alive == true){
        FX::drawBitmap(Enemies_Incubator.x-camera.x, Enemies_Incubator.y-camera.y, incubator_pic, 0, 1);
    
    
        
    }
    
    for(byte i=0; i< NO_OF_EGGS;i++){
        if (Enemies_Eggs[i].is_alive == true){
            FX::drawBitmap(Enemies_Eggs[i].x-camera.x, Enemies_Eggs[i].y-camera.y, egg_pic, random(0,2), 1);
            
            if (Enemies_Incubator.is_alive != true){
                check_if_player_is_hit(Enemies_Eggs[i].x, Enemies_Eggs[i].y, 6, 6);
            }
            
            Enemies_Eggs[i].x = Enemies_Eggs[i].x  + random(-2,3);
            Enemies_Eggs[i].y = Enemies_Eggs[i].y  + random(-3,3);
            
            while (Enemies_Eggs[i].y > Enemies_Incubator.y + 16) {
                Enemies_Eggs[i].y = Enemies_Eggs[i].y  + random(-3,0);
            }
            while (Enemies_Eggs[i].y < Enemies_Incubator.y + 8) {
                Enemies_Eggs[i].y = Enemies_Eggs[i].y  + random(0,3);
            }
            
            if (Enemies_Incubator.is_alive == true){
                while (Enemies_Eggs[i].x > Enemies_Incubator.x + 16) {
                    Enemies_Eggs[i].x = Enemies_Eggs[i].x  + random(-3,0);
                }
                while (Enemies_Eggs[i].x < Enemies_Incubator.x + 8) {
                    Enemies_Eggs[i].x = Enemies_Eggs[i].x  + random(0,3);
                }
               
            }
     
        }
        
    }
    
    

    
        if (Enemies_Incubator.explosion == true){
            Enemies_Incubator.explosion = draw_explosion(EXPLOSION_INCUBATOR, Enemies_Incubator.x, Enemies_Incubator.y);

        }
    
    
}


void draw_enemies_missile_tower(){
    
    if (Enemies_Missle_Tower.is_alive == true){
        FX::drawBitmap(Enemies_Missle_Tower.x-camera.x, Enemies_Missle_Tower.y-camera.y, missile_tower_pic, 0, 1);
    }
    
    if (Enemies_Missile_Guidance.is_alive == true){
        FX::drawBitmap(Enemies_Missile_Guidance.x-camera.x, Enemies_Missile_Guidance.y-camera.y, missile_guidance_pic, 0, 1);
        
        
        if (Missile.x >150){FX::drawBitmap(Missile.x-camera.x, Missile.y-camera.y, missile_pic, 0, 1);}
        else {FX::drawBitmap(Missile.x-camera.x, Missile.y-camera.y, missile_pic, 1, 1);}
        
        check_if_player_is_hit(Missile.x, Missile.y+5, 6, 5);

        
        if (arduboy.everyXFrames(2)) {
        
                Missile.x = Missile.x - Missile.speed;
                if (Missile.x < 0) {
                    Missile.x = BOARD_GAME_WIDTH;
                    Missile.y = player.y+32;
                    }
        }
        if (arduboy.everyXFrames(10)) {
            if (Missile.y > player.y){
                Missile.y = Missile.y - Missile.speed - 2;
            }
                if (Missile.y < player.y){
                    Missile.y = Missile.y + Missile.speed + 2;
                }  
        }
    }
    
    if (Enemies_Missile_Guidance.explosion == true){
        Enemies_Missile_Guidance.explosion = draw_explosion(EXPLOSION_MISSILE_TOWER, Enemies_Missile_Guidance.x, Enemies_Missile_Guidance.y);

            
        }
    
    
}
    

void draw_pistons(){
    for (byte x=0; x<MAX_PISTON; x++){
        if (Piston[x].is_alive == true){
            FX::drawBitmap(Piston[x].x-camera.x, Piston[x].y-camera.y, piston_pic, Piston[x].frame, 1);  
        
            if ((Piston[x].frame != 0) && (Piston[x].frame != 9)){   
                check_if_player_is_hit(Piston[x].x, Piston[x].y, 24, 32);     
            }
    }
        
        
        // 0 czarny
        // 4 5 - srodek
        // 0 czarny
        if (arduboy.everyXFrames(2)) {
        
        if ((Piston[x].frame != 4) && (Piston[x].frame != 9)){
            Piston[x].frame = Piston[x].frame + 1;
        }
    }
        if ((Piston[x].frame == 4) || (Piston[x].frame == 9)){
            if (arduboy.everyXFrames(FRAME_RATE/2)) {
                Piston[x].counter = Piston[x].counter + 1;
                if (Piston[x].counter >= Piston[x].timeout){
                    Piston[x].frame =  Piston[x].frame + 1;
                    Piston[x].counter = 0;
                }
            }
        
        }
        
        
            if (Piston[x].frame>9){
                Piston[x].frame = 0;
                //Piston[x].counter = 0;
                }
    }
    
}

void draw_mine(){
   for (byte x=0; x<MAX_MINES; x++){
        if (Mine[x].is_alive == true){
            FX::drawBitmap(Mine[x].x-camera.x, Mine[x].y-camera.y, mine_pic, 0, 0);  
            
            if(!player.is_super){check_if_player_is_hit(Mine[x].x+6, Mine[x].y-3, 4, 8);}    
            
        } 
    
    }
}


void draw_teleports(){
    
    if ((Teleport[0].is_alive == true) && (Teleport[1].is_alive == true)){
        FX::drawBitmap(Teleport[0].x-camera.x, Teleport[0].y-camera.y, teleport_pic, teleport_frame, 1);
        FX::drawBitmap(Teleport[1].x-camera.x, Teleport[1].y-camera.y, teleport_pic, teleport_frame, 1);
    
    
        if (arduboy.collide(Rect(Teleport[0].x+12, Teleport[0].y+20, 8, 28), Rect(player.x+6,  player.y, 4, 16))){
            
            if ((arduboy.justPressed(UP_BUTTON)) && (!Teleport[1].is_using)) {
                player.is_jumping = false;
                Teleport[0].is_using = true;
                Teleport[1].is_using = true;
                camera.offset = 0;
                camera.offset_y = CAMERA_Y_NORMAL_OFFSET;
                player.x = Teleport[1].x+4;
                player.y = Teleport[1].y+16;
                play_sound(SOUND_TELEPORT);
       
            }
            if (arduboy.notPressed(UP_BUTTON)) {
               if (arduboy.everyXFrames(15)) {
                    Teleport[0].is_using = false;
                    Teleport[1].is_using = false;
                }
            }
        } 
        
        
        if (arduboy.collide(Rect(Teleport[1].x+12, Teleport[1].y+20, 8, 28), Rect(player.x+6,  player.y, 4, 16))){
            if ((arduboy.justReleased(UP_BUTTON))  && (!Teleport[0].is_using)) {
                player.is_jumping = false;
                Teleport[0].is_using = true;
                Teleport[1].is_using = true;
                camera.offset = 0;
                camera.offset_y = CAMERA_Y_NORMAL_OFFSET;
                player.x = Teleport[0].x+4;
                player.y = Teleport[0].y+16;
                play_sound(SOUND_TELEPORT);
           }
            if (arduboy.notPressed(UP_BUTTON)) {
               if (arduboy.everyXFrames(15)) {
                Teleport[0].is_using = false;
                Teleport[1].is_using = false;
                }   
            }
            
        }   

    
    }
}


//#define TILE_LAST   50
//#define TILE_SOLID_FIRST    1
//#define TILE_SOLID_LAST     50
#define SHIP_FIRE       102



byte background_frame = 0;

void draw_background(){
    
    if (arduboy.everyXFrames(PLAYER_FRAME_RATE)) {
        background_frame = background_frame + 1;
        if (background_frame > 2) { background_frame = 0;}
        }
    
    
    if ((Current_Level >= 0) && (Current_Level < 25)){
        FX::drawBitmap(0-camera.x, 0-camera.y, Levels_bg_1, Current_Level, dbmWhite);
    } else if ((Current_Level > 24) && (Current_Level < 50)){
        FX::drawBitmap(0-camera.x, 0-camera.y, Levels_bg_2, Current_Level-25, dbmWhite);
    } else if ((Current_Level >= 50) && (Current_Level < 75)){
        FX::drawBitmap(0-camera.x, 0-camera.y, Levels_bg_3, Current_Level-50, dbmWhite);
    } else if ((Current_Level >= 75) && (Current_Level < 100)){
        FX::drawBitmap(0-camera.x, 0-camera.y, Levels_bg_4, Current_Level-75, dbmWhite);
    } else if ((Current_Level >= 100) && (Current_Level < 125)){
        FX::drawBitmap(0-camera.x, 0-camera.y, Levels_bg_5, Current_Level-100, dbmWhite);
    }
    
    
    
    for (int i=0; i<22; i++){
        FX::readDataArray(Level_Address + (32 * i), 0, 0, 32, tilemapBuffer, 32);
        for(int j=0; j<32; j++){ 

            if (tilemapBuffer[j] == SHIP_FIRE){ 
                FX::drawBitmap(j*8-camera.x, i*8-camera.y, ship_fire, background_frame, 1);
            }
            
        }
    }
}


bool solid_below(){
    
     for (int i=0; i<22; i++){
        FX::readDataArray(Level_Address + (32 * i), 0, 0, 32, tilemapBuffer, 32);
        for(int j=0; j<32; j++){ 
            if (tilemapBuffer[j] ==  1){
                if (arduboy.collide(Rect(player.x+1, player.y+32, 24-2, 1), Rect((j)*8, (i)*8, 8,1))){
                    return true;
                }
            }
        }
    }
    return false;   
}

bool player_collision_vertical(){
    for (int i=0; i<22; i++){
        FX::readDataArray(Level_Address + (32 * i), 0, 0, 32, tilemapBuffer, 32);
        for(int j=0; j<32; j++){ 
            //if ((tilemapBuffer[j] >=  TILE_SOLID_FIRST) && (tilemapBuffer[j] <=  TILE_SOLID_LAST)){
            if (tilemapBuffer[j] ==  1){
                if (arduboy.collide(Rect(player.x, player.y-1, 24, 10), Rect((j)*8, (i)*8, 8,8))){
                    play_sound(SOUND_COLLISION);
                    return true;
                }
            }
        }
    }
    return false; 
    
}

int player_x;
bool player_colision_horizontal(){
    

    if (player.direction == RIGHT){
        player_x = player.x+1;
    }    
    if (player.direction == LEFT){
         player_x = player.x;
    }
    
    
    for (int i=0; i<22; i++){
        FX::readDataArray(Level_Address + (32 * i), 0, 0, 32, tilemapBuffer, 32);
        for(int j=0; j<32; j++){ 
            if (tilemapBuffer[j] ==  1){
                    if (arduboy.collide(Rect(player_x, player.y, 24-1, 32), Rect((j)*8, (i)*8, 8,8))){
                    play_sound(SOUND_COLLISION);
                    return true;
                    }
            }
            if (tilemapBuffer[j] ==  10){
                if (arduboy.collide(Rect(player_x, player.y, 24-1, 32), Rect((j)*8, (i)*8, 8,8))){
                    player.is_super = true;
                }
            }
        }
    }
    
    
    for (byte i=0; i< NO_OF_STATIC_ENEMIES; i++){
        if ( Enemies_Static[i].is_alive == true){
                if (arduboy.collide(Rect(player_x, player.y, 23, 32), Rect(Enemies_Static[i].x, Enemies_Static[i].y, Enemies_Static[i].width, Enemies_Static[i].height))){
                    play_sound(SOUND_COLLISION);
                    return true;
                }
        }
    }
        
        if (Enemies_Incubator.is_alive){
            if (arduboy.collide(Rect(player_x, player.y, 23, 32), Rect(Enemies_Incubator.x, Enemies_Incubator.y, 32, 48))){
                play_sound(SOUND_COLLISION);
                return true;
            }
        }
        if (Enemies_CombinedLauncher_Top.is_alive){
            if (arduboy.collide(Rect(player_x, player.y, 23, 32), Rect(Enemies_CombinedLauncher_Top.x,Enemies_CombinedLauncher_Top.y, 56, 32))){
                play_sound(SOUND_COLLISION);
                return true;
            }            
        }
        for (byte i=0; i<2; i++){
            if (Enemies_Turret[i].is_alive){
                if (arduboy.collide(Rect(player_x, player.y, 23, 32), Rect(Enemies_Turret[i].x, Enemies_Turret[i].y, 48, 40))){
                    play_sound(SOUND_COLLISION);
                    return true;
                } 
            }
        }       
        if (Enemies_Missile_Guidance.is_alive){
             if (arduboy.collide(Rect(player_x, player.y, 23, 32), Rect(Enemies_Missile_Guidance.x, Enemies_Missile_Guidance.y, 32, 72))){
                play_sound(SOUND_COLLISION);
                return true;
            }       
        }
    
    
    return false;       
}


struct LevelData {
  uint24_t address;
};

// Lista poziomów
const LevelData levelList[] = {
  {Level_0},
  {Level_1},
  {Level_2},
  {Level_3},
  {Level_4},
  {Level_5},
  {Level_6},
  {Level_7},
  {Level_8},
  {Level_9},
  {Level_10},
  {Level_11},
  {Level_12},
  {Level_13},
  {Level_14},
  {Level_15},
  {Level_16},
  {Level_17},
  {Level_18},
  {Level_19},
  {Level_20},
  {Level_21},
  {Level_22},
  {Level_23},
  {Level_24},
  {Level_25},
  {Level_26},
  {Level_27},
  {Level_28},
  {Level_29},
  {Level_30},
  {Level_31},
  {Level_32},
  {Level_33},
  {Level_34},
  {Level_35},
  {Level_36},
  {Level_37},
  {Level_38},
  {Level_39},
  {Level_40},
  {Level_41},
  {Level_42},
  {Level_43},
  {Level_44},
  {Level_45},
  {Level_46},
  {Level_47},
  {Level_48},
  {Level_49},
  {Level_50},
  {Level_51},
  {Level_52},
  {Level_53},
  {Level_54},
  {Level_55},
  {Level_56},
  {Level_57},
  {Level_58},
  {Level_59},
  {Level_60},
  {Level_61},
  {Level_62},
  {Level_63},
  {Level_64},
  {Level_65},
  {Level_66},
  {Level_67},
  {Level_68},
  {Level_69},
  {Level_70},
  {Level_71},
  {Level_72},
  {Level_73},
  {Level_74},
  {Level_75},
  {Level_76},
  {Level_77},
  {Level_78},
  {Level_79},
  {Level_80},
  {Level_81},
  {Level_82},
  {Level_83},
  {Level_84},
  {Level_85},
  {Level_86},
  {Level_87},
  {Level_88},
  {Level_89},
  {Level_90},
  {Level_91},
  {Level_92},
  {Level_93},
  {Level_94},
  {Level_95},
  {Level_96},
  {Level_97},
  {Level_98},
  {Level_99},
  {Level_100},
  {Level_101},
  {Level_102},
  {Level_103},
  {Level_104},
  {Level_105},
  {Level_106},
  {Level_107},
  {Level_108},
  {Level_109},
  {Level_110},
  {Level_111},
  {Level_112},
  {Level_113},
  {Level_114},
  {Level_115},
  {Level_116},
  {Level_117},
  {Level_118},
  {Level_119},
  {Level_120},
  {Level_121},
  {Level_122},
  {Level_123},
  {Level_124}
};

//uint24_t Level_Address;

void prepare_level_parameters(){
    

    player.set();
    grenade.is_shooting = false;
    grenade.counter = 0;
    grenade.x = 0;
    camera.offset = 0;

    




if (Current_Level >= 0 && Current_Level < sizeof(levelList) / sizeof(levelList[0])) {
    Level_Address = levelList[Current_Level].address;
  } else {
    // Obsługa błędu, gdy wartość Current_Level jest poza zakresem
    // Tutaj można umieścić odpowiednią reakcję, np. przypisanie domyślnej wartości
    Level_Address = Level_0;
  }

    initialize_enemies_static();
    
    game_stage = STAGE_PLAY_GAME;
}


void title_screen(){
    bool exit_loop = false;
    
    if (Current_Level > 124) { Current_Level = 0; }
    do{
        arduboy.clear();
        arduboy.pollButtons();
        do{
            delay(1);
        }while (!(arduboy.nextFrame()));
        
    
        FX::drawBitmap(0, 10, title, 0, 1);
        
        tinyfont.setCursor(45, 35);
        tinyfont.print("SMBIT 2023");

        tinyfont.setCursor(0, 60);
        tinyfont.print("LEVEL:");
        
        tinyfont.setCursor(30, 60);
        tinyfont.print(Current_Level);
        
        tinyfont.setCursor(55, 60);
        tinyfont.print("SOUND:");
        tinyfont.setCursor(85, 60);
        if (enable_audio){tinyfont.print("Y");
        }else {tinyfont.print("N");}
        
       // tinyfont.setCursor(85, 60);
       // tinyfont.print("CHEAT:");
        tinyfont.setCursor(100, 0);
        if (cheat_mode){tinyfont.print("CHEAT");
        }else {tinyfont.print("");}
        
        
        if (arduboy.justPressed(RIGHT_BUTTON)) {
             player.clear_parameters = true;
             player.is_super = false;
             Current_Level = Current_Level + 1;
             if (Current_Level > 124) { Current_Level = 0;}
        }
    
        if (arduboy.justPressed(LEFT_BUTTON)) {
             player.clear_parameters = true;
             player.is_super = false;
             Current_Level = Current_Level - 1;
             if (Current_Level < 0) { Current_Level = 124;}
        }
        
        if (arduboy.justPressed(DOWN_BUTTON)) {
             enable_audio = !enable_audio;
        }
    
        if (arduboy.justPressed(UP_BUTTON)) {
             cheat_mode = !cheat_mode;
        }
         
         
    
         arduboy.display();
         FX::display(CLEAR_BUFFER);
         
            if (arduboy.justPressed(A_BUTTON)) {
                exit_loop = true;
          }
         
    }while(!exit_loop);
    
    if(enable_audio){ audio.on(); }
    else { audio.off(); }    
        
   prepare_level_parameters();
}

void setup() {
         
    arduboy.begin();
    arduboy.setFrameRate(FRAME_RATE);
    audio.begin();
    audio.on();  
    
    FX::begin(FX_DATA_PAGE);  // initialise FX chip
  
  
  
  
}


void loop() {
    
   if(game_stage == STAGE_GAME_TITLE){
        title_screen();
   // } else if (game_stage == STAGE_LEVEL_COMPLETED){
    } else if (game_stage == STAGE_PLAY_GAME){

    if (!arduboy.nextFrame()) return;
    
    if (player.x > BOARD_GAME_WIDTH-3){
            player.clear_parameters = false;
            game_stage = STAGE_GAME_TITLE;
            Current_Level = Current_Level+1;
            
    }

    arduboy.clear();
    arduboy.pollButtons();
        
        
    if(enable_audio){ audio.on(); }
    else { audio.off(); }    
    
    check_keys();
    shooting();
    launch_grenade();
    player_movement();
    
        
  
    if (camera.offset == 0){
        if (player.x > BOARD_GAME_WIDTH - SCREEN_GAME_WIDTH + (SCREEN_GAME_WIDTH/2)){
            camera.offset = CAMERA_LEFT_OFFSET;
           // player.direction = LEFT;
            camera.x = player.x-camera.offset;
            if (camera.x > BOARD_GAME_WIDTH - SCREEN_GAME_WIDTH){
                camera.x = BOARD_GAME_WIDTH - SCREEN_GAME_WIDTH;
            }
        } else {
            camera.offset = CAMERA_RIGHT_OFFSET;
           // player.direction = RIGHT;
            camera.x = player.x-camera.offset;
            if (camera.x < 0){
                camera.x = 0;
            }
        }
    }
    
    
    //zrobilem to bo jak sie telporotwalem za blisko prawej krawedzi to wychodzilo poza plasnze wyswietlanie
    if (camera.x > (BOARD_GAME_WIDTH - SCREEN_GAME_WIDTH)){
            camera.x=(BOARD_GAME_WIDTH - SCREEN_GAME_WIDTH);
    }
    
    camera.y = player.y - camera.offset_y;
  
    byte super_player_frame_position = 0;
    if (player.is_super){super_player_frame_position = 22;}
    else  {super_player_frame_position = 0;}
    
    
    if (player.direction == RIGHT){ 
        if (camera.offset > CAMERA_RIGHT_OFFSET){
            camera.offset = camera.offset -1;
        }
        
        if ((camera.x < BOARD_GAME_WIDTH - SCREEN_GAME_WIDTH)  && (player.x > camera.offset)){
            camera.x = player.x - camera.offset;
        }    
        
        FX::drawBitmap(player.x-camera.x, player.y-camera.y, vitorc, player.frame+super_player_frame_position, 1);
        
    }
    
   
    if (player.direction == LEFT){
        

        if (camera.offset < CAMERA_LEFT_OFFSET){
            camera.offset = camera.offset + 1;
        }

            
        if ((camera.x > 0) && (player.x-camera.offset < BOARD_GAME_WIDTH - SCREEN_GAME_WIDTH)){
            camera.x = player.x-camera.offset;
        }
    
        FX::drawBitmap(player.x-camera.x, player.y-camera.y, vitorc, player.frame+11+super_player_frame_position, 1);
        
        }
    
    
    draw_enemies_static();
    draw_enemies_turret();
    draw_enemies_doublelauncher();
    draw_enemies_combined_launcher();
    draw_enemies_incubator();
    draw_enemies_missile_tower();
    draw_enemy();
    draw_teleports();
    draw_pistons();

    tinyfont.setCursor(0, 0);
    tinyfont.print(Current_Level);
 
    
    draw_background();
    draw_mine();

    arduboy.display();
    FX::display(CLEAR_BUFFER); 
    
    }
}
