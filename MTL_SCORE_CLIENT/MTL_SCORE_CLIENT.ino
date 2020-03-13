#include "Arduino.h"

#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <Tone32.h>

//Visual
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Fonts/Picopixel.h>
#include "Adafruit_LEDBackpack.h"

#define DEBUG_PRINT false

#define ONE_MINUTE (60 * 1000)
#define ONE_HOUR (3600 * 1000)
#define PRE_GAME_HOURS 10
#define PRE_GAME_MILLIS (PRE_GAME_HOURS * ONE_HOUR)

#define SCROLL_DELAY 100

#define BUZZER_PIN 12

#define FREQUENCY 2000
#define BUZZER_CHANNEL 0
#define BUZZER_REZ 8

#define MTL_GOAL_SONG_SIZE 6
int MTL_GOAL_SONG[MTL_GOAL_SONG_SIZE][2] = {{NOTE_C4,150}, {NOTE_E4, 150}, {NOTE_G4, 150}, {NOTE_C5, 300}, {NOTE_G4, 150}, {NOTE_C5, 500}};
int CurrentMtlGoalNote = MTL_GOAL_SONG_SIZE;
unsigned long LastMtlGoalNoteTime = 0;

#define VS_GOAL_SONG_SIZE 4
int VS_GOAL_SONG[VS_GOAL_SONG_SIZE][2] = {{NOTE_C5,225}, {NOTE_B4, 225}, {NOTE_AS4, 225}, {NOTE_A4, 1200}};
int CurrentVsGoalNote = VS_GOAL_SONG_SIZE;
unsigned long LastVsGoalNoteTime = 0;

#define GAMESTART_SONG_SIZE 6
int GAMESTART_SONG[GAMESTART_SONG_SIZE][2] = {{NOTE_C5,300}, {NOTE_C5, 300}, {NOTE_A4, 300}, {NOTE_G4, 300}, {NOTE_C5, 300}, {NOTE_C5, 300}};
int CurrentGameStartNote = GAMESTART_SONG_SIZE;
unsigned long LastGameStartNoteTime = 0;

Adafruit_8x8matrix PeriodDisplay = Adafruit_8x8matrix();
Adafruit_8x8matrix MtlDisplay = Adafruit_8x8matrix();
Adafruit_8x8matrix VsDisplay = Adafruit_8x8matrix();

const char* ssid     = "youssidhere";
const char* password = "yourpskhere";

const char* URL = "your url here";  // Server URL
const int TEAM = 8; //MTL = 8
int port = 80;

const char* root_ca= \
"-----BEGIN CERTIFICATE-----\n" \
"MIIFmTCCBIGgAwIBAgIQDh+h475hcAblUszPHZYxzjANBgkqhkiG9w0BAQsFADBG\n" \
"MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRUwEwYDVQQLEwxTZXJ2ZXIg\n" \
"Q0EgMUIxDzANBgNVBAMTBkFtYXpvbjAeFw0xOTA4MDkwMDAwMDBaFw0yMDA5MDkx\n" \
"MjAwMDBaMDMxMTAvBgNVBAMMKCouZXhlY3V0ZS1hcGkuY2EtY2VudHJhbC0xLmFt\n" \
"YXpvbmF3cy5jb20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC0XrTZ\n" \
"t4Ogq32TOIIzZAqr8jqutRMV/mFfw3acKeeJcR/NQLj60XBzi8wCvw8hh99RkJYh\n" \
"cfXUK3FtRRbY3LBjmnT6BLHpuqrnITKAybAH9EmNLi+iPTMHszrJ/hM0ZZPOsBTe\n" \
"cdVhJSyrU+nZ/CCYEgiEPRso8J5pzguohrXqlYLxd5wWWKFc5SkDaCo7jQ01+Ejz\n" \
"XSFacqqbUo9zGrdwZCgMuVn5wY5+sIwdOVh1uIdn5pJH7cfKmTo1VJzNmhgFuNa4\n" \
"U3RnGKjA/A54nAZ2buShZGw+aUEfhl1uqvrkOxGpZT94+Csjbm+FbFjLxm76PHSD\n" \
"OYSADtZb80fQ/+J1AgMBAAGjggKUMIICkDAfBgNVHSMEGDAWgBRZpGYGUqB7lZI8\n" \
"o5QHJ5Z0W/k90DAdBgNVHQ4EFgQUou4wHEf9l4l+ifxv6Sy7az+jLfswMwYDVR0R\n" \
"BCwwKoIoKi5leGVjdXRlLWFwaS5jYS1jZW50cmFsLTEuYW1hem9uYXdzLmNvbTAO\n" \
"BgNVHQ8BAf8EBAMCBaAwHQYDVR0lBBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMCMDsG\n" \
"A1UdHwQ0MDIwMKAuoCyGKmh0dHA6Ly9jcmwuc2NhMWIuYW1hem9udHJ1c3QuY29t\n" \
"L3NjYTFiLmNybDAgBgNVHSAEGTAXMAsGCWCGSAGG/WwBAjAIBgZngQwBAgEwdQYI\n" \
"KwYBBQUHAQEEaTBnMC0GCCsGAQUFBzABhiFodHRwOi8vb2NzcC5zY2ExYi5hbWF6\n" \
"b250cnVzdC5jb20wNgYIKwYBBQUHMAKGKmh0dHA6Ly9jcnQuc2NhMWIuYW1hem9u\n" \
"dHJ1c3QuY29tL3NjYTFiLmNydDAMBgNVHRMBAf8EAjAAMIIBBAYKKwYBBAHWeQIE\n" \
"AgSB9QSB8gDwAHYAu9nfvB+KcbWTlCOXqpJ7RzhXlQqrUugakJZkNo4e0YUAAAFs\n" \
"d42X4wAABAMARzBFAiEAkD4GdjvUKZWOw9MOpVK5k02GdGnB23Y39LB+150FSF0C\n" \
"IHRH5htUkYF7HFjzf7IzM5jnS1UbfKoSs8DovtIfSQPHAHYAh3W/51l8+IxDmV+9\n" \
"827/Vo1HVjb/SrVgwbTq/16ggw8AAAFsd42YHQAABAMARzBFAiEAjGAekmDkSjuy\n" \
"mzjgGHcdrc/D9GmzzQXJdCnQ9wymjKsCIHHSyfzWV7dEOL1BKukZWsUSn40RAsFi\n" \
"rM/nSix7+YQuMA0GCSqGSIb3DQEBCwUAA4IBAQCq57Js3zuiwzUvV/SA1YbpUKDa\n" \
"U04dhDSPEhb5m4J+mjapTOFBzbvhIZKz8hgdNbmiu6V7eg94QBCzEJ1trKi/VHyU\n" \
"RufV5JWPesTjvUiX6QFGvuhQ1XzJ3rj0O8Xa73+Ns8GV8S2+5mQ7B6pnOorBubRT\n" \
"wwLtVmlehCczabnwpeqjyoeMIBOPd2EV5Y6S0gkyTO2sZ575lLAM17yEL1E1NHeD\n" \
"/xLO67IPin6Ip3qEV/7Zq+nFobbNh1fZne/oGtDpuZ23Q12J/f0RoDZHZDvhJir5\n" \
"TufNlFAxvNKoPXfgBQuyCUEZSGgGf4WymkgSeiWJg+p6DqlMIJ4/pVCPAETF\n" \
"-----END CERTIFICATE-----\n";


unsigned long LastGetDataMillis = 0;

char Message[256];

int error = 22;//Not connected to network
unsigned long waitTime = 0;
int mtl = 0;
int vs = 0;
int period = 1;

int PreviousMtl = 0;
int PreviousVs = 0;

unsigned long ErrorCount = 0;

const long ErrorUpdateDelay = ONE_MINUTE;
const long LiveUpdateDelay = 10000;

unsigned long LastAnimationTime = 0;
int CurrentAnimationFrame = 0;
#define ANIMATION_DELAY 60

unsigned long LastScrollTime = 0;
int CurrentScroll = 0;

enum State
{
  Connecting,
  Initialization,
  PreGame,
  InGame,
  PostGame
};

State CurrentState = Connecting;

//Forwards
void delayFor(long milliseconds);
void UpdateVisual();
bool GetData();
void TurnOffDisplays();
void Connect();
bool IsConnected();

void setup()
{
  if(DEBUG_PRINT) Serial.begin(115200);
  delay(100);

  PeriodDisplay.begin(0x70);
  MtlDisplay.begin(0x71);
  VsDisplay.begin(0x74); //Note, the A2 and A2 soldering pin are inverted

  TurnOffDisplays();

  ledcSetup(BUZZER_CHANNEL, FREQUENCY, BUZZER_REZ);
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);

  Connect();

  SetState(Initialization);
}

void SetState(State newState)
{
  if(DEBUG_PRINT)
  {
    Serial.print(("New state: "));
    Serial.println(newState);
  }
  
  switch(newState)
  {
    case Initialization:
    {
      GetData();
        
      break;
    }
    case PreGame:
    {
      LastScrollTime = millis();
      CurrentScroll = 17;

      LastAnimationTime = millis();
      CurrentAnimationFrame = 0;
             
      if(DEBUG_PRINT)
      {
        int waitHours = waitTime/ONE_HOUR;
        int waitMinutes = (waitTime - (waitHours*ONE_HOUR)) / ONE_MINUTE;

        if(DEBUG_PRINT)
        {
          Serial.print("Next game in: ");
          Serial.print(waitHours);
          Serial.print(":");
          Serial.println(waitMinutes);
        }
      }

      UpdateRemainingTime();
      
      break;
    }
    case InGame:
    {   
      UpdateRemainingTime();

      GetData();
      UpdateVisual();

      delay(10);

      PlayStartGame();
      delay(600);
      PlayStartGame();

      
      
      break;
    }

    case PostGame:
    {
      if(CurrentState == InGame)
      {
        PeriodDisplay.clear();
       
        PeriodDisplay.setCursor(1,1);
        PeriodDisplay.setTextSize(1);
        PeriodDisplay.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
        PeriodDisplay.setTextColor(LED_ON);
        PeriodDisplay.print("F");
        PeriodDisplay.writeDisplay();

        if(PreviousMtl > PreviousVs)
        {
          PlayMtlGoal();
        }
        else
        {
          PlayVsGoal();
        }
      }
      break;
    }
  }

  CurrentState = newState;
}

void DrawPeriodClock(int frame)
{
  switch(CurrentAnimationFrame)
    {
      case 0:
      {
        PeriodDisplay.drawLine(3,3, 3,0, LED_ON);
        break;
      }

      case 1:
      {
        PeriodDisplay.drawLine(3,3, 5,1, LED_ON);
        break;
      }

      case 2:
      {
        PeriodDisplay.drawLine(3,3, 5,3, LED_ON);
        break;
      }

      case 3:
      {
        PeriodDisplay.drawLine(3,3, 5,5, LED_ON);
        break;
      }

      case 4:
      {
        PeriodDisplay.drawLine(3,3, 3,5, LED_ON);
        break;
      }

      case 5:
      {
        PeriodDisplay.drawLine(3,3, 1,5, LED_ON);
        break;
      }

      case 6:
      {
        PeriodDisplay.drawLine(3,3, 1,3, LED_ON);
        break;
      }

      case 7:
      {
        PeriodDisplay.drawLine(3,3, 1,1, LED_ON);
        break;
      }
    }
}

void UpdateWaitAnimation()
{
  if((millis() - LastAnimationTime) > ANIMATION_DELAY)
  {
    CurrentAnimationFrame++;

    if(CurrentAnimationFrame > 7)
      CurrentAnimationFrame = 0;
    
    PeriodDisplay.setFont(NULL);
    PeriodDisplay.setTextSize(1);
    PeriodDisplay.setTextWrap(false);
    PeriodDisplay.clear();
    PeriodDisplay.drawCircle(3,3, 3, LED_ON);

    DrawPeriodClock(CurrentAnimationFrame);
    
    PeriodDisplay.writeDisplay();
    
    LastAnimationTime = millis();
  }
}

void UpdateRemainingTime()
{
  unsigned long remaining = waitTime - (millis() - LastGetDataMillis);
  
  int waitHours = (waitTime == 0) ? 0 : remaining/ONE_HOUR;
  int waitMinutes = (waitTime == 0) ? 0 : ((remaining - (waitHours*ONE_HOUR)) / ONE_MINUTE) + 1;

  MtlDisplay.setFont(&Picopixel);
  MtlDisplay.setTextSize(1);
  MtlDisplay.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
  MtlDisplay.setTextColor(LED_ON);
  MtlDisplay.clear();
  MtlDisplay.setCursor(1,6);
  if(waitHours < 10) MtlDisplay.print("0");
  MtlDisplay.print(waitHours);  
  MtlDisplay.writeDisplay();

  VsDisplay.setFont(&Picopixel);
  VsDisplay.setTextSize(1);
  VsDisplay.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
  VsDisplay.setTextColor(LED_ON);
  VsDisplay.clear();
  VsDisplay.setCursor(1,6);
  if(waitMinutes < 10) VsDisplay.print("0");
  VsDisplay.print(waitMinutes);
  VsDisplay.writeDisplay();
}

void UpdateWaitMessage()
{
  if((millis() - LastScrollTime) > SCROLL_DELAY)
  {
    CurrentScroll--;
    
    int scrollLimit = strlen(Message) * 6;
  
    if(CurrentScroll < -scrollLimit)
    {     
      CurrentScroll = 17;
    }

    MtlDisplay.setFont(NULL);
    MtlDisplay.setTextSize(1);
    MtlDisplay.setTextWrap(false);
    MtlDisplay.setTextColor(LED_ON);
    MtlDisplay.clear();
    MtlDisplay.setCursor(CurrentScroll,1);
    MtlDisplay.print(Message);      
    MtlDisplay.writeDisplay();

    VsDisplay.setFont(NULL);
    VsDisplay.setTextSize(1);
    VsDisplay.setTextWrap(false);
    VsDisplay.setTextColor(LED_ON);
    VsDisplay.clear();
    VsDisplay.setCursor(CurrentScroll - 8,1);
    VsDisplay.print(Message);    
    VsDisplay.writeDisplay();
  
    LastScrollTime = millis();
  }
}

void PlayMtlGoal()
{
  for(int i = 0; i < MTL_GOAL_SONG_SIZE; ++i)
  {
    ledcWriteTone(BUZZER_CHANNEL, MTL_GOAL_SONG[i][0]);
    delay(MTL_GOAL_SONG[i][1]);
  }

  ledcWriteTone(BUZZER_CHANNEL, 0);
}

void PlayVsGoal()
{
  for(int i = 0; i < VS_GOAL_SONG_SIZE; ++i)
  {
    ledcWriteTone(BUZZER_CHANNEL, VS_GOAL_SONG[i][0]);
    delay(VS_GOAL_SONG[i][1]);
  }

  ledcWriteTone(BUZZER_CHANNEL, 0);
}

void PlayStartGame()
{
  for(int i = 0; i < GAMESTART_SONG_SIZE; ++i)
  {
    ledcWriteTone(BUZZER_CHANNEL, GAMESTART_SONG[i][0]);
    delay(GAMESTART_SONG[i][1]);
  }

  ledcWriteTone(BUZZER_CHANNEL, 0);
}

void PlayMtlGoalNote(int noteIndex)//Async: call this once with 0 to start song
{
  CurrentMtlGoalNote = noteIndex;
  
  if(CurrentMtlGoalNote >= MTL_GOAL_SONG_SIZE)
  {
    ledcWriteTone(BUZZER_CHANNEL, 0);
    return;
  }
  
  ledcWriteTone(BUZZER_CHANNEL, MTL_GOAL_SONG[CurrentMtlGoalNote][0]);
  LastMtlGoalNoteTime = millis();
}

void PlayMtlGoalUpdate()//Async: call this every frame
{
  if(CurrentMtlGoalNote >= MTL_GOAL_SONG_SIZE)
  {
    return;
  }
  
  if(millis() - LastMtlGoalNoteTime > MTL_GOAL_SONG[CurrentMtlGoalNote][1])
  {
    PlayMtlGoalNote(CurrentMtlGoalNote++);
  }
}

void PlayVsGoalNote(int noteIndex)//Async: call this once with 0 to start song
{
  CurrentVsGoalNote = noteIndex;
  
  if(CurrentVsGoalNote >= VS_GOAL_SONG_SIZE)
  {
    ledcWriteTone(BUZZER_CHANNEL, 0);
    return;
  }
  
  ledcWriteTone(BUZZER_CHANNEL,VS_GOAL_SONG[CurrentVsGoalNote][0]);
  LastVsGoalNoteTime = millis();
}

void PlayVsGoalUpdate()//Async: call this every frame
{
  if(CurrentVsGoalNote >= VS_GOAL_SONG_SIZE)
  {
    return;
  }
  
  if(millis() - LastVsGoalNoteTime > VS_GOAL_SONG[CurrentVsGoalNote][1])
  {
    PlayVsGoalNote(CurrentVsGoalNote++);
  }
}

void loop()
{
  switch(CurrentState)
  {
    case Initialization:
    {
      if(error != 0 && (millis() - LastGetDataMillis > ErrorUpdateDelay))
      {
        GetData();

        if(error != 0)
        {
          sprintf(Message,"Erreur %i", error);
        }
      }

      if(strlen(Message) > 0)
        UpdateWaitMessage();

      if(error != 0)
      {
        break;
      }
  
      if(waitTime == 0)
      {
        SetState(InGame);
      }
      else
      {
        SetState(PreGame);
      }
      break;
    }

    case PreGame:
    {
      if((millis() < LastGetDataMillis) || (millis() - LastGetDataMillis) > ONE_MINUTE)
      {
        GetData();
        if(error != 0)
        {
          SetState(Initialization);
        }
        else 
        {
          UpdateRemainingTime();
          
          if(waitTime == 0)
          {
            SetState(InGame);
            break;
          }
        }
      }

      if(strlen(Message) > 0)
        UpdateWaitMessage();        

       UpdateWaitAnimation();

      break;
    }

    case InGame:
    {
      if((millis() - LastGetDataMillis) > LiveUpdateDelay)
      {
        GetData();
      }

      if(error != 0)
      {
        SetState(Initialization);
      }
      else
      {
        if(waitTime == 0)
        {
          UpdateVisual();
        }
        else if((millis() - LastGetDataMillis) >= (waitTime - ONE_HOUR))
        {
          SetState(PreGame);
        }
        else
        {
          SetState(PostGame);
        }
      }

      break;
    }

    case PostGame:
    {
      delay(ONE_HOUR);//1 hour
      SetState(PreGame);
      break;
    }
  }
}

bool IsConnected()
{
  return WiFi.status() == WL_CONNECTED;
}

void Connect()
{
  while(WiFi.status() != WL_CONNECTED)
  {
    if(DEBUG_PRINT) Serial.print("Attempting to connect to SSID: ");
    if(DEBUG_PRINT) Serial.println(ssid);
  
    WiFi.disconnect(false);
    WiFi.begin(ssid, password);
  
    //TODO: do in loop so we can display a message or use yield?
    for(int i = 0; i < 5; ++i)
    {
      if(DEBUG_PRINT) Serial.print(".");
      // wait 1 second for re-trying
      delayFor(1000);

      if(WiFi.status() == WL_CONNECTED)
      {
        if(DEBUG_PRINT) Serial.println("");
        if(DEBUG_PRINT) Serial.print("Connected to ");
        if(DEBUG_PRINT) Serial.println(ssid);
        
        break;
      }
    }
  }
}

bool GetData()
{
  LastGetDataMillis = millis();

  if(!IsConnected())
  {
    error = 22;
    Serial.println("Aucune connection reseau");
    sprintf(Message,"Aucune connection reseau\0");
    Connect();
    Message[0] = '\0';
  }
 
  HTTPClient http;

  String url = URL + String(TEAM);
  http.begin(url, root_ca);
  int httpCode = http.GET();

  String response;

  if (httpCode == 200)
  {
      response = http.getString();
      //if(DEBUG_PRINT) Serial.println(response);
  }
  else
  {
    error = 44;
    if(DEBUG_PRINT) Serial.println("Erreur serveur");
    if(DEBUG_PRINT) Serial.println(httpCode);
    sprintf(Message,"Erreur serveur\0");
    return false;
  }

  http.end(); //Free the resources
    
  // Allocate the JSON document
  // Use arduinojson.org/v6/assistant to compute the capacity.
  const size_t capacity = 150;//90 + buffer
  DynamicJsonDocument doc(capacity);

  // Parse JSON object
  DeserializationError deserializeError = deserializeJson(doc, response);

 
  if (deserializeError)
  {
    error = 33;
    if(DEBUG_PRINT) Serial.print(F("deserializeJson() failed: "));
    if(DEBUG_PRINT) Serial.println(deserializeError.c_str());
    sprintf(Message,"Erreur de format de fichier\0");
    return false;
  }

  int previousError = error;
  PreviousMtl = mtl;
  PreviousVs = vs;

  error = doc["e"];
  
  if(error == 0)
  {
    waitTime = doc["t"];
    mtl = doc["m"];
    vs = doc["v"];
    period = doc["p"];
    
    ErrorCount = 0;
     
    if(previousError != 0)
    {
      Message[0] = '\0';
    }
   
  }

  if(mtl > PreviousMtl)
  {
    PlayMtlGoal();
  }

  if(vs > PreviousVs)
  {
    PlayVsGoal();
  }

  if(error != 0)
  {
    ErrorCount++;

    if(ErrorCount > 3)
    {
      Serial.println("Displaying error");
      sprintf(Message,"Erreur %d\0", error);
    }
    else if(DEBUG_PRINT) 
    {
      Serial.print("ErrorCount: ");
      Serial.print(ErrorCount);
    }
  }

  //TODO: read message in Message

  if(DEBUG_PRINT) 
  {
    Serial.print("Error: ");
    Serial.print(error);
    Serial.print(" WaitTime: ");
    Serial.print(waitTime);
    Serial.print(" Period: ");
    Serial.print(period);
    Serial.print(" MTL: ");
    Serial.print(mtl);
    Serial.print(" VS: ");
    Serial.println(vs);
  }

  LastGetDataMillis = millis();

  return true;
}

void TurnOffDisplays()
{
  PeriodDisplay.clear();
  MtlDisplay.clear();
  VsDisplay.clear();

  PeriodDisplay.writeDisplay();
  MtlDisplay.writeDisplay();
  VsDisplay.writeDisplay();
}

void UpdateVisual()
{
  PeriodDisplay.clear();
  MtlDisplay.clear();
  VsDisplay.clear();

  PeriodDisplay.setFont(NULL);
  PeriodDisplay.setCursor(1,1);
  PeriodDisplay.setTextSize(1);
  PeriodDisplay.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
  PeriodDisplay.setTextColor(LED_ON);
  PeriodDisplay.print(period, DEC);
  PeriodDisplay.writeDisplay();

  MtlDisplay.setTextSize(1);
  MtlDisplay.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
  MtlDisplay.setTextColor(LED_ON);
  
  if(mtl < 10)
  {
    MtlDisplay.setFont(NULL);
    MtlDisplay.setCursor(1,1);
  }
  else
  {
    MtlDisplay.setFont(&Picopixel);
    MtlDisplay.setCursor(0,6);
  }

  MtlDisplay.print(mtl, DEC);
  MtlDisplay.writeDisplay();

  VsDisplay.setTextSize(1);
  VsDisplay.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
  VsDisplay.setTextColor(LED_ON);

  if(vs < 10)
  {
    VsDisplay.setFont(NULL);
    VsDisplay.setCursor(1,1);
  }
  else
  {
    VsDisplay.setFont(&Picopixel);
    VsDisplay.setCursor(0,6);
  }
  
  VsDisplay.print(vs, DEC);
  VsDisplay.writeDisplay();
}

void delayFor(long milliseconds)
{
    long now = millis();

    while (millis() - now < milliseconds)
    {
        yield();
    }
}
