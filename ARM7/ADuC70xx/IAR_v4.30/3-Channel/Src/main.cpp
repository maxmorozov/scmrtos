//******************************************************************************
//*
//*     FULLNAME:  Single-Chip Microcontroller Real-Time Operating System
//*
//*     NICKNAME:  scmRTOS
//*
//*     PROCESSOR: ADuC 70xx (Analog Devices)
//*
//*     TOOLKIT:   EWARM (IAR Systems)
//*
//*     PURPOSE:   Port Test File
//*
//*     Version: 3.10
//*
//*     $Revision$
//*     $Date::             $
//*
//*     Copyright (c) 2003-2010, Harry E. Zhurov
//*
//*     Permission is hereby granted, free of charge, to any person 
//*     obtaining  a copy of this software and associated documentation 
//*     files (the "Software"), to deal in the Software without restriction, 
//*     including without limitation the rights to use, copy, modify, merge, 
//*     publish, distribute, sublicense, and/or sell copies of the Software, 
//*     and to permit persons to whom the Software is furnished to do so, 
//*     subject to the following conditions:
//*
//*     The above copyright notice and this permission notice shall be included 
//*     in all copies or substantial portions of the Software.
//*
//*     THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
//*     EXPRESS  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
//*     MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
//*     IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
//*     CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
//*     TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH 
//*     THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//*
//*     =================================================================
//*     See http://scmrtos.sourceforge.net for documentation, latest
//*     information, license and contact details.
//*     =================================================================
//*
//******************************************************************************
//*     ARM port by Sergey A. Borshch, Copyright (c) 2006-2010


//---------------------------------------------------------------------------
#include <scmRTOS.h>

//---------------------------------------------------------------------------
//
//      Process types
//
typedef OS::process<OS::pr0, 200> TProc1;
typedef OS::process<OS::pr1, 200> TProc2;
typedef OS::process<OS::pr2, 200> TProc3;
//---------------------------------------------------------------------------
//
//      Process objects
//
TProc1 Proc1;
TProc2 Proc2;
TProc3 Proc3;
dword T;                         // global variable for OS::GetTickCount testing
                                 //

//------------------------------------------------------------------------------
//
//   Message "body"
//
//
class TSlon
{
public:
    TSlon() { }
    virtual void eat() = 0;      // feed the slon. For non-russians: slon == elephant ;)
};
//------------------------------------------------------------------------------
class TAfricanSlon : public TSlon
{
public:
    virtual void eat()
    {
        TCritSect cs;

        GP0SET = (1 << (7 + 16));
        GP0CLR = (1 << (7 + 16));
    }
};
//------------------------------------------------------------------------------
class TIndianSlon : public TSlon
{
public:
    virtual void eat()
    {
        TCritSect cs;

        GP0SET = (1 << (7 + 16));
        GP0CLR = (1 << (7 + 16));
        GP0SET = (1 << (7 + 16));
        GP0CLR = (1 << (7 + 16));
    }
};
//------------------------------------------------------------------------------

TAfricanSlon African;
TIndianSlon  Indian;

OS::channel<TSlon*, 8> SlonQueue; // OS::channel object for 8 'TSlon*' items

OS::TEventFlag Timer_Ovf;
//---------------------------------------------------------------------------
int main()
{
    OS::Run();
}
//---------------------------------------------------------------------------
OS_PROCESS void TProc1::Exec()
{
    for(;;)
    {
        Timer_Ovf.Wait();
        SlonQueue.push(&African);
    }
}
//---------------------------------------------------------------------------
OS_PROCESS void TProc2::Exec()
{
    for(;;)
    {
        T += OS::GetTickCount();

        Sleep(1);
        SlonQueue.push(&Indian);
    }
}
//---------------------------------------------------------------------------
OS_PROCESS void TProc3::Exec()
{
    for(;;)
    {
        //--------------------------------------------------
        //
        //            Channel test
        //
        //
        //     Get data through channel
        //
        TSlon *p;
        SlonQueue.pop(p);     // get pointer from queue
        p->eat();             // feed the appropriate Slon
    }
}
//---------------------------------------------------------------------------
void OS::SystemTimerUserHook() { }
//---------------------------------------------------------------------------
void OS::IdleProcessUserHook() { }
//---------------------------------------------------------------------------
OS_INTERRUPT void Timer_ISR()
{
    OS::TISRW ISRW;
    T1CLRI = 0;

    Timer_Ovf.SignalISR();
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
extern "C" __arm void IRQ_Switch()
{
    dword irq = IRQSIG;
    irq &= IRQSTA;
    if(irq & GP_TIMER_BIT)
    {
        Timer_ISR();
    }
    if(irq & RTOS_TIMER_BIT)
    {
        OS::SystemTimer_ISR();
    }
}

_C_LIB_DECL
#pragma language=extended
#pragma location="ICODE"

#define RTOS_TICK_RATE  1000        // Hz

#define INT_OSC         32768UL
#define PLL_CLK         (INT_OSC * 1376)
#define SLCLK           (INT_OSC)
#define ECLK            (PLL_CLK)

#define TEST_TIMER_RATE 3500        // Hz

int __low_level_init(void)
{
    POWKEY1 = 0x01;
    POWCON = MODE_ACTIVE | CLK_DIV_1;
    POWKEY2 = 0xF4;

    GP0CON = 0;                                 // All pins are IO
    GP0DAT = (1UL<<(7+24)) | (0UL<<(7+16));     // GP0.7 = output, set to 0

    T0LD = ECLK / RTOS_TICK_RATE - 1;
    T0CON = T0_ENABLED | T0_PERIODIC | T0_DIV_1;
    IRQEN = RTOS_TIMER_BIT;

    T1LD = ECLK / TEST_TIMER_RATE - 1;
    T1CON = T1_HCLK | T1_DOWN | T1_ENABLED | T1_PERIODIC | T1_BINARY | T1_DIV_1;
    IRQEN = GP_TIMER_BIT;
    return 1;
}
#pragma language=default
_END_C_LIB_DECL