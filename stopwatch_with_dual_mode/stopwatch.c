/*
 * stopwatch.c
 *      Author: Menna Tullah Medhat ALi EL-Din
 *      F.B: Menna Medhat
 *      Mini_Project: Stop Watch with Dual Mode
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/* Setting the needed variables */
unsigned char countdown_flag = 0;
unsigned char Reset_flag = 0;
unsigned char Zero_flag = 0;
unsigned char inc_hr_Button_flag = 0;
unsigned char dec_hr_Button_flag = 0;
unsigned char inc_min_Button_flag = 0;
unsigned char dec_min_Button_flag = 0;
unsigned char inc_sec_Button_flag = 0;
unsigned char dec_sec_Button_flag = 0;

unsigned char sec = 0, min = 0, hr = 0;
unsigned char sec_dig1 = 0, sec_dig2 = 0;
unsigned char min_dig1 = 0, min_dig2 = 0;
unsigned char hr_dig1 = 0, hr_dig2 = 0;

/* Timer1 interrupt service routine for stopwatch */
ISR(TIMER1_COMPA_vect) {
    if (countdown_flag == 0) {
        ++sec;
        if (sec == 59) {
            ++min;
            sec = 0;
        }
        if (min == 59) {
            ++hr;
            min = 0;
        }
        if (hr == 23) {
            hr = 0;
        }
    } else {
        if (sec == 0 && min == 0 && hr == 0) {
            sec = 0; min = 0; hr = 0;
        } else {
            if (sec == 0) {
                sec = 59;
                if (min == 0) {
                    min = 59;
                    if (hr > 0)
                    hr--;
                } else {
                    min--;
                }
            } else {
                sec--;
            }
        }
    }
}

/* Initialize Timer1 */
void Timer1_init(void) {
    TCNT1 = 0;
    TCCR1A |= (1 << FOC1A);          // Force output compare
    OCR1A = 15625;                   // Compare match value for 1 second
    TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10);  // CTC mode, Prescaler 1024
    TIMSK |= (1 << OCIE1A);          // Enable Timer1 Compare A Interrupt
}

/* Interrupt service routine for INT0 (Reset Button) */
ISR(INT0_vect) {
    Reset_flag = 1;
}

/* Initialize INT0 */
void INT0_init() {
    DDRD &= ~(1 << PD2);             // Set PD2 as input
    PORTD |= (1 << PD2);             // Enable pull-up on PD2
    GICR |= (1 << INT0);             // Enable INT0 interrupt
    MCUCR |= (1 << ISC01);           // Trigger on falling edge
}

/* Interrupt service routine for INT1 (Pause Button) */
ISR(INT1_vect) {
    TCCR1B &= ~(1 << CS10) & ~(1 << CS11) & ~(1 << CS12);  // Stop timer
}

/* Initialize INT1 */
void INT1_init(void) {
    DDRD &= ~(1 << PD3);             // Set PD3 as input
    PORTD &= ~(1 << PD3);             // Enable pull-up on PD3
    GICR |= (1 << INT1);             // Enable INT1 interrupt
    MCUCR |= (1 << ISC11) | (1 << ISC10);  // Trigger on rising edge
}

/* Interrupt service routine for INT2 (Resume Button) */
ISR(INT2_vect) {
    TCCR1B |= (1 << CS12) | (1 << CS10);  // Start timer with prescaler 1024
}

/* Initialize INT2 */
void INT2_init(void) {
    DDRB &= ~(1 << PB2);             // Set PB2 as input
    PORTB |= (1 << PB2);             // Enable pull-up on PB2
    GICR |= (1 << INT2);             // Enable INT2 interrupt
    MCUCSR &= ~(1 << ISC2);          // Trigger on falling edge
}

/* Display time on 7-segment display */
void display_digit(unsigned char digit, unsigned char display) {
    PORTA = display;  // Enable the selected display
    PORTC = (PORTC & 0xF0) | (digit & 0x0F);  // Send 4-bit segment data
    _delay_ms(4);  // Small delay for multiplexing
}

/* Increment seconds */
void increment_second(void) {
    ++sec;
    if (sec == 59) {
        ++min;
        sec = 0;
    }
    if (min == 59) {
        ++hr;
        min = 0;
    }
    if (hr == 23) {
        hr = 0;
    }
}

/* Increment minutes */
void increment_minute(void) {
    ++min;
    if (min == 59) {
        ++hr;
        min = 0;
    }
    if (hr == 23) {
        hr = 0;
    }
}

/* Increment hours */
void increment_hr(void) {
    ++hr;
    if (hr == 23) {
        hr = 0;
    }
}

/* Decrement seconds */
void decrement_second(void) {
    if (sec == 0 && min == 0 && hr == 0) {
        PORTC |= (1 << PD0);  // Activate buzzer
    } else {
        if (sec == 0) {
            sec = 59;
            if (min == 0) {
                min = 59;
                if (hr > 0) hr--;
            } else {
                min--;
            }
        } else {
            sec--;
        }
    }
}

/* Decrement minutes */
void decrement_minute(void) {
    if (sec == 0 && min == 0 && hr == 0) {
        PORTC |= (1 << PD0);  // Activate buzzer
    } else if ((min == 0)&&((hr > 0))) {
        min = 59;
        hr--;
    }else if ((min == 0)&&((hr == 0))){
        	min =0;
        hr =0;
    } else {
        min--;
    }
}

/* Decrement hours */
void decrement_hour(void) {
    if (sec == 0 && min == 0 && hr == 0) {
        PORTC |= (1 << PD0);  // Activate buzzer
    } else if (hr > 0) {
        hr--;
    }
}

/* Display the full time (seconds, minutes, hours) */
void display_time() {
    sec_dig1 = sec % 10;
    sec_dig2 = sec / 10;
    min_dig1 = min % 10;
    min_dig2 = min / 10;
    hr_dig1 = hr % 10;
    hr_dig2 = hr / 10;

    display_digit(sec_dig1, 0x01);
    display_digit(sec_dig2, 0x02);
    display_digit(min_dig1, 0x04);
    display_digit(min_dig2, 0x08);
    display_digit(hr_dig1, 0x10);
    display_digit(hr_dig2, 0x20);
}

/* Main function */
int main(void) {
    /* Initialize push buttons and display pins */
    DDRB &= 0x00;          // Set PORTB as input
    PORTB |= 0xFB;         // Enable pull-up for buttons

    DDRA = 0x3F;           // Set PORTA for display control
    DDRC = 0x0F;           // Set PORTC for 7-segment data

    /* Initialize other output pins (PD4, PD5 for LEDs, PD0 for buzzer) */
    DDRD |= (1 << PD4) | (1 << PD5) | (1 << PD0);
    PORTD |= (1 << PD4);  // Counting up LED on
    PORTD &= ~(1 << PD5); // Counting down LED off
    PORTD &= ~(1 << PD0); // Buzzer off

    Timer1_init();
    INT0_init();
    INT1_init();
    INT2_init();

    TCCR1B |= (1 << CS12) | (1 << CS10);

    SREG |= (1 << 7);  // Enable global interrupts

    /* Main loop */
    for(;;) {
        display_time();

        /* Reset button action */
        if (Reset_flag == 1) {
            sec = 0; min = 0; hr = 0;
            display_time();
            Reset_flag = 0;
        }

        /* Countdown toggle button (PB7) */

        if (!(PINB &(1<<PB7))){
        	_delay_ms(30);
        	if(!(PINB & (1<<PB7))){
        		sec = 0; min = 0; hr = 0;
        		if (countdown_flag == 0){
        			countdown_flag = 1;
        			 PORTD |= (1 << PD5);    // Counting down LED on
        			 PORTD &= ~(1 << PD4);  // Counting up LED off
        		}
        	}
        }
        /* Handle increment/decrement buttons if countdown is active */
        //hour increment button
        if (countdown_flag == 1) {
            if (!(PINB & (1 << PB1))) {
                if (!inc_hr_Button_flag) {
                    increment_hr();
                    inc_hr_Button_flag = 1;
                }
            } else {
                inc_hr_Button_flag = 0;
            }
            //minute increment button
            if (!(PINB & (1 << PB4))) {
                if (!inc_min_Button_flag) {
                    increment_minute();
                    inc_min_Button_flag = 1;
                }
            } else {
                inc_min_Button_flag = 0;
            }
            //second increment button

            if (!(PINB & (1 << PB6))) {
                if (!inc_sec_Button_flag) {
                    increment_second();
                    inc_sec_Button_flag = 1;
                }
            } else {
                inc_sec_Button_flag = 0;
            }
            //hour decrement button
            if (!(PINB & (1 << PB0))) {
                if (!dec_hr_Button_flag) {
                    decrement_hour();
                    dec_hr_Button_flag = 1;
                }
            } else {
                dec_hr_Button_flag = 0;
            }
            //minute decrement button
            if (!(PINB & (1 << PB3))) {
                if (!dec_min_Button_flag) {
                    decrement_minute();
                    dec_min_Button_flag = 1;
                }
            } else {
                dec_min_Button_flag = 0;
            }
            //second decrement button
            if (!(PINB & (1 << PB5))) {
                if (!dec_sec_Button_flag) {
                    decrement_second();
                    dec_sec_Button_flag = 1;
                }
            } else {
                dec_sec_Button_flag = 0;
            }
        }

        /* Activate buzzer if time reaches zero in countdown mode */
        if (sec == 0 && min == 0 && hr == 0 && countdown_flag == 1) {
            PORTD |= (1 << PD0);  // Turn on the buzzer
        } else {
        	 PORTD &= ~(1 << PD0);
        }
    }
}
