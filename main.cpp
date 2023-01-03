#define F_CPU 1000000

#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <util/crc16.h>

void init(){
	DDRB |= (1 << DDB2);
	PORTB &= ~(1 << PORTB2);
	_delay_us(480);
	
	//release the bus
	DDRB &= ~(1 << DDB2);
	PORTB |= 1 << PORTB2;
	_delay_us(480);
}

inline void write_bit(bool one) noexcept {
	DDRB |= (1 << DDB2);
	PORTB &= ~(1 << PORTB2);
	_delay_us(1);
	if(one) {
		PORTB |= (1 << PORTB2);
	}
	_delay_us(60); //hold the signal for 59us + 1us(recovery time)

	//release the bus
	DDRB &= ~(1 << DDB2);
	PORTB |= 1 << PORTB2;
}

inline void write(uint8_t byte) noexcept {
	for(uint8_t i{}; i < 8; ++i, byte >>= 1)
	write_bit(byte & 0x01);
}

inline bool read_bit() noexcept {
	
	DDRB |= (1 << DDB2);
	PORTB &= ~(1 << PORTB2);
	_delay_us(1);
	
	DDRB &= ~(1 << DDB2);
	PORTB |= 1 << PORTB2;
	
	_delay_us(13);
	auto ret = (PINB & (1 << PINB2)) ? true : false;
	
	_delay_us(47);
	return ret;
}

inline uint8_t read() noexcept {
	uint8_t byte{};
	for(uint8_t i{8}, mask{0x01}; i > 0; --i, mask <<= 1)
	if(read_bit()){
		byte |= mask;
	}
	return byte;
}


inline void skip_rom() noexcept {
	init();
	write(0xCC);
}

inline void convert() noexcept {
	write(0x44) ;
}

inline void read_scratchpad_com() noexcept {
	write(0xBE /*Read scratchpad*/);
}

struct array2elem{
	uint8_t array[2];
	};

inline array2elem read_scratchpad() noexcept {
	uint8_t scratchpad[9];
	
	for(uint8_t i = 0; i < 9; ++i) {
		scratchpad[i] = read();
	}
	
	uint8_t whole_lo = (scratchpad[0]& 0xF0) >> 4;
	uint8_t whole_up = (scratchpad[1]& 0x07) << 4;
	uint8_t ret[2];
	ret[1] = static_cast<uint8_t>(whole_up | whole_lo);
	ret[0] = static_cast<uint8_t>(scratchpad[0]& 0x0F);
	
	array2elem retArray;
	retArray.array[1] = ret[1];
	retArray.array[0] = ret[0];
	
	return retArray;
}

#define LCD_Command_Dir DDRC
#define LCD_Data_Dir DDRD
#define LCD_Data_Port PORTD
#define LCD_Command_Port PORTC
#define RS PORTC0				/* Define Register Select (data/command reg.)pin */
#define RW PORTC1				/* Define Read/Write signal pin */
#define EN PORTC2				/* Define enable pin */

void LCD_Command(unsigned char cmnd)
{
	LCD_Data_Port= cmnd;
	LCD_Command_Port &= ~(1<<RS);	/* RS=0 command reg. */
	LCD_Command_Port &= ~(1<<RW);	/* RW=0 Write operation */
	LCD_Command_Port |= (1<<EN);	/* Enable pulse */
	_delay_us(1);
	LCD_Command_Port &= ~(1<<EN);
	_delay_ms(3);
}

void LCD_Init (void)	/* LCD Initialize function */
{
	LCD_Command_Dir |= 0x0F;	/* Make LCD command port direction as o/p */
	LCD_Data_Dir = 0xFF;	/* Make LCD data port direction as o/p */

	_delay_ms(20);		/* LCD Power ON delay always >15ms */
	LCD_Command(0x38);	/* Initialization of 16X2 LCD in 8bit mode */
	LCD_Command(0x0C);	/* Display ON Cursor OFF */
	LCD_Command(0x06);	/* Auto Increment cursor */
	LCD_Command(0x01);	/* clear display */
	LCD_Command(0x80);	/* cursor at home position */
}

void shiftCursor(){ /* shift cursor right so temperature value is on the right side of lcd display*/
	LCD_Command(0x14);
	LCD_Command(0x14);
	LCD_Command(0x14);
	LCD_Command(0x14);
	LCD_Command(0x14);
	LCD_Command(0x14);
	LCD_Command(0x14);
	LCD_Command(0x14);
	LCD_Command(0x14);
	LCD_Command(0x14);
}

void LCD_Char (unsigned char char_data)	/* LCD data write function */
{
	LCD_Data_Port = char_data;
	LCD_Command_Port |= (1<<RS);	/* RS=1 Data reg. */
	LCD_Command_Port &= ~(1<<RW);	/* RW=0 write operation */
	LCD_Command_Port |= (1<<EN);	/* Enable Pulse */
	_delay_us(1);
	LCD_Command_Port &= ~(1<<EN);
	_delay_ms(1);
}

void LCD_String (char *str)
{
	int i;
	for(i=0;str[i]!=0;i++)  /* send each char of string till the NULL */
	{
		LCD_Char (str[i]);  /* call LCD data write */
	}
}

void LCD_Clear()
{
	LCD_Command (0x01);		/* clear display */
	LCD_Command (0x80);		/* cursor at home position */
}

int main(void)
{
	skip_rom();
	convert();
	while(!read_bit());
	skip_rom();
	read_scratchpad_com();
	auto ret = read_scratchpad();
			
	LCD_Init();
	shiftCursor();
	
	bool end = true;
	char temp[3], temp2[1];
	
		switch(ret.array[0]){
			case 0:
			temp2[0] = '0';
			break;
			case 1:
			temp2[0] = '0';
			break;
			case 2:
			temp2[0] = '0';
			break;
			case 3:
			temp2[0] = '0';
			break;
			case 4:
			temp2[0] = '5';
			break;
			case 5:
			temp2[0] = '5';
			break;
			case 6:
			temp2[0] = '5';
			break;
			case 7:
			temp2[0] = '5';
			break;
			temp2[0] = '5';
			case 8:
			temp2[0] = '5';
			break;
			case 9:
			temp2[0] = '5';
			break;
			case 10:
			temp2[0] = '5';
			break;
			temp2[0] = '5';
			case 11:
			temp2[0] = '5';
			break;
			
			case 12:
			ret.array[1]++;
			temp2[0] = '0';
			break;
			ret.array[1]++;
			temp2[0] = '0';
			case 13:
			ret.array[1]++;
			temp2[0] = '0';
			break;
			case 14:
			ret.array[1]++;
			temp2[0] = '0';
			break;
			case 15:
			ret.array[1]++;
			temp2[0] = '0';
			break;
		}
	
	int i = 0;
	while(end){
		int remainder = ret.array[1] % 10;
		if(ret.array[1] < 10){
			end = false;
		}else{
			ret.array[1] = ret.array[1] / 10;
		}
		
			switch(remainder){
				case 0:
				 temp[i++] = '0';
				break;
				case 1:
				temp[i++] = '1';
				break;
				case 2:
				temp[i++] = '2';
				break;
				case 3:
				temp[i++] = '3';
				break;
				case 4:
				temp[i++] = '4';
				case 5:
				temp[i++] = '5';
				break;
				case 6:
				temp[i++] = '6';
				break;
				case 7:
				temp[i++] = '7';
				break;
				case 8:
				temp[i++] = '8';
				break;
				case 9:
				temp[i++] = '9';
				break;
			}
		
	}
	for(int j = 2; j>=i;--j){
		temp[j] = '\0';
	}
	
	for(int i=2;i>=0;--i){
		if(temp[i] == 0){
			continue;
		}else{
			LCD_Char(temp[i]);
		}
	}
	
	LCD_Char('.');
	LCD_Char(temp2[0]);
	LCD_Char((char)223);
	LCD_Char('C');
	i=0;
	
	while (1) 
    {
		
    }
}

