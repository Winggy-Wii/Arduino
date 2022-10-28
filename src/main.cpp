
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <EEPROM.h>


// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 2. Define the API Key */
#define API_KEY "AIzaSyBrJCZkqregTjsy4wPPlVlxF7n-Bs1Oppo"

/* 3. Define the RTDB URL */
#define DATABASE_URL "https://fir-esp-data-demo-default-rtdb.asia-southeast1.firebasedatabase.app/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "nghiepquy6@gmail.com"
#define USER_PASSWORD "QuyPhat123"

#define EEPROM_SIZE 1

// Define Firebase Data objects
FirebaseData fbdo;
FirebaseData stream;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int ran1 = 0;
int ran2 = 0;
int ran3 = 0;
int ran4 = 0;

boolean read_and_parse_serial_data()
{
  if (!Serial.available())
  {
    return false;
  }
  String rxString = "";
  String strArr[4]; // Set the size of the array to equal the number of values you will be receiveing.

  // Keep looping until there is something in the buffer.
  while (Serial.available())
  {
    delay(2);
    // Delay to allow byte to arrive in input buffer.

    // Read a single character from the buffer.
    char ch = Serial.read();
    // Append that single character to a string.
    rxString += ch;
  }

  int stringStart1 = 0;
  int dataStorage = 0;
  for (int i = 0; i < rxString.length(); i++)
  {
    // Get character and check if it's our "special" character.
    /* Chỗ này là mặc định sẽ lấy phần String từ dấu , trở lên có thể thay dấu , bằng cái khác giả sử như là dấu .
    hoặc kí tự A, B, C, D*/
    if (rxString.charAt(i) == ',')
    {
      // Clear previous values from array.
      strArr[dataStorage] = "";
      // Save substring into array.
      strArr[dataStorage] = rxString.substring(stringStart1, i);
      // Set new string starting point.
      stringStart1 = (i + 1);
      dataStorage += 1;
    }
  }
  String x1 = strArr[0];
  String y1 = strArr[1];
  String x2 = strArr[2];
  String y2 = strArr[3];
  ran1 = x1.toInt();
  ran2 = y1.toInt();
  ran3 = x2.toInt();
  ran4 = y2.toInt();
  return true;
}
void setup()
{
  EEPROM.begin(EEPROM_SIZE);
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  int signedup = EEPROM.read(0);
  Serial.println(signedup);
  String MacAdress = WiFi.macAddress();
  MacAdress.replace(":", "");
  String display = "<p>Your ID: " + MacAdress + "</p>";
  String email = MacAdress + "@gmail.com";
  Serial.println(MacAdress);
  WiFiManagerParameter custom_text((const char *)display.c_str());
  WiFiManager wifiManager;
  wifiManager.resetSettings();

  wifiManager.addParameter(&custom_text);
  wifiManager.autoConnect("WING's WiFi Manager");
  Serial.println("connected :)");

  /* kết nối vào wifi theo thư viện esp8266Wifi*/
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  // For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

  /* Assign the api key (required) */
  config.api_key = API_KEY;
  if (signedup != 1)
  {
    if (Firebase.signUp(&config, &auth, email, MacAdress))
    {
      Serial.println("ok");
      EEPROM.write(0, 1);
      EEPROM.commit();
    }
    else
    {
      Serial.printf("%s\n", config.signer.signupError.message.c_str());
    }
  }
  else
  {
    auth.user.email = email;
    auth.user.password = MacAdress;
  }

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Or use legacy authenticate method
  // config.database_url = DATABASE_URL;
  // config.signer.tokens.legacy_token = "<database secret>";

  Firebase.begin(&config, &auth);

  /* Tự động kết nối lại database khi bị mất kết nôi*/
  Firebase.reconnectWiFi(true);
}

void loop()
{
  String uid = auth.token.uid.c_str();
  /* Cái này để lấy dữ liệu từ Arduino */

  /* sau 15 giây gửi đi tín hiệu 1 lần, hàm millis cho biết thời gian chạy chương trình cho đến lúc hàm được gọi*/
  if (Firebase.ready() && read_and_parse_serial_data())
  {
    // Write an Int number on the database path test/int
    /* Đoạn này để set dữ liệu lên realtime database, dữ liệu phải có cùng dạng với lệnh chẳng hạn setInt thì
    kiểu dữ liệu phải là int mới được không báo lỗi*/
    /* Cái test/int là cái nhánh mình muốn set dữ liệu trên database, ran1 là giá trị của dữ liệu sẽ set lấy
    từ biến ran1*/
    if (Firebase.RTDB.setInt(&fbdo, +"/Oxygen_Level", ran1))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setInt(&fbdo, "test/int1", ran2))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setInt(&fbdo, "test/int2", ran3))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setInt(&fbdo, "test/int3", ran4))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    // Write an Float number on the database path test/float
    if (Firebase.RTDB.setFloat(&fbdo, "test/float", 0.01 + random(0, 100)))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
}