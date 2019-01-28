void convertToChar(Data* data, A7Client a7Client){
	dtostrf(data->temperature, -6, 2, temperature );
	dtostrf(data->pH, -5, 2, pH );
	dtostrf(data->DO, -5, 2, DO );
	removeSpace(temperature);
	removeSpace(pH);
	removeSpace(DO);
dtostrf(data->turbidity,0,4,turbidity);	
	dtostrf(a7Client.latitudeDegrees, -8, 4, lat);	
	dtostrf(a7Client.longitudeDegrees, -8, 4, lng);
	removeSpace(lat);
	removeSpace(lng);
}

void printCharData(){ int sensorValue = analogRead(A0);
float voltage = sensorValue * (5.0 / 1024.0); 
	Serial.print(F("Temperature: "));
        sensors.requestTemperatures();
        Serial.println(sensors.getTempCByIndex(0));
  	Serial.println(freeRam());
  	Serial.print(F("pH: "));
  	Serial.println(pH);
  	Serial.println(freeRam());
  	Serial.print(F("DO: "));
  	Serial.println(DO);
  	Serial.println(freeRam());
  	Serial.print(F("Turbidity: "));
  	Serial.println(voltage);
  	Serial.print(F("Latitude: "));
  	Serial.println(lat);
  	Serial.print(F("Longitude: "));
  	Serial.println(lng);
}

void removeSpace(char* s)
{
    for (char* s2 = s; *s2; ++s2) {
        if (*s2 != ' ')
            *s++ = *s2;
    }
    *s = 0;
}
