#include <Arduino.h>
#include <lvgl.h>
#include <LovyanGFX.hpp>
#include "FT6236.h"
#include "ui.h"


#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h"
#include "esp_rom_gpio.h"
 
#define DHT_22  GPIO_NUM_40
 
#define uchar unsigned char
#define uint8 unsigned char
#define uint16 unsigned short

//温湿度定义
uchar uchartemp;//COM函数调用,传递数据
float Humi,Temp;
//uchar ucharT_data_H,ucharT_data_L,ucharRH_data_H,ucharRH_data_L,ucharcheckdata;//验证过后传递数据
uchar ucharT_data_H_temp,ucharT_data_L_temp,ucharRH_data_H_temp,ucharRH_data_L_temp,ucharcheckdata_temp;
uchar ucharcomdata;//存储com获取的数据


const int i2c_touch_addr = TOUCH_I2C_ADD;
#define LCD_BL 46
#define SDA_FT6236 38
#define SCL_FT6236 39
int led;

static void InputInitial()
{
	esp_rom_gpio_pad_select_gpio(DHT_22);
	gpio_set_direction(DHT_22, GPIO_MODE_INPUT);
}
static void OutputHigh(void)//输出1
{
	esp_rom_gpio_pad_select_gpio(DHT_22);
	gpio_set_direction(DHT_22, GPIO_MODE_OUTPUT);
	gpio_set_level(DHT_22, 1);
}
static void OutputLow(void)//输出1
{
	esp_rom_gpio_pad_select_gpio(DHT_22);
	gpio_set_direction(DHT_22, GPIO_MODE_OUTPUT);
	gpio_set_level(DHT_22, 0);
}
static uint8 getData()//读取状态
{
	return gpio_get_level(DHT_22);
}
//读取一个字节数据
static void COM()
{
	uchar i;
	for (i=0;i<8;i++)
	{
		while(!getData())
			uchartemp=0;
		ets_delay_us(35);//延时35us
		if(getData()==1)
			uchartemp=1;
		ucharcomdata <<= 1;
		ucharcomdata |= uchartemp;
		while(getData());
	}
}

void DHT22(void)   //温湿传感启动
{
  OutputLow();
  ets_delay_us(1000);
  OutputHigh();
	ets_delay_us(40);
  InputInitial(); //输入
  if(!getData())//表示传感器拉低总线
  {
    //等待总线被传感器拉高
    while(!getData())
			ets_delay_us(10);
    //等待总线被传感器拉低
    while(getData()) 
		  ets_delay_us(10);
    COM();//读取第1字节，
    ucharRH_data_H_temp=ucharcomdata;
    COM();//读取第2字节，
    ucharRH_data_L_temp=ucharcomdata;
    COM();//读取第3字节，
    ucharT_data_H_temp=ucharcomdata;
    COM();//读取第4字节，
    ucharT_data_L_temp=ucharcomdata;
    COM();//读取第5字节，
    ucharcheckdata_temp=ucharcomdata;
    OutputHigh();
    //判断校验和是否一致
    uchartemp=(ucharT_data_H_temp+ucharT_data_L_temp+ucharRH_data_H_temp+ucharRH_data_L_temp);
    if(uchartemp==ucharcheckdata_temp)
      {
        //校验和一致，
        /*ucharRH_data_H=ucharRH_data_H_temp;
        ucharRH_data_L=ucharRH_data_L_temp;
        ucharT_data_H=ucharT_data_H_temp;
        ucharT_data_L=ucharT_data_L_temp;
        ucharcheckdata=ucharcheckdata_temp;*/
        //保存温度和湿度
        Humi=ucharRH_data_H_temp;
        Humi=((uint16)Humi<<8|ucharRH_data_L_temp)/10;
        Temp=ucharT_data_H_temp;
        Temp=((uint16)Temp<<8|ucharT_data_L_temp)/10;
      }
    else
    {
      Humi=100;
      Temp=100;
    }
  }
  else //没用成功读取，返回0
  {
    Humi=0,
    Temp=0;
    Serial.println(123);
  }
  OutputHigh(); //输出
}




class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ILI9488 _panel_instance;
  lgfx::Bus_Parallel16 _bus_instance;

public:
  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();
      cfg.port = 0;
      cfg.freq_write = 80000000;
      cfg.pin_wr = 18;
      cfg.pin_rd = 48;
      cfg.pin_rs = 45;

      cfg.pin_d0 = 47;
      cfg.pin_d1 = 21;
      cfg.pin_d2 = 14;
      cfg.pin_d3 = 13;
      cfg.pin_d4 = 12;
      cfg.pin_d5 = 11;
      cfg.pin_d6 = 10;
      cfg.pin_d7 = 9;
      cfg.pin_d8 = 3;
      cfg.pin_d9 = 8;
      cfg.pin_d10 = 16;
      cfg.pin_d11 = 15;
      cfg.pin_d12 = 7;
      cfg.pin_d13 = 6;
      cfg.pin_d14 = 5;
      cfg.pin_d15 = 4;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();

      cfg.pin_cs = -1;
      cfg.pin_rst = -1;
      cfg.pin_busy = -1;
      cfg.memory_width = 320;
      cfg.memory_height = 480;
      cfg.panel_width = 320;
      cfg.panel_height = 480;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 2;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = true;
      cfg.invert = false;
      cfg.rgb_order = false;
      cfg.dlen_16bit = true;
      cfg.bus_shared = true;
      _panel_instance.config(cfg);
    }
    setPanel(&_panel_instance);
  }
};

LGFX tft;
/*Change to your screen resolution*/
static const uint16_t screenWidth = 480;
static const uint16_t screenHeight = 320;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * screenHeight / 5];

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.writePixels((lgfx::rgb565_t *)&color_p->full, w * h);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  int pos[2] = {0, 0};

  ft6236_pos(pos);
  if (pos[0] > 0 && pos[1] > 0)
  {
    data->state = LV_INDEV_STATE_PR;
    //    data->point.x = tft.width()-pos[1];
    //    data->point.y = pos[0];
    data->point.x = tft.width() - pos[1];
    data->point.y = pos[0];
    Serial.printf("x-%d,y-%d\n", data->point.x, data->point.y);
  }
  else
  {
    data->state = LV_INDEV_STATE_REL;
  }
}

void touch_init()
{
  // I2C init
  Wire.begin(SDA_FT6236, SCL_FT6236);
  byte error, address;

  Wire.beginTransmission(i2c_touch_addr);
  error = Wire.endTransmission();

  if (error == 0)
  {
    Serial.print("I2C device found at address 0x");
    Serial.print(i2c_touch_addr, HEX);
    Serial.println("  !");
  }
  else if (error == 4)
  {
    Serial.print("Unknown error at address 0x");
    Serial.println(i2c_touch_addr, HEX);
  }
}

extern "C" void app_main()
{
  Serial.begin(115200); /* prepare for possible serial debug */
  //IO口引脚
  pinMode(19, OUTPUT);
  digitalWrite(19, LOW);
  
  tft.begin();        /* TFT init */
  tft.setRotation(1); /* Landscape orientation, flipped */
  tft.fillScreen(TFT_BLACK);
  delay(500);
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);

  touch_init();

  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * screenHeight / 5);

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  /*Change the following line to your display resolution*/
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /*Initialize the (dummy) input device driver*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  
  Serial.println("Setup done");

  ui_init();
  
  while (1)
  {
    DHT22();
    Serial.println(Temp);
    Serial.println(Humi);
    printf("Temp=%.2f--Humi=%.2f%%RH \r\n", Temp,Humi);
    char DHT_buffer[6];
    int a = (int)Temp;
    int b = (int)Humi;
    if(a!=0 || b!=0){
      snprintf(DHT_buffer, sizeof(DHT_buffer), "%d", a);
      lv_label_set_text(ui_Label1, DHT_buffer);
      snprintf(DHT_buffer, sizeof(DHT_buffer), "%d", b);
      lv_label_set_text(ui_Label2, DHT_buffer);
    }
    if(led == 1)
      digitalWrite(19, HIGH);
    if(led == 0)
      digitalWrite(19, LOW);
    lv_timer_handler(); /* let the GUI do its work */
    delay(100);
  }
}
