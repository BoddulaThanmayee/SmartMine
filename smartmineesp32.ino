#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "SMARTMINE";
const char* password = "12345678";

WebServer server(80);

void sendCommand(char command)
{
  Serial.write(command);
}

void handleRoot()
{
  String page = R"rawliteral(
<!DOCTYPE html>
<html>

<head>

<meta name="viewport"
content="width=device-width,initial-scale=1">

<title>SMARTMINE ROVER</title>

<style>

body {
  font-family: Arial;
  text-align: center;
  background: #f4f6f8;
}

button {
  width: 130px;
  height: 55px;
  margin: 6px;
  font-size: 18px;
  border-radius: 10px;
  border: none;
}

</style>

</head>

<body>

<h1>SMARTMINE ROVER</h1>

<h2>MODE</h2>

<button onclick="location.href='/auto'">
AUTO
</button>

<button onclick="location.href='/manual'">
MANUAL
</button>

<h2>MOVEMENT</h2>

<button onclick="location.href='/forward'">
FORWARD
</button>

<br>

<button onclick="location.href='/left'">
LEFT
</button>

<button onclick="location.href='/stop'">
STOP
</button>

<button onclick="location.href='/right'">
RIGHT
</button>

<br>

<button onclick="location.href='/backward'">
BACKWARD
</button>

<h2>SERVO SCAN</h2>

<button onclick="location.href='/servoleft'">
LEFT
</button>

<button onclick="location.href='/servocenter'">
CENTER
</button>

<button onclick="location.href='/servoright'">
RIGHT
</button>

<h2>SAFETY</h2>

<button onclick="location.href='/emergency'">
EMERGENCY STOP
</button>

</body>

</html>
)rawliteral";

  server.send(200, "text/html", page);
}

void setupRoutes()
{
  server.on("/", handleRoot);

  server.on("/auto", []()
  {
    sendCommand('A');
    server.send(200, "text/plain", "AUTO MODE");
  });

  server.on("/manual", []()
  {
    sendCommand('M');
    server.send(200, "text/plain", "MANUAL MODE");
  });

  server.on("/forward", []()
  {
    sendCommand('F');
    server.send(200, "text/plain", "FORWARD");
  });

  server.on("/backward", []()
  {
    sendCommand('B');
    server.send(200, "text/plain", "BACKWARD");
  });

  server.on("/left", []()
  {
    sendCommand('L');
    server.send(200, "text/plain", "LEFT");
  });

  server.on("/right", []()
  {
    sendCommand('R');
    server.send(200, "text/plain", "RIGHT");
  });

  server.on("/stop", []()
  {
    sendCommand('S');
    server.send(200, "text/plain", "STOP");
  });

  server.on("/servoleft", []()
  {
    sendCommand('Q');
    server.send(200, "text/plain", "SERVO LEFT");
  });

  server.on("/servocenter", []()
  {
    sendCommand('C');
    server.send(200, "text/plain", "SERVO CENTER");
  });

  server.on("/servoright", []()
  {
    sendCommand('E');
    server.send(200, "text/plain", "SERVO RIGHT");
  });

  server.on("/emergency", []()
  {
    sendCommand('X');
    server.send(200, "text/plain", "EMERGENCY STOP");
  });
}

void setup()
{
  Serial.begin(115200);

  WiFi.softAP(ssid, password);

  setupRoutes();

  server.begin();
}

void loop()
{
  server.handleClient();

  delay(2);
}