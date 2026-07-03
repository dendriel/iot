#include <Servo.h>

#include <TM1637Display.h>

class Timer {
private:
    unsigned long startTime = 0;
    unsigned long interval = 0;

public:
    Timer(unsigned long ms = 0)
        : interval(ms) {}

    void start(unsigned long ms) {
      interval = ms;
      startTime = millis();
    }

    bool expired() {
        return millis() - startTime >= interval;
    }

    unsigned long remaining() {
      unsigned long elapsed = millis() - startTime;

      if (elapsed >= interval) {
          return 0;
      }

      return interval - elapsed;
    }
};

const String CLICK_TIMER= "CLICK";
const String RUNTIME_TIMER = "RUNTIME";

const int RUN_TIME_LED_PIN = 2;
const int NEXT_CLICK_LED_PIN = 3;
const int SWITCH_TARGET_LED_PIN = 4;

const int BUZZER_PIN = 5;

const int DISPLAY_CLK = 6;
const int DISPLAY_DIO = 7;

const int SERVO_A_PIN = 8;
const int SERVO_B_PIN = 9;
const int SERVO_C_PIN = 10;

const int EXECUTE_BUTTON_PIN = 11;
const int FUNCTION_BUTTON_PIN = 12;

// ajuste conforme sua mecânica
const int REST_POS  = 10;
const int CLICK_POS = 45;

// Suporta 9500 flechas antes de parar.
const unsigned long MAX_RUNTIME =
    12UL * 60UL * 60UL * 1000UL;    // 12 horas
    // + 30UL * 60UL * 1000UL;          // 30 minutos

const unsigned long SHOW_RUNTIME_INTERVAL = 10 * 1000UL; // 10s
const unsigned long SHOW_CLICKTIME_INTERVAL = 30 * 1000UL; // 10s

TM1637Display display(DISPLAY_CLK, DISPLAY_DIO);
Timer nextClickTimer;
Timer runtimeTimer;
Timer switchTargetTimer;
Timer displayTimer;

String displayedTimerName;

Servo myServoA;
Servo myServoB;
Servo myServoC;



/**************************************
 * BUTTON START
 *************************************/
enum Function {
  FUNCTION_A,
  FUNCTION_B,
  FUNCTION_C,
  FUNCTION_COUNT
};

Function selectedFunction = FUNCTION_A;

bool lastFunctionButtonState = HIGH;
bool lastExecuteButtonState = HIGH;

Timer functionDisplayTimer;
bool showingFunctionSelection = false;
bool switchTargetEnabled = true;

const unsigned long FUNCTION_DISPLAY_TIME = 1000;

const char* getFunctionName() {
  switch (selectedFunction) {
    case FUNCTION_A: return "FUNA";
    case FUNCTION_B: return "FUNB";
    case FUNCTION_C: return "FUNC";
    default:         return "UNKN";
  }
}

void executeSelectedFunction() {
  switch (selectedFunction) {
    case FUNCTION_A:
      functionA();
      break;
    case FUNCTION_B:
      functionB();
      break;
    case FUNCTION_C:
      functionC();
      break;
  }
}

void functionA() {
  toggleDisplayMode();
}

void functionB() {
  nextClickTimer.start(0);
  switchTargetTimer.start(0);

  showText("RSET");
  beep(200);

  delay(1000);
}

void functionC() {
  toggleSwitchTarget();
}

void handleButtons() {
  bool functionButtonState = digitalRead(FUNCTION_BUTTON_PIN);
  bool executeButtonState = digitalRead(EXECUTE_BUTTON_PIN);

  // botão de função
  if (lastFunctionButtonState == LOW && functionButtonState == HIGH) {
    // soltou o botão => clique completo

    selectedFunction = (Function)((selectedFunction + 1) % FUNCTION_COUNT);

    showText(getFunctionName());

    showingFunctionSelection = true;
    functionDisplayTimer.start(FUNCTION_DISPLAY_TIME);
  }

  // botão executar
  if (lastExecuteButtonState == LOW && executeButtonState == HIGH) {
    executeSelectedFunction();
  }

  lastFunctionButtonState = functionButtonState;
  lastExecuteButtonState = executeButtonState;
}

/**************************************
 * BUTTON END
 *************************************/

void showText(const char* text) {
    uint8_t data[4] = {0, 0, 0, 0};

    for (int i = 0; i < 4 && text[i] != '\0'; i++) {

        if (text[i] >= '0' && text[i] <= '9') {
          data[i] = display.encodeDigit(text[i] - '0');
          continue;
        }

        switch (toupper(text[i])) {
            case 'A': data[i] = 0x77; break;
            case 'B': data[i] = 0x7C; break;
            case 'C': data[i] = 0x39; break;
            case 'D': data[i] = 0x5E; break;
            case 'E': data[i] = 0x79; break;
            case 'F': data[i] = 0x71; break;
            case 'H': data[i] = 0x76; break;
            case 'I': data[i] = 0x06; break;
            case 'L': data[i] = 0x38; break;
            case 'N': data[i] = 0x54; break; // aproximação
            case 'O': data[i] = 0x3F; break;
            case 'P': data[i] = 0x73; break;
            case 'R': data[i] = 0x50; break; // aproximação
            case 'S': data[i] = 0x6D; break;
            case 'T': data[i] = 0x78; break;
            case 'U': data[i] = 0x1C; break; // U = 0x3E; u = 0x1C
            case 'Y': data[i] = 0x6E; break;
            case '-': data[i] = 0x40; break;
            case ' ': data[i] = 0x00; break;
            default:  data[i] = 0x00; break;
        }
    }

    display.setSegments(data);
}

void setRuntimeDisplay() {
  displayedTimerName = RUNTIME_TIMER;
  displayTimer.start(SHOW_RUNTIME_INTERVAL);
  
  digitalWrite(NEXT_CLICK_LED_PIN, LOW);
  digitalWrite(RUN_TIME_LED_PIN, HIGH);
}

void setClicktimeDisplay() {
  displayedTimerName = CLICK_TIMER;
  displayTimer.start(SHOW_CLICKTIME_INTERVAL);
  
  digitalWrite(NEXT_CLICK_LED_PIN, HIGH);
  digitalWrite(RUN_TIME_LED_PIN, LOW);
}

void updateDisplay() {
    if (showingFunctionSelection) {

    showText(getFunctionName());

    if (functionDisplayTimer.expired()) {
      showingFunctionSelection = false;
    }

    return;
  }

  if (displayTimer.expired()) {
    toggleDisplayMode();
  }

  if (displayedTimerName == CLICK_TIMER) {
    doShowClickTimer();
  } else {
    doShowElapsedTime();
  }
}

void toggleDisplayMode() {
  if (displayedTimerName == CLICK_TIMER) {
    setRuntimeDisplay();
  } else {
    setClicktimeDisplay();
  }
}

void disableTimerLeds() {
  digitalWrite(NEXT_CLICK_LED_PIN, LOW);
  digitalWrite(RUN_TIME_LED_PIN, LOW);
}

void enableTimerLeds() {
  if (displayedTimerName == CLICK_TIMER) {
    digitalWrite(NEXT_CLICK_LED_PIN, HIGH);
  } else {
    digitalWrite(RUN_TIME_LED_PIN, HIGH);
  }
}

void doShowClickTimer() {
    unsigned long remaining = nextClickTimer.remaining() / 1000;
    int minutes = remaining / 60;
    int seconds = remaining % 60;
    showTime(minutes, seconds);
}

void doShowElapsedTime() {
    unsigned long elapsed = millis() / 1000;
    int hours = elapsed / 3600;
    int minutes = (elapsed % 3600) / 60;
    showTime(hours, minutes);
}

void showTime(int left, int right) {
    uint8_t data[4];

    data[0] = display.encodeDigit((left / 10) % 10);
    data[1] = display.encodeDigit(left % 10) | 0x80; // ':'
    data[2] = display.encodeDigit((right / 10) % 10);
    data[3] = display.encodeDigit(right % 10);

    display.setSegments(data);
}

void setSwitchTargetEnabled(bool enabled) {
  switchTargetEnabled = enabled;

  digitalWrite(
    SWITCH_TARGET_LED_PIN,
    enabled ? LOW : HIGH
  );
}

void toggleSwitchTarget() {
  setSwitchTargetEnabled(!switchTargetEnabled);
}

void setup() {
  // Serial.begin(9600);
  // Serial.println(minutes);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(NEXT_CLICK_LED_PIN, OUTPUT);
  pinMode(RUN_TIME_LED_PIN, OUTPUT);
  pinMode(SWITCH_TARGET_LED_PIN, OUTPUT);  

  pinMode(FUNCTION_BUTTON_PIN, INPUT_PULLUP);
  pinMode(EXECUTE_BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(NEXT_CLICK_LED_PIN, HIGH);
  digitalWrite(RUN_TIME_LED_PIN, HIGH);
  digitalWrite(SWITCH_TARGET_LED_PIN, HIGH);

  // Servos
  myServoA.attach(SERVO_A_PIN);
  myServoB.attach(SERVO_B_PIN);
  myServoC.attach(SERVO_C_PIN);

  randomSeed(analogRead(A0));

  // posição inicial
  myServoA.write(REST_POS);
  myServoB.write(REST_POS);
  myServoC.write(REST_POS);

  // Display
  display.setBrightness(0);
  display.clear();

  runtimeTimer.start(MAX_RUNTIME);
  nextClickTimer.start(0); // executa imediatamente na primeira vez

  switchTargetTimer.start(0);

  showText("BOOT");
  delay(500);
  
  digitalWrite(NEXT_CLICK_LED_PIN, LOW);
  digitalWrite(RUN_TIME_LED_PIN, LOW);
  digitalWrite(SWITCH_TARGET_LED_PIN, LOW);
  countdown(5);
  showText("PLAY");
  beep(500);
  delay(500);

  setClicktimeDisplay();
}

void countdown(int secs) {
  for (int i = 5; i > 0; i--) {
    char text[5];
    sprintf(text, "%4d", i);
    showText(text);
    delay(1000);
  }
}

void beep(int durationMs) {
  tone(BUZZER_PIN, 1000); // 1000 Hz
  delay(durationMs);
  noTone(BUZZER_PIN);
}

void beepFinish() {
  tone(BUZZER_PIN, 800);
  delay(200);

  tone(BUZZER_PIN, 1200);
  delay(200);

  tone(BUZZER_PIN, 1600);
  delay(300);

  noTone(BUZZER_PIN);
}

void click(Servo& servo, int waitTime) {
    servo.write(CLICK_POS);
    delay(waitTime);
    servo.write(REST_POS);
    // Wait so rest can go back to start pos
    delay(random(1000, 2000));
}

void loop() {
  handleButtons();

  if (nextClickTimer.expired()) {   
    disableTimerLeds();
    showText("RUNA");

    click(myServoA, random(2300, 5200));

    showText("RUNB");
    click(myServoB, random(1300, 2500));
    
    enableTimerLeds();
    // agenda a próxima execução
    nextClickTimer.start(random(390000UL, 570000UL));
  }

  if (switchTargetTimer.expired()) {
    if (switchTargetEnabled) {
      disableTimerLeds();
      showText("SCHT");
      click(myServoC, random(300, 500));
      enableTimerLeds();
    }
    switchTargetTimer.start(random(90000UL, 180000UL));
  }
  
  if (runtimeTimer.expired()) {
    myServoA.write(REST_POS);
    myServoB.write(REST_POS);
    myServoC.write(REST_POS);

    myServoA.detach();
    myServoB.detach();
    myServoC.detach();

    showText("DONE");
    beepFinish();

    while (true) { /* 4ever */ }
  }
  
  updateDisplay();
}