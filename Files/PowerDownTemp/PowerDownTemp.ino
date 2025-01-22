#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
uint16_t val = 100;
int prevval=0;
#include <LiquidCrystal.h>
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define wdt_reset() __asm__ __volatile__ ("wdr");
#define SLEEP_MODE_PWR_DOWN (0x02 << 1)
#define set_sleep_mode(mode) \
  do \
  { \
    SMCR = ((SMCR & ~((1 << SM0) | (1 << SM1) | (1 << SM2))) | (mode));; \
  } while(0);


#define sleep_enable() \
  do \
  { \
    SMCR |= (uint8_t)(1<<SE); \
  } while (0);

#define sleep_cpu() \
  do \
  { \
    __asm__ __volatile__("sleep" \
                         "\n\t" ::);  \
  } while (0);


#define sleep_disable() \
  do \
  { \
    SMCR &= (uint8_t)(~(1<<SE)); \
  } while (0);

#define sleep_mode() \
  do \
  { \
    sleep_enable(); \
    sleep_cpu(); \
    sleep_disable(); \
  } while (0);\
{}


void setupADC()
{
  ADMUX = 0b01000011;
  ADCSRA = (1<<ADEN) | (1<<ADIE)| (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2);
  DIDR0 = (1<<ADC3D);
}

void startConversion()
{
  ADCSRA |= (1<<ADSC);
}

void watchdogSetup(void)
{
  cli();
  wdt_reset();
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  WDTCSR = (1<<WDIE) | (0<<WDE) | (1<<WDP3) | (1<<WDP0);  // 8s / interrupt, no system reset
  sei();
}

int main()
{
  setupADC();
  lcd.begin(16, 2);
  watchdogSetup();
  while(1)
  {    
    sei();
    startConversion(); 
    
    wdt_reset();
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_mode();  
  }
}


ISR(WDT_vect)
{
  lcd.setCursor(0,0);
}


ISR(ADC_vect)
{
  val=ADC;
  int msg = map(val,0,306,0,150);
  if(val!=prevval) //refresh value
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(msg);
    lcd.print(" deg C");
  }
  prevval = val;
}
