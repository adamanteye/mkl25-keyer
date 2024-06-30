#ifndef MORSE_H_
#define MORSE_H_

unsigned int DOT_D = 70; /* DOT duration, which will be changed during run by ADC read in */
#define DASH_D (3 * DOT_D)

#define TIMEOUT_D (7 * DOT_D)

#define INVALID 0
#define DOT     1
#define DASH    2
#define SPACE   3

#define ltr(x) (((x) << 1) + 1)
#define rtr(x) (((x) + 1) << 1)

unsigned char L2C[][5] = {1, 2, 0, 0, 0, 2, 1, 1, 1, 0, 2, 1, 2, 1, 0, 2, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 2, 1, 0, 2, 2, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 2, 2, 2, 0, 2, 1, 2, 0, 0, 1, 2, 1, 1, 0, 2, 2, 0, 0, 0, 2, 1, 0, 0, 0, 2, 2, 2, 0, 0, 1, 2, 2, 1, 0, 2, 2, 1, 2, 0, 1, 2, 1, 0, 0, 1, 1, 1, 0, 0, 2, 0, 0, 0, 0, 1, 1, 2, 0, 0, 1, 1, 1, 2, 0, 1, 2, 2, 0, 0, 2, 1, 1, 2, 0, 2, 1, 2, 2, 0, 2, 2, 1, 1, 0}; /* letter to morse codes conversion table */

unsigned char N2C[][5] = {2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 1, 1, 2, 2, 2, 1, 1, 1, 2, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 2, 2, 1, 1, 1, 2, 2, 2, 1, 1, 2, 2, 2, 2, 1, 2, 0, 0, 0, 0}; /* numbers to morse codes conversion table */

unsigned char MORSE[] = {0, 'E', 'T', 'I', 'A', 'N', 'M', 'S', 'U', 'R', 'W', 'D', 'K', 'G', 'O', 'H', 'V', 'F', 'L', 'P', 'J', 'B', 'X', 'C', 'Y', 'Z', 'Q', '5', '4', '3', '2', '1', '6', '7', '8', '9', '0', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; /* morse codes expressed as binary tree */

int is_letter(char ch)
{
    return ch >= 'A' && ch <= 'Z';
}

/* return the  */
int morse_next_index(int index, char ch)
{
    if (ch == DOT) /* left as dot */
        return ltr(index);
    else if (ch == DASH) /* right as dash */
        return rtr(index);
    else /* invalid */
        return 0;
}

#endif
