#include <Arduino.h>

// GxEPD2_HelloWorld.ino by Jean-Marc Zingg

// see GxEPD2_wiring_examples.h for wiring suggestions and examples
// if you use a different wiring, you need to adapt the constructor parameters!

// uncomment next line to use class GFX of library GFX_Root instead of Adafruit_GFX
//#include <GFX.h>

#include <GxEPD2_BW.h>
//#include <GxEPD2_3C.h>
//#include <GxEPD2_7C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <painlessMesh.h>
#include <Button2.h>
#include "MeshCredentials.h"

// select the display class and display driver class in the following file (new style):
#include "GxEPD2_display_selection_new_style.h"

//mesh credentials are defined in MeshCredentials.h
//#define MESH_PREFIX "MeshNetwork"
//#define MESH_PASSWORD "123"
//#define MESH_PORT ****

//uncomment one of the following lines per node
#define RED
//#define GREEN
//#define WHITE

Scheduler userScheduler; // to control your personal task
painlessMesh mesh;

#ifdef WHITE
#define ROLE "white"
#define VERSION "WHITE v1.0.0"
#define MESSAGE "White "
#endif

#ifdef RED
#define ROLE "red"
#define VERSION "RED v1.0.0"
#define MESSAGE "Red   "
#endif

#ifdef GREEN
#define ROLE "green"
#define VERSION "GREEN v1.0.0"
#define MESSAGE "Green "
#endif

#define BUTTON_1 39

Button2 btn1(BUTTON_1);
//Button2 btn2(BUTTON_2);

// or select the display constructor line in one of the following files (old style):
//#include "GxEPD2_display_selection.h"
//#include "GxEPD2_display_selection_added.h"

bool calc_delay = false;
SimpleList<uint32_t> nodes;
uint32_t nsent=0;
char buff[512];

//void helloWorld();
void sendMessage(String msg);

void initDisplay() {
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setFullWindow();
  display.firstPage();
  display.fillScreen(GxEPD_WHITE);
  //display.setCursor(0, 18);
}

void receivedCallback(uint32_t from, String &msg) {
  int16_t tbx, tby; uint16_t tbw, tbh;
  Serial.printf("Rx from %u msg=%s\n", from, msg.c_str());
  if (strcmp(msg.c_str(), "GETRT") == 0) {
    mesh.sendBroadcast(mesh.subConnectionJson(true).c_str());
  }
  else {
    sprintf(buff, "Rx:%s", msg.c_str());
    display.setCursor(0, 54);
    display.getTextBounds(buff, 0, 54, &tbx, &tby, &tbw, &tbh);
    display.setPartialWindow(tbx, tby, tbw, tbh);
    //display.fillRect(0, 54, tbw, tbh, GxEPD_WHITE);
    display.fillScreen(GxEPD_WHITE);
    do
    {
      display.print(buff);
    }
    while (display.nextPage());
  }
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("--> Start: New Connection, nodeId = %u\n", nodeId);
  Serial.printf("--> Start: New Connection, %s\n", mesh.subConnectionJson(true).c_str());
}

void changedConnectionCallback() {
  int16_t tbx, tby; uint16_t tbw, tbh;
  Serial.printf("Changed connections\n");
  nodes = mesh.getNodeList();
  Serial.printf("Num nodes: %d\n", nodes.size());
  Serial.printf("Connection list:");
  SimpleList<uint32_t>::iterator node = nodes.begin();
  while (node != nodes.end()) {
    Serial.printf(" %u", *node);
    node++;
  }
  Serial.println();
  calc_delay = true;

  sprintf(buff, "Nodes: %d", nodes.size());
  display.setCursor(0, 36);
  display.getTextBounds(buff, 0, 36, &tbx, &tby, &tbw, &tbh);
  display.setPartialWindow(tbx, tby, tbw, tbh);
  display.fillScreen(GxEPD_WHITE);
  do
  {
    display.println(buff);
  }
  while (display.nextPage());
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void onNodeDelayReceived(uint32_t nodeId, int32_t delay) {
  Serial.printf("Delay to node %u is %d us\n", nodeId, delay);
}

void sendMessage(String msg) {
  int16_t tbx, tby; uint16_t tbw, tbh;
  if (strcmp(msg.c_str(), "") == 0) {
    nsent++;
    msg = MESSAGE;
    msg += nsent;
  }
  mesh.sendBroadcast(msg);
  Serial.printf("Tx--> %s\n", msg.c_str());
  sprintf(buff, "Tx:%s", msg.c_str());
  display.setCursor(0, 72);
  display.getTextBounds(buff, 0, 72, &tbx, &tby, &tbw, &tbh);
  display.setPartialWindow(tbx, tby, tbw, tbh);
  display.fillScreen(GxEPD_WHITE);
  do
  {
    display.println(buff);
  }
  while (display.nextPage());

  if (calc_delay) {
    SimpleList<uint32_t>::iterator node = nodes.begin();
    while (node != nodes.end()) {
      mesh.startDelayMeas(*node);
      node++;
    }
    calc_delay = false;
  }
}

void button_setup() {
  btn1.setPressedHandler([](Button2 & b) {
    Serial.println("Button 1 pressed");
    sendMessage("");
    Serial.println("Free memory: " + String(esp_get_minimum_free_heap_size()) + " bytes");
  });
  /* btn1.setLongClickHandler([](Button2 & b) {
    Serial.println("Button 1 long pressed");
    sendMessage("GETRT");
  }); */
}



// alternately you can copy the constructor from GxEPD2_display_selection.h or GxEPD2_display_selection_added.h to here
// e.g. for Wemos D1 mini:
//GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(GxEPD2_154_D67(/*CS=D8*/ SS, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 4)); // GDEH0154D67

void setup()
{
  Serial.begin(115200);
  //display.init(115200); // default 10ms reset pulse, e.g. for bare panels with DESPI-C02
  display.init(115200, true, 2, false); // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse
  initDisplay();
  //helloWorld();
  display.setCursor(0, 18);
  mesh.setDebugMsgTypes(ERROR | STARTUP);  // set before init() so that you can see startup messages
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  sprintf(buff, "Id: %u", mesh.getNodeId());
  do
  {
    display.println(buff);
  }
  while (display.nextPage());
  //display.hibernate();
  button_setup();
}

//char HelloWorld[] = "Wert: 5 von Node 1";

void loop() {
  mesh.update();
  btn1.loop();
}

/* void helloWorld()
{
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center the bounding box by transposition of the origin:
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(0, 18);
    display.println(HelloWorld);
    display.println("Hello World!");
  }
  while (display.nextPage());
} */