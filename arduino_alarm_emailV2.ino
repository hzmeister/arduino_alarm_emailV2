#include <SPI.h>
#include <Ethernet.h>

// set alarm signal pin
const byte alarmPin = 7;
boolean statusCheck = false;

// set w5100 mac. this must be unique on LAN. ip is assigned by dhcp server.
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x55, 0x65 };

char server[] = "smtp.ispemail.com";
int port = 25; // must NOT use any kind of encryption.

EthernetClient client;

void setup() {
  pinMode(alarmPin, INPUT_PULLUP);
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  Serial.begin(9600);

  // For debugging, wait until the serial console is connected.
  delay(4000);
  while (!Serial);
  Serial.print("DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("FAIL");
    while (true);
  }
  Serial.println("OK");
}

void loop()
{
  if (digitalRead(alarmPin) == HIGH && statusCheck == false)
  {
    if (sendEmail()) Serial.println(F("Email sent"));
    else Serial.println(F("Email failed"));
    statusCheck = true;
  }
  else if (digitalRead(alarmPin) == LOW)
  {
    statusCheck = false;
  }
}

byte sendEmail()
{
  byte thisByte = 0;
  byte respCode;

  if (client.connect(server, port) == 1) {
    Serial.println(F("connected"));
  } else {
    Serial.println(F("connection failed"));
    return 0;
  }
  if (!eRcv()) return 0;

  Serial.println(F("Sending helo"));
  // change to your public ip
  client.println(F("helo 1.22.333.4"));
  if (!eRcv()) return 0;

  Serial.println(F("Sending auth login"));
  client.println("auth login");
  if (!eRcv()) return 0;

  Serial.println(F("Sending User"));
  // change to your base64 encoded username
  client.println(F("base64user"));
  if (!eRcv()) return 0;

  Serial.println(F("Sending Password"));
  // change to your base64 encoded password
  client.println(F("base64pass"));
  if (!eRcv()) return 0;

  Serial.println(F("Sending From"));
  // change to your email address (sender)
  client.println(F("MAIL From: <user@ispemail.com>"));
  if (!eRcv()) return 0;

  // change to recipient address
  Serial.println(F("Sending To"));
  client.println("RCPT To: <recip1@gmail.com>");
  if (!eRcv()) return 0;
  /*
    Serial.println(F("Sending To"));  //can have multiple recipients.
    client.println("RCPT To: <recip2@gmail.com>");
    if (!eRcv()) return 0;
    Serial.println(F("Sending To"));
    client.println("RCPT To: <recip3@gmail.com>");
    if (!eRcv()) return 0;
  */

  Serial.println(F("Sending DATA"));
  client.println(F("DATA"));
  if (!eRcv()) return 0;

  Serial.println(F("Sending email"));

  // change to recipient address. not needed, but can be included.
  // client.println(F("To: RP <recip@gmail.com>"));

  // change to your address
  client.println(F("From: Arduino <user@ispemail.com"));
  client.println(F("Subject: EMAIL FROM ARDUINO"));
  client.println(F("Message body."));
  client.println(F("."));
  if (!eRcv()) return 0;

  Serial.println(F("Sending QUIT"));
  client.println(F("QUIT"));
  if (!eRcv()) return 0;

  client.stop();

  Serial.println(F("disconnected"));

  return 1;
}

byte eRcv()
{
  byte respCode;
  byte thisByte;
  int loopCount = 0;

  while (!client.available()) {
    delay(1);
    loopCount++;

    // if nothing received for 10 seconds, timeout
    if (loopCount > 10000) {
      client.stop();
      Serial.println(F("\r\nTimeout"));
      return 0;
    }
  }

  respCode = client.peek();

  while (client.available())
  {
    thisByte = client.read();
    Serial.write(thisByte);
  }

  if (respCode >= '4')
  {
    efail();
    return 0;
  }

  return 1;
}


void efail()
{
  byte thisByte = 0;
  int loopCount = 0;

  client.println(F("QUIT"));

  while (!client.available()) {
    delay(1);
    loopCount++;

    // if nothing received for 10 seconds, timeout
    if (loopCount > 10000) {
      client.stop();
      Serial.println(F("\r\nTimeout"));
      return;
    }
  }

  while (client.available())
  {
    thisByte = client.read();
    Serial.write(thisByte);
  }

  client.stop();

  Serial.println(F("disconnected"));
}
