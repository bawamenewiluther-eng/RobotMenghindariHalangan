#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

WebServer server(80);
// WIFI ACCESS POINT
const char* ssid = "ESP32_CONTROL";
const char* password = "synel 1.1";


// PIN L298N
// MOTOR A
#define ENA 25
#define IN1 26
#define IN2 27

// MOTOR B
#define ENB 33
#define IN3 14
#define IN4 13

// PWM ESP32
const int freq = 5000;
const int resolution = 8;

const int channelA = 0;
const int channelB = 1;


// SPEED SYSTEM
int speedMode = 1; // 0=SLOW 1=MEDIUM 2=FAST
int pwmSpeed = 170;

//servo
#define SERVO_LEFT_PIN 4
#define SERVO_RIGHT_PIN 5

Servo servoLeft;
Servo servoRight;

int servoLeftPos = 0;
int servoRightPos = 0;

bool servoLeftUp = false;
bool servoLeftDown = false;

bool servoRightUp = false;
bool servoRightDown = false;

//servo kepala
#define SERVO_HEAD_PIN 15
Servo servoHead;

int rightDistance = 0;
int leftDistance = 0;

//Sensor Ultrasonik
#define TRIG_PIN 18
#define ECHO_PIN 19

unsigned long lastUltraRead = 0;

long duration;
int distanceCm;

static bool brakePrinted = false;

int readDistance(){

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 30000);

  int distance = duration * 0.034 / 2;

  return distance;
}
bool autoBlocked = false;

//mode otomatis
bool autoMode = false;

// MOTOR FUNCTION
void stopMotor() {

  ledcWrite(channelA, 0);
  ledcWrite(channelB, 0);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  Serial.println("STOP");
}

void maju() {

  ledcWrite(channelA, pwmSpeed);
  ledcWrite(channelB, pwmSpeed);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  Serial.println("MAJU");
}

void mundur() {

  ledcWrite(channelA, pwmSpeed);
  ledcWrite(channelB, pwmSpeed);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  Serial.println("MUNDUR");
}

void kiri() {

  ledcWrite(channelA, pwmSpeed);
  ledcWrite(channelB, pwmSpeed);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  Serial.println("KIRI");
}

void kanan() {

  ledcWrite(channelA, pwmSpeed);
  ledcWrite(channelB, pwmSpeed);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  Serial.println("KANAN");
}

// HTML WEB PAGE

String webpage = R"rawliteral(

<!DOCTYPE html>
<html>

<head>

<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">

<title>ESP32 RC</title>

<style>

body{
  margin:0;
  background:#e9e9e9;
  font-family:Arial;
  overflow:hidden;
  user-select:none;
}

/* ======================================================
   TOP BUTTON
====================================================== */

.top-left{
  position:absolute;
  top:20px;
  left:20px;
}

.top-right{
  position:absolute;
  top:20px;
  right:20px;
}

/* ======================================================
   MIDDLE BUTTON
====================================================== */

.middle-left{
  position:absolute;
  left:20px;
  top:50%;
  transform:translateY(-50%);
}

.middle-right{
  position:absolute;
  right:20px;
  top:50%;
  transform:translateY(-50%);
}

.mode-btn{
  width:80px;
  height:80px;

  border-radius:25px;
  border:4px solid black;

  background:white;

  font-size:40px;
  font-weight:bold;

  box-shadow:4px 4px 0px #666;
}

/* ======================================================
   WASD
====================================================== */

.keyboard{
  position:absolute;
  bottom:40px;
  left:30px;
}

.row{
  display:flex;
  justify-content:center;
}

.key{
  width:70px;
  height:70px;

  margin:5px;

  border-radius:15px;
  border:3px solid #444;

  background:white;

  font-size:35px;

  box-shadow:4px 4px 0px #666;
}

/* ======================================================
   SPEED
====================================================== */

.speed-box{
  position:absolute;
  bottom:50px;
  left:50%;
  transform:translateX(-50%);

  display:flex;
  align-items:center;
}

.speed-btn{
  width:70px;
  height:70px;

  border-radius:20px;
  border:4px solid black;

  background:white;

  font-size:45px;
  font-weight:bold;

  box-shadow:4px 4px 0px #666;
}

.slider-box{
  margin:0 20px;
  text-align:center;
}

.slider-box h1{
  margin:0;
  font-style:italic;
  font-weight:normal;
}

.speed-level{

  width:220px;
  height:35px;

  border-radius:20px;
  border:3px solid black;

  background:white;

  display:flex;
  align-items:center;
  justify-content:center;

  font-size:22px;
  font-weight:bold;

  margin-top:10px;
}

/* ======================================================
   ARROW
====================================================== */

.arrow-pad{
  position:absolute;
  bottom:40px;
  right:30px;
}

.arrow-row{
  display:flex;
  justify-content:center;
}

.arrow{
  width:70px;
  height:70px;

  margin:5px;

  border-radius:12px;
  border:2px solid #666;

  background:#f4ddb2;

  font-size:35px;

  box-shadow:3px 3px 0px #777;
}

.empty{
  visibility:hidden;
}
/* ======================================================
   AUTO MODE
====================================================== */

.auto-box{

  position:absolute;

  top:20px;
  left:50%;

  transform:translateX(-50%);
}

.auto-btn{

  width:170px;
  height:70px;

  border-radius:20px;
  border:4px solid black;

  background:white;

  font-size:28px;
  font-weight:bold;

  box-shadow:4px 4px 0px #666;
}

/* ======================================================
   BUTTON EFFECT
====================================================== */

button:active{
  transform:scale(0.92);
}

</style>

</head>

<body>

<!-- ======================================================
     TOP BUTTON
====================================================== -->
<div class="auto-box">

  <button class="auto-btn"
    onclick="toggleAuto()"
    id="autoBtn">

    AUTO OFF

  </button>

</div>

<div class="top-left">
 <button class="mode-btn"

  onmousedown="sendCommand('q')"
  onmouseup="sendCommand('servoStop')"

  ontouchstart="sendCommand('q')"
  ontouchend="sendCommand('servoStop')">

  Q

</button>
</div>

<div class="top-right">
  <button class="mode-btn"

  onmousedown="sendCommand('e')"
  onmouseup="sendCommand('servoStop')"

  ontouchstart="sendCommand('e')"
  ontouchend="sendCommand('servoStop')">

  E

</button>
</div>

<!-- ======================================================
     MIDDLE BUTTON
====================================================== -->

<div class="middle-left">
 <button class="mode-btn"

  onmousedown="sendCommand('z')"
  onmouseup="sendCommand('servoStop')"

  ontouchstart="sendCommand('z')"
  ontouchend="sendCommand('servoStop')">

  Z

</button>
</div>

<div class="middle-right">
  <button class="mode-btn"

  onmousedown="sendCommand('x')"
  onmouseup="sendCommand('servoStop')"

  ontouchstart="sendCommand('x')"
  ontouchend="sendCommand('servoStop')">

  X

</button>
</div>

<!-- ======================================================
     WASD
====================================================== -->

<div class="keyboard">

  <div class="row">

    <button class="key"

      onmousedown="sendCommand('forward')"
      onmouseup="sendCommand('stop')"

      ontouchstart="sendCommand('forward')"
      ontouchend="sendCommand('stop')">

      W

    </button>

  </div>

  <div class="row">

    <button class="key"

      onmousedown="sendCommand('left')"
      onmouseup="sendCommand('stop')"

      ontouchstart="sendCommand('left')"
      ontouchend="sendCommand('stop')">

      A

    </button>

    <button class="key"

      onmousedown="sendCommand('backward')"
      onmouseup="sendCommand('stop')"

      ontouchstart="sendCommand('backward')"
      ontouchend="sendCommand('stop')">

      S

    </button>

    <button class="key"

      onmousedown="sendCommand('right')"
      onmouseup="sendCommand('stop')"

      ontouchstart="sendCommand('right')"
      ontouchend="sendCommand('stop')">

      D

    </button>

  </div>

</div>

<!-- ======================================================
     SPEED
====================================================== -->

<div class="speed-box">

  <button class="speed-btn" id="minusBtn">
    -
  </button>

  <div class="slider-box">

    <h1>Kecepatan</h1>

    <div class="speed-level" id="speedText">
      MEDIUM
    </div>

  </div>

  <button class="speed-btn" id="plusBtn">
    +
  </button>

</div>

<!-- ======================================================
     ARROW
====================================================== -->

<div class="arrow-pad">

  <div class="arrow-row">

    <div class="empty arrow"></div>

    <button class="arrow"

      onmousedown="sendCommand('forward')"
      onmouseup="sendCommand('stop')"

      ontouchstart="sendCommand('forward')"
      ontouchend="sendCommand('stop')">

      ↑

    </button>

    <div class="empty arrow"></div>

  </div>

  <div class="arrow-row">

    <button class="arrow"

      onmousedown="sendCommand('left')"
      onmouseup="sendCommand('stop')"

      ontouchstart="sendCommand('left')"
      ontouchend="sendCommand('stop')">

      ←

    </button>

    <button class="arrow"

      onmousedown="sendCommand('backward')"
      onmouseup="sendCommand('stop')"

      ontouchstart="sendCommand('backward')"
      ontouchend="sendCommand('stop')">

      ↓

    </button>

    <button class="arrow"

      onmousedown="sendCommand('right')"
      onmouseup="sendCommand('stop')"

      ontouchstart="sendCommand('right')"
      ontouchend="sendCommand('stop')">

      →

    </button>

  </div>

</div>

<script>

// ======================================================
// SEND COMMAND
// ======================================================

function sendCommand(cmd){

  // blok manual movement saat auto
  if(autoMode){

    if(
      cmd == 'forward' ||
      cmd == 'backward' ||
      cmd == 'left' ||
      cmd == 'right'
    ){
      return;
    }

  }

  fetch('/' + cmd)
  .catch(err => console.log(err));

}
let autoMode = false;

function toggleAuto(){

  autoMode = !autoMode;

  fetch('/auto');

  const btn = document.getElementById("autoBtn");

  if(autoMode){

    btn.innerHTML = "AUTO ON";
    btn.style.background = "#7dff7d";

  }else{

    btn.innerHTML = "AUTO OFF";
    btn.style.background = "white";

  }

}

// ======================================================
// SPEED SYSTEM
// ======================================================

let speedMode = 1;

const speedNames = [
  "SLOW",
  "MEDIUM",
  "FAST"
];

function updateSpeed(){

  document.getElementById("speedText").innerHTML =
    speedNames[speedMode];

  fetch('/speed?value=' + speedMode)
  .catch(err => console.log(err));
}

// ======================================================
// BUTTON SPEED
// ======================================================

document.getElementById("minusBtn")
.addEventListener('click', () => {

  if(speedMode > 0){

    speedMode--;
    updateSpeed();

  }

});

document.getElementById("plusBtn")
.addEventListener('click', () => {

  if(speedMode < 2){

    speedMode++;
    updateSpeed();

  }

});

// ======================================================
// KEYBOARD CONTROL
// ======================================================

document.addEventListener('keydown', function(event){

  let key = event.key.toLowerCase();

  // SPEED

  if(event.key === '+' || event.key === '='){

    if(speedMode < 2){

      speedMode++;
      updateSpeed();

    }

  }

  if(event.key === '-'){

    if(speedMode > 0){

      speedMode--;
      updateSpeed();

    }

  }

  // MOVEMENT

if(!autoMode){

  if(key == 'w'){
    sendCommand('forward');
  }

  if(key == 's'){
    sendCommand('backward');
  }

  if(key == 'a'){
    sendCommand('left');
  }

  if(key == 'd'){
    sendCommand('right');
  }

}

  if(key == 'q'){
  sendCommand('q');
}

if(key == 'e'){
  sendCommand('e');
}

if(key == 'z'){
  sendCommand('z');
}

if(key == 'x'){
  sendCommand('x');
}
if(key == 'p'){

  toggleAuto();

}

});

// STOP

document.addEventListener('keyup', function(event){

  let key = event.key.toLowerCase();

  // MOTOR STOP
  if(
    key == 'w' ||
    key == 'a' ||
    key == 's' ||
    key == 'd'
  ){
    sendCommand('stop');
  }

  // SERVO STOP
  if(
    key == 'q' ||
    key == 'e' ||
    key == 'z' ||
    key == 'x'
  ){
    sendCommand('servoStop');
  }

});

</script>

</body>
</html>

)rawliteral";

// SETUP

void setup() {

  Serial.begin(115200);
  // MOTOR PIN

  servoHead.attach(SERVO_HEAD_PIN);
  servoHead.write(90);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  // PWM

  ledcSetup(channelA, freq, resolution);
  ledcAttachPin(ENA, channelA);

  ledcSetup(channelB, freq, resolution);
  ledcAttachPin(ENB, channelB);

  stopMotor();

  // START ACCESS POINT

  WiFi.softAP(ssid, password);

  Serial.println("");
  Serial.println("ESP32 WIFI START");

  Serial.print("IP ADDRESS: ");
  Serial.println(WiFi.softAPIP());

  servoLeft.attach(SERVO_LEFT_PIN);
  servoRight.attach(SERVO_RIGHT_PIN);

  // paksa ke posisi 0 saat startup

  servoLeft.write(0);
  servoRight.write(0);

  servoLeftPos = 0;
  servoRightPos = 0;

  delay(500);

  // WEB PAGE

  server.on("/", []() {

    server.send(200, "text/html", webpage);

  });
  // MOVEMENT
  server.on("/forward", []() {

  // kalau depan ada halangan
  if(autoBlocked){

    stopMotor();

    Serial.println("BLOCKED!");

  }else{

    maju();

  }

  server.send(200, "text/plain", "OK");

});

  server.on("/backward", []() {

    mundur();

    server.send(200, "text/plain", "OK");

  });

  server.on("/left", []() {

    kiri();

    server.send(200, "text/plain", "OK");

  });

  server.on("/right", []() {

    kanan();

    server.send(200, "text/plain", "OK");

  });

  // SPEED
  server.on("/speed", []() {

    if(server.hasArg("value")){

      speedMode = server.arg("value").toInt();

      // SLOW
      if(speedMode == 0){
        pwmSpeed = 130;
      }

      // MEDIUM
      if(speedMode == 1){
        pwmSpeed = 160;
      }

      // FAST
      if(speedMode == 2){
        pwmSpeed = 255;
      }

      Serial.print("PWM SPEED: ");
      Serial.println(pwmSpeed);

    }

    server.send(200, "text/plain", "OK");

  });

//servo kiri naik
  server.on("/q", []() {

    servoLeftUp = true;
    servoLeftDown = false;

    if(servoLeftPos > 180){
      servoLeftPos = 180;
    }

    servoLeft.write(servoLeftPos);

    Serial.print("SERVO LEFT: ");
    Serial.println(servoLeftPos);

    server.send(200, "text/plain", "OK");
  });

// servo kiri turun
  server.on("/z", []() {
    servoLeftDown = true;
    servoLeftUp = false;
    if(servoLeftPos < 0){
      servoLeftPos = 0;
    }

    servoLeft.write(servoLeftPos);

    Serial.print("SERVO LEFT: ");
    Serial.println(servoLeftPos);

    server.send(200, "text/plain", "OK");
  });

//servo kanan naik
  server.on("/e", []() {
    servoRightUp = true;
    servoRightDown = false;
    if(servoRightPos > 180){
      servoRightPos = 180;
    }

    servoRight.write(servoRightPos);

    Serial.print("SERVO RIGHT: ");
    Serial.println(servoRightPos);

    server.send(200, "text/plain", "OK");
});
//servo kanan turun
  server.on("/x", []() {
    servoRightDown = true;
    servoRightUp = false;
    if(servoRightPos < 0){
      servoRightPos = 0;
    }

    servoRight.write(servoRightPos);

    Serial.print("SERVO RIGHT: ");
    Serial.println(servoRightPos);

    server.send(200, "text/plain", "OK");
  });

  // STOP

  server.on("/stop", []() {

    stopMotor();

    server.send(200, "text/plain", "OK");

  });
  server.on("/servoStop", []() {

  servoLeftUp = false;
  servoLeftDown = false;

  servoRightUp = false;
  servoRightDown = false;

  server.send(200, "text/plain", "OK");
});
  // otomatis 
  server.on("/auto", []() {

  autoMode = !autoMode;
  if(!autoMode){

  stopMotor();

}
  Serial.print("AUTO MODE: ");

  if(autoMode){
    Serial.println("ON");
  }else{
    Serial.println("OFF");
  }

  server.send(200, "text/plain", "OK");

});


  // START SERVER

  server.begin();

  Serial.println("WEB SERVER START");

}


// LOOP

void loop() {

  server.handleClient();

  //ultrasonik
  if(millis() - lastUltraRead > 100){

    lastUltraRead = millis();

    distanceCm = readDistance();

    Serial.print("Distance: ");
    Serial.print(distanceCm);
    Serial.println(" cm");

  }

    // AUTO BLOCK

if(distanceCm < 12 && !autoMode){

  autoBlocked = true;

  stopMotor();

  if(!brakePrinted){

    Serial.println("AUTO BRAKE!");
    brakePrinted = true;

  }

}else{

  autoBlocked = false;
  brakePrinted = false;

}

  if(autoMode){

  // kalau depan aman
  if(distanceCm > 12){

    maju();

  }else{

    // STOP
    stopMotor();
    delay(200);

    // =========================
    // CEK KANAN
    // =========================

    servoHead.write(180);
    delay(1000);

    rightDistance = readDistance();

    Serial.print("RIGHT: ");
    Serial.println(rightDistance);

    servoHead.write(90);
    delay(1000);

    // =========================
    // CEK KIRI
    // =========================

    servoHead.write(0);
    delay(1000);

    leftDistance = readDistance();

    Serial.print("LEFT: ");
    Serial.println(leftDistance);

    // =========================
    // KEMBALI DEPAN
    // =========================

    servoHead.write(90);
    delay(1000);

    // =========================
    // DECISION
    // =========================

    // kanan kosong
    // kanan & kiri buntu
 // ====================================
// KANAN & KIRI BUNTU
// ====================================

if(rightDistance < 20 && leftDistance < 20){

  Serial.println("PUTAR BALIK");

  kanan();

  if(speedMode == 0){

    delay(1100);

  }
  else if(speedMode == 1){

    delay(700);

  }
  else{

    delay(450);

  }

}

// ====================================
// KANAN KOSONG
// ====================================

else if(rightDistance > 20 && leftDistance < 20){

  Serial.println("BEL0K KANAN");

  kanan();
  delay(300);

}

// ====================================
// KIRI KOSONG
// ====================================

else if(leftDistance > 20 && rightDistance < 20){

  Serial.println("BEL0K KIRI");

  kiri();
  delay(300);

}

// ====================================
// KANAN & KIRI KOSONG
// PRIORITAS KANAN
// ====================================

else{

  Serial.println("KANAN PRIORITAS");

  kanan();
  delay(300);

}
  }
  }
// SERVO LEFT UP

if(servoLeftUp){

  servoLeftPos++;

  if(servoLeftPos > 360){
    servoLeftPos = 360;
  }

  servoLeft.write(servoLeftPos);

  delay(2);
}

// SERVO LEFT DOWN

if(servoLeftDown){

  servoLeftPos--;

  if(servoLeftPos < 0){
    servoLeftPos = 0;
  }

  servoLeft.write(servoLeftPos);

  delay(2);
}

// SERVO RIGHT UP

if(servoRightUp){

  servoRightPos++;

  if(servoRightPos > 360){
    servoRightPos = 360;
  }

  servoRight.write(servoRightPos);

  delay(2);
}


// SERVO RIGHT DOWN

if(servoRightDown){

  servoRightPos--;

  if(servoRightPos < 0){
    servoRightPos = 0;
  }

  servoRight.write(servoRightPos);

  delay(2);
}
}