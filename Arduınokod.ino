#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD ve Pin Tanımlamaları - I2C için güncellendi (Adres genellikle 0x27 olur)
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int btnJump = 2;   
const int btnShoot = 7;  
const int btnExit = 9;   
const int btnVol = 13;   
const int buzzerPin = 8; 

// Oyun Değişkenleri
int activeGame = 0; 
int score = 0;
int noteIndex = 0;
int gameSpeed = 90;
int ballSpeed = 120; 
bool shootReleased = true;
bool jumpReleased = true;
bool bootDone = false; 
bool volStatus = true; 

// Karakter Tanımlamaları
byte charDino[8] = { 7, 5, 7, 4, 14, 10, 10, 0 };      
byte charPartner[8] = { 28, 20, 28, 4, 14, 10, 10, 0 }; 
byte charCactus[8] = { 4, 5, 21, 21, 23, 4, 0, 0 };
byte charBird[8] = { 4, 14, 31, 4, 4, 0, 0, 0 };
byte charTarget[8] = { 4, 10, 10, 10, 4, 4, 8, 0 };
byte charBullet[8] = { 0, 0, 0, 7, 7, 0, 0, 0 };
byte charHeart[8] = { 0, 10, 31, 31, 31, 14, 4, 0 };
byte charSpeaker[8] = { 1, 3, 15, 15, 15, 3, 1, 0 }; 

int dY = 1, oX = 15, oY = 1;
unsigned long jumpTime = 0;
bool jumping = false;
int pY = 1, bullX = -1, bullY = 0, tarX = 15, tarY = 0; 

const int marioLen = 135;
const int marioMelody[] = { 659, 659, 0, 659, 0, 523, 659, 0, 784, 0, 392, 0, 523, 392, 330, 440, 494, 466, 440, 392, 659, 784, 880, 698, 784, 659, 523, 587, 494, 523, 392, 330, 440, 494, 466, 440, 392, 659, 784, 880, 698, 784, 659, 523, 587, 494, 0, 784, 740, 698, 622, 659, 0, 415, 440, 523, 0, 440, 523, 587, 0, 784, 740, 698, 622, 659, 0, 1046, 1046, 1046, 0, 784, 740, 698, 622, 659, 0, 415, 440, 523, 0, 440, 523, 587, 0, 622, 0, 587, 0, 523, 523, 523, 0, 523, 0, 523, 587, 0, 659, 523, 440, 392, 0, 523, 523, 0, 523, 0, 523, 587, 659, 0, 784, 740, 698, 622, 659, 0, 415, 440, 523, 0, 440, 523, 587, 0, 622, 0, 587, 0, 523, 330, 0, 262, 0, 196, 0, 165, 0, 220, 0, 247, 0, 233, 0, 220, 0 };
const int ballLen = 16;
const int ballMelody[] = { 523, 0, 523, 587, 659, 0, 659, 0, 587, 523, 440, 0, 392, 440, 523, 0 };

void playTone(int freq, int dur) {
  if (volStatus && freq > 0) tone(buzzerPin, freq, dur);
}

void clickSound() {
  if (volStatus) tone(buzzerPin, 1000, 15);
}

// GÜNCELLENMİŞ MUTE: Yazıyı geri getirir
void toggleMute(String currentMsg) {
  unsigned long muteStart = millis();
  volStatus = !volStatus;
  while(millis() - muteStart < 3000) {
    lcd.clear();
    if (!volStatus) {
      lcd.setCursor(4, 0); lcd.print("VOL:"); lcd.write(7); lcd.print(" X");
      lcd.setCursor(6, 1); lcd.print("OFF");
    } else {
      lcd.setCursor(5, 0); lcd.print("VOL:"); lcd.write(7);
      lcd.setCursor(7, 1); lcd.print("ON");
    }
    delay(150); 
    if (digitalRead(btnVol) == LOW) {
      volStatus = !volStatus;
      muteStart = millis(); 
      delay(200);
    }
    if (digitalRead(btnJump) == LOW || digitalRead(btnShoot) == LOW) break;
  }
  lcd.clear();
  // Yazı ekranına geri dönüyorsak yazıyı tekrar bas
  if (currentMsg != "") {
    lcd.setCursor((16 - currentMsg.length()) / 2, 0); 
    lcd.print(currentMsg);
  } else if (activeGame == 0) {
    showMenu();
  }
}

void playCMajorScale() {
  int scale[] = {262, 294, 330, 349, 392, 440, 494, 523};
  for (int i = 0; i < 8; i++) {
    if(volStatus) tone(buzzerPin, scale[i], 150);
    delay(200);
  }
}

void playDeathSound() {
  if(!volStatus) return;
  tone(buzzerPin, 494, 150); delay(150);
  tone(buzzerPin, 440, 150); delay(150);
  tone(buzzerPin, 349, 300); delay(300);
  noTone(buzzerPin);
}

void startSequence() {
  noTone(buzzerPin);
  lcd.clear();
  lcd.setCursor(3, 0); lcd.print("PLAY GAMES");
  lcd.setCursor(0, 1); lcd.print("PRESS THE BUTTON");
  delay(500); 
  while(digitalRead(btnJump) == HIGH && digitalRead(btnShoot) == HIGH && digitalRead(btnExit) == HIGH) {
    if(digitalRead(btnVol) == LOW) toggleMute("");
    delay(10); 
  }
  lcd.clear();
  lcd.setCursor(2, 0); lcd.print("LOADING ...");
  delay(5000); 
  bootDone = true;
  showMenu();
}

void setup() {
  // I2C ekran başlatma komutları eklendi
  lcd.init();
  lcd.backlight();
  
  pinMode(btnJump, INPUT_PULLUP);
  pinMode(btnShoot, INPUT_PULLUP);
  pinMode(btnExit, INPUT_PULLUP);
  pinMode(btnVol, INPUT_PULLUP); 
  lcd.createChar(0, charDino);
  lcd.createChar(1, charCactus);
  lcd.createChar(2, charTarget);
  lcd.createChar(3, charBird);
  lcd.createChar(4, charBullet);
  lcd.createChar(5, charHeart);
  lcd.createChar(6, charPartner);
  lcd.createChar(7, charSpeaker); 
  startSequence();
}

void showMenu() {
  noTone(buzzerPin);
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("1:BALLON  2:DINO"); 
  lcd.setCursor(1, 1); lcd.print("SELECT & PLAY!");
}

void showFinal(String msg, int gameType) {
  noTone(buzzerPin);
  lcd.clear();
  
  if (msg == "VICTORY!") {
    if (gameType == 1) { 
      lcd.setCursor(15, 1); lcd.write(byte(6)); 
      for (int i = 0; i <= 14; i++) {
        lcd.setCursor(i-1, 1); lcd.print(" ");
        lcd.setCursor(i, 1); lcd.write(byte(0));
        if(volStatus) tone(buzzerPin, 200, 50); 
        delay(350);
      }
      lcd.setCursor(14, 0); lcd.write(byte(5));
      playCMajorScale();
    } 
    else {
      lcd.setCursor((16 - msg.length()) / 2, 0); lcd.print(msg);
      String sFinal = "SCORE: " + String(score);
      lcd.setCursor((16 - sFinal.length()) / 2, 1); lcd.print(sFinal);
      playCMajorScale();
    }
  } else {
    lcd.setCursor((16 - msg.length()) / 2, 0); lcd.print(msg);
    playDeathSound();
  }

  lcd.clear();
  lcd.setCursor((16 - msg.length()) / 2, 0); lcd.print(msg);

  bool toggle = true;
  unsigned long lastFlash = 0;
  while (digitalRead(btnJump) == HIGH && digitalRead(btnShoot) == HIGH && digitalRead(btnExit) == HIGH) {
    if(digitalRead(btnVol) == LOW) toggleMute(msg); // Mesajı korumaya gönder
    if (millis() - lastFlash > 1000) {
      lastFlash = millis();
      lcd.setCursor(0, 1); lcd.print("                "); 
      if (toggle) {
        String s = "SCORE: " + String(score);
        lcd.setCursor((16 - s.length()) / 2, 1); lcd.print(s);
      } else {
        lcd.setCursor(1, 1); lcd.print("PRESS RESTART");
      }
      toggle = !toggle;
    }
  }
  clickSound(); 
  activeGame = 0; noteIndex = 0; showMenu(); delay(300);
}

void playDino() {
  if(digitalRead(btnVol) == LOW) toggleMute("");
  if (marioMelody[noteIndex] != 0) playTone(marioMelody[noteIndex], 45);
  else noTone(buzzerPin);
  noteIndex++; if (noteIndex >= marioLen) noteIndex = 0;

  if (digitalRead(btnJump) == LOW && !jumping) {
    jumping = true; jumpTime = millis(); dY = 0; 
  }
  if (jumping && (millis() - jumpTime > 310)) { jumping = false; dY = 1; }

  oX--;
  if (oX < 0) {
    oX = random(12, 16); oY = random(0, 2); score += 5;
    if (score >= 400) { showFinal("VICTORY!", 1); return; }
    if (score > 0 && score % 20 == 0 && gameSpeed > 40) gameSpeed -= 4;
  }
  if (oX == 0 && oY == dY) { showFinal("GAME OVER", 1); return; }

  lcd.clear();
  lcd.setCursor(0, dY); lcd.write(byte(0));
  lcd.setCursor(oX, oY); if (oY == 1) lcd.write(byte(1)); else lcd.write(byte(3));
  lcd.setCursor(11, 0); lcd.print("S:"); lcd.print(score);
  delay(gameSpeed);
}

void playBalloon() {
  if(digitalRead(btnVol) == LOW) toggleMute("");
  if (ballMelody[noteIndex] != 0) playTone(ballMelody[noteIndex], 60);
  else noTone(buzzerPin);
  noteIndex++; if (noteIndex >= ballLen) noteIndex = 0;

  if (digitalRead(btnJump) == LOW) { 
    if(jumpReleased) { pY = !pY; jumpReleased = false; }
  } else jumpReleased = true;

  if (digitalRead(btnShoot) == LOW) {
    if (shootReleased && bullX == -1) { 
      bullX = 1; bullY = pY; 
      shootReleased = false; 
    }
  } else shootReleased = true;

  lcd.clear();
  lcd.setCursor(0, pY); lcd.write(byte(0));
  tarX--; 
  if (tarX < 0 || (tarX == 0 && tarY == pY)) { showFinal("GAME OVER", 2); return; }
  lcd.setCursor(tarX, tarY); lcd.write(byte(2));

  if (bullX != -1) {
    lcd.setCursor(bullX, bullY); lcd.write(byte(4)); 
    bullX++;
    if (bullY == tarY && (bullX >= tarX)) { 
      score += 5; tarX = 15; tarY = random(0, 2); bullX = -1;
      if(volStatus) tone(buzzerPin, 2000, 30);
      if (score % 20 == 0 && ballSpeed > 50) ballSpeed -= 10;
      if (score >= 200) { showFinal("VICTORY!", 2); return; }
    }
    if (bullX > 15) bullX = -1;
  }
  lcd.setCursor(11, 0); lcd.print("S:"); lcd.print(score);
  delay(ballSpeed); 
}

void loop() {
  if (digitalRead(btnExit) == LOW) { 
    clickSound();
    if (activeGame != 0) {
      activeGame = 0; noteIndex = 0; showMenu(); delay(500); 
    } else {
      bootDone = false; noteIndex = 0; activeGame = 0; startSequence();
    }
  }
  if (activeGame == 0 && bootDone) {
    if(digitalRead(btnVol) == LOW) toggleMute("");
    if (digitalRead(btnJump) == LOW) { 
      clickSound(); 
      activeGame = 1; score = 0; oX = 15; dY = 1; noteIndex = 0; gameSpeed = 90; 
      lcd.clear(); delay(300); 
    }
    if (digitalRead(btnShoot) == LOW) { 
      clickSound(); 
      activeGame = 2; score = 0; tarX = 15; bullX = -1; noteIndex = 0; ballSpeed = 120; 
      shootReleased = false; lcd.clear(); delay(300); 
    }
  } 
  else if (activeGame == 1) playDino();
  else if (activeGame == 2) playBalloon();
}
