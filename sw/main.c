#include <avr/io.h>
#include <util/delay.h>

#define Dm_HIGH (1 << PORTA0) // Data+ high
#define Dm_LOW (1 << PORTA1)  // Data- low
#define Dp_HIGH (1 << PORTA2) // Data+ high
#define Dp_LOW (1 << PORTA3)  // Data+ low
#define SW2 (1 << PORTA4)     // Dip switch position 1
#define SW1 (1 << PORTA5)     // Dip switch position 2

typedef enum QC_VOLTAGE
{
    QC_5V = 0x3,
    QC_9V = 0x2,
    QC_12V = 0x1,
    QC_20V = 0x00
} QC_VOLTAGE;

void qc_init();
QC_VOLTAGE read_switch();
void qc_set_voltage(QC_VOLTAGE voltage);

int main(void)
{
    // PA4..5 input, PA0..3 output
    DDRA |= 0x0F;
    PORTA = 0x00;

    qc_init();
    QC_VOLTAGE volt = read_switch();
    qc_set_voltage(volt);
    while (1)
    {
        _delay_ms(200);
        QC_VOLTAGE volt_new = read_switch();
        if (volt_new != volt)
        {
            qc_set_voltage(volt_new);
            volt = volt_new;
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

QC_VOLTAGE read_switch()
{
    // Read bits 4 and 5 of input, bitshift them right so they are first two bits of return value
    return (QC_VOLTAGE)((PINA & (SW1 | SW2)) >> PORTA4);
};

void qc_set_voltage(QC_VOLTAGE voltage)
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
    default:
        break;
    }
}
