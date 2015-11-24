// Response button firmware for TeensyLC, built with PlatformIO.
//
// Implements a modified SNTP timing protocol,
// where 'server' is a PC and 'client' is this device:
// 1. Server sends "initiate timing procedure" request to client
// 2. Client sets an internal stopwatch timer to 0 (T1), and replies with
//    the number of microseconds that elapsed since it received the message (T2).
//    Note that this is the same value as (T2-T1).
// 3. Server notes when it receives reply (T3)
// 4. Server repeats steps 1-3 often enough to be statistically meaningful

#include <SimpleSwitch.h> // debounce a pin
#include <CapSwitch.h> // capacitive sensor as-a-switch

// perform loop body n times, with capture of loop index i
#define DO(n) for(int i=0,_n=(n); i<_n;++i)

// TIMING
const uint16_t LOOP_TIME{10000}; // in microseconds
elapsedMicros outputTimer; // data rate

// MODEL
const uint8_t NUM_BUTTONS{4};
char l[]{"wxyz"}; // default keyboard letters
char h[]{"_"}; // home button key '_'
struct KeyState {bool keys[NUM_BUTTONS],changed,homebutton;} ks;

// HARDWARE CONNECTIONS
SimpleSwitch buttons[NUM_BUTTONS]{SimpleSwitch(2),SimpleSwitch(3),SimpleSwitch(4),SimpleSwitch(5)}; // mech. switches
CapSwitch cap(15); // capacitive 'switch' at the given pin

bool valid_letter(char c){return ((c>='0'&&c<='9')||(c>='A'&&c<='Z')||(c>='a'&&c<='z'));} // true for [09AZaz]
void update_buttons(void){DO(NUM_BUTTONS){buttons[i].update();}} // update buttons with pin readings
void update_touch(void) {cap.update();}
void customize_keys(void)
{
    // Keyboard letter customization.
    // Power-cycling restores default (wxyz).
    char nl[NUM_BUTTONS]; // new letters
    Serial.readBytes(nl,NUM_BUTTONS+1); // utilizes Serial.setTimeout() default (1000ms)
    DO(NUM_BUTTONS){l[i] = valid_letter(nl[i]) ? nl[i] : l[i];}
    Serial.print("replaced letters with ");
    Serial.println(l);
}

void read_serial(void)
{
    // Consume and process at most 1 command from the Serial buffer.
    uint32_t t2{micros()};
    if (Serial.available()) {
        switch (Serial.read()) {
        case 'T': Serial.println(micros() - t2); break; // Time: T3 - T2
        case 'L': customize_keys(); break; // L (change letters)
        case 'l': Serial.println(l); break; // l (list letters)
        case 'c': Serial.println(cap.get_raw()); break; // TODO: test/debug
        case 'a': Serial.println(cap.get_avg()); break; // TODO: test/debug
        default: break;
        }
    }
}

KeyState update_state(KeyState k, KeyState o)
{
    if (cap.pressed())  {
        k.changed = true;
        k.homebutton = true;
    }
    if (cap.released()) {
        k.changed = true;
        k.homebutton = false;
    }
    if (k.homebutton == o.homebutton) { k.changed = false; } // ugly hack prevents retriggering
    DO(NUM_BUTTONS) {
        if (buttons[i].pressed())  { k.changed = true; k.keys[i] = true; }
        if (buttons[i].released()) { k.changed = true; k.keys[i] = false; }
    }
    return k;
}

void render_keyboard(const KeyState &kn, const KeyState &ko)
{
    // 'press' if old was low and new is high
    DO(NUM_BUTTONS) {
        if ((kn.keys[i]) && (!ko.keys[i])) Keyboard.press(l[i]);
        if ((!kn.keys[i]) && (ko.keys[i])) Keyboard.release(l[i]);
    }
    if (kn.homebutton && !ko.homebutton) Keyboard.press(h[0]);
    if (!kn.homebutton && ko.homebutton) Keyboard.release(h[0]);
}

void render_serial(const KeyState & k)
{
    const uint8_t BLEN{NUM_BUTTONS * 2 + sizeof(".\n\0")}; // ~13
    char buf[BLEN]{0}; // begin with an array of '\0'
    // print button states
    snprintf(buf,BLEN,"%d %d %d %d %d\n",
             k.keys[0],k.keys[1],k.keys[2],k.keys[3],k.homebutton);
    Serial.print(buf);
}

// PROGRAM
void setup() {
    Serial.begin(9600);
    Keyboard.begin();
}

void loop() {
    // Gather input data at maximum rate.
    // Inputs: buttons, touch, Serial
    update_buttons();
    update_touch();
    read_serial();

    // Update state and render output at a controlled rate.
    if (outputTimer >= LOOP_TIME) {
        outputTimer -= LOOP_TIME;
        // UPDATE
        KeyState oldks = ks;
        ks = update_state(ks,oldks); // Update internal state
        // RENDER
        if (ks.changed) {
            ks.changed = false; // reset
            render_serial(ks); // Send data out at a controlled rate.
            render_keyboard(ks,oldks);
        }
    }
    // Alternatively, UPDATE and RENDER could be done at different rates:
    // if (stateTimer >= STATE_TIME) {
    //     stateTimer -= STATE_TIME;
    //     update_state(ks);
    // }
    // if (renderTimer >= RENDER_TIME) {
    //     renderTimer -= RENDER_TIME;
    //     render_serial(ks); // Send data out at a controlled rate.
    // }
}
