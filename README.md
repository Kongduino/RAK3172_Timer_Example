# RAK3172_Timer_Example

Setting up timers on STM32 MCUs is quite the daunting task, but again, [RUI3 makes this easy and seamless](https://docs.rakwireless.com/RUI3/System/#timer). There are 5 timers available for general use, `RAK_TIMER_0` to `RAK_TIMER_4`. Each can be set up as a periodic task, `RAK_TIMER_PERIODIC`, or a one-time thing, `RAK_TIMER_ONESHOT`.

As shown in the docs, setting it up is easy:

```c
void handler(void *data) {
  Serial.printf("[%lu]This is the Timer handler function\r\n", millis());
}

void setup() {
  Serial.begin(115200);
  if (api.system.timer.create(RAK_TIMER_0, (RAK_TIMER_HANDLER)handler, RAK_TIMER_PERIODIC) != true) {
    Serial.printf("Creating timer failed.\r\n");
  } else if (api.system.timer.start(RAK_TIMER_0, 1000, NULL) != true) {
    Serial.printf("Starting timer failed.\r\n");
  }
}
```

But showing a string of text is a bit limited, so I went ahead and produced an example that would be slightly more useful. I added 2 sensors, a RAK1901 and a RAK1902, and sample code from the libraries. I set up the LoRa chip to work in P2P mode, and we were good to go.

As you can see in the code, the `loop()` event is empty – no need to check for the elapsed time and call a function if the minimum delay has passed: the timer does it all. The one thing you have to be careful of is how much time is required for your callback to run: it should not exceed, or even be close to, the periodicity of your timer. In this case, I am reading from two sensors, and, the most time-consuming task, sending by LoRa at a rate optimized for distance, so a couple of seconds are required (sending the LoRa packet takes about 1.4 seconds). And we don't want to flood the airwaves either. For the purpose of this example, I set up the periodicity at 30 seconds, and this will work without breaking, but it should be much longer. You probably don't need to be updated every 30 seconds anyway...

## Sample Run

```
Time elapsed: 30592
Temperature = 23.04°C
Humidity = 61.24%
Pressure = 1010.60 hPa
Sending `T: 23.04; H: 61.24; P: 1010.60`, len 30 via P2P: ok
Tx done!
Time elapsed: 30592
Temperature = 23.17°C
Humidity = 61.08%
Pressure = 1010.59 hPa
Sending `T: 23.17; H: 61.08; P: 1010.59`, len 30 via P2P: ok
Tx done!
Time elapsed: 30592
Temperature = 23.21°C
Humidity = 60.92%
Pressure = 1010.59 hPa
Sending `T: 23.21; H: 60.92; P: 1010.59`, len 30 via P2P: ok
Tx done!
```

As you can see, even though I requested 30000 ms, I am getting intervals of 30592 ms. Good enough for a sample app, maybe not for a real-life application. This is an issue in older RUI versions, and should be fixed in an upcoming version.
