#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

const byte numRows = 4;
const byte numCols = 4;

char keymap[numRows][numCols] =
{
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[numRows] = {9,8,7,6};
byte colPins[numCols] = {5,4,3,2};

Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Memory-efficient char arrays
char equation[2][20] = { "", "" };  // Two operands, max 20 chars each
char display[17] = "";              // Display buffer, max 16 chars + null
char operation = ' ';               // Current operator (+, -, *, /) or ' ' if none
byte dispLen = 0;                   // Track display length for efficient updates

// Clear line by overwriting with spaces
void clearLine(byte row) {
  lcd.setCursor(0, row);
  for (byte i = 0; i < 16; i++) {
    lcd.print(' ');
  }
}

// Reset calculator state
void reset() {
  operation = ' ';
  equation[0][0] = '\0';
  equation[1][0] = '\0';
  display[0] = '\0';
  dispLen = 0;
  clearLine(0);
  clearLine(1);
}

// Update display without clearing (avoid blinking)
void updateDisplay() {
  byte len = strlen(display);
  if (len > 16) {
    // Show only rightmost 16 chars
    lcd.setCursor(0, 0);
    for (byte i = len - 16; i < len; i++) {
      lcd.print(display[i]);
    }
  } else {
    // Show full display and pad with spaces
    lcd.setCursor(0, 0);
    for (byte i = 0; i < len; i++) {
      lcd.print(display[i]);
    }
    for (byte i = len; i < 16; i++) {
      lcd.print(' ');
    }
  }
  dispLen = len;
}

// Show error message temporarily
void showError(const char* msg) {
  clearLine(0);
  clearLine(1);
  lcd.setCursor(0, 0);
  lcd.print(msg);
  delay(1000);
}

// Get length of char array safely (up to 20 chars)
byte getStrLen(const char* str) {
  byte len = 0;
  while (str[len] != '\0') {
    len++;
    if (len > 19) break;  // Safety limit
  }
  return len;
}

// Append char to buffer if space allows
void appendChar(char* buffer, char ch) {
  byte len = getStrLen(buffer);
  if (len < 19) {
    buffer[len] = ch;
    buffer[len + 1] = '\0';
  }
}

// Convert char array to float
float strToFloat(const char* str) {
  return atof(str);
}

// Convert float to char array with reasonable precision
void floatToStr(float val, char* buffer, byte bufferSize) {
  dtostrf(val, 1, 2, buffer);  // Format: width=1, precision=2
}

// Map keypad char to operator
char op(char key) {
  switch (key) {
    case 'A': return '+';
    case 'B': return '-';
    case 'C': return '*';
    case 'D': return '/';
    default:  return ' ';
  }
}

// Perform calculation based on operator
float calculate(float num1, float num2, char op) {
  switch (op) {
    case '+': return num1 + num2;
    case '-': return num1 - num2;
    case '*': return num1 * num2;
    case '/':
      if (num2 != 0) {
        return num1 / num2;
      } else {
        return 0;  // Error flag will be handled by caller
      }
    default:
      return 0;
  }
}

// Handle negative sign input - allow only at start of operand
bool handleNegative(char key) {
  if (key != '-') {
    return false;
  }
  
  if (operation == ' ' && getStrLen(equation[0]) == 0) {
    // Start first number with negative
    equation[0][0] = '-';
    equation[0][1] = '\0';
    display[0] = '-';
    display[1] = '\0';
    updateDisplay();
    return true;
  } 
  else if (operation != ' ' && getStrLen(equation[1]) == 0) {
    // Start second number with negative
    equation[1][0] = '-';
    equation[1][1] = '\0';
    appendChar(display, '-');
    updateDisplay();
    return true;
  }
  
  showError("Error: Invalid -");
  return false;
}

// Perform calculation based on current state
void performCalculation() {
  byte len0 = getStrLen(equation[0]);
  byte len1 = getStrLen(equation[1]);
  
  if (len0 == 0 || operation == ' ' || len1 == 0) {
    return;
  }
  
  float num1 = strToFloat(equation[0]);
  float num2 = strToFloat(equation[1]);
  
  if (operation == '/' && num2 == 0) {
    showError("Error: Div by 0"); // Show error but keep current state for correction
    reset();
    return;
  }
  // Perform calculation and prepare for next input
  float result = calculate(num1, num2, operation);
  floatToStr(result, equation[0], 20);
  equation[1][0] = '\0';
  strcpy(display, equation[0]);
  operation = ' ';
}

// Handle number key press - append to current operand and update display
void handleNumber(char key) {
  if (operation == ' ') {
    appendChar(equation[0], key);
  } else {
    appendChar(equation[1], key);
  }
  appendChar(display, key);
  updateDisplay();
}

// Handle operator key press - set operator or perform calculation if already set
void handleOperator(char newOp) {
  byte len0 = getStrLen(equation[0]);
  byte len1 = getStrLen(equation[1]);
  
  // Need first number
  if (len0 == 0) {
    showError("Need number");
    updateDisplay();
    return;
  }
  
  // No operator set yet
  if (operation == ' ') {
    operation = newOp;
    appendChar(display, operation);
    updateDisplay();
    return;
  }
  
  // Operator set, second number is just "-" (negative sign) - replace operator and remove negative
  if (len1 == ' ' && equation[1][0] == '-') {
    display[strlen(display) - 1] = newOp;  // Replace minus with new operator
    equation[1][0] = '\0';  // Clear the negative
    operation = newOp;
    updateDisplay();
    return;
  }
  
  // Operator set, no second number - just replace operator
  if (len1 == 0) {
    display[strlen(display) - 1] = newOp;  // Replace last char (old operator)
    operation = newOp;
    updateDisplay();
    return;
  }
  
  // Both operands exist - calculate first, then set new operator
  performCalculation();
  operation = newOp;
  appendChar(display, operation);
  updateDisplay();
}

// Handle equals key press - perform calculation and show result
void handleEquals() {
  byte len0 = getStrLen(equation[0]);
  byte len1 = getStrLen(equation[1]);
  
  if (len0 == 0 || operation == ' ' || len1 == 0) {
    showError("Incomplete expr");
    updateDisplay();
    return;
  }
  
  performCalculation();
  clearLine(1);
  lcd.setCursor(0, 1);
  lcd.print("= ");
  lcd.print(equation[0]);
}

void setup() {
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Calculator");
  lcd.setCursor(0, 1);
  lcd.print("Ready!");
  delay(2000);
  
  reset();
  Serial.begin(9600);
}

void loop() { 
  char key = myKeypad.getKey();
  if (!key) return;
  
  // Number input (0-9)
  if (key >= '0' && key <= '9') {
    handleNumber(key);
  } 
  // Subtraction or negative sign
  else if (key == 'B') {
    if ((operation == ' ' && getStrLen(equation[0]) == 0) || (operation != ' ' && getStrLen(equation[1]) == 0)) {
      // Start with negative first number or second number
      handleNegative('-');
    } else {
      // Use as subtraction operator
      handleOperator('-');
      updateDisplay();
    }
  } 
  // Other operators
  else if (key == 'A') {
    handleOperator('+'); // Addition
    updateDisplay();
  } else if (key == 'C') {
    handleOperator('*'); // Multiplication
    updateDisplay();
  } else if (key == 'D') {
    handleOperator('/'); // Division
    updateDisplay();
  } 
  // Calculate result
  else if (key == '#') {
    handleEquals();
  }
  // Clear
  else if (key == '*') {
    reset();
    lcd.setCursor(0, 0);
    lcd.print("Cleared!");
    delay(800);
    clearLine(0);
  }
}