class ShiftRegister {
    private:
        static const unsigned int D = 1;
        int SCK, SI, RCK;
        size_t size;
        uint8_t *state;
    public:
        ShiftRegister(size_t n, int sck, int si, int rck) : 
            size(n), state(new uint8_t[n]), SCK(sck), SI(si), RCK(rck) 
        {
            pinMode(SCK, OUTPUT);
            pinMode(SI, OUTPUT);
            pinMode(RCK, OUTPUT);
            for (size_t i=0; i<n; i++)
                state[i] = 0;
        }
        bool get(size_t pos){
            uint8_t reg_i = pos / 8;
            uint8_t bit_i = pos % 8;
            return (((state[reg_i])>>(bit_i)) & 0x01);
        }
        void set(size_t pos, bool on){
            uint8_t reg_i = pos / 8;
            uint8_t bit_i = pos % 8;
            if (on)
                bitSet(state[reg_i], bit_i);
            else
                bitClear(state[reg_i], bit_i);
        }
        void output(){
            for (size_t i=0; i<8*size; i++){
                bool on = get(i);

                digitalWrite(SI, on ?  HIGH : LOW);
                digitalWrite(SCK, HIGH);
                delayMicroseconds(D);
                digitalWrite(SCK, LOW);
                delayMicroseconds(D);
            }
            digitalWrite(RCK, HIGH);
            delayMicroseconds(D);
            digitalWrite(RCK, LOW);
        }
};

ShiftRegister registre(1, 2, 4, 3);

#define n_leds sizeof(leds)
char leds[] = {1};
int buttons[] = {13};

bool is_same_color(){
    for (int i=1; i<n_leds; i++){
        if (leds[i] != leds[0])
            return false;
    }
    return true;
}

void light_leds(){
    for (int i=0; i<n_leds; i++)
        for (int j=0; j<3; j++)
            registre.set(3*i+j, leds[i] & (1<<j));
    registre.output();
}

void play(){
    unsigned long int last_change = millis();

    /* 1) Choose initial colors */
    while (is_same_color()){
        for (int i=0; i<n_leds; i++)
            leds[i] = random(1, 8);
    }
    light_leds();

    Serial.println("Game started");

    while (! is_same_color()){
        for (int i=0; i<n_leds; i++){
            if (digitalRead(buttons[i]) == HIGH)
                leds[i] = (leds[i] + 1) % 8;
        }
        light_leds();
    }

    Serial.println("Game finished");

    for (int k=0; k<10; k++){
        for (int i=0; i<n_leds; i++)
            leds[i] = 7;
        light_leds();
        delay(333);
        for (int i=0; i<n_leds; i++)
            leds[i] = 0;
        light_leds();
        delay(333);
    }
}

void setup(){
    Serial.begin(115200);
    for (int i=0; i<n_leds; i++)
        pinMode(buttons[i], INPUT);
    Serial.println("Starting...");
}

void loop(){
    //play();

    for (int i=0; i<n_leds; i++){
        if (digitalRead(buttons[i]) == HIGH){
            leds[i] = 1 + (leds[i] + 1) % 7;
            Serial.println("HIGH");
            while (digitalRead(buttons[i]) == HIGH);
        }
    }

    light_leds();
}
