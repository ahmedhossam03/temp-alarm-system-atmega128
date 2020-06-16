#include "temp_alarm_system.h"
#include "adc/adc.h"
#include "lcd/lcd.h"
#include "keypad/keypad.h"
#include "uart/uart.h"

//global variables shared between functions in the field
uint8 c = 0;					//current temperature value
uint8 aa_state = 1;				//alarm activation state, normally high
uint8 threshold = 100;			//threshold value, begins at 100

//all states this system goes through
typedef enum  
{
	TAS_MAIN,
	TAS_ALARM,
	TAS_THRESHOLD_CONFIG_KEY,
	TAS_THRESHOLD_CONFIG_TERM,
	TAS_NONE
}TAS;

TAS TAS_CURRENT_STATE = TAS_MAIN;
TAS TAS_PREV_STATE = TAS_NONE;

//main display
uint8 TAS_MAIN_DISP_1[] = "C:    T:    AA: ";
uint8 TAS_MAIN_DISP_2[] = "K:15  T:G AAC:13";

//alarm display
uint8 TAS_ALAR_DISP_1[] = "    ALARMING!   ";
uint8 TAS_ALAR_DISP_2[] = "  KAC:12 TAC:S  ";

//key config display
uint8 TAS_KCON_DISP_1[] = "      T:        ";
uint8 TAS_KCON_DISP_2[] = "   OK:15 CN:12  ";

//terminal config display
uint8 TAS_TCON_DISP_1[] = "      T:        ";
uint8 TAS_TCON_DISP_2[] = "   OK:O  CN:C   ";

void tas_init()
{
	//enable global interrupt
	setBit(SREG, 7);
	lcd_init();
	//lcd back light enable
	setBit(DDRE, 4);
	setBit(PORTE, 4);
	adc_init();
	keypad_init();
	uart_init(0);
	//buzzer pin init
	setBit(DDRE, 7);
}

void tas_main_disp()
{
	//clear display
	lcd_send_cmd(LCD_CLR);
	_delay_ms(2);
	//display first line
	lcd_send_str(TAS_MAIN_DISP_1, sizeof(TAS_MAIN_DISP_1));
	//go to second line
	lcd_goto_xy(0,1);
	//display second line
	lcd_send_str(TAS_MAIN_DISP_2, sizeof(TAS_MAIN_DISP_2));
}

void tas_alar_disp()
{
	//clear display
	lcd_send_cmd(LCD_CLR);
	_delay_ms(2);
	//display first line
	lcd_send_str(TAS_ALAR_DISP_1, sizeof(TAS_ALAR_DISP_1));
	//go to second line
	lcd_goto_xy(0,1);
	//display second line
	lcd_send_str(TAS_ALAR_DISP_2, sizeof(TAS_ALAR_DISP_2));
}

void tas_kcon_disp()
{
	//clear display
	lcd_send_cmd(LCD_CLR);
	_delay_ms(2);
	//display first line
	lcd_send_str(TAS_KCON_DISP_1, sizeof(TAS_KCON_DISP_1));
	//go to second line
	lcd_goto_xy(0,1);
	//display second line
	lcd_send_str(TAS_KCON_DISP_2, sizeof(TAS_KCON_DISP_2));
}

void tas_tcon_disp()
{
	//clear display
	lcd_send_cmd(LCD_CLR);
	_delay_ms(2);
	//display first line
	lcd_send_str(TAS_TCON_DISP_1, sizeof(TAS_TCON_DISP_1));
	//go to second line
	lcd_goto_xy(0,1);
	//display second line
	lcd_send_str(TAS_TCON_DISP_2, sizeof(TAS_TCON_DISP_2));
}

void tas_update_temp()
{
	//get c from adc and convert it between 0-150
	c = (adc_get_data(0) / 1024.0) * 150;
	//variable to check whether to update display
	//static so it stays, initialized with 255 to be out of c range at start
	static uint8 prev_c = 255;
	
	//only display c if value changed and the system is in the main state
	//to avoid display at ALARM STATE
	if(c != prev_c && TAS_CURRENT_STATE == TAS_MAIN)
	{
		//display new c at location
		lcd_goto_xy(TAS_TEMP_EDIT, TAS_LINE_EDIT);
		lcd_send_itoa(c);
	}
	//assigned the c in prev_c to keep the check going
	prev_c = c;
}

void tas_toggle_aa()
{
	//toggling alarm activation state
	aa_state = !aa_state;
	
	//display routine
	if(aa_state == 0)
	{
		lcd_goto_xy(TAS_AACT_EDIT, TAS_LINE_EDIT);
		lcd_send_char('N');
	}
	else if(aa_state == 1)
	{
		lcd_goto_xy(TAS_AACT_EDIT, TAS_LINE_EDIT);
		lcd_send_char('Y');
	}
}

void tas_threshold_edit_key()
{
	uint8 i = 0;
	uint8 ind = 0;
	uint8 tem = 0;
	uint8 arr[3] = {0};
	
	//go to the threshold edit position in lcd
	lcd_goto_xy(TAS_THRE_EDIT, TAS_LINE_EDIT);
	//get key and waiting here for key to be pressed
	uint8 key = key_wait_pressed();
	while((key <= 10) && ind < 3)
	{
		if(key == 10)   //key 10 is the zero
		{
			key = 0;
		}
		//display key as char
		lcd_send_char(key + '0');
		//assign key in arr
		arr[ind] = key;
		//increment index
		ind++;
		//get next key
		key = key_wait_pressed();
	}
	
	//converting arr into single number
	for(i = 0; i < ind; i++)
	{
		tem = (tem * 10) + arr[i];
	}
	
	//check last key value after exiting while loop if it was 15, confirm new threshold
	if(key == 15)
		threshold = tem;
}

void tas_threshold_edit_term()
{
	uint8 i = 0;
	uint8 ind = 0;
	uint8 tem = 0;
	uint8 arr[3] = {0};
	
	//go to the threshold edit position in lcd
	lcd_goto_xy(TAS_THRE_EDIT, TAS_LINE_EDIT);
	//get key not with interrupt to keep waiting here for uart receiving
	uint8 key = uart_rece_byte_blocking(0);
	while((key != 'O' && key != 'C') && ind < 3)
	{
		//display input
		lcd_send_char(key);
		//put the unsigned int form in the arr
		arr[ind] = key - '0';
		//increment ind
		ind++;
		//wait for next input
		key = uart_rece_byte_blocking(0);
	}
	
	//convert arr to a single number
	for(i = 0; i < ind; i++)
	{
		tem = (tem * 10) + arr[i];
	}
	
	//if last input after exiting while was 'O' then confirm new threshold
	if(key == 'O')
		threshold = tem;
}

void tas_run()
{
	uint8 key_choice = 0;
	uint8 uart_choice = 0;
	
	//choosing between states
	switch (TAS_CURRENT_STATE)
	{
		case TAS_MAIN:
			//avoiding multiple initializations of state if state didn't change
			if(TAS_PREV_STATE != TAS_MAIN)
			{
				tas_main_disp();
				//display initial alarm activation
				if(aa_state == 0)
				{
					lcd_goto_xy(TAS_AACT_EDIT, TAS_LINE_EDIT);
					lcd_send_char('N');
				}
				else if(aa_state == 1)
				{
					lcd_goto_xy(TAS_AACT_EDIT, TAS_LINE_EDIT);
					lcd_send_char('Y');
				}
				//display initial threshold
				lcd_goto_xy(TAS_THRE_EDIT, TAS_LINE_EDIT);
				lcd_send_itoa(threshold);
				//display initial temp
				lcd_goto_xy(TAS_TEMP_EDIT, TAS_LINE_EDIT);
				lcd_send_itoa(c);
				TAS_PREV_STATE = TAS_MAIN;
			}
			//check temperature and whether to go to alarm state
			tas_update_temp();
			if(c > threshold && aa_state == 1)
			{
				TAS_CURRENT_STATE = TAS_ALARM;
			}
			
			key_choice = keypad_getKey();
			uart_choice = uart_rece_byte(0);
			if(uart_choice == 'G')			//uart threshold config mode
			{
				TAS_CURRENT_STATE = TAS_THRESHOLD_CONFIG_TERM;
			}
			else if(key_choice == 15)	   //keypad threshold config mode
			{
				TAS_CURRENT_STATE = TAS_THRESHOLD_CONFIG_KEY;
			}
			else if(key_choice == 13)		//toggle alarm activation state
			{
				tas_toggle_aa();
			}
		break;
		
		case TAS_ALARM:
			//avoiding multiple initializations of state if state didn't change
			if(TAS_PREV_STATE != TAS_ALARM)
			{
				tas_alar_disp();
				TAS_PREV_STATE = TAS_ALARM;
			}
			//toggle buzzer in alarm mode
			togBit(PORTE, 7);
			_delay_ms(500);
			
			key_choice = keypad_getKey();
			uart_choice = uart_rece_byte(0);
			
			//checking return to main state and stop buzzer whether by temperature decrease or
			//alarm activation off by keypad or uart
			tas_update_temp();
			if(c < threshold)
			{
				TAS_CURRENT_STATE = TAS_MAIN;
				clearBit(PORTE, 7);
			}
			else if(uart_choice == 'S')
			{
				aa_state = 0;
				TAS_CURRENT_STATE = TAS_MAIN;
				clearBit(PORTE, 7);
			}
			else if(key_choice == 12)
			{
				aa_state = 0;
				TAS_CURRENT_STATE = TAS_MAIN;
				clearBit(PORTE, 7);
			}
		break;
		
		case TAS_THRESHOLD_CONFIG_KEY:
			//avoiding multiple initializations of state if state didn't change
			if(TAS_PREV_STATE != TAS_THRESHOLD_CONFIG_KEY)
			{
				tas_kcon_disp();
				TAS_PREV_STATE = TAS_THRESHOLD_CONFIG_KEY;
			}
			tas_threshold_edit_key();
			//after function finish whether with new threshold or not return to main state
			TAS_CURRENT_STATE = TAS_MAIN;
		break;
		
		case TAS_THRESHOLD_CONFIG_TERM:
			//avoiding multiple initializations of state if state didn't change
			if(TAS_PREV_STATE != TAS_THRESHOLD_CONFIG_TERM)
			{
				tas_tcon_disp();
				TAS_PREV_STATE = TAS_THRESHOLD_CONFIG_TERM;
			}
			tas_threshold_edit_term();
			//after function finish whether with new threshold or not return to main state
			TAS_CURRENT_STATE = TAS_MAIN;
		break;
		
		case TAS_NONE:
		break;
	}
}