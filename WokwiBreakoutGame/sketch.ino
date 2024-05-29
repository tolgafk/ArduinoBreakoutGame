#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SevSeg.h>
SevSeg sevseg;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const unsigned char img_heart_small[] PROGMEM = {
  0b01010000,
  0b11111000,
  0b01110000,
  0b00100000,
  0b00000000,
};

//photoresistor baglantı
const int luxThreshold = 500; // Lux eşik değeri


int canX = 200;
int canY = 100;
int speedCan = 0;

int led1 = 13;
int led2 = 12;
int led3 = 11;

float ballX = 30;
float ballY = 50;
float speedX = 1.0f;
float speedY = -1.0f;

int blokX = 16;
int blokY = 8;

int level = 0;
int points = 0;
int life = 3;
bool game = false;
int potansiyometreDeger;

int bloklar[4][8];

const int buttonPin = 10;
bool buttonPressed = false;

enum MenuState{START, QUIT};
MenuState currentState = START;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

//sevensegment çalıştırma
byte numDigits = 2;
  byte digitPins[] = {A1,A2};
  byte segmentPins[] = {2, 3, 4, 5, 6, 7, 8};
  bool resistorsOnSegments = false; 
  byte hardwareConfig = COMMON_ANODE; 
  bool updateWithDelays = true; 
  bool leadingZeros = false; 
  bool disableDecPoint = true; 

  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments,
               updateWithDelays, leadingZeros, disableDecPoint);
  sevseg.setBrightness(90);


  //Ekran çalıştırma
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("zattiri.shit");
  display.drawBitmap(75, 1, img_heart_small, 5, 5, WHITE);


  display.display();

  pinMode(buttonPin, INPUT_PULLUP);

  //Ledleri açma
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  level1setup();
}

void loop() {
  // put your main code here, to run repeatedly:

  
  


  //level kontrolleri
  int sayac = 0;
  
  for(int i = 0; i < 4; i++){
    for(int j = 0; j < 8; j++){
      if(bloklar[i][j] == 1){
        sayac ++;
        break;
      }
    }
    if(sayac > 0) break;
  }

  if(sayac == 0){
    level++;
    float toplam = speedX + speedY;
    float additionalSpeed = toplam * 0.2;
    speedY += additionalSpeed;
    switch (level){
      case 1:
        level2setup();
        break;
      case 2:
        level3setup();
        break;
      case 3:
        level4setup();
        break;
      case 4:
        level5setup();
        break;
      case 5:
        gameOver();
        break;
    }
    delay(500);
    launch();
  }
  //buton kontrolleri
  buttonPressed = !digitalRead(buttonPin);
  if(currentState == START && buttonPressed){
    launch();
    game = true;
  }
  else if(currentState == QUIT && buttonPressed){
    game = false;
    gameOver();
  }
  if(game){
    //led kontrolleri
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    
    if(life == 1) digitalWrite(led1, HIGH);
    if(life == 2) {
      digitalWrite(led1, HIGH);
      digitalWrite(led2, HIGH);
    }
    if(life == 3){
      digitalWrite(led1, HIGH);
      digitalWrite(led2, HIGH);
      digitalWrite(led3, HIGH);
    }
    
    
    //potansiyometre okuma
    potansiyometreDeger = analogRead(A0);
    potansiyometreDeger = map(potansiyometreDeger, 0, 1023, 0, 100);

    //top işlemleri
      //collision control
    //duvara dik gelirse duvarın içinden geçebiliyor.
    if((ballX > -1 && ballX < 1)|| (ballX < SCREEN_WIDTH +1 && ballX > SCREEN_WIDTH - 1)){
      speedX *= -1;
    }
    if(ballY > -1 && ballY < 1) speedY *= -1;
    //platform ile temas kontrolü
    if(ballY <= SCREEN_HEIGHT && ballY >= SCREEN_HEIGHT - 3 && ballX + 2 > potansiyometreDeger && ballX < potansiyometreDeger + 28){
      speedY *= -1;
      float fark = ((ballX + 2) - (potansiyometreDeger + 14))/14;
      float toplam = abs(speedX) + abs(speedY);
      if(fark < 0.30 && fark > 0) fark = 0.30;
      if(abs(fark) < 0.30 && fark < 0) fark = -0.30;
      speedX = toplam * fark;
      speedY = toplam * (1 - abs(fark)) * -1; 
    } 
    //canın alınması kontrolü
    if(canY <= SCREEN_HEIGHT && canY >= SCREEN_HEIGHT - 3 && canX + 5> potansiyometreDeger && canX < potansiyometreDeger + 28){
      canY = 100;
      canX = 200;
      speedCan = 0;
      if(life < 3) life++;
    }
    //top düşme kontrolü
    if(ballY > SCREEN_HEIGHT + 5){
      if(life == 1) {
        digitalWrite(led1, LOW);
        waitScreen();
        delay(3000);
        game = false;
        level = 0;
        level1setup();
        life = 3;
        points = 0;
      }
      else {
        life--;
        launch();
      }
      
    }
    //blok kırma kontrolü
    if(ballY < 36){
      int tempY = ballY + speedY;
      int tempX = ballX + speedX;
      //y-y1 = (y2 - y1)/(x2 - x1) * (x - x1)
      
      int i = tempY / blokY;
      int j = tempX / blokX;

      int y1 = ((tempY - ballY) / (tempX - ballX)) * (((j + 1) * blokX) - ballX) + ballY;
      int y2 = ((tempY - ballY) / (tempX - ballX)) * ((j * blokX) - ballX) + ballY;
      int x1 = ((tempX - ballX) / (tempY - ballY)) * (((i + 1) * blokY) - ballY) + ballX;
      int x2 = ((tempX - ballX) / (tempY - ballY)) * ((i * blokY) - ballY) + ballX;
      //alttan üstten algılıyo ama sağ sol oldu mu içinden geçiyor.
      if(bloklar[i][j] == 1){
        bloklar[i][j] = 0;
        //eylemler
        points++;
        //can düşürme
        int randomNumber = random(101);
        if(randomNumber < 10){
          canX = tempX;
          canY = tempY;
          speedCan = 1;
        }
        //sol
        if(ballX < (j*blokX)){
          if(ballY < i*blokY){
            if(y2 < (i+1)*blokY && y2 > i*blokY) speedX *= -1;
            if(x2 < (j+1)*blokX && x2 > j*blokX) speedY *= -1;
            else{
              speedX *= -1;
              speedY *= -1;
            }
          }
          else if(ballY > (i+1) * blokY){
            if(y2 < (i+1)*blokY && y2 > i*blokY) speedX *= -1;
            if(x1 < (j+1)*blokX && x1 > j*blokX) speedY *= -1;
            else{
              speedX *= -1;
              speedY *= -1;
            }
          }
        }

        //sağ
        if(ballX > ((j+1)*blokX)){
          if(ballY > (i+1) * blokY){
            if(y1 < (i+1)*blokY && y1 > i*blokY) speedX *= -1;
            if(x1 < (j+1)*blokX && x1 > j*blokX) speedY *= -1;
            else{
              speedX *= -1;
              speedY *= -1;
            }
          }
          else if(ballY < i * blokY){
            if(y1 < (i+1)*blokY && y1 > i*blokY) speedX *= -1;
            if(x2 < (j+1)*blokX && x2 > j*blokX) speedY *= -1;
            else{
              speedX *= -1;
              speedY *= -1;
            }
          }
        }
        
        else speedY *= -1;
      }
    }
    
      //hareket
    ballX += speedX;
    ballY += speedY;
    canY += speedCan;
    
    draw();
  }
  else{
    displayMenu();
  }


//Sevseg gösterim
  sevseg.refreshDisplay(); // Must run repeatedly
  sevseg.setNumber(points);


  //photoresistor 
    int luxValue = analogRead(A1);
     // Eğer okunan değer eşik değerden büyükse
  if (luxValue > luxThreshold) {
    // Ekranı tersine çevirme
    display.invertDisplay(true);
  } else {
    // Ekranı normale döndürme
    display.invertDisplay(false);
  }


  
}

void gameOver(){
  digitalWrite(led1, LOW);
  display.clearDisplay();
  display.println("Oynadiginiz icin tesekkurler...");
  display.display();
  delay(1000000);

}

void draw(){
  //ekrana çizim
  display.clearDisplay();
  leveldraw();
  display.drawBitmap(canX, canY, img_heart_small, 5, 5, WHITE);
  display.fillRect(potansiyometreDeger, 63, 28, 1, SSD1306_WHITE);
  display.fillCircle(ballX, ballY, 2, SSD1306_INVERSE);
  
  display.display();


  


}

void leveldraw(){
  int i, j;
  for(i = 0; i < 4; i++){
    for(j = 0; j < 8; j++){
      if(bloklar[i][j] == 1){
        display.fillRect(j * blokX, i * blokY, blokX - 1, blokY - 1, SSD1306_WHITE);
      }
    }
  }
}
void level1setup(){
  int i;
  bloklar[0][2] = 1;
  bloklar[0][4] = 1;
  for(i = 1; i < 6; i++){
    bloklar[1][i] = 1;
  }
  for(i = 2; i < 5; i++){
    bloklar[2][i] = 1;
  }
  bloklar[3][3] = 1;
}

void level2setup(){
  for(int i = 0; i < 8; i++){
    bloklar[3][i] = 1;
  }
  for(int i = 0; i < 8; i++){
    bloklar[1][i] = 1;
  }
}
void level3setup(){
  bloklar[0][3] = 1;
  bloklar[0][4] = 1;

  bloklar[1][2] = 1;
  bloklar[1][5] = 1;

  bloklar[2][1] = 1;
  bloklar[2][6] = 1;

  for(int i = 0; i < 8; i++){
    bloklar[3][i] = 1;
  }
}

void level4setup(){
  int i;
  for(i = 0; i < 8; i += 2) bloklar[0][i] = 1;
  for(i = 1; i < 8; i += 2) bloklar[1][i] = 1;
  for(i = 0; i < 8; i += 2) bloklar[2][i] = 1;
  for(i = 1; i < 8; i += 2) bloklar[3][i] = 1;
}

void level5setup(){
  int i, j;
  for(i = 0; i < 2; i++){
    for(j = 0; j < 8; j++){
      bloklar[i][j] = 1;
    }
  }
}

void launch(){
  ballX = potansiyometreDeger + 14;
  ballY = 61;
  draw();
  float tempY = speedY;
  float tempX = speedX;
  speedX = 0;
  speedY = 0;
  int temp = potansiyometreDeger;
  while(true){
    potansiyometreDeger = analogRead(A0);
    potansiyometreDeger = map(potansiyometreDeger, 0, 1023, 0, 100);
    if(temp == potansiyometreDeger) continue;
    if(potansiyometreDeger > temp) speedX = abs(tempX);
    else speedX = abs(tempX)* -1;
    speedY = abs(tempY)* -1;
    break;
  }
  return;
}

void displayMenu(){
  display.clearDisplay();
  updateMenuState();
  
  if(currentState == START){
    display.setTextSize(2);
    display.setCursor(20, 10);
    display.setTextColor(WHITE);
    display.println("START");
    display.setTextSize(1);
    display.setCursor(20, 30);
    display.setTextColor(BLACK, WHITE);  // Inverted text for non-selected option
    display.println("QUIT");
  }
  else if(currentState == QUIT){
    display.setTextSize(2);
    display.setCursor(20, 30);
    display.setTextColor(WHITE);
    display.println("QUIT");
    display.setTextSize(1);
    display.setCursor(20, 10);
    display.setTextColor(BLACK, WHITE);  // Inverted text for non-selected option
    display.println("START");
  }
  display.display();
}

void updateMenuState(){
  potansiyometreDeger = analogRead(A0);
  potansiyometreDeger = map(potansiyometreDeger, 0, 1023, 0, 100);
  if (potansiyometreDeger < 25) {
    currentState = START;
  } 
  if (potansiyometreDeger > 75) {
    currentState = QUIT;
  }

}

void waitScreen(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Puan: " + (String)points);
  display.display();
}