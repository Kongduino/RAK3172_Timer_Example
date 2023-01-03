#include "rak1901.h"
#include "rak1902.h"

rak1901 rak1901;
rak1902 rak1902;
uint32_t lastTime;
#define period 30 // seconds
double myFreq = 868125000;
uint16_t sf = 12;
uint16_t bw = 0;
uint16_t cr = 0;
uint16_t preamble = 8;
uint16_t txPower = 22;
uint32_t myBWs[10] = {125, 250, 500, 7.8, 10.4, 15.63, 20.83, 31.25, 41.67, 62.5};

void OnTimer0Interrupt(void *data) {
  digitalWrite(LED_BLUE, HIGH);
  Serial.print("Time elapsed: ");
  Serial.println(millis() - lastTime);
  float t = 255.0, h = 255.0, p;
  if (rak1901.update()) {
    t = rak1901.temperature();
    h = rak1901.humidity();
    Serial.printf("Temperature = %.2f°C\n", t);
    Serial.printf("Humidity = %.2f%%\n", h);
  }
  p = rak1902.pressure();
  Serial.printf("Pressure = %.2f hPa\n", p);
  char msgToSend[96];
  sprintf(msgToSend, "T: %.2f; H: %.2f; P: %.2f", t, h, p);
  uint8_t ln = strlen(msgToSend);
  bool rslt = api.lorawan.precv(0);
  // Serial.printf("Stopped Rx mode: %s\n", rslt ? "ok" : "x");
  bool isSending = api.lorawan.psend(ln, (uint8_t*)msgToSend);
  Serial.printf("Sending `%s`, len %d via P2P: %s\n", msgToSend, ln, isSending ? "ok" : "x");

  lastTime = millis();
  delay(500);
  digitalWrite(LED_BLUE, LOW);
}

void send_cb(void) {
  // TX callback
  Serial.println("Tx done!");
  api.lorawan.precv(65534);
}

void recv_cb(rui_lora_p2p_recv_t data) {
  uint16_t ln = data.BufferSize;
  char buff[92];
  sprintf(buff, "Incoming message, length: %d, RSSI: %d, SNR: %d", data.BufferSize, data.Rssi, data.Snr);
  Serial.println(buff);
}

void failMsg(char *txt) {
  Serial.printf("Failed at %s. Stopping...", txt);
  while (1);
}

void setup() {
  pinMode(LED_BLUE, OUTPUT);
  Serial.begin(115200);
  delay(5000);
  uint8_t x = 5;
  while (x > 0) {
    Serial.printf("%d, ", x--);
    delay(500);
  }
  Serial.println("0!");
  Wire.begin();
  // check whether RAK1901 sensor is working
  bool rslt = rak1901.init();
  Serial.printf("RAK1901 init %s\r\n", rslt ? "Success" : "Fail");
  if (!rslt) while (1) ;
  rslt = rak1902.init();
  Serial.printf("RAK1902 init %s\r\n", rslt ? "Success" : "Fail");
  if (!rslt) while (1) ;

  rslt = api.lorawan.nwm.set(0);
  char msg[32];
  sprintf(msg, "P2P mode: %s", rslt ? "ok" : "x");
  Serial.println(msg);
  if (!rslt) failMsg("nwm");

  rslt = api.lorawan.pfreq.set(myFreq);
  sprintf(msg, "Freq %3.3f: %s", (myFreq / 1e6), rslt ? "ok" : "x");
  Serial.println(msg);

  if (!rslt) failMsg("pfreq");

  rslt = api.lorawan.psf.set(sf);
  sprintf(msg, "SF %d: %s", sf, rslt ? "ok" : "x");
  Serial.println(msg);

  if (!rslt) failMsg("psf");

  rslt = api.lorawan.pbw.set(bw);
  sprintf(msg, "BW %d, %d KHz: %s", bw, myBWs[bw], rslt ? "ok" : "x");
  Serial.println(msg);

  if (!rslt) failMsg("pbw");

  rslt = api.lorawan.pcr.set(cr);
  sprintf(msg, "C/R 4/%d: %s", (cr + 5), rslt ? "ok" : "x");
  Serial.println(msg);

  if (!rslt) failMsg("pcr");

  rslt = api.lorawan.ppl.set(preamble);
  sprintf(msg, "preamble %d b: %s", preamble, rslt ? "ok" : "x");
  Serial.println(msg);

  if (!rslt) failMsg("ppl");

  rslt = api.lorawan.ptp.set(txPower);
  sprintf(msg, "TX power %d: %s", txPower, rslt ? "ok" : "x");
  Serial.println(msg);

  if (!rslt) failMsg("ptp");

  api.lorawan.registerPRecvCallback(recv_cb);
  api.lorawan.registerPSendCallback(send_cb);

  rslt = api.lorawan.precv(65534);
  Serial.printf("Set to Rx mode: %s\n", rslt ? "ok" : "x");
  if (!rslt) failMsg("precv");

  OnTimer0Interrupt(NULL);
  api.system.timer.create(RAK_TIMER_0, OnTimer0Interrupt, RAK_TIMER_PERIODIC);
  api.system.timer.start(RAK_TIMER_0, period * 1000, NULL);
}

void loop() {
}
