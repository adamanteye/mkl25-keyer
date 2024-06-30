#include "adc.h"
#include "clk.h"
#include "derivative.h"
#include "gpio.h"
#include "morse.h"
#include "oled.h"
#include "speaker.h"

#define TX_MODE 1

#define DECODE_MODE 0

#define TX_SCROLL_BUFFER_SIZE 17 /* 1 char larger than zhe screen size (16) */

char op_mode; /* indicate transmit mode or decode mode, see TX_MODE and DECODE_MODE */

char codes[256]; /* already decoded morse codes */

int decode_buffer_index; /* current code index */

int prev_input_timer; /* indicate how long there has been no input of DOT or DASH */

int effect_duration; /* indicate how long the effect will continue in ms, 0 for no effect now */

int cursor_result; /* current node in MORSE the binary tree */

unsigned int total_time; /* time beteween first input after init and tx */

unsigned int code_x; /* 0 - larger, the frontier of already decoded morse codes */

unsigned int code_y; /* 0 - 15, the frontier of already decoded morse codes */

unsigned int cursor_x; /* 0 - larger */

unsigned int cursor_y; /* 0 - 15 */

int cursor_shift; /* counts of dots and dashes have been input but not yet submitted */

unsigned int backspace_deshake; /* since dot and dash deshake automatically beacause of duration, only backspace needs to implement deshake logic */

char statistics[] = "00 CHARS IN 000s"; /* statistics display string */

unsigned char tx_scroll_offset; /* used in scrolling the screen in tx mode */

unsigned int cursor_blink_timer; /* controls blink */

unsigned char tx_buffer_queue[TX_SCROLL_BUFFER_SIZE] = {SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE, SPACE}; /* this queue is always full and only needs one of head or tail */

unsigned int tx_buffer_queue_head; /* here I use head for handling the queue */

void en_tx_buffer_queue(char ch) /* enqueue for tx_buffer */
{
    tx_buffer_queue[tx_buffer_queue_head] = ch;
    if (tx_buffer_queue_head != TX_SCROLL_BUFFER_SIZE - 1)
        tx_buffer_queue_head++;

    else
        tx_buffer_queue_head = 0;
}

char get_tx_buffer_queue(unsigned int index) /* get for tx_buffer */
{
    if (index + tx_buffer_queue_head > TX_SCROLL_BUFFER_SIZE - 1)
        return tx_buffer_queue[index + tx_buffer_queue_head - TX_SCROLL_BUFFER_SIZE];
    else
        return tx_buffer_queue[index + tx_buffer_queue_head];
}

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
    statistics[0] = (decode_buffer_index + 1) / 10 + '0';
    statistics[1] = (decode_buffer_index + 1) % 10 + '0';
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
    cursor_x = 2;
    cursor_y = 0;
    cursor_shift = 0;
}

void pre_code_draw() /* clear the displayed chars between code and cursor */
{
    while (cursor_shift)
    {
        oled_w_ch(cursor_x, cursor_y * 8, ' ');
        if (cursor_y == 0)
        {
            cursor_y = 15;
            cursor_x--;
        }
        else
            cursor_y--;
        cursor_shift--;
    }
}

void post_cursor_draw() /* advance cursor by 1 */
{
    cursor_y++;
    cursor_x += cursor_y / 16;
    cursor_y %= 16;
}

void post_code_draw() /* advance code and cursor by 1 */
{
    code_y++;
    code_x += code_y / 16;
    code_y %= 16;
    cursor_x = code_x;
    cursor_y = code_y;
}

void register_cursor_change()
{
    cursor_shift++;
    prev_input_timer = 1;
    GPIOC_PDOR |= 1 << 12;
    GPIOC_PDOR |= 1 << 13;
}

void update_cursor_result(char input)
{
    if (total_time == 0)
        total_time++;
    cursor_result = morse_next_index(cursor_result, input);
}

void backspace_draw() /* backspace by 1 */
{

    if (code_x <= 2 && code_y == 0) /* cannot backspace and will reset the whole screen */
        draw_init();
    else
    {
        oled_w_ch(code_x, code_y * 8, ' '); /* clear blink */
        /* backspace code and reset cursor to code */
        if (code_y == 0)
        {
            code_y = 15;
            code_x--;
        }
        else
            code_y--;
        cursor_x = code_x;
        cursor_y = code_y;
        oled_w_ch(code_x, code_y * 8, ' ');
    }
}

void dot_effect() /* this effect will not transmit but merely use speaker to beep and LED to blink */
{
    effect_duration = DOT_D;
    GPIOC_PDOR &= ~(1 << 12);
    speaker_set_note(SPEAKER_E0);
}

void dot_effect_with_tx() /* TX_MODE does extra keyer logic by the use of PTE3 */
{
    dot_effect();
    GPIOE_PDOR |= (1 << 3);
    en_tx_buffer_queue(DOT);
}

void dash_effect() /* this effect will not transmit but merely use speaker to beep and LED to blink */
{
    effect_duration = DASH_D;
    GPIOC_PDOR &= ~(1 << 13);
    speaker_set_note(SPEAKER_G0);
}

void dash_effect_with_tx() /* TX_MODE does extra keyer logic by the use of PTE3 */
{
    /* actually it can be implemented within a loop, but implicitly executing 3 times might be efficient as well */
    effect_duration = DOT_D;
    GPIOE_PDOR |= (1 << 3);
    GPIOC_PDOR &= ~(1 << 13);
    speaker_set_note(SPEAKER_G0);
    en_tx_buffer_queue(DOT);
    while (effect_duration != 0)
        ;
    effect_duration = DOT_D;
    GPIOE_PDOR |= (1 << 3);
    GPIOC_PDOR &= ~(1 << 13);
    speaker_set_note(SPEAKER_G0);
    en_tx_buffer_queue(DOT);
    while (effect_duration != 0)
        ;
    effect_duration = DOT_D;
    GPIOE_PDOR |= (1 << 3);
    GPIOC_PDOR &= ~(1 << 13);
    speaker_set_note(SPEAKER_G0);
    en_tx_buffer_queue(DOT);
}

void stop_effect() /* for stopping dot and dash effects */
{
    speaker_set_note(SPEAKER_N0);
    GPIOC_PDOR |= 1 << 12;
    GPIOC_PDOR |= 1 << 13;
    GPIOE_PDOR &= ~(1 << 3);
}

void PORTA_IRQHandler()
{
    /* DOT */
    if ((GPIOA_PDIR & 0x0002) == 0 && op_mode == DECODE_MODE) /* PTA1 Pressed */
    {
        if (effect_duration == 0)
        {
            register_cursor_change();
            update_cursor_result(DOT);
            dot_effect();
            oled_w_ch(cursor_x, cursor_y * 8, '.');
            post_cursor_draw();
        }
        PORTA_PCR1 |= 0x01000000;
    }
    /* DASH */
    if ((GPIOA_PDIR & (1 << 5)) == 0 && op_mode == DECODE_MODE) /* PTA5 Pressed */
    {
        if (effect_duration == 0)
        {
            register_cursor_change();
            update_cursor_result(DASH);
            dash_effect();
            oled_w_ch(cursor_x, cursor_y * 8, '-');
            post_cursor_draw();
        }
        PORTA_PCR5 |= 0x01000000;
    }
    /* BACKSPACE */
    if ((GPIOA_PDIR & (1 << 12)) == 0 && op_mode == DECODE_MODE)
    {
        if (backspace_deshake == 0)
        {
            backspace_deshake = 400;
            pre_code_draw();
            if (decode_buffer_index >= 0)
                decode_buffer_index--;
            cursor_result = 0;
            prev_input_timer = 0;
            backspace_draw();
        }
        PORTA_PCR12 |= 0x01000000;
    }
    /* TX */
    if ((GPIOA_PDIR & (1 << 14)) == 0)
    {
        if (effect_duration == 0 && decode_buffer_index >= 0 && op_mode == DECODE_MODE)
            op_mode = TX_MODE;
        PORTA_PCR14 |= 0x01000000;
    }
}

void wave_render() /* draw scrolling wave sequence at 5th row */
{
    unsigned int i;
    char wave;
    tx_scroll_offset = 8 * (DOT_D - effect_duration) / DOT_D;
    for (i = 0; i < TX_SCROLL_BUFFER_SIZE; i++)
    {
        wave = get_tx_buffer_queue(i);
        if (wave == SPACE)
            wave = ':'; /* shape of _ */
        else
            wave = ';'; /* shape of - */
        if (i * 8 - tx_scroll_offset >= 0 && i * 8 - tx_scroll_offset <= 15 * 8)
            oled_w_ch(5, i * 8 - tx_scroll_offset, wave); /* only render those can be displayed within the screen */
    }
}

void SysTick_Handler()
{
    unsigned int i;
    cursor_blink_timer++;
    if (effect_duration == 1)
        stop_effect();
    if (effect_duration > 0)
        effect_duration--;
    if (backspace_deshake > 0)
        backspace_deshake--;
    if (prev_input_timer > 0 && cursor_result != 0)
        prev_input_timer++;
    if (prev_input_timer >= TIMEOUT_D) /* timeout occurs, submit cur_decode_result and reset it */
    {
        char ch = MORSE[cursor_result];
        pre_code_draw();
        if (cursor_shift <= 5 && ch != INVALID)
        {
            if (decode_buffer_index < 255)
                codes[++decode_buffer_index] = ch;
            else if (decode_buffer_index == 255) /* maxium size excelled */
                decode_buffer_index = -1;
            oled_w_ch(code_x, code_y * 8, ch);
            post_code_draw();
        }
        cursor_result = 0;
        prev_input_timer = 0;
    }
    /* complete cursor blink period is 400ms */
    if (cursor_blink_timer == 199 && op_mode == DECODE_MODE)
        oled_w_ch(cursor_x, cursor_y * 8, cursor_shift + '0');
    else if (cursor_blink_timer == 399 && op_mode == DECODE_MODE)
        oled_w_ch(cursor_x, cursor_y * 8, '|');
    else if (cursor_blink_timer >= 400)
        cursor_blink_timer = 0;

    if (cursor_blink_timer % 30 == 0) /* update DOT_D every 30ms  */
    {
        DOT_D = 20 + 70 * adc0_data() / 4095; /* should be 20ms - 90ms */
        if (op_mode == TX_MODE)
            wave_render();
    }

    if (total_time > 0) /* > 0 means total time should be counted, or else the counting is not started */
        total_time++;

    i = SYST_CSR;
}

void device_init() /* initiate all necessary variables as the initial status of the device */
{
    effect_duration = 0;
    op_mode = DECODE_MODE;
    decode_buffer_index = -1;
    prev_input_timer = 0;
    cursor_result = 0;
    backspace_deshake = 0;
    total_time = 0;
}

int main()
{
    clk_init();
    speaker_init();
    adc_init();
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
        if (op_mode == TX_MODE)
        {
            cursor_result = 0;
            prev_input_timer = 0;
            pre_code_draw();
            cursor_x = 2;
            cursor_y = 0;
            cur_code = -1;
            GPIOB_PDOR &= ~(1 << 19);
            statistics_update();
            oled_w_str(7, 0, statistics);
            while (cur_code < decode_buffer_index)
            {
                cur_code++;
                oled_w_ch(cursor_x, cursor_y * 8, '|');
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
                        en_tx_buffer_queue(SPACE);
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
                        en_tx_buffer_queue(SPACE);
                    }
                }
                while (effect_duration != 0)
                    ;
                for (i = 0; i < 3; i++) /* SPACE between letters should be 3 times the DOT_D */
                {
                    effect_duration = DOT_D;
                    en_tx_buffer_queue(SPACE);
                    while (effect_duration != 0)
                        ;
                }
                post_cursor_draw();
            }
            GPIOB_PDOR |= 1 << 19;
            device_init();
            statistics_init();
            draw_init();
        }
    }
    return 0;
}
