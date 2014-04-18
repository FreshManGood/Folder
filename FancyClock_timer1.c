/*
 * Timer 1  ������������ ��� �������� �������
 * � ����� 6 ���� ������������� �����������
 *
 * ������������ ��������� ������������ �� ������� �2
 * ������ ������ �������� � ������ Fast PWM � �������� .......
 * � ��������� �� ������� ������ ��� ����� �� �155��1 � �������� ���� ��������� �����
 * � ������������ �� ��������� ������ �155��1 � ����� ��� �����
 * ������� �������� ������� �� ������� � OCR2 (������� ��������� ������� �2) �� �������� �����
 * ������������ ���������� "�� ����������" � ��� ����� ���������� ������� ��������������� �����
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define AnodRazryda5 (PORTD|=(1<<3)) // ����� �������������� ����������, ������� �����
#define AnodRazryda4 (PORTD|=(1<<4)) // ����� �������������� ����������, ������� �����
#define AnodRazryda3 (PORTB|=(1<<6)) // ����� �������������� ����������, ������� �����
#define AnodRazryda2 (PORTB|=(1<<7)) // ����� �������������� ����������, ������� �����
#define AnodRazryda1 (PORTB|=(1<<4)) // ����� �������������� ����������, ������� ������
#define AnodRazryda0 (PORTB|=(1<<5)) // ����� �������������� ����������, ������� ������

#define TurnOff_Nixie() {PORTB=0x0A, PORTD&= ~(1<<4 | 1<<3);} // ��������� ������� ����� ���� � ��������� ������
                                                              // ����������� �155��1 ������� �� ��� ����
                                                              // ��� "�" (����������������� �������)

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

unsigned char fr[6];  // ����� ������ ������� ��������
unsigned char Screen[6]; // ������ ������, ���� �� ������ ������ �������� �������
unsigned char TimeScreen[6];  // ������ ���������� ������, ���� �� ����� "��������������" �����
unsigned char BrightnessOldNumber[6]={10,10,10,10,10,10};// ����� ���������� �������� �������� ��� ������� ������� ������ �����
unsigned char BrightnessNewNumber[6]={250,250,250,250,250,250};// ����� ���������� �������� �������� ��� ������� ������� ����� �����


unsigned char psc[26]={10,30,50,70,90,110,130,150,170,190,210,230,250, // � ������ ������� �������� �������� ��� �������� OCR2
		               250,230,210,190,170,150,130,110,90,70,50,30,10};// ����  OCR2=10 �� ����� ����� ������� �� ���������
                                                                       // ����  OCR2=250 �� ����� ����� ������� �� ��������

//uint8_t *br_0[4] = {&BrightnessOldNumber[2], &BrightnessOldNumber[3], &BrightnessNewNumber[0], &BrightnessNewNumber[1]}; // ��� fl1 == 0;
//uint8_t *br_1[4] = {&BrightnessNewNumber[2], &BrightnessNewNumber[3], &BrightnessOldNumber[0], &BrightnessOldNumber[1]}; // ��� fl1 == 1;


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
//// ������� ////
/////////////////

// Timer 2 overflow interrupt service routine
ISR (TIMER2_OVF_vect)                             // ������ ���������� ���������� � �������������� ��� � 2��
{
	static unsigned char i;

	TurnOff_Nixie();

	if (flagSS==1) //���� ����� ����� ������� ����������� �-��� SetScreen
    {
    	switch (mode)
    	{
//---------------------------------- ������ ������� ����� -------------------------------------------------------
    		case 0: if (++counter==15)                  // ���� ������ 30��
					{
						counter=0;
						iocr++;
						for (i=0; i<=5; i++)
						{
							if (fr[i]==1)               // ������� � ����� ������� ���������� �����
							{
								BrightnessOldNumber[i]=psc[iocr];    // �������� ������� �� max �� minute
							}

							if (iocr==13)               // ���� ������� �������� minute � ����� �������
							{
								Screen[i]=TimeScreen[i];// ������� ����� ��������
							}
						}

						if (iocr>=25)
						{
							flagSS=0;
							iocr=0;
						}
					 } break;

//------------------------------------- ������ �������� -----------------------------------------------------
    		case 1: if (++counter==28)                      // ���� ������ 76 �� (������ ����� ������������ �����)
					{
						counter=0;
						number++;                           // ������ ���������� �����

						for (i=0; i<=5; i++)
						{
							if (fr[i]==1)                   // ������� � ����� ������� ��������� ��������� �����
							{
								if (number<=9)
								{
									Screen[i]=number;       // ��� ��� ���� ��������� �������� ������ �������
								}
								else
								{
									Screen[i]=TimeScreen[i];// ������� ����� ��������
									fr[i]=0;                // ���������� ��
									number=0;
									flagSS=0;
								}
							}
						}
					 } break;

//--------------------------------- ������ �������� -----------------------------------------------------------

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

	if (mode!=2) // ���� ������ ����� ����� ����� ��������
	{
		OCR2=BrightnessOldNumber[t];
		if (++t>=6) t=0;
		if (++var>=6) var=0;

	}
	else       // ��� ������ ������� ����
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
////  ���������  ////
/////////////////////

//Timer 2 output compare interrupt service routine
ISR (TIMER2_COMP_vect)
{
	if (mode!=2) // ���� �� ������ ������� ����
	{
		//PORTB=Screen[var];
		Input_Decoder(Screen[var]); // �������� ����� �� ���� �155��1
	}
	else
	{
		//PORTB=TimeScreen[var];
		//Input_Decoder(Screen[var]); // �������� ����� �� ���� �155��1
		if (fl==0) Input_Decoder(TimeScreen[var]);// ���� ��������� ����� �����
		if (fl==1) Input_Decoder(Screen[var]);    // ���� ��������� ������ �����
	}

	switch (var)
	{
		case 0: AnodRazryda0; break; // �������� ����� �������� �����
		case 1: AnodRazryda1; break; // �������� ����� ������ �����
		case 2: AnodRazryda2; break; // �������� ����� �������� �����
		case 3: AnodRazryda3; break; // �������� ����� ������ �����
		case 4: AnodRazryda4; break; // �������� ����� �������� ������
		case 5: AnodRazryda5; break; // �������� ����� ������ ������

		default:break;
	}

}
//***********************************************************************************
void SetScreen (char hour, char minute, char second)
{
	static unsigned char i;

//	if (mode!=3) // ���� �� ������ ������� ����� ����� ����
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
//			if (TimeScreen[i]!=Screen[i]) //����� � ����� ������� ���������� �����
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
			if (TimeScreen[i]!=Screen[i]) //����� � ����� ������� ���������� �����
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
						if (Screen[i]!=TimeScreen[i]) //����� � ����� ������� ���������� �����
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

	flag=0;  //���������� ���� ������
    flagSS=1;// ���� ���������� �-��� SetScreen
}

//***********************************************************************************

int main(void)
{
DDRB =0b11111111;//The Port B Data Direction Registe   0-��� ����� �� ����               1-��� ����� �� �����
PORTB=0b00000000;//The Port B Data Register            1/0 - on/off pull-up resistor

DDRC= 0b00000011;
PORTC=0b00000000;

DDRD =0b00011011;
PORTD=0b11100000;


// Timer/Counter 1 initialization
TCCR1A=0x00;
TCCR1B|= 1<<CS12 | // clk/256 - ����� �������� ������� �� �� 256 (������������ ��� �������), Clock value: 31,250 kHz
		 1<<WGM12;  // Mode: CTC top=OCR1A (����� ��� ���������� � OCR1A)
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
	    1 << CS22;                // clk/256 - ����� �������� ������� �� �� 256 (������������ ��� �������)
TCNT2=0x00;
OCR2=0x00;

// Timer(s)/Counter(s) Interrupt(s) initialization
TIMSK|= 1<<OCIE2 | // ���� ���������� ���������� �� ������� "����������" �������/�������� �2
		1<<TOIE2 | // ���� ���������� ���������� �� ������������ �������/�������� �2
		1<<OCIE1A; // ���� ���������� ���������� �� ������� "��������� �" �������/�������� �2

sei (); // Global enable interrupts

 while (1)
 {


		if (flag==1)                                          // ���� ������ 1�
		{
			if (++metka>=9)
     		switch (mode)                                     // �������� ����� ����������� �������
         	{
                case 0: SetScreen(hour,minute,second); break; // ������� ����� ���� (�:�:c)
             	case 1: SetScreen(hour,minute,second); break; // ������� ����(�:�:c)
             	case 2: SetScreen(hour,minute,second); break; // ������� ���� (�:�:c)
             	case 3: SetScreen(hour,minute,second); break; // ������� ����� ���� (�:�:c)

            	default: break;
         	}
    	}



	}

}



















