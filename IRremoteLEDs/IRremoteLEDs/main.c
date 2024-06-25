#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <IRremote.h> // Include IRremote library for using IR remote functionalities

// Pin definitions for LED connections
#define RED_LED_PIN   PD2
#define GREEN_LED_PIN PD4
#define BLUE_LED_PIN  PD7
#define WHITE_LED_1   PD3
#define WHITE_LED_2   PD5
#define WHITE_LED_3   PD6
#define WHITE_LED_4   PB3

// Pin definitions for IR receiver
#define IR_RECEIVE_PIN PB0

// IR Remote button codes
#define VOLUME_UP_BUTTON 21 //code for Volume Up
#define VOLUME_DOWN_BUTTON 7 //code for Volume Down
#define CHANNEL_DOWN_BUTTON 69 //code for Channel Down

// State variables
int ledState = 0; //current state of the LEDs
bool toggleColors = false; // Flag to toggle color LEDs
int colorCounter = 1; // Counter to track the current color of multi-color LED
int led_brightness[4] = {255, 255, 255, 255}; // Array to store brightness levels of LEDs
int reduced[4] = {0, 0, 0, 0};
int looped = 0; // Counter to track the number of times colors have looped

// Function prototypes
void turnOnLED();
void turnOffLED();
void cycleColorLED();
void resetForColorToggle();
void resetToWhiteLED();
void setAllLEDsOff();
void setWhiteLEDs(int count);
void setColorLED(int ledPin);
void updateLEDState();
void handleVolumeUp();
void handleVolumeDown();
void reduceLEDBrightness();









int main(void) {
	uint16_t address = 0; // Variable to store IR address
	uint16_t command = 0; // Variable to store IR command

	// Initialize white LEDs as outputs
	DDRD |= ((1 << WHITE_LED_1) | (1 << WHITE_LED_2) | (1 << WHITE_LED_3));
	DDRB |= (1 << WHITE_LED_4);

	// Initialize RGB LEDs as outputs
	DDRD |= ((1 << RED_LED_PIN) | (1 << GREEN_LED_PIN) | (1 << BLUE_LED_PIN));
	
	IR_init(); // Initialize IR remote control

	while(1) {
		if (IR_codeAvailable()) { // Check if IR code is available
			if (!IR_isRepeatCode()) { // Check if IR code is not a repeat
				IR_getCode(&address, &command); // Get IR address and command
				if (command == VOLUME_UP_BUTTON) {
					handleVolumeUp(); // Handle volume up button press
					} else if (command == VOLUME_DOWN_BUTTON) {
					handleVolumeDown(); // Handle volume down button press
					} else if (command == CHANNEL_DOWN_BUTTON) {
					reduceLEDBrightness(); // Handle channel down button to reduce brightness
				}
			}
		}
	}
}





void turnOnLED() {
	ledState++; // Increment the LED state to turn on the next LED
	if (ledState > 7) {
		looped++; // Increase loop count indicating full cycle
		ledState = 7;
		toggleColors = true; // Enable color toggling for RGB LED
		colorCounter = 1; // Reset color counter for RGB LED
	}
}

void turnOffLED() {
	ledState--; // Decrement the LED state to turn off the current LED
	if (ledState <= 0) {
		ledState = 0; // Ensure ledState does not go negative
	}
	led_brightness[ledState] = 255; // Reset brightness to maximum when turning off LED
}

void cycleColorLED() {
	colorCounter++;
	if (colorCounter > 3) {
		colorCounter = 1;
		looped++;
	}
}


void resetForColorToggle() {
	ledState--;
	if (ledState <= 0) {
		ledState = 0;
	}
}

void resetToWhite() {
	colorCounter--;
	if (colorCounter < 1 && looped > 0) {
		colorCounter = 3;
		looped--;
		if (looped <= 0) {
			toggleColors = false;
			looped = 0;
		}
	}
}

void setAllLEDsOff() {
	OCR2B = OCR0B = OCR0A = OCR2A = 0;
	DDRD &= ~((1 << WHITE_LED_1) | (1 << WHITE_LED_2) | (1 << WHITE_LED_3));
	DDRB &= ~(1 << WHITE_LED_4);
	PORTD &= ~((1 << RED_LED_PIN) | (1 << GREEN_LED_PIN) | (1 << BLUE_LED_PIN));
}

//set the necessary registers and port pins to control the brightness of the white LEDs.
void setWhiteLEDs(int count) {
	if (count >= 1) {
		DDRD |=  _BV(DDD3);
		TCCR2A |= (_BV(COM2B1) | _BV(WGM21) | _BV(WGM20));
		TCCR2B |=  (_BV(CS20));
		OCR2B = 255;
		
		if (reduced[0] == 1) {
			OCR2B = led_brightness[0];
			_delay_ms(10);
		}
		
		} else {
		OCR2B = 0;
		DDRD &= ~(1 << WHITE_LED_1);
	}

	if (count >= 2) {
		DDRD |=  _BV(DDD5);
		TCCR0A |= (_BV(COM0B1) | _BV(WGM01) | _BV(WGM00));
		TCCR0B |=  (_BV(CS00));
		OCR0B = 255;
		
		if (reduced[1] == 1) {
			OCR0B = led_brightness[1];
			_delay_ms(10);
		}
		
		} else {
		OCR0B = 0;
		DDRD &= ~(1 << WHITE_LED_2);
	}

	if (count >= 3) {
		DDRD |=  _BV(DDD6);
		TCCR0A |= (_BV(COM0A1) | _BV(WGM01) | _BV(WGM00));
		TCCR0B |=  (_BV(CS00));
		OCR0A = 255;
		
		if (reduced[2] == 1) {
			OCR0A = led_brightness[2];
			_delay_ms(10);
		}

		} else {
		OCR0A = 0;
		DDRD &= ~(1 << WHITE_LED_3);
	}

	if (count >= 4) {
		DDRB |=  _BV(DDB3);
		TCCR2A |= (_BV(COM2A1) | _BV(WGM21) | _BV(WGM20));
		TCCR2B |= (_BV(CS20));
		OCR2A = 255;

		if (reduced[3] == 1) {
			OCR2A = led_brightness[3];
			_delay_ms(10);
		}

		} else {
		OCR2A = 0;
		DDRB &= ~(1 << WHITE_LED_4);
	}

	PORTD &= ~((1 << RED_LED_PIN) | (1 << GREEN_LED_PIN) | (1 << BLUE_LED_PIN));
}

void setColorLED(int ledPin) {
	PORTD &= ~((1 << RED_LED_PIN) | (1 << GREEN_LED_PIN) | (1 << BLUE_LED_PIN));
	PORTD |= (1 << ledPin);
}

void updateLEDState () {
	if (!toggleColors) {
		// Handle white LEDs and RGB LEDs for non-color toggle mode
		switch (ledState) {
			case 0: setAllLEDsOff(); break; // All off
			case 1: setWhiteLEDs(1); break;
			case 2: setWhiteLEDs(2); break;
			case 3: setWhiteLEDs(3); break;
			case 4: setWhiteLEDs(4); break;
			case 5: setColorLED(RED_LED_PIN); break; // Red LED on
			case 6: setColorLED(GREEN_LED_PIN); break; // Green LED on
			case 7: setColorLED(BLUE_LED_PIN); break; // Blue LED on
		}
		} else {
		// Handle RGB LED for color toggle mode
		switch (colorCounter) {
			case 0: setWhiteLEDs(4); break; // All off
			case 1: setColorLED(RED_LED_PIN); break; // Red LED on
			case 2: setColorLED(GREEN_LED_PIN); break; // Green LED on
			case 3: setColorLED(BLUE_LED_PIN); break; // Blue LED on
		}
	}
}


void handleVolumeUp() {
	if (toggleColors) {
		cycleColorLED();
		} else {
		turnOnLED();
	}
	updateLEDState();
	_delay_ms(10); // Debounce delay
}

void handleVolumeDown() {
	if (ledState > 4 && !toggleColors) {
		resetForColorToggle();
		} else if (toggleColors) {
		resetToWhite();
		} else {
		turnOffLED();
	}
	updateLEDState();
	_delay_ms(10); // Debounce delay
}

void reduceLEDBrightness() {
	if (ledState == 0) {
		return;
	}
	led_brightness[ledState - 1] = 30;
	reduced[ledState - 1] = 1;
	updateLEDState();
}

