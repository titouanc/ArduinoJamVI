class ShiftRegister {
    private:
        static const unsigned int D = 1;
        int SCK, SI, RCK, SCLR, G;
        size_t size;
        uint8_t *state;
    public:
        ShiftRegister(int sck, int rck, int si, int sclr, int g, size_t n=1) : 
            size(n), state(new uint8_t[n]), SCK(sck), SI(si), RCK(rck), SCLR(sclr), G(g)
        {
            pinMode(SCK, OUTPUT);
            pinMode(SI, OUTPUT);
            pinMode(RCK, OUTPUT);
            pinMode(SCLR, OUTPUT);
            pinMode(G, OUTPUT);

            for (size_t i=0; i<n; i++)
                state[i] = 0;
            digitalWrite(SCLR, HIGH);
            digitalWrite(G, LOW);
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
        void clear(){
            digitalWrite(SCLR, LOW);
            digitalWrite(G, HIGH);
            delayMicroseconds(D);
            digitalWrite(SCLR, HIGH);
            digitalWrite(G, LOW);
        }
};