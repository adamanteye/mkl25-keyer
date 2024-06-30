#include "clk.h"
#include "derivative.h"
#include "gpio.h"
#include "morse.h"
#include "oled.h"
#include "speaker.h"

#define TX_MODE 1
#define DECODE_MODE 0

char is_tx; /* 1 for transmit mode and 0 for decode mode */

char codes[256]; /* already decoded morse codes */
int code_index;  /* current code index */

int prev_input_timer;

int effect_duration; /* indicate how long the effect will continue in ms, 0 for no effect now */

int buf_len; /* length of dots and dashes to be decoded */

int buf_morse_index; /* current node of morse code */

unsigned int total_time; /* time beteween first input after init and tx */

unsigned int code_x; /* 0~ */

unsigned int code_y; /* 0-15 */

unsigned int buf_x; /* 0~ */

unsigned int buf_y; /* 0-15 */

unsigned int backspace_deshake; /* since dot and dash deshake automatically beacause of duration, only backspace needs to implement deshake logic */

char statistics[] = "00 CHARS IN 000s";

void statistics_init()
{
    statistics[0] = '0';
    statistics[1] = '0';
    statistics[12] = '0';
    statistics[13] = '0';
    statistics[14] = '0';
}

void statistics_update()
{
    unsigned int seconds = total_time / 1000;
    statistics[0] = (code_index + 1) / 10 + '0';
    statistics[1] = (code_index + 1) % 10 + '0';
    statistics[12] = seconds / 100 + '0';
    statistics[13] = (seconds / 10) % 10 + '0';
    statistics[14] = seconds % 10 + '0';
}

void draw_init()
{
    oled_clr();
    oled_w_str(0, 0, "EXPT CW yangzheh");
    oled_w_str(1, 0, "----------------");
    code_x = 2;
    code_y = 0;
    buf_x = 2;
    buf_y = 0;
    buf_len = 0;
}

void pre_code_draw()
{
    while (buf_len)
    {
        oled_w_ch(buf_x, buf_y * 8, ' ');
        if (buf_y == 0)
        {
            buf_y = 15;
            buf_x--;
        }
        else
            buf_y--;
        buf_len--;
    }
}

void post_buf_draw()
{
    buf_y++;
    buf_x += buf_y / 16;
    buf_y %= 16;
}

void post_code_draw()
{
    code_y++;
    code_x += code_y / 16;
    code_y %= 16;
    buf_x = code_x;
    buf_y = code_y;
}

void register_buf_change()
{
    buf_len++;
    prev_input_timer = 1;
    GPIOC_PDOR |= 1 << 12;
    GPIOC_PDOR |= 1 << 13;
}

void update_morse(int i)
{
    if (total_time == 0)
        total_time++;
    buf_morse_index = morse_next_index(buf_morse_index, i);
}

void backspace_draw()
{

    if (code_x <= 2 && code_y == 0)
        draw_init();
    else
    {
        oled_w_ch(code_x, code_y * 8, ' '); /* clear blink */
        if (code_y == 0)
        {
            code_y = 15;
            code_x--;
        }
        else
            code_y--;
        buf_x = code_x;
        buf_y = code_y;
        oled_w_ch(code_x, code_y * 8, ' ');
    }
}

void dot_effect()
{
    effect_duration = DOT_D;
    GPIOC_PDOR &= ~(1 << 12);
    speaker_set_note(SPEAKER_E0);
}

void dot_effect_with_tx()
{
    dot_effect();
    GPIOE_PDOR |= (1 << 3);
}

void dash_effect()
{
    effect_duration = DASH_D;
    GPIOC_PDOR &= ~(1 << 13);
    speaker_set_note(SPEAKER_G0);
}

void dash_effect_with_tx()
{
    dash_effect();
    GPIOE_PDOR |= (1 << 3);
}

void stop_effect()
{
    speaker_set_note(SPEAKER_N0);
    GPIOC_PDOR |= 1 << 12;
    GPIOC_PDOR |= 1 << 13;
    GPIOE_PDOR &= ~(1 << 3);
}

void PORTA_IRQHandler()
{
    /* DOT */
    if ((GPIOA_PDIR & 0x0002) == 0) /* PTA1 Pressed */
    {
        if (effect_duration == 0)
        {
            register_buf_change();
            update_morse(1);
            dot_effect();
            oled_w_ch(buf_x, buf_y * 8, '.');
            post_buf_draw();
        }
        PORTA_PCR1 |= 0x01000000;
    }
    /* DASH */
    if ((GPIOA_PDIR & (1 << 5)) == 0) /* PTA5 Pressed */
    {
        if (effect_duration == 0)
        {
            register_buf_change();
            update_morse(2);
            dash_effect();
            oled_w_ch(buf_x, buf_y * 8, '-');
            post_buf_draw();
        }
        PORTA_PCR5 |= 0x01000000;
    }
    /* BACKSPACE */
    if ((GPIOA_PDIR & (1 << 12)) == 0)
    {
        if (backspace_deshake == 0)
        {
            backspace_deshake = 400;
            pre_code_draw();
            if (code_index >= 0)
                code_index--;
            buf_morse_index = 0;
            prev_input_timer = 0;
            backspace_draw();
        }
        PORTA_PCR12 |= 0x01000000;
    }
    /* TX  */
    if ((GPIOA_PDIR & (1 << 14)) == 0)
    {
        if (effect_duration == 0 && code_index >= 0 && is_tx == DECODE_MODE)
            is_tx = TX_MODE;
        PORTA_PCR14 |= 0x01000000;
    }
}

unsigned int blink_cnt;

void SysTick_Handler()
{
    unsigned int i;
    blink_cnt++;
    if (effect_duration == 1)
        stop_effect();
    if (effect_duration > 0)
        effect_duration--;
    if (backspace_deshake > 0)
        backspace_deshake--;
    if (prev_input_timer > 0 && buf_morse_index != 0)
        prev_input_timer++;
    if (prev_input_timer >= TIMEOUT_D) /* 超时, 重置输入 buffer */
    {
        char ch = MORSE[buf_morse_index];
        i = buf_len;
        pre_code_draw();
        if (i <= 5 && ch != INVALID)
        {
            if (code_index < 255)
                codes[++code_index] = ch;
            else if (code_index == 255) /* maxium size excelled */
                code_index = -1;
            oled_w_ch(code_x, code_y * 8, ch);
            post_code_draw();
        }
        buf_morse_index = 0;
        prev_input_timer = 0;
    }
    if (blink_cnt == 199 && is_tx == DECODE_MODE)
        oled_w_ch(buf_x, buf_y * 8, buf_len + '0');
    else if (blink_cnt == 399 && is_tx == DECODE_MODE)
        oled_w_ch(buf_x, buf_y * 8, '|');
    else if (blink_cnt >= 400)
        blink_cnt = 0;
    if (total_time > 0)
        total_time++;
    i = SYST_CSR;
}

void device_init()
{
    effect_duration = 0;
    is_tx = DECODE_MODE;
    code_index = -1;
    prev_input_timer = 0;
    buf_morse_index = 0;
    backspace_deshake = 0;
    total_time = 0;
}

int main()
{
    clk_init();
    speaker_init();
    oled_init();
    gpio_init();
    device_init();
    draw_init();
    int i = 0; /* tmp variable, used in enumerating only */
    for (i = 0; i < 256; i++)
        codes[i] = 0;
    int cur_code;
    while (1) /* 轮询是否为发送模式 */
    {
        if (is_tx)
        {
            buf_morse_index = 0;
            prev_input_timer = 0;
            pre_code_draw();
            buf_x = 2;
            buf_y = 0;
            cur_code = -1;
            GPIOB_PDOR &= ~(1 << 19);
            statistics_update();
            oled_w_str(7, 0, statistics);
            while (cur_code < code_index)
            {
                cur_code++;
                oled_w_ch(buf_x, buf_y * 8, '|');
                if (is_letter(codes[cur_code]))
                    for (i = 0; i <= 4 && L2C[codes[cur_code] - 'A']; i++)
                    {
                        while (effect_duration != 0)
                            ;
                        if (L2C[codes[cur_code] - 'A'][i] == DOT)
                            dot_effect_with_tx();
                        else if (L2C[codes[cur_code] - 'A'][i] == DASH)
                            dash_effect_with_tx();
                        else
                            break;
                        while (effect_duration != 0)
                            ;
                        effect_duration = DOT_D;
                    }
                else
                {
                    for (i = 0; i <= 4 && N2C[codes[cur_code] - 'A']; i++)
                    {
                        while (effect_duration != 0)
                            ;
                        if (N2C[codes[cur_code] - 'A'][i] == DOT)
                            dot_effect_with_tx();
                        else if (N2C[codes[cur_code] - 'A'][i] == DASH)
                            dash_effect_with_tx();
                        else
                            break;
                        while (effect_duration != 0)
                            ;
                        effect_duration = DOT_D;
                    }
                }
                while (effect_duration != 0)
                    ;
                effect_duration = DASH_D;
                post_buf_draw();
            }
            GPIOB_PDOR |= 1 << 19;
            device_init();
            statistics_init();
            draw_init();
        }
    }
    return 0;
}
