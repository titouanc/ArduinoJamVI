#include "shiftregister.h"

static const unsigned char sinusoid[255] = {
    127, 130, 133, 136, 139, 142, 145, 148, 151, 154, 157, 161, 164, 166, 169,
    172, 175, 178, 181, 184, 187, 189, 192, 195, 197, 200, 202, 205, 207, 210,
    212, 214, 217, 219, 221, 223, 225, 227, 229, 231, 232, 234, 236, 237, 239,
    240, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 251, 252, 252, 253,
    253, 253, 253, 253, 253, 253, 253, 253, 253, 252, 252, 251, 251, 250, 249,
    249, 248, 247, 246, 245, 243, 242, 241, 239, 238, 236, 235, 233, 231, 230,
    228, 226, 224, 222, 220, 218, 215, 213, 211, 209, 206, 204, 201, 199, 196,
    193, 191, 188, 185, 182, 180, 177, 174, 171, 168, 165, 162, 159, 156, 153,
    150, 147, 144, 141, 137, 134, 131, 128, 125, 122, 119, 116, 112, 109, 106,
    103, 100, 97, 94, 91, 88, 85, 82, 79, 76, 73, 71, 68, 65, 62, 60, 57, 54,
    52, 49, 47, 44, 42, 40, 38, 35, 33, 31, 29, 27, 25, 23, 22, 20, 18, 17, 15,
    14, 12, 11, 10, 8, 7, 6, 5, 4, 4, 3, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 2, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14, 16, 17, 19, 21, 22, 24,
    26, 28, 30, 32, 34, 36, 39, 41, 43, 46, 48, 51, 53, 56, 58, 61, 64, 66, 69,
    72, 75, 78, 81, 84, 87, 89, 92, 96, 99, 102, 105, 108, 111, 114, 117, 120,
    123
};

ShiftRegister registre(2, 3, 4, 5, 6, 2);

#define n_leds sizeof(leds)
char leds[] = {1, 1, 1, 1, 1};
int simon_colors[] = {1, 6, 4, 7, 2};
int buttons[] = {0, 1, 2, 3, 4};

int level = 0;

#define BUZZER 8
#define LEDSTRIP 9

unsigned int notes[] = {440, 494, 523, 587, 659};
unsigned int melody[] = {
    262, 262, 262, 0, 330, 330, 330, 0, 392, 392, 392, 0, 523, 523, 523, 523,
    523, 523, 523, 0, 392, 392, 392, 0, 523, 523, 523, 523, 523, 523, 523, 0
};
unsigned int game_over_melody[] = {
    587, 587, 587, 0, 554, 554, 554, 0, 523, 523, 523, 0
};

bool is_same_color(){
    for (int i=1; i<n_leds; i++){
        if (leds[i] != leds[0])
            return false;
    }
    return true;
}

void light_leds(){
    registre.clear();
    for (int i=0; i<n_leds; i++)
        for (int j=0; j<3; j++)
            registre.set(3*i+j, (leds[i] & (1<<j)) ? true : false);
    registre.output();
}

static inline bool button_on(int pin, unsigned int n_active=30){
    for (int i=0; i<n_active; i++)
        if (analogRead(pin) > 512)
            return false;
    return true;
}

void anim_level_up(){
    for (int i=0; i<sizeof(melody); i++){
        tone(BUZZER, melody[i]);
        for (int j=0; j<5; j++){
            leds[j] = 1 + (i+j)%7;
            light_leds();
        }
        for (int j=0; j<10; j++){
            analogWrite(LEDSTRIP, sinusoid[(5*i+j)%255]);
            delay(2);
        }
    }
    tone(BUZZER, 0);
    analogWrite(LEDSTRIP, 0);
}

void anim_game_over(){
    for (int i=0; i<n_leds; i++)
        leds[i] = 1; //RED
    light_leds();
    for (int i=0; i<sizeof(game_over_melody)/sizeof(int); i++){
        tone(BUZZER, game_over_melody[i]);
        delay(200);
    }
    tone(BUZZER, 0);
}

bool play(){
    unsigned long int last_change = millis();
    unsigned long int turn_start = millis();

    /* 1) Choose initial colors */
    while (is_same_color()){
        for (int i=0; i<n_leds; i++)
            leds[i] = random(1, 8);
    }
    light_leds();

    while (! is_same_color()){

        /* Check for buttons */
        for (int i=0; i<n_leds; i++){
            /* If button active, go to next color */
            if (button_on(buttons[i])){
                tone(BUZZER, notes[i]);
                leds[i] ++;
                if (leds[i] > 7)
                    leds[i] = 1;
                light_leds();
                while (button_on(buttons[i]));
                tone(BUZZER, 0);
            }
        }
        
        /* If level > 0: random changes */
        if (level > 0 && millis() - last_change > 10000/level){
            for (int i=0; i<n_leds; i++){
                if (random(11)%7 == 0){
                    do {leds[i] = random(1, 8);} while (is_same_color());
                }
            }
            last_change = millis();
        }

        /* Display colorz */
        light_leds();

        if (millis() - turn_start > 15000){
            anim_game_over();
            return false;
        }
    }

    anim_level_up();
    return true;
}

bool a_button_pressed(){
    for (int i=0; i<n_leds; i++)
        if (button_on(buttons[i]))
            return true;
    return false;
}

void game(){
    int i = 0;
    unsigned long int last_frame = 0;
    while (! a_button_pressed()){
        if (millis() - last_frame > 20){
            i = (i+1)%255;
            analogWrite(LEDSTRIP, sinusoid[i]);
            last_frame = millis();
        }
    }
    level = 0;
    analogWrite(LEDSTRIP, 0);
    while (play()) level++;
}

void clear(){
    for (int i=0; i<n_leds; i++)
        leds[i] = 0;
    light_leds();
    tone(BUZZER, 0);
}

void simon_led_on(int ledid){
    leds[ledid] = simon_colors[ledid];
    light_leds();
    tone(BUZZER, notes[ledid]);
}

void simon_led_off(int ledid){
    leds[ledid] = 0;
    light_leds();
    tone(BUZZER, 0);
}

char simon_vector[1000];
unsigned int simon_turn = 1;

bool simon_game_turn(){
    /* Simon's play */
    for (int i=0; i<simon_turn; i++){
        simon_led_on(simon_vector[i]);
        delay(250);
        simon_led_off(simon_vector[i]);
        delay(15);
    }

    /* User's play */
    for (int i=0; i<simon_turn; i++){
        bool played = false;
        while (! played){
            for (int j=0; j<n_leds; j++){
                if (button_on(j)){
                    played = true;
                    simon_led_on(j);
                    while (button_on(j));
                    simon_led_off(j);
                    if (j != simon_vector[i])
                        return false;
                }
            }
        }
    }

    return true;
}

void simon_game(){
    clear();

    simon_turn = 1;
    for (int i=0; i<sizeof(simon_vector); i++)
        simon_vector[i] = random(0, 5);

    while (simon_game_turn()){
        simon_turn++;
        anim_level_up();
        clear();
    }

    anim_game_over();
}

void setup(){
    randomSeed(analogRead(5));
    analogWrite(LEDSTRIP, 0);
}

void loop(){
    //game();
    simon_game();
}
