#include <WiFi.h>
#include <FS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
/* If using software SPI (the default case): */
#define OLED_MOSI 23  // 9
#define OLED_CLK 18   // 10
#define OLED_DC 33    // 11
#define OLED_CS 32    // 12
#define OLED_RESET 5 // 13
Adafruit_SH1106 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
// Uncomment this block to use hardware SPI
// #define OLED_DC 26
// #define OLED_CS 15 // HSPI
// #define OLED_RESET 27
// Adafruit_SH1106 display(OLED_DC, OLED_RESET, OLED_CS);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH 16
static const unsigned char PROGMEM logo16_glcd_bmp[] =
    {B00000000, B11000000,
     B00000001, B11000000,
     B00000001, B11000000,
     B00000011, B11100000,
     B11110011, B11100000,
     B11111110, B11111000,
     B01111110, B11111111,
     B00110011, B10011111,
     B00011111, B11111100,
     B00001101, B01110000,
     B00011011, B10100000,
     B00111111, B11100000,
     B00111111, B11110000,
     B01111100, B11110000,
     B01110000, B01110000,
     B00000000, B00110000};
#if (SH1106_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SH1106.h!");
#endif

const char* ssid = "Realmadrid";
const char* password = "kingofmadrid";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en" data-theme="cupcake">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>Baby Monitoring Dashboard</title>
    <link rel="stylesheet" type="text/css" href="https://cdn.jsdelivr.net/npm/daisyui@4.4.6/dist/full.min.css"/>
    <link rel="stylesheet" type="text/css" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.2/css/all.min.css"/>
    <script src="https://cdn.tailwindcss.com"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/crypto-js/4.0.0/crypto-js.min.js"></script>
    <style>
      @keyframes sirenAnimation { 0%, 50% {background-color: gray;} 100% {background-color: red;} }
      @keyframes alertAnimation { 0%, 100% {background-color: orange;} 50% {background-color: gray;} }
      @keyframes swing-rotate { 0%,
        25% {transform: translateX(10px) rotate(5deg);}
        50% {transform: translateX(0) rotate(0deg);}
        75% {transform: translateX(-10px) rotate(-5deg);}
        100% {transform: translateX(0) rotate(0deg);}
      }
      
      .emergency-mode { opacity: 50%; animation: sirenAnimation 0.5s linear infinite; }
      .alert-mode { opacity: 50%; animation: alertAnimation 0.5s linear infinite; }
      .ringing { animation: swing-rotate 1s ease infinite; }

      #info > i { font-size: 1.25rem; margin-right: 0.5rem; }
      #info > i > span { font-size: 1rem; margin-left: 0.5rem; }
      #controller > div { display: flex; align-items: center; }
      #controller > div > label { margin-left: auto; }
    </style>
  </head>
  <body class="bg-gray-100 font-mono">
    <div class="bg-blue-300 py-4 px-8 text-white flex justify-between text-xl">
      <div><i class="fas fa-lightbulb"><span id="alarm-message" class="mx-2 text-base">특이사항 없음</span></i></div>
      <div><i class="fas fa-baby"><span id="timer-box" class="mx-2"> 00: 00: 00</span></i></div>
    </div>
    <main class="container mx-auto lg:flex lg:space-x-8">
      <div class="lg:w-7/12 my-2">
        <!-- Video iframe goes here -->
        <iframe src="http://192.168.56.166/mjpeg/1" class="p-1 m-auto h-[490px] w-[655px] scale-x-[-1] scale-y-[-1]"></iframe>
      </div>
      <div class="lg:w-5/12 my-6">
        <div class="bg-gray-200 py-2 px-4 flex rounded-lg justify-between items-center">
          <div id="info">
            <i class="fas fa-thermometer-half"><span id="temp">25 °C</span></i>
            <i class="fas fa-tint"><span id="humid">60 %</span></i>
            <i class="fas fa-weight"><span id="weight">5.2 kg</span></i>
          </div>
          <div>
            <button id="refresh-button" class="refresh-data-button btn btn-primary"><i class="fas fa-sync-alt text-white"></i></button>
          </div>
        </div>
        <div class="bg-gray-200 p-4 my-6 rounded-lg">
          <div class="flex items-center">
            <i class="fas fa-tv text-xl"><span class="ml-2 text-base">리모콘</span></i>
          </div>
          <div id="controller" class="mt-4 space-y-4">
            <div>
              <i class="fas"><span>모빌 회전</span></i>
              <label><input id="mobile" type="checkbox" class="toggle toggle-primary" /></label>
            </div>
            <div>
              <i class="fas"><span>요람 흔들기</span></i>
              <label><input id="cradle" type="checkbox" class="toggle toggle-primary" /></label>
            </div>
            <div>
              <i class="fas"><span>선풍기 동작</span></i>
              <label><input id="fan" type="checkbox" class="toggle toggle-primary" /></label>
            </div>
            <div>
              <i class="fas"><span>타이머</span></i>
              <i class="fas ml-auto">
                <label>
                  <div class="relative inline-flex">
                    <select id="hours" class="select select-bordered focus:outline-none">
                      <option value="0">0시간</option>
                      <option value="1">1시간</option>
                      <option value="2">2시간</option>
                    </select>
                  </div>
                  <div class="relative inline-flex p-2">
                    <select id="minutes" class="select select-bordered focus:outline-none">
                      <option value="0">0분</option>
                      <option value="1">1분</option>
                      <option value="15">15분</option>
                      <option value="30">30분</option>
                    </select>
                  </div>
                  <i class="fas"><button id="timer-button" class="btn btn-primary text-white">시작</button></i>
                </label>
              </i>
            </div>
          </div>
        </div>
      </div>
    </main>
    <!-- Add this HTML for the modalBg modal -->
    <div id="modalBg" class="fixed inset-0 flex z-10 bg-gray-900 bg-opacity-20 hidden"></div>
    <div id="modal" class="fixed inset-0 flex items-center justify-center hidden z-50">
      <div id="modalBox" class="bg-white p-8 px-8 rounded-lg shadow-lg justify-center items-center">
        <div class="flex items-center justify-center" id="iconBox">
          <i class="fas text-4xl mt-2" id="alert-icon"></i>
        </div>
        <i class="fas"><p class="text-center mt-2" id="alert-message">알람 메시지</p></i>
        <div class="flex items-center justify-center mt-3">
          <i class="fas"><button id="confirm-button" class="btn btn-secondary btn-sm text-white">확인</button></i>
        </div>
      </div>
    </div>
    <script>
      function sign(key, msg) {
        return CryptoJS.HmacSHA256(msg, key);
      }

      function getSignatureKey(key, dateStamp, regionName, serviceName, terminator) {
        let kDate = sign("AWS4" + key, dateStamp);
        let kRegion = sign(kDate, regionName);
        let kService = sign(kRegion, serviceName);
        let kSigning = sign(kService, terminator);
        return kSigning;
      }
 
      function signed_header(region_name, service_name, access_key, secret_key, method, url, body, terminator) {
        const now = new Date().toISOString();
        const amz_date = now.split('.')[0].replace(/[:-]/g, '') + 'Z';    // change format to YYYYMMDD'T'HHMMSS'Z'
        const date_stamp = now.split('T')[0].replace(/-/g, '');        // change format to YYYYMMDD
        
        const content_type = 'application/json';
        const host = url.split('/')[2];                                          // <your-endpoint>.<service-name>.<region>.amazonaws.com
          const canonical_uri = url.split('com')[1].split('?')[0];                 // <path>
            const canonical_querystring = url.split('com')[1].split('?')[1] || '';   // <query string>
        const request_parameters = body || '';                                   // <request body>
        const request_terminator = terminator || 'aws4_request';


        const payload_hash = CryptoJS.SHA256(request_parameters).toString();
        const canonical_headers = `content-type:${content_type}\nhost:${host}\nx-amz-date:${amz_date}\n`;
        const signed_headers = `content-type;host;x-amz-date`;
        
        const canonical_request = `${method}\n${canonical_uri}\n${canonical_querystring}\n${canonical_headers}\n${signed_headers}\n${payload_hash}`;
        
        const algorithm = `AWS4-HMAC-SHA256`;
        const credential_scope = `${date_stamp}/${region_name}/${service_name}/${request_terminator}`;
        const string_to_sign = `${algorithm}\n${amz_date}\n${credential_scope}\n${CryptoJS.SHA256(canonical_request)}`;
        
        const signing_key = getSignatureKey(secret_key, date_stamp, region_name, service_name, request_terminator);
        const signature = sign(signing_key, string_to_sign).toString(CryptoJS.enc.Hex);

        const authorization_header = `${algorithm} Credential=${access_key}/${credential_scope}, SignedHeaders=${signed_headers}, Signature=${signature}`;
        
        return {
          'Content-Type': content_type,
          'X-Amz-Date': amz_date,
          'Authorization': authorization_header
        };
      }

      async function get_shadow(shadow_name) {
        var url = 'https://a1fmltb7n8klk1-ats.iot.ap-northeast-2.amazonaws.com/things/Smart_Cradle/shadow';
        if (shadow_name != "classic") url += '?name=' + shadow_name;

        const method = 'GET';
        try {
          const response = await fetch(url, {
            method: method,
            headers: signed_header('ap-northeast-2', 'iotdata', 'AKIA4TP7DHY4FSGI5TMF', '3kTOhwPWtTBkVSKxvFZSGBrr3w02yLJ8JRQ68Bbs', method, url)
          })
          const data = await response.json();
          return data.state;
        } catch (err) {
          console.log(err);
          return null;
        }
      }

      function post_shadow(shadow_name, body) {
        var url = 'https://a1fmltb7n8klk1-ats.iot.ap-northeast-2.amazonaws.com/things/Smart_Cradle/shadow';
        if (shadow_name != "classic") url += '?name=' + shadow_name;

        const method = 'POST';
        fetch(url, {
          method: method,
          headers: signed_header('ap-northeast-2', 'iotdata', 'AKIA4TP7DHY4FSGI5TMF', '3kTOhwPWtTBkVSKxvFZSGBrr3w02yLJ8JRQ68Bbs', method, url, JSON.stringify(body)),
          body: JSON.stringify(body)
        }).then(res => res.json())
          .then(data => console.log(data))
          .catch(err => console.log(err));
      }
    </script>
    <script>
      const timerBox = document.getElementById("timer-box");
      const timerButton = document.getElementById("timer-button");
      const refreshButton = document.getElementById("refresh-button");
      const alarmMessage = document.getElementById("alarm-message");
      const alertMessage = document.getElementById("alert-message");
      const modalBg = document.getElementById("modalBg");
      const modal = document.getElementById("modal");
      const modalBox = document.getElementById("modalBox");
      const alertIcon = document.getElementById("alert-icon");
      const iconBox = document.getElementById("iconBox");
      const confirmButton = document.getElementById("confirm-button");
      const radioButtons = document.getElementsByName("radio-1");
      const tempText = document.getElementById("temp");
      const humidText = document.getElementById("humid");
      const weightText = document.getElementById("weight");
      const mobileCheckbox = document.getElementById("mobile");
      const cradleCheckbox = document.getElementById("cradle");
      const fanCheckbox = document.getElementById("fan");
      let countdownSeconds;
      let countdownInterval;
    </script>
    <script>
      function startTimer(totalSeconds) {
        if (totalSeconds <= 0) {
          timerBox.classList.remove("animate-pulse");
          return;
        }

        timerButton.innerHTML = "취소";
        timerButton.classList.add("btn-error");
        timerBox.classList.add("animate-pulse");

        countdownSeconds = totalSeconds;
        countdownTimer();
        countdownInterval = setInterval(countdownTimer, 1000);

        current_time = new Date()
        timer_time = new Date(current_time.getTime() + (totalSeconds+1) * 1000).toISOString();
        post_shadow("classic", {"state": {"reported": {"timer": timer_time}}});
      }

      function countdownTimer() {
        if (countdownSeconds <= 0) {
          stopTimer();
          showBabyEscapeAlert();
          timerBox.classList.remove("animate-pulse");
        } else {
          setTimerDisplay(countdownSeconds);
          countdownSeconds--;
        }
      }
      
      function stopTimer() {
        clearInterval(countdownInterval);
        setTimerDisplay(0);
        timerBox.classList.remove("animate-pulse");
        timerButton.classList.remove("btn-error");
        timerButton.innerHTML = "시작";
        post_shadow("classic", {"state": {"reported": {"timer": "stop"}}});
      }
      
      function setTimerDisplay(totalSeconds) {
        const hours = Math.floor(totalSeconds / 3600);
        const minutes = Math.floor((totalSeconds % 3600) / 60);
        const seconds = totalSeconds % 60;
        timerBox.textContent = `
          ${hours.toString().padStart(2, "0")}:
          ${minutes.toString().padStart(2, "0")}:
          ${seconds.toString().padStart(2, "0")}
        `;
      }
    </script>
    <script>
      function showModal(message, iconClass, animationMode, iconAnime) {
        modalBg.classList.remove("hidden");
        modal.classList.remove("hidden");

        alarmMessage.innerHTML = message;
        alertMessage.textContent = message;
        const icon = document.createElement("i");
        icon.className = `fas ${iconClass} text-4xl mt-2`;
        iconBox.appendChild(icon);
        iconBox.classList.add(iconAnime);
        modalBg.classList.add(animationMode);
      }

      function showDiaperChangeAlert() { showModal("끙차~아기 기저귀를 갈아주세요!", "fa-poo", "alert-mode", "animate-bounce");}
      function showBabyCryAlert() { showModal("응애응애. 아기가 울고 있어요.", "fa-baby", "emergency-mode", "ringing"); }
      function showBabyEscapeAlert() { showModal("아기가 요람에서 벗어났습니다!", "fa-circle-exclamation", "emergency-mode", "ringing"); }
      function showBabyMealAlert() { showModal("응애 ~ 맘마 주세요!", "fa-person-breastfeeding", "alert-mode", "animate-bounce"); }
    </script>
    <script>
      function updateInfo() {
        get_shadow("ESP32_bottom").then(data => {
          const state = data.reported;
          console.log(state);
          tempText.innerHTML = state.temp + " °C";
          humidText.innerHTML = state.humid + " %";
          weightText.innerHTML = parseInt(state.weight/10)/100 + " kg";
          cradleCheckbox.checked = (state.servo=="ON");
        });
        get_shadow("ESP32_top").then(data => {
          const state = data.reported;
          console.log(state);
          mobileCheckbox.checked = (state.servo=="ON");
          fanCheckbox.checked = (state.servo=="ON");
        });
      }

      // 모달 확인 버튼 클릭 시
      confirmButton.addEventListener("click", () => {
        alarmMessage.innerHTML = "특이사항 없음.";
        modal.classList.add("hidden");
        modalBg.classList.add("hidden");
        
        iconBox.innerHTML = "";
        modalBg.classList.remove("alert-mode", "emergency-mode");
        iconBox.classList.remove("ringing", "anime-bounce");
        alertIcon.classList.remove("fa-poo", "fa-baby", "fa-circle-exclamation", "fa-person-breastfeeding");
      });

      // 타이머 시작(취소) 버튼 클릭 시
      timerButton.addEventListener("click", () => {
        if (timerButton.innerHTML == "시작") {
          const hours = parseInt(document.getElementById("hours").value);
          const minutes = parseInt(document.getElementById("minutes").value);
          const totalSeconds = (hours * 60 + minutes);
          startTimer(totalSeconds);
        } else {
          stopTimer();
        }
      });
      
      // // 아기 배변모드 예시: 5초마다 배변 감지 알림 표시
      // setTimeout(showDiaperChangeAlert, 500);
      // setTimeout(showBabyEscapeAlert, 2000);
      // setTimeout(showBabyCryAlert, 2000);
      // setTimeout(showBabyMealAlert, 2000);

      refreshButton.addEventListener("click", () => updateInfo());
      mobileCheckbox.addEventListener("click", () => {
        post_shadow("ESP32_top", {"state":{"desired": {"servo": mobileCheckbox.checked ? "ON" : "OFF"}}});
      });
      cradleCheckbox.addEventListener("click", () => {
        post_shadow("ESP32_bottom", {"state":{"desired": {"servo": cradleCheckbox.checked ? "ON" : "OFF"}}});
      });
      fanCheckbox.addEventListener("click", () => {
        post_shadow("ESP32_top", {"state":{"desired": {"servo": fanCheckbox.checked ? "ON" : "OFF"}}});
      });

      function setFirstTimer() {
        get_shadow("classic").then(data => {
          const state = data.reported;
          console.log(state);
          if (state.timer == "stop") return;
          const timer_time = new Date(state.timer);
          const current_time = new Date();
          const totalSeconds = Math.floor((timer_time - current_time) / 1000);
          if (totalSeconds <= 0) return;

          countdownSeconds = totalSeconds;
          countdownTimer();
          countdownInterval = setInterval(countdownTimer, 1000);
        });
      }

      updateInfo();
      setFirstTimer();
    </script>
  </body>
</html>
)rawliteral";

AsyncWebServer server(80);

void WiFiSetup() {
  Serial.print("WIFI status = "); Serial.println(WiFi.getMode());
  WiFi.disconnect(true); delay(1000);

  WiFi.mode(WIFI_STA); delay(1000);
  Serial.print("WIFI status = "); Serial.println(WiFi.getMode());
  WiFi.begin(ssid, password);


  for(; WiFi.status() != WL_CONNECTED; delay(1000))
    Serial.println("Connecting to WiFi..");

  Serial.println("Connected to wifi");
  Serial.println("STA IP address: "); Serial.println(WiFi.localIP());
}

void ServerSetup() {
  server.on("/", HTTP_GET, [] (AsyncWebServerRequest *request) {
      request->send(200, "text/html", index_html);
    });
  server.begin();
}

void LCDSetup() {
  Serial.begin(115200);
  display.begin(SH1106_SWITCHCAPVCC);
  display.display();
  delay(1000);
  display.clearDisplay();

  // text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  display.println("Temperature: 36.5 *C");  
  display.println("Humid: 100 %");
  display.println("Weight: 2.3 Kg");
  display.display();
}


void setup() {
  Serial.begin(115200);
  WiFiSetup();
  ServerSetup();
  LCDSetup();
}

void loop() {
  char ch;
  if (Serial.available() > 0) {
    ch = Serial.read();
    if (ch == '0') {
      display.clearDisplay();
      display.setCursor(0, 0);
    }
    else if (ch >= '1' && ch < '9')
      display.setTextSize(ch - '0');
    else
      display.print(ch);
    display.display();
  }
}