#include <Arduino.h>
#include <Wire.h>
#include <virtuabotixRTC.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define pinButtonSelect 9
#define pinButtonPlus 3
#define pinButtonMinus 2
#define pinButtonAccept 4


// Definicja pinów: CLK=6, DAT=7, RST=8
virtuabotixRTC myRTC(6, 7, 8);

// Deklaracja wyświetlacza OLED podłączonego do magistrali I2C
// Wyjaśnienie parametrów:
// SCREEN_WIDTH (128) - szerokość ekranu - ile pikseli jest w poziomie (im więcej, tym więcej możemy napisać)
// SCREEN_HEIGHT (64) - wysokość ekranu - ile pikseli jest w pionie (im więcej, tym więcej linii tekstu)
// &Wire - mówimy Arduino żeby używało magistrali I2C (SDA/SCL piny) do komunikacji z wyświetlaczem
// -1 - oznacza że wyświetlacz resetuje się przez magistralę I2C, nie ma osobnego kabla reset
Adafruit_SSD1306 oled(128, 64, &Wire, -1);

// Tablica char zamiast String (klasa Arduino) - oszczędzamy RAM na mikrokontrolerze!
// String zużywa więcej pamięci dynamicznej, a na Arduino RAM jest ograniczony.
// Tablica char o stałym rozmiarze zajmuje mniej miejsca i jest bardziej przewidywalna.
char dayName[20];
char bufTime[10];
char bufData[12];

bool setTimeModeStan = false;
byte oldButtonStan = HIGH;

//Zmiene do void setMode()
byte oldButtonPlus = HIGH;
byte oldButtonMinus = HIGH;
byte oldButtonSelect = HIGH;
byte oldButtonAccept = HIGH;
byte menuOption = 1;
byte sek = 0;
byte min = 0;
byte hour = 0;
byte day = 1;
byte dayweek = 1;
byte mon = 1;
int yr = 2026;

unsigned long timer = 0;
  
void setup() {
  pinMode(pinButtonSelect, INPUT_PULLUP);
  pinMode(pinButtonPlus, INPUT_PULLUP);
  pinMode(pinButtonMinus, INPUT_PULLUP);
  pinMode(pinButtonAccept, INPUT_PULLUP);
  
  Serial.begin(9600);
  Wire.begin(); // Zainicjalizuj I2C przed OLEDem

  // ZMIANA: używamy false, false i dodajemy małe opóźnienie
  if(!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false)) {
    Serial.println(F("SSD1306 failed"));
    while(true); // Jeśli tu wisi, to znaczy że OLED nadal odrzuca komunikację
  }

  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(0,0);
  oled.println("Start systemu...");
  oled.display();
  
  delay(1000); // Daj mu sekundę na "zaskoczenie"
}


// Funkcja określa nazwę dnia tygodnia na podstawie numeru dnia z modułu RTC
// Używamy strcpy() do kopiowania stringów do tablicy char.
// strcpy jest szybszy i zużywa mniej zasobów niż operacje na klasie String,
// co jest istotne na urządzeniach embedded z ograniczoną pamięcią RAM.
void checkday(byte howCheck){
switch (howCheck){
  case 1:
    strcpy(dayName, "Poniedzialek"); // Kopiowanie tekstu do tablicy dayName
    break;
  case 2:
    strcpy(dayName, "Wtorek");
    break;
  case 3:
    strcpy(dayName, "Sroda");
    break;
  case 4:
    strcpy(dayName, "Czwartek");
    break;
  case 5:
    strcpy(dayName, "Piatek");
    break;
  case 6:
    strcpy(dayName, "Sobota");
    break;
  case 7:
    strcpy(dayName, "Niedziela");
    break;
  default:
    strcpy(dayName , "Blad");
    break;
  }
}
// Funkcja writedisplay() ustawia rozmiar tekstu, kolor, pozycję kursora i wyświetla tekst na ekranie OLED.
void writedisplay(byte curX ,byte curY, const char* printing){
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(curX, curY);
  oled.println(printing);
}


// Funkcja normalMode() odświeża ekran OLED co sekundę, wyświetlając aktualny czas i datę.
void normalMode(){
  myRTC.updateTime();

  // Wywołanie sprawdzania dnia tygodnia
  checkday(myRTC.dayofweek);

  // Wyświetlanie na ekranie OLED czasu i daty
  oled.clearDisplay();
    
  writedisplay(30, 15, dayName);

  // Formatowanie daty do bufData wyrażenie "%02d/%02d/%4d" oznacza, że chcemy wyświetlić dzień i miesiąc jako dwucyfrowe liczby (z wiodącym zerem, jeśli to konieczne) oraz rok jako czterocyfrową liczbę.
  sprintf(bufData, "%02d/%02d/%4d", myRTC.dayofmonth, myRTC.month, myRTC.year); 
  writedisplay(36, 25, bufData);

  writedisplay(28, 35, "Mamy godzine:");

  // Formatowanie czasu do bufTime wyerażenie "%02d:%02d:%02d" oznacza, że chcemy wyświetlić godziny, minuty i sekundy jako dwucyfrowe liczby (z wiodącym zerem, jeśli to konieczne).
  sprintf(bufTime, "%02d:%02d:%02d", myRTC.hours, myRTC.minutes, myRTC.seconds);
  writedisplay(40, 45, bufTime);

  oled.display();
}


// Funkcja setMode() obsługuje tryb ustawiania czasu, reagując na przyciski i aktualizując wartości czasu.
void setMode(){
  byte buttonPlus = digitalRead(pinButtonPlus);
  byte buttonMinus = digitalRead(pinButtonMinus);
  byte buttonAccept = digitalRead(pinButtonAccept);

  checkday(dayweek);
  oled.clearDisplay();
// Obsługa przycisków do zmiany wartości czasu w zależności od wybranej opcji menu////////////////////////////////////////////
  if(buttonPlus == LOW && oldButtonPlus == HIGH){
    switch (menuOption){
    case 1:
      sek ++;
      if(sek >= 60) sek = 0;
      break;
    case 2:
      min ++;
      if(min >= 60) min = 0;
      break;
    case 3:
      hour ++;
      if(hour >= 24) hour = 0;
      break;
    case 4:
      day ++;
      if(day >= 31) day = 1;
      break;
    case 5:
      mon ++;
      if(mon >= 12) mon = 1;
      break;
    case 6:
      yr ++;
      if(yr >= 2099 || yr <= 2000) yr = 2000;
    case 7:
      dayweek ++;
      if (dayweek >= 8) dayweek = 1;
      break;
    default:
      break;
    }
  }

  if(buttonMinus == LOW && oldButtonMinus == HIGH){
    switch (menuOption){
    case 1:
      sek --;
      if(sek >= 60) sek = 0;
      break;
    case 2:
      min --;
      if(min >= 60) min = 0;
      break;
    case 3:
      hour --;
      if(hour >= 24) hour = 0;
      break;
    case 4:
      day --;
      if(day >= 31) day = 1;
      break;
    case 5:
      mon --;
      if(mon >= 12) mon = 1;
      break;
    case 6:
      yr --;
      if(yr >= 2099 || yr <= 2000) yr = 2000;
    case 7:
      dayweek --;
      if (dayweek >= 8) dayweek = 1;
      break;
    default:
      break;
    }
  }
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Obsługa przycisku akceptacji do przechodzenia między opcjami menu
  if (buttonAccept == LOW && oldButtonAccept == HIGH)
  {
    menuOption ++;
  }
  
  writedisplay(15, 0, "Tryb Ustawiania");
  
  switch (menuOption){
  case 1:
    writedisplay(15, 15, "EDYCJA: Sekundy");
    break;
  case 2:
    writedisplay(15, 15, "EDYCJA: Minuty");
    break;
  case 3:
    writedisplay(15, 15, "EDYCJA: Godziny");
    break;
  case 4:
    writedisplay(15, 15, "EDYCJA: Dni");
    break;
  case 5:
    writedisplay(15, 15, "EDYCJA: Miesiaca");
    break;
  case 6:
    writedisplay(15, 15, "EDYCJA: Roku");
    break;
  case 7:
    writedisplay(0, 15, "EDYCJA:Dzien tygodnia");
    break;
  default:
    break;
  }


  writedisplay(30, 25, dayName);
  sprintf(bufTime, "%02d:%02d:%02d", hour, min, sek);
  writedisplay(40, 55, bufTime);

  sprintf(bufData, "%02d/%02d/%4d", day, mon, yr);
  writedisplay(36, 43, bufData);
  oled.display();
  // Jeśli użytkownik przeszedł przez wszystkie opcje menu, ustawiamy czas w RTC i wracamy do trybu normalnego
  if(menuOption >= 8){
    setTimeModeStan = false;
    menuOption = 1;
    
    oled.clearDisplay();
    writedisplay(32, 0, "!!!USTAWIONO ZEGAR!!!");
    oled.display();

    //Ustawienie czasu: sekundy, minuty, godziny, dzień tygodnia, dzień miesiąca, miesiąc, rok  
    myRTC.setDS1302Time(sek, min, hour, dayweek, day, mon, yr);
    delay(1000);
  }
  // Aktualizacja stanu przycisków po obsłudze
  oldButtonAccept = buttonAccept;
  oldButtonPlus = buttonPlus;
  oldButtonMinus = buttonMinus;
}


void loop() {
  byte buttonStanSelect = digitalRead(pinButtonSelect);

  // 1. Obsługa przycisku (niezależna od timera)
  if(buttonStanSelect == LOW && oldButtonStan == HIGH){
    setTimeModeStan = !setTimeModeStan;
    menuOption = 1;
    oled.clearDisplay();
  }
  // 2. Obsługa trybu ustawiania czasu lub trybu normalnego w zależności od stanu setTimeModeStan
  if(setTimeModeStan == true){
    setMode();
  }
  else{
        // W trybie normalnym odświeżamy ekran dokładnie raz na 1000ms
    if (millis() - timer >= 1000) {
      timer = millis();
      normalMode();

    }
  }
  // Aktualizacja stanu przycisku po obsłudze
  oldButtonStan = buttonStanSelect;
}