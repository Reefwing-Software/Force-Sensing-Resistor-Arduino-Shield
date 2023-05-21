# Force Sensing Resistor (FSR) Arduino Shield

An Arduino Shield to measure an FSR using a voltage divider or op-amp solution.

We needed is an instrument that can plot the voltage/thrust curve for any motor and propeller combination. We use Pulse Width Modulation (PWM) to control the Electronic Speed Controller (ESC) which drives the motor, and it would be nice to be able to relate this PWM percentage to thrust.

A detailed explanation of this shield design with diagrams, may be found on the [Reefwing Medium](https://medium.com/@reefwing) page.

Schematics and PCB layout files may be found in the [Reefwing Software GitHub Repository](https://github.com/Reefwing-Software/Force-Sensing-Resistor-Arduino-Shield).

## Force Sensing Resistors

An FSR will not be as accurate as a a strain gauge, load cell, or pressure transducer, but it is a lot cheaper. It can be used for measurement after calibration but it is more traditionally used for detecting relative values. We used two Interlink 400 Series FSRs in our testing, the 0.25" and the 0.5". 

The FSR is a passive component which decreases in resistance when the force applied increases. The sensing range is from 0.2 N to 20 N, however the linear range (on a logarithmic scale) is less than this. Immediately after turn-on, the resistance decreases rapidly, until a turn-on threshold is reached. The turn-on threshold is the pressure required to bring the sensor resistance below 100kΩ. At a certain point, the sensor response will saturate, and further increases in pressure wont result in reduced resistance.

## FSR - Voltage Divider

The simplest FSR measurement method suggested by the [Interlink Integration Guide](https://www.pololu.com/file/0J749/FSR400-Series-Integration-Guide-13.pdf), is a voltage divider (Figure 6). With no pressure, the Analog to Digital Converter (ADC) on your Arduino will read 0V and with maximum pressure, the ADC should read around 5V (Vcc = 5V).

While FSR resistance vs Force is logarithmic, if we use conductance (G = 1/R) instead, the relation is linear-ish, and our calculations are simpler. The current to voltage converter circuit that we use in our shield, measures conductance.

## FSR - Current to Voltage Converter

For dynamic FSR measurements, a current-to-voltage converter is recommended by the Integration Guide. This circuit produces an output voltage that is inversely proportional to FSR resistance (i.e., conductance); in other words, this circuit gives a linear increase in output voltage for increases in applied force. This linearization of the response optimizes the resolution and simplifies data interpretation.

The reference circuit for this solution is provided by the integration guide, and the output is that of a standard inverting op-amp circuit. With a positive reference voltage, the output of the op-amp must be able to swing below ground, from 0V to –VREF, therefore dual sided supplies are necessary.

## FSR - Non-inverting Op-Amp Design

We would prefer not to have the added components and complexity associated with a dual rail supply, and have selected an op-amp designed to operate from a single supply, the AS358 (a derivative of the LM358).

The AS358 data sheet, states that with a single supply, the Output Voltage Swing: is 0V to VCC -1.5V (i.e., 3.5V). Our force reading range will be similar to the voltage divider solution, but with a more linear curve, the resolution should be improved for higher force readings. Op-amps that have a better rail-to-rail output than the AS358 include the MCP6004 and the MCP6L02.

Note that the non-inverting op-amp acts as a voltage amplifier, with the gain dependent on the ratio of R1 and R2. We need to take care that we don't overload the ADC and choose an appropriate Vref.

Tekscan are another brand of FSR and in their [Electrical Integration Guide](https://www.tekscan.com/sites/default/files/FLX-Best-Practice-Electrical-Integration-RevB.pdf) they include a non-inverting circuit configuration.

We will adopt a number of elements from this design:

1. Using PWM from the Arduino as the Vref input to the (+) terminal of the op-amp. We can then adjust this based on what the ADC is reading at the output.
2. Using a potentiometer for the feedback resistor. This can adjust the gain and slope of the force vs voltage curve to optimise our measurements. Reducing the feedback resistor value will also improve the op-amp frequency stability (see the next point),
3. Placing a capacitor in parallel with the feedback resistor. Parasitic capacitance at the inverting (-) terminal of an op-amp can cause instability. This manifests as oscillation or ringing on the output. The frequency at which the op-amp gain will increase and possibly start causing instability is 1.6 kHz.

Assuming a parasitic capacitance value of 5 pF and the worst case FSR resistance value (no force) of 20 MΩ, we could start seeing instability at around 1.6 kHz. Maximum PWM frequency on the Arduino UNO is 980 Hz, so we should be ok, but we will test the circuit with and without a 47 pF feedback capacitor (C1).

## FSR - Non-inverting Op-Amp Circuit

Our FSR amplifier circuit is based on the Tekscan design, with some tweaks. We have included C1 (the feedback capacitor) for stability and added C2, to form a low pass filter with R1 and smooth out the PWM voltage reference from the Arduino.

## FSR - Buffered Voltage Divider

The AS358 IC includes two op-amps. As we had space on the shield, we figured we may as well use this as a unity gain buffer for the voltage divider circuit described initially.

## FSR - Arduino Amplifier Shield

The complete circuit for the FSR amplifier shield is provided in this repository. It includes the non-inverting amplifier and the buffered voltage divider. We can test and compare both solutions by attaching an FSR to the appropriate input and reading the relevant ADC.

LED1 will be programmed to vary its brightness with the applied force. LED2 is power indication. We have replicated the UNO reset button, so that it is accessible with the shield in place. The shield is shorter than an UNO, so that it doesn't touch the DC Jack or ethernet port.