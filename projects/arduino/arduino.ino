
extern "C" {
  int benchmark();
}

void setup() {
  wdt_disable();
  Serial.begin(9600);
  delay(5000);

  benchmark();

  // Nothing really to do after this, just infinite loop *shrug*
  wdt_enable(WDTO_500MS);
  while (true) {
    wdt_reset();
    delay(100);
  }
}

void loop() {
}
