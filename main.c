
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

#define CE      BIT0    // P6.0 chip enable
#define RESET   BIT6    // P6.6 reset
#define DC      BIT5    // P6.7 data/control (D/C)

// size of displayESP32C3Family’sOscillator
#define GLCD_WIDTH  84
#define GLCD_LENGTH 48

// function prototypes
void GLCD_clear(void);
void GLCD_setCursor(unsigned char x, unsigned char y);
void GLCD_init(void);
void GLCD_command_write(unsigned char data);
void GLCD_data_write(unsigned char data);
void SPI_write(unsigned char data);
void SPI_init(void);
//void UART_init(void);
void GLCD_putchar(int c);
void Display_Time(void);

void LED_setup();
void RTC_setup();
void RTC_set_alarm();
uint8_t Keypad_Decode();

// Port 3 Pins (Data Available signal)
#define DA_BIT BIT6

// Port 4 Pins (sound signal)
#define S_BIT BIT2

// Port 4 Pins (Keypad Data Pins)
#define DATABITS (BIT0|BIT1|BIT2|BIT3)


int getCharInt(char c);
void GLCD_write_string(char str[14]);

//setup
static volatile RTC_C_Calendar new_time;
const RTC_C_Calendar initial_time = {50, 9, 2, 0, 25, 3, 2023}; // sec, min, hour, date, month, year

//settings


//state
int light_on = 0;


// font table
const char font_table[][6] = {
        {0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00}, // '0'
        {0x00, 0x00, 0x7F, 0x00, 0x00, 0x00}, // '1'
        {0x62, 0x51, 0x49, 0x49, 0x46, 0x00}, // '2'
        {0x22, 0x41, 0x49, 0x49, 0x36, 0x00}, // '3'
        {0x0F, 0x08, 0x08, 0x7F, 0x08, 0x00}, // '4'
        {0x2F, 0x45, 0x45, 0x45, 0x39, 0x00}, // '5'
        {0x3E, 0x49, 0x49, 0x49, 0x32, 0x00}, // '6'
        {0x41, 0x21, 0x11, 0x09, 0x07, 0x00}, // '7'
        {0x36, 0x49, 0x49, 0x49, 0x36, 0x00}, // '8'
        {0x46, 0x29, 0x19, 0x09, 0x06, 0x00}, // '9'
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // space
        {0x7E, 0x11, 0x11, 0x11, 0x7E, 0x00}, // 'A'
        {0x7F, 0x49, 0x49, 0x49, 0x36, 0x00}, // 'B'
        {0x3E, 0x41, 0x41, 0x41, 0x22, 0x00}, // 'C'
        {0x7F, 0x41, 0x41, 0x41, 0x3E, 0x00}, // 'D'
        {0x7F, 0x49, 0x49, 0x49, 0x49, 0x00}, // 'E'
        {0x7F, 0x09, 0x09, 0x09, 0x09, 0x00}, // 'F'
        {0x3E, 0x41, 0x41, 0x49, 0x3A, 0x00}, // 'G'
        {0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00}, // 'H'
        {0x41, 0x41, 0x7F, 0x41, 0x41, 0x00}, // 'I'
        {0x21, 0x41, 0x41, 0x41, 0x3F, 0x00}, // 'J'
        {0x7F, 0x08, 0x14, 0x22, 0x41, 0x00}, // 'K'
        {0x7F, 0x40, 0x40, 0x40, 0x40, 0x00}, // 'L'
        {0x7F, 0x02, 0x04, 0x02, 0x7F, 0x00}, // 'M'
        {0x7F, 0x04, 0x08, 0x10, 0x7F, 0x00}, // 'N'
        {0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00}, // 'O'
        {0x7F, 0x09, 0x09, 0x09, 0x06, 0x00}, // 'P'
        {0x3E, 0x41, 0x49, 0x51, 0x3E, 0x00}, // 'Q'
        {0x7F, 0x09, 0x19, 0x29, 0x46, 0x00}, // 'R'
        {0x26, 0x49, 0x49, 0x49, 0x32, 0x00}, // 'S'
        {0x01, 0x01, 0x7F, 0x01, 0x01, 0x00}, // 'T'
        {0x3f, 0x40, 0x40, 0x40, 0x3f, 0x00}, // 'U'
        {0x0f, 0x30, 0x40, 0x30, 0x0f, 0x00}, // 'V'
        {0x7F, 0x20, 0x10, 0x20, 0x7F, 0x00}, // 'W'
        {0x63, 0x14, 0x08, 0x14, 0x63, 0x00}, // 'X'
        {0x03, 0x04, 0x78, 0x04, 0x03, 0x00}, // 'Y'
        {0x61, 0x51, 0x49, 0x45, 0x43, 0x00}, // 'Z'
        {0x08, 0x08, 0x08, 0x08, 0x08, 0x08}, // '-'
        {0x00, 0x00, 0x66, 0x66, 0x00, 0x00}, // ':'
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // ''
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // ''
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // ''
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // ''
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // ''
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // ''
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // ''
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // ''

};

/**
 * main.c
 */
void main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer

    // Initialize GLCD
    GLCD_init();
    GLCD_clear();

    GLCD_putchar(4);

    LED_setup();
    RTC_setup();

    // Configure Data Input Pins (P4.0, 4.1, 4.2, 4.3)
      P4->DIR &= ~DATABITS;          // set up data pins as inputs

    // Configure "Data Available" Signal Pin (P3.8)
      P3->DIR &= ~(DA_BIT | S_BIT);       // set up DA pin as input
      P3->REN |=  (DA_BIT);       // connect pull resistor to DA
      P3->OUT |=  (DA_BIT);       // configure resistor as pull-up
      P3->IFG &= ~(DA_BIT | S_BIT);       // clear interrupt flag for DA
      P3->IE  |=  (DA_BIT | S_BIT);       // enable pin-level interrupt for DA


      //NVIC->ISER[1] |= 0x00000040;  // enable Port 4 interrupts
      NVIC->ISER[1] |= 0x00000020;  // enable Port 3 interrupts




    MAP_Interrupt_enableMaster();

    RTC_set_alarm();

    GLCD_setCursor(1,1);
    GLCD_write_string("HI");

    //GLCD_write_string("F ");

    //UART_init();

    //new_time = RTC_C_getCalendarTime();
    //RTC_C_Calendar ts = RTC_C_getCalendarTime();

    while(1);
}

void Loud()
{
    GLCD_setCursor(1,1);
    GLCD_write_string("LOUD");
}

/***
* IRQ handler for Port 3
***/
void PORT3_IRQHandler(void)
{
    uint8_t key = 0;

    // if DA interrupt received
    if(P3->IFG & DA_BIT){

        // Clear interrupt flag for DA
        P3->IFG &= ~DA_BIT;

        // Read data
        key = P4->IN & DATABITS;   // determine which key was pressed

        if (key == 0x0F)
        {
            MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN5);
        }
    }
    if(P3->IFG & S_BIT){
        P3->IFG &= ~S_BIT;
        MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN5);
    }

}

void LED_setup()
{
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN5);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN5);
}

void RTC_setup()
{
    MAP_RTC_C_initCalendar(&initial_time, RTC_C_FORMAT_BINARY);

    MAP_RTC_C_setCalendarAlarm(0, 0, RTC_C_ALARMCONDITION_OFF, RTC_C_ALARMCONDITION_OFF);

    //MAP_RTC_C_setCalendarEvent(RTC_C_CALENDAREVENT_MINUTECHANGE);

    MAP_RTC_C_clearInterruptFlag(RTC_C_CLOCK_ALARM_INTERRUPT);

    MAP_RTC_C_enableInterrupt(RTC_C_CLOCK_ALARM_INTERRUPT | RTC_C_TIME_EVENT_INTERRUPT | RTC_C_CLOCK_READ_READY_INTERRUPT);

    MAP_RTC_C_startClock();

    MAP_Interrupt_enableInterrupt(INT_RTC_C);
}

void RTC_set_alarm()
{
    MAP_RTC_C_setCalendarAlarm(10, 2, RTC_C_ALARMCONDITION_OFF, RTC_C_ALARMCONDITION_OFF);
}

void Display_Time(void)
{
    GLCD_setCursor(0,0);
    //GLCD_clear();

    int minutes_seconds = RTC_C->TIM0;
    int days_hours = RTC_C->TIM1;

    int hours = (days_hours & 0x001F);
    int hoursH = hours/10;
    int hoursL = hours%10;

    int minutes = (minutes_seconds & 0x7F00) >> 8;
    int minutesH = minutes/10;
    int minutesL = minutes%10;

    int seconds = (minutes_seconds & 0x007F);
    int secondsH = seconds/10;
    int secondsL = seconds%10;

    GLCD_putchar(hoursH);
    GLCD_putchar(hoursL);
    GLCD_write_string(":");
    GLCD_putchar(minutesH);
    GLCD_putchar(minutesL);
    GLCD_write_string(":");
    GLCD_putchar(secondsH);
    GLCD_putchar(secondsL);
}

/* RTC ISR */
void RTC_C_IRQHandler(void)
{
    //GLCD_write_string("INR ");

    uint32_t status;

    status = MAP_RTC_C_getEnabledInterruptStatus();
    MAP_RTC_C_clearInterruptFlag(status);

    if (status & RTC_C_CLOCK_ALARM_INTERRUPT)
    {
        //GLCD_write_string("LIGHT ");
        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN5);
    }
/*
    if (status & RTC_C_TIME_EVENT_INTERRUPT)
    {
        /GLCD_write_string("TIME EVENT ");
        GLCD_clear();

        //new_time.minutes = (RTC_C->TIM0 & (0x0F00 | 0x7000))>> 8;

        int minutes_seconds = RTC_C->TIM0;

        int minutes = (minutes_seconds & 0x7F00) >> 8;
        int minutesH = minutes/10;
        int minutesL = minutes%10;

        int seconds = (minutes_seconds & 0x007F);
        int secondsH = seconds/10;
        int secondsL = seconds%10;

        GLCD_putchar(minutesH);
        GLCD_putchar(minutesL);
        GLCD_write_string(":");
        GLCD_putchar(secondsH);
        GLCD_putchar(secondsL);
    }*/

    if (status & RTC_C_CLOCK_READ_READY_INTERRUPT)
    {
        Display_Time();
    }
}

void GLCD_clear(void){
    int i;
    for(i = 0; i < (GLCD_WIDTH * GLCD_LENGTH / 8); i++){
        GLCD_data_write(0x00);
    }
    GLCD_setCursor(0,0);    // return to top left
}

void GLCD_setCursor(unsigned char x, unsigned char y){
    GLCD_command_write(0x80 | x);   // column
    GLCD_command_write(0x40 | y);   // bank
}

void GLCD_init(void){

    // Initialize SPI
    SPI_init();

    P6->DIR |= (CE | RESET | DC);   // P6.0, P6.6, P6.7
    P6->OUT |= CE;                  // CE idle high
    P6->OUT &= ~RESET;              // assert reset
    P6->OUT |= RESET;               // de-assert reset

    // Configure the screen
    GLCD_command_write(0x21);       // set extended command mode
    GLCD_command_write(0xB8);       // set LCD Vop for contrast
    GLCD_command_write(0x04);       // set temp coefficient
    GLCD_command_write(0x14);       // set LCD bias mode
    GLCD_command_write(0x20);       // set normal command mode
    GLCD_command_write(0x0C);       // set display mode normal


}

void GLCD_command_write(unsigned char data){
    P6->OUT &= ~DC;     // Enter "command" mode
    SPI_write(data);    // send data via SPI
}

void GLCD_data_write(unsigned char data){
    P6->OUT |= DC;      // Enter "data" mode
    SPI_write(data);    // send data via SPI
}

void SPI_write(unsigned char data){
    P6->OUT &= ~CE;     // assert CE
    EUSCI_B0->TXBUF = data;
    while(EUSCI_B0->STATW & BIT0);
    P6->OUT |= CE;      // de-assert CE
}

void SPI_init(void){
    EUSCI_B0->CTLW0 = 0x0001;   // put UCB0 in reset mode
    EUSCI_B0->CTLW0 = 0x69C1;   // MSB first, Master, SPI, SMCLK
    EUSCI_B0->BRW = 3;          // 3 MHz / 3 = 1 million baud
    EUSCI_B0->CTLW0 &= ~BIT0;   // enable UCB0 after config

    P1->SEL0 |=  (BIT5 | BIT6); // P1.5 and P1.6 for UCB0
    P1->SEL1 &= ~(BIT5 | BIT6);
}

/*
void UART_init(void){

    //set up idDA receive on P3.2
    //9600 bps


    EUSCI_A2->CTLW0 = 0x0001;  // put UCA2 in reset mode

    //EUSCI_A2->CTLW0 = 00DC000006000001b;
    EUSCI_A2->CTLW0 = 0x3041;

    EUSCI_A2->BRW = 3;
    EUSCI_A2->MCTLW = 0x9200;
    //EUSCI_A2->IRCTL = 0000009000000211b;
    EUSCI_A2->IRCTL = 0x0207;

    EUSCI_A2->CTLW0 &= ~BIT0;   // enable UCA2 after config

    P3->SEL0 |=  (BIT2); // P1.5 and P1.6 for UCA2

}*/

void GLCD_putchar(int c){
    int i;
    for (i=0; i<6; i++){
        GLCD_data_write(font_table[c][i]);
    }
}

int getCharInt(char code)
{
    switch(code) {
      case '0': return 0;
      case '1': return 1;
      case '2': return 2;
      case '3': return 3;
      case '4': return 4;
      case '5': return 5;
      case '6': return 6;
      case '7': return 7;
      case '8': return 8;
      case '9': return 9;
      case ' ': return 10;
      case 'A': return 11;
      case 'B': return 12;
      case 'C': return 13;
      case 'D': return 14;
      case 'E': return 15;
      case 'F': return 16;
      case 'G': return 17;
      case 'H': return 18;
      case 'I': return 19;
      case 'J': return 20;
      case 'K': return 21;
      case 'L': return 22;
      case 'M': return 23;
      case 'N': return 24;
      case 'O': return 25;
      case 'P': return 26;
      case 'Q': return 27;
      case 'R': return 28;
      case 'S': return 29;
      case 'T': return 30;
      case 'U': return 31;
      case 'V': return 32;
      case 'W': return 33;
      case 'X': return 34;
      case 'Y': return 35;
      case 'Z': return 36;
      case '-': return 37;
      case ':': return 38;
      default:
          return 10;
    }
}

void GLCD_write_string(char str[14]){

    char *pos = str;

    while (*pos != '\0')
    {
        int code = getCharInt(*pos);
        GLCD_putchar(code);
        ++pos;
    }
}

/***
* keypad decoder function
***/
uint8_t Keypad_Decode(){
  uint8_t key = 0;

  // Read bits from keypad IC
  uint8_t data = P4->IN & DATABITS;

  // Interpret key type from data
  switch(data){
    case 0x0D: key = 0x0; break; /* 0 */
    case 0x00: key = 0x1; break; /* 1 */
    case 0x01: key = 0x2; break; /* 2 */
    case 0x02: key = 0x3; break; /* 3 */
    case 0x04: key = 0x4; break; /* 4 */
    case 0x05: key = 0x5; break; /* 5 */
    case 0x06: key = 0x6; break; /* 6 */
    case 0x08: key = 0x7; break; /* 7 */
    case 0x09: key = 0x8; break; /* 8 */
    case 0x0A: key = 0x9; break; /* 9 */
    case 0x03: key = 0xA; break; /* A */
    case 0x07: key = 0xB; break; /* B */
    case 0x0B: key = 0xC; break; /* C */
    case 0x0F: key = 0xD; break; /* D */
    case 0x0C: key = 0xE; break; /* * */
    case 0x0E: key = 0xF; break; /* # */
  }

  return key;
}
