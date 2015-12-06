#include "config.h"
#include "GPIO.h"
#include "delay.h"
#include "soft_uart.h"
#include "ir.h"
#include "key_code.h"


#define SET_LED(led, state)     do {led = !state;} while (0)

void gpio_init(void)
{
    GPIO_InitTypeDef t_gpio_type;

    t_gpio_type.Mode = GPIO_HighZ;
    t_gpio_type.Pin = GPIO_Pin_0;	// UART RXD
    GPIO_Inilize(GPIO_P3, &t_gpio_type);

    t_gpio_type.Mode = GPIO_PullUp;
    t_gpio_type.Pin = GPIO_Pin_1;	// UART TXD
    GPIO_Inilize(GPIO_P3, &t_gpio_type);

    t_gpio_type.Mode = GPIO_PullUp;
    t_gpio_type.Pin = GPIO_Pin_2;	// white LED
    GPIO_Inilize(GPIO_P3, &t_gpio_type);
    WHITE_LED = 1;

    t_gpio_type.Mode = GPIO_PullUp;
    t_gpio_type.Pin = GPIO_Pin_3;	// Color LED
    GPIO_Inilize(GPIO_P3, &t_gpio_type);
    COLOR_LED = 1;

    t_gpio_type.Mode = GPIO_PullUp;
    t_gpio_type.Pin = GPIO_Pin_4;	// IR send
    GPIO_Inilize(GPIO_P3, &t_gpio_type);
    IR_SEND = 1;

    t_gpio_type.Mode = GPIO_HighZ;
    t_gpio_type.Pin = GPIO_Pin_5;	// IR rev
    GPIO_Inilize(GPIO_P3, &t_gpio_type);
}

#if 0
void slow_led(bit on, unsigned int sec)
{
    unsigned int i, cnt, m;
#define INTER_MS	600
    m = sec * 600;

    WHITE_LED = on;
    for (i=0; i<m; i++) {
        cnt = i;
        WHITE_LED = !WHITE_LED;
        while (cnt--);

        cnt = m - i;
        WHITE_LED = !WHITE_LED;
        while (cnt--);
    }
    WHITE_LED = !on;
}

void breath_led(void)
{
    while (1) {
        slow_led(1, 2);
        slow_led(0, 2);
    }
}
#endif

static void send_key_to_android(enum key_code key_code)
{
    print_str("\n\r");
    print_str("input keyevent ");
    print_dec(key_code);
    print_str("\n\r");
}

static enum key_code check_key(unsigned char *key)
{
    unsigned char size = sizeof(key_map) / sizeof(key_map[0]);
    unsigned char i;
    unsigned long key_val;

    key_val = ((unsigned long)key[0] << 24) |
              ((unsigned long)key[1] << 16) |
              ((unsigned long)key[2] << 8) |
              ((unsigned long)key[3]);

    for (i=0; i<size; i++) {
        if (key_val == key_map[i].ir_code) {
            return key_map[i].key_code;
        }
    }

    return KEYCODE_UNKNOWN;
}


static void handle_key(enum key_code key_code)
{
    static bit led_on = FALSE;
    bit led_ctl = FALSE;

    switch (key_code) {
    case KEYCODE_LED_ON:
        led_on = TRUE;
        led_ctl = TRUE;
        break;
    case KEYCODE_LED_OFF:
        led_on = FALSE;
        led_ctl = TRUE;
        break;
    case KEYCODE_LED_REV:
        led_on = !led_on;
        led_ctl = TRUE;
        break;
    case KEYCODE_UNKNOWN:
        break;
    default:
        send_key_to_android(key_code);
        break;
    }

    if (led_ctl) {
        SET_LED(WHITE_LED, led_on);
    }
}

unsigned char Ir_Buf[4];
unsigned int ir_loop_cnt = 0;


main()
{
    EA = 1;
    gpio_init();
    ir_rcv_init();

#if 0
    //0x00FF0DF2

    //breath_led();
    Ir_Buf[0] = 0x00;
    Ir_Buf[1] = 0xFF;
    Ir_Buf[2] = 0x0D;
    Ir_Buf[3] = 0xF2;
    while (1) {
        ir_send(Ir_Buf);
        delay_ms(200);
        COLOR_LED = 0;
        delay_ms(200);
        COLOR_LED = 1;
    }
#endif

    while (1) {
#if 0
        if (ir_loop_cnt++ > 5000) {
            ir_loop_cnt = 0;
            if (check_ir_loop()) {
                slow_led(1, 4);
                delay_ms(30000);
                slow_led(0, 4);
            }
        }
#endif
        //MCU_IDLE();
        if (ir_rcv(Ir_Buf)) {
            print_str("rcv key: 0x");
            print_hex(Ir_Buf[0]);
            print_hex(Ir_Buf[1]);
            print_hex(Ir_Buf[2]);
            print_hex(Ir_Buf[3]);
            print_str("\n\r");

            handle_key(check_key(Ir_Buf));
        }
    }
}




