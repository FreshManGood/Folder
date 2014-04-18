/*
 * Timer 1  используется как источник времени
 * в часах 6 штук газоразрядных индикаторов
 *
 * динамическая индикация организована на таймере Т2
 * данный таймер работает в режиме Fast PWM с частотой .......
 * в сравнении мы выводим нужную нам цифру на К155ИД1 и зажигаем анод требуемой лампы
 * в переполнении мы выключаем выходы К155ИД1 и тушим все лампы
 * изменяя значение которое мы заносим в OCR2 (регистр сравнения таймера Т2) мы изменяем время
 * срабатывания прерывания "по совпадению" и тем самым регулируем яркость соответствующей лампы
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define AnodRazryda5 (PORTD|=(1<<3)) // анаод газоразрядного индикатора, десятки часов
#define AnodRazryda4 (PORTD|=(1<<4)) // анаод газоразрядного индикатора, единици часов
#define AnodRazryda3 (PORTB|=(1<<6)) // анаод газоразрядного индикатора, десятки минут
#define AnodRazryda2 (PORTB|=(1<<7)) // анаод газоразрядного индикатора, еденици минут
#define AnodRazryda1 (PORTB|=(1<<4)) // анаод газоразрядного индикатора, десятки секунд
#define AnodRazryda0 (PORTB|=(1<<5)) // анаод газоразрядного индикатора, еденици секунд

#define TurnOff_Nixie() {PORTB=0x0A, PORTD&= ~(1<<4 | 1<<3);} // закрываем анодные ключи ламп и выключаем выходы
                                                              // дешифратора К155ИД1 подавая на его вход
                                                              // код "А" (шестнадцатеричная система)

//#define Input_Decoder(x) {PORTB = (PORTB & 0xF6) | ((x)&1) | (((x)&2)<<2), PORTC = (PORTC & 0xFD) | (((x)&4)<<1), PORTD = (PORTD & 0xFD) | (((x)&8)<<1);}
#define Input_Decoder(x) {PORTB = (PORTB & 0xF6) | ((x)&1) | (((x)&2)<<2);\
	                      PORTC = (PORTC & 0xFD) | (((x)&4)>>1);\
	                      PORTD = (PORTD & 0xFD) | (((x)&8)>>2);}

#define button_1 (PIND&(1<<5))
#define button_2 (PIND&(1<<6))
#define button_3 (PIND&(1<<7))

#define drebezg 10
#define ChangeOfNumbers 300
//#define stop 700
#define time_blink 240

//uint8_t mode;
unsigned char mode=2;
unsigned char second;
unsigned char minute;
unsigned char hour;
unsigned char minuteAlarm;
unsigned char HourAlarm;

volatile unsigned char flag;
volatile unsigned char flg;
volatile unsigned char metka;
volatile unsigned char number;
volatile unsigned char flagSS;
unsigned int counter;
unsigned int cnt_timer;
volatile unsigned char var;
volatile unsigned char t=2;
volatile unsigned char t0;
volatile unsigned char t1;
//volatile unsigned char iocr=25;
//volatile unsigned char iocr1=50;
volatile unsigned char iocr=0;
volatile unsigned char iocr1=13;
volatile unsigned char fl=0;
volatile unsigned char fl1=0;
volatile unsigned char flag1=0;
volatile unsigned char fll=0;

unsigned char fr[6];  // какой разряд изменил значение
unsigned char Screen[6]; // массив экрана, сюда мы храним старое значение времени
unsigned char TimeScreen[6];  // массив временного экрана, сюда мы пишем "новоиспеченную" цифру
unsigned char BrightnessOldNumber[6]={10,10,10,10,10,10};// буфер временного хранения яркостей для каждого разряда СТАРОЙ цифры
unsigned char BrightnessNewNumber[6]={250,250,250,250,250,250};// буфер временного хранения яркостей для каждого разряда НОВОЙ цифры


unsigned char psc[26]={10,30,50,70,90,110,130,150,170,190,210,230,250, // в данном массиве хранятся значения для регистра OCR2
		               250,230,210,190,170,150,130,110,90,70,50,30,10};// если  OCR2=10 то цифра будет светить по максимуму
                                                                       // если  OCR2=250 то цифра будет светить по минимуму

//uint8_t *br_0[4] = {&BrightnessOldNumber[2], &BrightnessOldNumber[3], &BrightnessNewNumber[0], &BrightnessNewNumber[1]}; // для fl1 == 0;
//uint8_t *br_1[4] = {&BrightnessNewNumber[2], &BrightnessNewNumber[3], &BrightnessOldNumber[0], &BrightnessOldNumber[1]}; // для fl1 == 1;


// Timer 1 output compare A interrupt service routine
ISR (TIMER1_COMPA_vect)  // interrupt service routine (ISR)
{
	if(++second>=60)
	{
		second=0;
		minute++;
	}

	if(minute>=60)
	{
		minute=0;
		hour++;
	}

	if(hour>=24) hour=0;

	flag=1;
	metka=1;
	cnt_timer=0;

}

/////////////////
//// ЭФФЕКТЫ ////
/////////////////

// Timer 2 overflow interrupt service routine
ISR (TIMER2_OVF_vect)                             // данное прерывание происходит с периодичностью раз в 2мс
{
	static unsigned char i;

	TurnOff_Nixie();

	if (flagSS==1) //если после смены времени выполнилась ф-ция SetScreen
    {
    	switch (mode)
    	{
//---------------------------------- эффект плавной смены -------------------------------------------------------
    		case 0: if (++counter==15)                  // если прошло 30мс
					{
						counter=0;
						iocr++;
						for (i=0; i<=5; i++)
						{
							if (fr[i]==1)               // смотрим в каком разряде изменилась цифра
							{
								BrightnessOldNumber[i]=psc[iocr];    // изменяем яркость от max до minute
							}

							if (iocr==13)               // если яркость достигла minute и цифра потухла
							{
								Screen[i]=TimeScreen[i];// выводим новое значение
							}
						}

						if (iocr>=25)
						{
							flagSS=0;
							iocr=0;
						}
					 } break;

//------------------------------------- эффект перебора -----------------------------------------------------
    		case 1: if (++counter==28)                      // если прошло 76 мс (период смены перебираемой цифры)
					{
						counter=0;
						number++;                           // меняем переборную цифру

						for (i=0; i<=5; i++)
						{
							if (fr[i]==1)                   // смотрим в каком разряде произошло изменение цифры
							{
								if (number<=9)
								{
									Screen[i]=number;       // там где было изменение начинаем делать перебор
								}
								else
								{
									Screen[i]=TimeScreen[i];// выводим новое значение
									fr[i]=0;                // сбрасываем фл
									number=0;
									flagSS=0;
								}
							}
						}
					 } break;

//--------------------------------- эффект перелива -----------------------------------------------------------

			case 2: if (++counter==15)
					{
						cnt_timer=cnt_timer+(counter*2);
						counter=0;

						if (fl1==0)
						{
							iocr1++;
							for (i=0; i<=5; i++)
							{
								if (fr[i]==1)
								{
									BrightnessNewNumber[i]=psc[iocr1];
								}
							}
						}


						if (fl1==1)
						{
							iocr++;
							for (i=0; i<=5; i++)
							{
								if (fr[i]==1)
								{
									BrightnessOldNumber[i]=psc[iocr];
								}

							}
						}

						fl1^=1;

						if (iocr>=12)
						{
							//fl=0;
							fl1=0;
							iocr=0;
							iocr1=13;
							flagSS=0;

							for (i=0; i<=5; i++)
							{
								Screen[i]=TimeScreen[i];
								BrightnessOldNumber[i]=10;
								BrightnessNewNumber[i]=250;
							}
						}
					} break;
			default: break;
    	}
    }

	if (mode!=2) // если выбран ЛЮБОЙ режим КРОМЕ перелива
	{
		OCR2=BrightnessOldNumber[t];
		if (++t>=6) t=0;
		if (++var>=6) var=0;

	}
	else       // при режиме ПЕРЕЛИВ цифр
	{
		if (fl==0)
		{
			if (++t0>=6) t0=0;
			OCR2=BrightnessNewNumber[t0];
		}
		if (fl==1)
		{
			if (++t1>=6) t1=0;
			OCR2=BrightnessOldNumber[t1];

			if(++var>=6) var=0;
		}

		fl^=1;
	}

}

/////////////////////
////  ИНДИКАЦИЯ  ////
/////////////////////

//Timer 2 output compare interrupt service routine
ISR (TIMER2_COMP_vect)
{
	if (mode!=2) // если НЕ выбран ПЕРЕЛИВ цифр
	{
		//PORTB=Screen[var];
		Input_Decoder(Screen[var]); // посылаем число на вход К155ИД1
	}
	else
	{
		//PORTB=TimeScreen[var];
		//Input_Decoder(Screen[var]); // посылаем число на вход К155ИД1
		if (fl==0) Input_Decoder(TimeScreen[var]);// если выводится НОВАЯ цифра
		if (fl==1) Input_Decoder(Screen[var]);    // если выводится СТАРАЯ цифра
	}

	switch (var)
	{
		case 0: AnodRazryda0; break; // зажигаем лампу десятков часов
		case 1: AnodRazryda1; break; // зажигаем лампу единиц часов
		case 2: AnodRazryda2; break; // зажигаем лампу десятков минут
		case 3: AnodRazryda3; break; // зажигаем лампу единиц минут
		case 4: AnodRazryda4; break; // зажигаем лампу десятков секунд
		case 5: AnodRazryda5; break; // зажигаем лампу единиц секунд

		default:break;
	}

}
//***********************************************************************************
void SetScreen (char hour, char minute, char second)
{
	static unsigned char i;

//	if (mode!=3) // если НЕ выбран обычный режим смены цифр
//	{
//		TimeScreen[0]=hour/10;
//		TimeScreen[1]=hour%10;
//		TimeScreen[2]=minute/10;
//		TimeScreen[3]=minute%10;
//		TimeScreen[4]=second/10;
//		TimeScreen[5]=second%10;
//
//
//		for (i=0; i<=5; i++)
//		{
//			if (TimeScreen[i]!=Screen[i]) //узнаём в каком разряде изменялась цифра
//			{
//				fr[i]=1;
//				//BrightnessOldNumber[i]=10;
//				//BrightnessNewNumber[i]=250;
//			}
//			else
//			{
//				fr[i]=0;
//			}
//		}
//	}
//	else
//	{
//		Screen[0]=hour/10;
//		Screen[1]=hour%10;
//		Screen[2]=minute/10;
//		Screen[3]=minute%10;
//		Screen[4]=second/10;
//		Screen[5]=second%10;
//	}
	if ((mode==2)&&(BrightnessNewNumber[0]=250))
	{
		TimeScreen[0]=hour/10;
		TimeScreen[1]=hour%10;
		TimeScreen[2]=minute/10;
		TimeScreen[3]=minute%10;
		TimeScreen[4]=second/10;
		TimeScreen[5]=second%10;

		for (i=0; i<=5; i++)
		{
			if (TimeScreen[i]!=Screen[i]) //узнаём в каком разряде изменялась цифра
			{
				fr[i]=1;
				//BrightnessOldNumber[i]=10;
				//BrightnessNewNumber[i]=250;
			}
			else
			{
				fr[i]=0;
			}
		}
	}
	else if ((mode==2)&&(BrightnessOldNumber[0]=250))
		 {
			Screen[0]=hour/10;
			Screen[1]=hour%10;
			Screen[2]=minute/10;
			Screen[3]=minute%10;
			Screen[4]=second/10;
			Screen[5]=second%10;

			for (i=0; i<=5; i++)
					{
						if (Screen[i]!=TimeScreen[i]) //узнаём в каком разряде изменялась цифра
						{
							fr[i]=1;
							//BrightnessOldNumber[i]=10;
							//BrightnessNewNumber[i]=250;
						}
						else
						{
							fr[i]=0;
						}
					}
		 }

	flag=0;  //сбрасываем флаг чтения
    flagSS=1;// флаг выполнения ф-ции SetScreen
}

//***********************************************************************************

int main(void)
{
DDRB =0b11111111;//The Port B Data Direction Registe   0-пин порта на вход               1-пин порта на выход
PORTB=0b00000000;//The Port B Data Register            1/0 - on/off pull-up resistor

DDRC= 0b00000011;
PORTC=0b00000000;

DDRD =0b00011011;
PORTD=0b11100000;


// Timer/Counter 1 initialization
TCCR1A=0x00;
TCCR1B|= 1<<CS12 | // clk/256 - делим основную частоту МК на 256 (предделидель для таймера), Clock value: 31,250 kHz
		 1<<WGM12;  // Mode: CTC top=OCR1A (сброс при совпадении с OCR1A)
TCNT1H=0x00;
TCNT1L=0x00;
ICR1H=0x00;
ICR1L=0x00;
OCR1AH=0x7A;
OCR1AL=0x11;
OCR1BH=0x00;
OCR1BL=0x00;

// Timer/Counter 2 initialization
ASSR=0x00;
TCCR2|= 1 << WGM20 | 1 << WGM21 | // Fast PWM mode
	    1 << CS22;                // clk/256 - делим основную частоту МК на 256 (предделидель для таймера)
TCNT2=0x00;
OCR2=0x00;

// Timer(s)/Counter(s) Interrupt(s) initialization
TIMSK|= 1<<OCIE2 | // флаг разрешения прерывания по событию "совпадение" таймера/счетчика Т2
		1<<TOIE2 | // флаг разрешения прерывания по переполнению таймера/счетчика Т2
		1<<OCIE1A; // флаг разрешения прерывания по событию "сопадение А" таймера/счетчика Т2

sei (); // Global enable interrupts

 while (1)
 {


		if (flag==1)                                          // если прошла 1с
		{
			if (++metka>=9)
     		switch (mode)                                     // выбираем режим отображения времени
         	{
                case 0: SetScreen(hour,minute,second); break; // плавная смена цифр (ч:м:c)
             	case 1: SetScreen(hour,minute,second); break; // перебор цифр(ч:м:c)
             	case 2: SetScreen(hour,minute,second); break; // перелив цифр (ч:м:c)
             	case 3: SetScreen(hour,minute,second); break; // обычная смена цифр (ч:м:c)

            	default: break;
         	}
    	}



	}

}



















