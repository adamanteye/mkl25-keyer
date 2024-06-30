#ifndef MYCLK_H_
#define MYCLK_H_

#include "derivative.h"

void clk_init()
{
    SYST_RVR = 20970; /* 1ms @ 20.97MHz */
    SYST_CVR = 0x00;
    SYST_CSR = 0x07;
}

#endif
