#include "DualCore.h"

DualCoreESP32 DualCore;

void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:
  DualCore.ConfigCores();
}

void loop() {
  // put your main code here, to run repeatedly:

}
