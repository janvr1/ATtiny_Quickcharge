#include <avr/io.h>
#include <util/delay.h>

#define Dp_HIGH (1 << PORTA0) // Data+ high
#define Dp_LOW (1 << PORTA1)  // Data+ low
#define Dm_LOW (1 << PORTA2)  // Data- low
#define Dm_HIGH (1 << PORTA3) // Data- high

#define SW3 (1 << PORTA6)    // Dip switch position 3
#define SW2 (1 << PORTA5)    // Dip switch position 2
#define SW1 (1 << PORTA4)    // Dip switch position 1
#define SW_DEC (1 << PORTB1) // Decrement button
#define SW_INC (1 << PORTB0) // Increment button

#define READ_SW_INC ((PINB & SW_INC) >> PORTB0) // Macro to read button value
#define READ_SW_DEC ((PINB & SW_DEC) >> PORTB1) // -||-

typedef enum QC_VOLTAGE
{
    QC_5V = 0x07,
    QC_9V = 0x03,
    QC_12V = 0x05,
    QC_20V = 0x01,
    QC_CONT = 0x06
} qc_voltage_t;

void qc_init();
qc_voltage_t read_switch();
void qc_set_voltage(qc_voltage_t voltage);
void qc_increment(void);
void qc_decrement(void);

int main(void)
{

    MCUCR |= (1 << PUD);   // Disable all pullups
    DDRA |= 0b00001111;    // PORTA0..3 are outputs
    DDRA &= ~(0b01110000); // PORTA4..6 are inputs
    DDRB &= ~(0b00000011); // PORTB0..1 inputs

    qc_init();
    qc_voltage_t volt = read_switch();
    qc_set_voltage(volt);
    qc_voltage_t volt_new;

    while (1)
    {
        _delay_ms(50);
        volt_new = read_switch();
        if (volt_new != volt)
        {
            if (volt == QC_CONT)
            {
                qc_set_voltage(QC_5V);
                _delay_ms(50);
            }
            qc_set_voltage(volt_new);
            volt = volt_new;
        }
        if (volt == QC_CONT)
        {
            // Read increment switch
            if (READ_SW_INC == 0)
            {
                qc_increment();
                uint8_t cnt = 0;
                while (READ_SW_INC == 0)
                {
                    _delay_ms(5);
                    cnt++;
                    if (cnt > 50)
                    {
                        qc_increment();
                        cnt = 0;
                    }
                }
            }
            if (READ_SW_DEC == 0)
            {
                qc_decrement();
                uint8_t cnt = 0;
                while (READ_SW_INC == 0)
                {
                    _delay_ms(5);
                    cnt++;
                    if (cnt > 50)
                    {
                        qc_decrement();
                        cnt = 0;
                    }
                }
            }
        }
    }
    return 0;
}

void qc_init()
{
    // Apply a voltage between 0.325V and 2V to D+ for atleast 1.25 seconds
    // D+_high = HIGH & D+_low = LOW -> D+ = 0,6V
    PORTA &= ~(Dp_LOW | Dm_LOW);
    PORTA |= (Dp_HIGH | Dm_HIGH);
    _delay_ms(1500);
    // Discharge the D-voltage below 0.325V for at least 1ms while keep the D+ voltage above 0.325V
    // D-_high = LOW & D-_low = LOW -> D- = 0V
    PORTA &= ~(Dm_HIGH | Dm_LOW);
    _delay_ms(50); // Wait for D- to fully discharge
}

qc_voltage_t read_switch()
{
    // Read bits 4 and 5 of input, bitshift them right so they are first two bits of return value
    return (qc_voltage_t)((PINA & (SW1 | SW2 | SW3)) >> 4);
};

void qc_increment()
{
    qc_set_voltage(QC_20V);
    _delay_ms(50);
    qc_set_voltage(QC_CONT);
    _delay_ms(50);
}

void qc_decrement()
{
    qc_set_voltage(QC_12V);
    _delay_ms(50);
    qc_set_voltage(QC_CONT);
    _delay_ms(50);
}
void qc_set_voltage(qc_voltage_t voltage)
{
    switch (voltage)
    {
    case QC_5V:
        // D+ = 0,6V; D- = 0V
        PORTA |= Dp_HIGH;
        PORTA &= ~(Dp_LOW | Dm_HIGH | Dm_LOW);
        break;
    case QC_9V:
        // D+ = 3,3V; D- = 0,6V
        PORTA |= (Dp_HIGH | Dp_LOW | Dm_HIGH);
        PORTA &= ~Dm_LOW;
        break;
    case QC_12V:
        // D+ = 0,6V; D- = 0,6V
        PORTA |= (Dp_HIGH | Dm_HIGH);
        PORTA &= ~(Dp_LOW | Dm_LOW);
        break;
    case QC_20V:
        // D+ = 3,3V; D- = 3,3V
        PORTA |= (Dp_HIGH | Dp_LOW | Dm_HIGH | Dm_LOW);
        break;
    case QC_CONT:
        // D+ = 0,6V; D- = 3,3V
        PORTA |= (Dp_HIGH | Dm_LOW | Dm_HIGH);
        PORTA &= ~Dp_LOW;
        break;
    default:
        // D+ = 0,6V; D- = 0V
        PORTA |= Dp_HIGH;
        PORTA &= ~(Dp_LOW | Dm_HIGH | Dm_LOW);
        break;
        break;
    }
}
