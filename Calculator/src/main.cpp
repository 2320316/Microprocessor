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

char operators(char key) {
  switch (key) {
    case 'A':
      return '+';
    case 'B':
      return '-';
    case 'C':
      return '*';
    case 'D':
      return '/';
    case '#':
      return '=';
    default:
      return ' ';
  }
}

int calculate(int num1, int num2, char op) {
  switch (op) {
    case '+':
      return num1 + num2;
    case '-':
      return num1 - num2;
    case '*':
      return num1 * num2;
    case '/':
      if (num2 != 0) {
        return num1 / num2;
      } else {
        lcd.clear();
        lcd.print("Error: Div by 0");
        delay(2000);
        return 0;
      }
    default:
      return 0;
  }
}

int toInt(char key) {
  return key - '0';
}

void setup() {
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0,0);
  lcd.print("Calculator is ready!");
}

void loop() { 
  char key = myKeypad.getKey();
  
  if (key) {
    lcd.clear();
    lcd.print("Key Pressed: ");
    lcd.print(key);
    delay(1000);

    if (key >= '0' && key <= '9') {
      int num1 = toInt(key);
      lcd.clear();
      lcd.print("Num1: ");
      lcd.print(num1);
      delay(1000);

      char opKey = myKeypad.waitForKey();
      char op = operators(opKey);
      lcd.clear();
      lcd.print("Operator: ");
      lcd.print(op);
      delay(1000);

      char num2Key = myKeypad.waitForKey();
      int num2 = toInt(num2Key);
      lcd.clear();
      lcd.print("Num2: ");
      lcd.print(num2);
      delay(1000);

      int result = calculate(num1, num2, op);
      lcd.clear();
      lcd.print("Result: ");
      lcd.print(result);
    }
  }
}