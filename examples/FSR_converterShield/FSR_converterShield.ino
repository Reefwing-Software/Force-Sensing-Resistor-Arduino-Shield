/******************************************************************
  @file       FSR_converterShield.ino
  @brief      Measure voltage on pin A1 and convert to resistance.
              For use with the Reefwing FSR Amplifier Shield.
              This sketch is for the current to voltage converter
              circuit on the shield.
  @author     David Such
  
  Code:        David Such
  Version:     1.0
  Date:        28/04/23

  1.0 Original Release          28/04/23

******************************************************************/

#define ADC0_PIN  14                //  Analogue Pin A0
#define ADC1_PIN  15                //  Analogue Pin A1
#define LED_PIN   3                 //  D3 connected to FORCE LED
#define PWM_PIN   5                 //  D5 connected to op-amp Vref

#define VCC       5000.0            //  Supply Voltage in mV
#define R1        10.0              //  10K Voltage Divider Resistor
#define FLT_SIZE  11                //  Force Lookup Table Size

int adcRaw = 0;                     //  Value returned by ADC0, 0 - 1023
int adcAmp = 0;                     //  Value returned by ADC1, 0 - 1023
uint8_t ledDutyCycle = 0;           //  Force LED brightness, 0 - 255
float adcVolts = 0.0;               //  ADC voltage divider value scaled to mV
float fsr = 0.0;                    //  FSR Calculated Resistance in kΩ
float g = 0.0;                      //  FSR Conductance = 1/R in mS
float f = 0.0;                      //  Interpolated Force in grams

/******************************************************************
   ADC Voltage x 100 vs Force (g) Lookup Table
 ******************************************************************/

 struct ForceLookup {
  uint16_t voltage;   //  millivolts (mV)
  uint16_t force;     //  grams (g)
 };

 ForceLookup table[FLT_SIZE] = {  {0, 0}, {900, 20}, {1400, 50}, {1750, 100}, {2250, 200}, {2500, 310},
                            {2800, 450}, {3000, 580}, {3200, 710}, {3400, 900}, {3450, 1000}   };

 float interpolate(uint16_t adc_mV) {
  //  Based on ADC voltage (mV), lookup the force (g) and interpolate between adjacent values if required.

  //  Constrain range to lookup table min and max
  adc_mV = constrain(adc_mV, 0, 3450);
  
  for (int i = 0; i < FLT_SIZE - 1; i++) {
        if (table[i].voltage <= adc_mV && table[i+1].voltage >= adc_mV) {
            float diffx = adc_mV - table[i].voltage;
            float diffn = table[i+1].voltage - table[i].voltage;

            return (table[i].force + (table[i+1].force - table[i].force) * diffx / diffn); 
        }
    }

    return -1.0f; // Not in Range
}

/******************************************************************
   Increase accuracy of ADC readings by allowing time for sampling
   capacitor to charge and discarding the first reading.
 ******************************************************************/

int readADC(int pin) {
  int adc = analogRead(pin);
  
  delay(10);
  adc = analogRead(pin);
  delay(10);

  return adc;
}

/******************************************************************
   Pin Configuration and Loop
 ******************************************************************/

void setup() {
  //  Pin Configuration
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(PWM_PIN, OUTPUT);
  pinMode(ADC0_PIN, INPUT);
  pinMode(ADC1_PIN, INPUT);

  //  Ensure op-amp current to voltage circuit is OFF
  //  Ref: FSH_voltageConverterShield.ino for this function
  analogWrite(PWM_PIN, 0);

  Serial.begin(115200);
}

void loop() {
  //  Read voltage output from buffered voltage divider
  adcRaw = readADC(ADC0_PIN);
  adcVolts = map(adcRaw, 0, 1023, 0, 5000);
  f = interpolate(adcVolts);
  
  fsr = R1 * (VCC/adcVolts - 1.0f);

  if (fsr > 0.0) {  
    g = 1.0f/fsr;  // conductance, G = 1/R
  }

  //  Read amplified FSR voltage
  adcAmp = readADC(ADC1_PIN);

  //  Convert force to LED brightness
  ledDutyCycle = map(adcRaw, 0, 800, 0, 255);
  analogWrite(LED_PIN, ledDutyCycle);

  //  Digital (ON/OFF) force present indication on LED13
  if (adcVolts > 0.5) {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else {
    digitalWrite(LED_BUILTIN, LOW);
  }

  Serial.print("Divider ADC: ");
  Serial.print(adcRaw);
  Serial.print("\tConverter ADC: ");
  Serial.print(adcAmp);
  Serial.print("\tADC (mV): ");
  Serial.print(adcVolts, 0);
  Serial.print("\tFSR Resistance (kΩ): ");
  Serial.print(fsr, 3);
  Serial.print("\tFSR Conductance (mS): ");
  Serial.print(g, 4);
  Serial.print("\tForce (g): ");
  Serial.println(f);

  delay(1000);
}
