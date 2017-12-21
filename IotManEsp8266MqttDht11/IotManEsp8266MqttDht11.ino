#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define UpdateFrq 70 //上传温湿度的周期（单位为毫秒）
#define DHTPIN 2     // 定义dht11数据线连接的引脚，2是D4

//定义dht的型号，默认是DHT11
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "your-wifi-ssid";//连接的路由器的名字
const char* password = "your-wifi-passwd";//连接的路由器的密码
const char* mqtt_server = "cnxel.cn";//mqtt服务器的地址
const uint16_t mqtt_port = 1883;//mqtt服务器的端口

#define MQTT_KEEPALIVE 5
const char* mqtt_userid = "1512226921";//mqtt的设备id
const char* mqtt_username = "1512226921";//mqtt的设备账号
const char* mqtt_userpasswd = "1512226921";//mqtt的设备密码
const char* mqtt_topic = "iotman/1512226921";//订阅和发布的主题

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;//存放时间的变量
int settem;//温控变量

void setup_wifi() {//自动接入网络
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {//用于接收服务器接收的数据
/*
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);//串口打印出接收到的数据让你看看
  }
  Serial.println();//换行


  if ((char)payload[0] == 'a') {//如果收到的数据是a
    settem++;
  }
*/

}

void reconnect() {//等待，直到连接上服务器
  while (!client.connected()) {
    if (client.connect(mqtt_userid, mqtt_username, mqtt_userpasswd)) { //接入时的用户名，尽量取一个不易与别人重复的用户名
      client.subscribe(mqtt_topic);//接收外来的数据时的intopic
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());//重新连接
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {//初始化程序，只运行一遍
  settem = 0;
  Serial.begin(115200);//设置串口波特率（与烧写用波特率不是一个概念）
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);//mqtt_port为端口号
  client.setCallback(callback);
  dht.begin();
}



void loop() {//主循环

  digitalWrite(BUILTIN_LED, HIGH);
  reconnect();//确保连上服务器，否则一直等待。
  client.loop();//MUC接收数据的主循环函数。
  float tem = 24.68; //温度，测试使用
  long now = millis();//记录当前时间
  if (now - lastMsg > UpdateFrq) {//每隔2秒发一次信号
    lastMsg = now;//刷新上一次发送数据的时间
    //读取dht
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht.readTemperature(true);

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
      //return;
    } else {
      // Compute heat index in Fahrenheit (the default)
      float hif = dht.computeHeatIndex(f, h);
      // Compute heat index in Celsius (isFahreheit = false)
      float hic = dht.computeHeatIndex(t, h, false);

      char msg[50]; //存放要发的数据
      //"{\"t\":\"\",\"h\":\"\"}"
      char tempjson[50] = {};
      char tempjson2[50] = {};
      tem = t;
      dtostrf(tem, 1, 2, msg); //将float转为char的数组msg，其中第三个传入值(2)是保留的位数(2位）
      sprintf(tempjson,"{\"t\":\"%s\"",msg);

      tem = h;
      dtostrf(tem, 1, 2, msg); //将float转为char的数组msg，其中第三个传入值(2)是保留的位数(2位）
      sprintf(tempjson2,"%s,\"h\":\"%s\"}",tempjson,msg);
      client.publish(mqtt_topic, tempjson2);//发送数据，其中temperature是发出去的topic(不清楚请百度mqtt)
      /*
        tem=h;
        dtostrf(tem,1,2,msg);//将float转为char的数组msg，其中第三个传入值(2)是保留的位数(2位）
        client.publish(mqtt_topic, msg);//发送数据，其中temperature是发出去的topic(不清楚请百度mqtt)
        tem=t;
        dtostrf(tem,1,2,msg);//将float转为char的数组msg，其中第三个传入值(2)是保留的位数(2位）
        client.publish(mqtt_topic, msg);//发送数据，其中temperature是发出去的topic(不清楚请百度mqtt)

        //snprintf (msg, 75, "%d", settem);//将int类型的settem转为char的数组.
        //client.publish(mqtt_topic, msg);//反馈设定的温度值
      */

      //Serial.println("ok!");//串口打印OK！
    }
  }
}

