void constructHTTPRequest(char* msg){  int sensorValue = analogRead(A0);
float voltage = sensorValue * (5.0 / 1024.0); 
  msg[0] = 0; //reset the string  
  strcat(msg, "GET /update?api_key=2VWFSWL3N5N0CUBD");  
  strcat(msg, "&field1=");
  strcat(msg, temperature);
  strcat(msg, "&");
  strcat(msg, "&field3=");
  strcat(msg, DO);
  strcat(msg, "&");
  strcat(msg, "&field2=");
  strcat(msg, pH);
  strcat(msg, "&");
  strcat(msg, "&field4=");
  strcat(msg, turbidity);
  strcat(msg, "&");
  strcat(msg, "&field5=");
  strcat(msg, lat);
  strcat(msg, "&");
  strcat(msg, "&field6=");
  strcat(msg, lng);  
  strcat(msg, " HTTP/1.1\r\nHost: ");
  strcat(msg, server);
  strcat(msg, "\r\nConnection: close\r\n\r\n");
}


void sendHTTPRequest(char* msg){
  // strcat(msg, "t=2018-12-18,23:01:00");
  // strcat(msg, "&");
  // strcat(msg, "te=30.0");
  // strcat(msg, "&");
  // strcat(msg, "do=7.05");
  // strcat(msg, "&");
  // strcat(msg, "pH=5.66");
  // strcat(msg, "&");
  // strcat(msg, "tu=7");
  // strcat(msg, "&");
  // strcat(msg, "la=28.5441421");
  // strcat(msg, "&");
  // strcat(msg, "ln=77.2720617");
  // strcat(msg, " HTTP/1.1\r\nHost: ");
  // strcat(msg, server);
  // strcat(msg, "\r\nConnection: close\r\n\r\n");
  a7Client.write(msg);
}
