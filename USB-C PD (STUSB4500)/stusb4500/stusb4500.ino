/*
This uses the SparkFun library to make writing to NVM registers easy.
PDOs: 20V 3A, 20V 1.5A, and default 5V
*/

#include <Wire.h>
#include <SparkFun_STUSB4500.h>

STUSB4500 pd; // Create an instance of the STUSB4500 class

void setup()
{
  Serial.begin(115200);
  while(!Serial); // Wait for serial monitor to open (optional)
  
  // Initialize the I2C bus
  Wire.begin();
  delay(500); // Give some time for devices to power up

  // Initialize communication with the STUSB4500
  if (!pd.begin()) {
    Serial.println("ERROR: Cannot connect to STUSB4500.");
    Serial.println("Check your wiring, I2C address, and connections.");
    while(1); // Halt if device not found
  }
  
  Serial.println("STUSB4500 connected successfully!");

  // Set PDO priority to PDO3 so that our settings are used first.
  pd.setPdoNumber(3);

  // Configure PDO3 for 12V, 1A:
  // Note: PDO1 is fixed at 5V and cannot be changed.
  pd.setVoltage(3, 20.0);  // Set PDO3 voltage to 20.0V
  pd.setCurrent(3, 3.0);   // Set PDO3 current to 1.0A

  // Optionally, set voltage tolerance percentages.
  // These define the acceptable voltage window for the negotiated voltage.
  // For example, setting 10 means a ±10% tolerance.
  pd.setLowerVoltageLimit(3, 10); // Under voltage tolerance: 10%
  pd.setUpperVoltageLimit(3, 10); // Over voltage tolerance: 10%

  pd.setPdoNumber(2);

  pd.setVoltage(2, 20.0);  // Set PDO2 voltage to 20.0V
  pd.setCurrent(2, 1.5);   // Set PDO2 current to 1.5A

   pd.setLowerVoltageLimit(2, 10); // Under voltage tolerance: 10%
  pd.setUpperVoltageLimit(2, 10); // Over voltage tolerance: 10%
  // Write and save the new settings to the STUSB4500
  pd.write();

  // Optionally, perform a soft reset to force a renegotiation with the source.
  pd.softReset();

  // Read back the settings to verify
  pd.read();

  // Print the new PDO3 settings to the Serial Monitor
  Serial.println("\nNew PDO3 Settings:");
  Serial.print("Voltage (V): ");
  Serial.println(pd.getVoltage(3));
  Serial.print("Current (A): ");
  Serial.println(pd.getCurrent(3));
  Serial.print("Lower Voltage Tolerance (%): ");
  Serial.println(pd.getLowerVoltageLimit(3));
  Serial.print("Upper Voltage Tolerance (%): ");
  Serial.println(pd.getUpperVoltageLimit(3));

  Serial.println("\nNew PDO2 Settings:");
  Serial.print("Voltage (V): ");
  Serial.println(pd.getVoltage(2));
  Serial.print("Current (A): ");
  Serial.println(pd.getCurrent(2));
  Serial.print("Lower Voltage Tolerance (%): ");
  Serial.println(pd.getLowerVoltageLimit(2));
  Serial.print("Upper Voltage Tolerance (%): ");
  Serial.println(pd.getUpperVoltageLimit(2));
}

void loop()
{
  // Nothing is required in the loop.
}
