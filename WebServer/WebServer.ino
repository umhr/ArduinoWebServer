/*
  Web Server

 A simple web server that shows the value of the analog input pins.
 using an Arduino Wiznet Ethernet shield.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)

 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe
 modified 02 Sept 2015
 by Arturo Guadalupi
 
 */

#include <SPI.h>
//#include <Ethernet.h>
#include <UIPEthernet.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 177);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);
String pathname = "";
String query = "";

void setup() {
  // You can use Ethernet.init(pin) to configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields
  //Ethernet.init(5);   // MKR ETH shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit Featherwing Ethernet
  //Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet

  // Open serial communications and wait for port to open:
  Serial.begin(230400);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Ethernet WebServer Example");

  // start the Ethernet connection and the server:
  Ethernet.begin(mac);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  // start the server
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}

// クエリーに対応する値があれば返す
String getQuery(String command)
{
  if(query.indexOf(command + "=") > -1){
    int num = query.indexOf(command + "=");
    int start = num + command.length() + 1;
    int end = query.indexOf("&", start);
    if(end == -1){
      end = query.length();
    }
    return query.substring(start, end);
  }
  return ""; 
}

void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    boolean isGETLine = true;
    String header = "";
    String headerGET = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        //Serial.write(c);
        // 最初の\nまでをヘッダーとして取得
        if(isGETLine){
          if(c == '\n'){
            isGETLine = false;
            Serial.println(header);
          }else{
            header += String(c);
          }
        }
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          headerGET = header.substring(header.indexOf(" ")+1, header.lastIndexOf(" "));
          // favicon.ico対策
          if(headerGET == "/favicon.ico"){
            client.println("HTTP/1.1 204 OK");
            client.println("Connection: close");
            client.println("");
            break;
          }
          // ?より前までをパスとして取得
          pathname = headerGET.substring(0, headerGET.indexOf("?"));
          // ?があれば、クエリーとして取得
          query = headerGET.indexOf("?") > -1?headerGET.substring(headerGET.indexOf("?")+1):"";
          Serial.println("pathname=" + pathname + ", query=" + query);

          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // output the value of each analog input pin
          for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
            int sensorReading = analogRead(analogChannel);
            client.print("analog input ");
            client.print(analogChannel);
            client.print(" is ");
            client.print(sensorReading);
            client.println("<br />");
          }
          // ディレクトリやクエリーで分けてみたらいかが？提案用
          client.println("<a href='../abc/?a=123&b=456'>../abc/?a=123&b=456</a>");
          client.println("<br />");
          // API ぽく使いたい時はディレクトリ分けしたら良いかも
          if(pathname == "/abc/"){
            String a = getQuery("a");
            String b = getQuery("b");
            if(a != "" && b != ""){
             client.println("a=" + a + ", b=" + b);
             Serial.println("a=" + a + ", b=" + b);
            }
          }
          client.println("<br />");
          client.println("pathname:" + pathname + ", query:" + query);
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}
