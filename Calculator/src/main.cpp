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

String equation[3] = {"", "", ""};
String display = "";

char op(char key) {
  switch (key) {
    case 'A':
      return '+';
    case 'B':
      return '-';
    case 'C':
      return '*';
    case 'D':
      return '/';
    default:
      return ' ';
  }
}

float calculate(float num1, float num2, char op) {
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

float toFloat(char key) {
  return key - '0';
}

void setup() {
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0,0);
  lcd.print("Calculator is ");
  lcd.setCursor(0,1);
  lcd.print("Ready!");
  delay(2000);
  lcd.clear();
  Serial.begin(9600);
}

void loop() { 
  char key = myKeypad.getKey();
  
  lcd.setCursor(0,0);
  if (key) {
    if (key >= '0' && key <= '9') {
      if (equation[1] == "" || equation[0] == "") {
        /*if ( equation[1] != "" && equation[0] == "") {
          equation[1] = "";
        } */
        equation[1] += key;
        display += key;
        lcd.clear();
        lcd.print(display);
        Serial.println(equation[1]);
      } else if (equation[2] == "" || key != '#') {
        equation[2] += key;
        display += key;
        lcd.clear();
        lcd.print(display);
        Serial.println(equation[2]);
      }
    } else if (key == 'A' || key == 'B' || key == 'C' || key == 'D') {
      if (equation[1] != "" && equation[2] == "") {
        equation[0] = op(key);
        display += equation[0];
        lcd.clear();
        lcd.print(display);
        Serial.println(equation[0]);
      }
    } else if (key == '#') {
      if (equation[1] != "" && equation[2] != "") {
        float num1 = (equation[1]).toInt();
        float num2 = (equation[2]).toInt();
        char operation = equation[0][0];
        float result = calculate(num1, num2, operation);

        lcd.setCursor(0,1);
        lcd.print("Equals: ");
        lcd.print(result);
        equation[0] = "";
        equation[1] = String(result);
        equation[2] = "";
        display = "";
        display += equation[1];
      }
    }
    else if (key == '*') {
      equation[0] = "";
      equation[1] = "";
      equation[2] = "";
      display = "";
      lcd.clear();
      lcd.print("Cleared!");
      delay(1000);
      lcd.clear();
      lcd.setCursor(0,0);
    }
  }
}