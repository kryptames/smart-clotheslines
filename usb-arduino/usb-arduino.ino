#include <usbdrv.h>
#include <DHT.h>

DHT dht(PIN_PC5,DHT11);

#include <avr/io.h>
#include <util/delay.h>

#define RED    PC0
#define YELLOW PC1
#define GREEN  PC2

#define ON     1
#define OFF    0
#define REVERSE 2

#define IS_SWITCH_PRESSED() ( (PINC & (1<<PC3)) == 0)

#define RQ_SET_LED         0
#define RQ_SET_LED_VALUE   1
#define RQ_GET_SWITCH      2
#define RQ_GET_LIGHT       3
#define RQ_GET_TEMP        4
#define RQ_GET_HUMI        5
#define RQ_GET_CHK         6

int16_t chk;
uint16_t temp;
uint16_t humi;
uint8_t count = 0;
uint16_t result;
uint8_t state = 0;

void init_peripheral()
{
    DDRC |= (1<<PC0) | (1<<PC1) | (1<<PC2); //PC0-PC2 is output
    DDRC &= ~(1<<PC3) & ~(1<<PC4) & ~(1<<PC5); // PC3-PC5 is input PC3 SWITCH PC4 LDR PC5 DHT
    PORTC |= (1<<PC3);
    PORTC &= ~(1<<PC0) & ~(1<<PC1) & ~(1<<PC2) & ~(1<<PC4) & ~(1<<PC5);
    DDRD |= (1<<PD0) | (1<<PD1) | (1<<PD3) | (1<<PD5); //PD0-PD1 PD5 is output  PD0-PD1 MOTOR PD5 FAN
    PORTD |= (1<<PD3);
}
void set_led(uint8_t pin,uint8_t state)
{
    if(state == ON){
        PORTC |= (1<<pin);
    }else{
        PORTC &= ~(1<<pin);
    }
}

void set_led_value(uint8_t value)
{
    PORTC &= ~(0b111);
    PORTC |= value & 0b111;
}
uint16_t read_adc(uint8_t channel)
{
    ADMUX = (0<<REFS1)|(1<<REFS0)|(0<<ADLAR)|(channel &0b1111);
    ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADSC);
    while ((ADCSRA & (1<<ADSC)));
    return ADCL + ADCH*256;

}
void set_fan(uint8_t state){
    if(state == ON){
        PORTD |= (1<<PD5);
    }else{
        PORTD &= ~(1<<PD5);
    }
}
void set_motor(uint8_t state){
  
    if(state == ON){
        PORTD |= (1<<PD0);
        _delay_ms(1000);
        PORTD &= ~(1<<PD0);
    }else if (state == REVERSE){
        PORTD |= (1<<PD1);
        _delay_ms(1000);
        PORTD &= ~(1<<PD1);
    }
}


usbMsgLen_t usbFunctionSetup(uint8_t data[8])
{
    usbRequest_t *rq = (void *)data;

    /* declared as static so they stay valid when usbFunctionSetup returns */
    static uint8_t switch_state;  
    static uint16_t light;
    

    if (rq->bRequest == RQ_SET_LED)
    {
        uint8_t led_val = rq->wValue.bytes[0];
        uint8_t led_no  = rq->wIndex.bytes[0];
        set_led(led_no, led_val);
        return 0;
    }

    else if (rq->bRequest == RQ_GET_SWITCH)
    {
        switch_state = IS_SWITCH_PRESSED();

        /* point usbMsgPtr to the data to be returned to host */
        usbMsgPtr = &switch_state;

        /* return the number of bytes of data to be returned to host */
        return 1;
    }
    else if (rq->bRequest == RQ_SET_LED_VALUE)
    {
        uint8_t led_val = rq->wValue.bytes[0];
        set_led_value(led_val);
        return 0;
         
    }
    else if (rq->bRequest == RQ_GET_LIGHT)
    {
        light = read_adc(4);
        usbMsgPtr = &light;
        return sizeof(light);
    }
    else if (rq->bRequest == RQ_GET_TEMP)
    {
        
        usbMsgPtr = &temp;
        return sizeof(temp);
    }
    else if (rq->bRequest == RQ_GET_HUMI)
    {
        usbMsgPtr = &humi;
        return sizeof(humi);
    }
//    else if (rq->bRequest == RQ_GET_CHK)
//    {
//        usbMsgPtr = &chk;
//        return sizeof(chk);
//    }

    /* default for not implemented requests: return no data back to host */
    return 0;
}

void setup() {
  init_peripheral();
  usbInit();
  usbDeviceDisconnect();
  delay(300);
  usbDeviceConnect();
  dht.begin();
}

void loop() {
  usbPoll();
  temp = dht.readTemperature();
  humi = dht.readHumidity();
  result = read_adc(PC4);
  if (result >2*1023/3)
  {
      set_led(GREEN,ON);
      set_led(YELLOW,OFF);
      set_led(RED,OFF);
  }
  else if ((result > 1023/3) && (result < 2*1023/3))
  {
      set_led(GREEN,OFF);
      set_led(YELLOW,ON);
      set_led(RED,OFF);
  }
  else
  {
      set_led(GREEN,OFF);
      set_led(YELLOW,OFF);
      set_led(RED,ON);
  }
  if (count){
    set_fan(ON);
    state = 1;
  }
  else{
    set_fan(OFF);
    state = 0;
  }
  if (temp < 20 and humi > 50 and result < 2*1023/3){
    if (count==0){
      set_motor(ON);
    }
    count = 1;
  }
  else if (temp > 30 and humi < 20 and result > 2*1023/3){
    if (count==1){
      set_motor(REVERSE);
    }
    count = 0;
  }
}
  

