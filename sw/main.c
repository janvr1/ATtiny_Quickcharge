#include <avr/io.h>
#include <util/delay.h>

#define Dm_HIGH ((unsigned char)1) // PA0
#define Dm_LOW ((unsigned char)2)  // PA1
#define Dp_HIGH ((unsigned char)4) // PA2
#define Dp_LOW ((unsigned char)8)  // PA3

typedef enum QC_VOLTAGE
{
    QC_5V = 0x00,
    QC_9V = 0x01,
    QC_12V = 0x02,
    QC_20V = 0x03
} QC_VOLTAGE;

void qc_init();
QC_VOLTAGE read_switch();
void qc_set_voltage(QC_VOLTAGE voltage);

int main()
{
    // PA4..5 input, PA0..3 output
    DDRA |= 0x0F;
    PORTA = 0x00;

    qc_init();
    QC_VOLTAGE volt = read_switch();
    qc_set_voltage(volt);
    while (1)
    {
        _delay_ms(100);
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
    unsigned char port_a = PORTA;
    port_a &= ~(Dp_LOW | Dm_LOW);
    port_a |= (Dp_HIGH | Dm_HIGH);
    PORTA = port_a;
    _delay_ms(1500);
    // Discharge the D-voltage below 0.325V for at least 1mswhile keep the D+ voltage above 0.325V
    // D-_high = LOW & D-_low = LOW -> D- = 0V
    PORTA &= ~(Dm_HIGH | Dm_LOW); // Sets PA0 and PA1 low
    _delay_ms(50);
}

QC_VOLTAGE read_switch()
{
    return (QC_VOLTAGE)((~(PINA >> 4)) & 0x03);
};

void qc_set_voltage(QC_VOLTAGE voltage)
{
    unsigned char port_a = PORTA;
    switch (voltage)
    {
    case QC_5V:
        // D+ = 0,6V; D- = 0V
        port_a |= Dp_HIGH;
        port_a &= ~(Dp_LOW | Dm_HIGH | Dm_LOW);
        break;
    case QC_9V:
        // D+ = 3,3V; D- = 0,6V
        port_a |= (Dp_HIGH | Dp_LOW | Dm_HIGH);
        port_a &= ~Dm_LOW;
        break;
    case QC_12V:
        // D+ = 0,6V; D- = 0,6V
        port_a |= (Dp_HIGH | Dm_HIGH);
        port_a &= ~(Dp_LOW | Dm_LOW);
        break;
    case QC_20V:
        // D+ = 3,3V; D- = 3,3V
        port_a |= (Dp_HIGH | Dp_LOW | Dm_HIGH | Dm_LOW);
        break;
    default:
        break;
    }
    PORTA = port_a;
}
