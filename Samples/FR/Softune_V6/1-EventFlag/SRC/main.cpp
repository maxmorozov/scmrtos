//********************************************************************************
//*
//*     FULLNAME:  Single-Chip Microcontroller Real-Time Operating System
//*
//*     NICKNAME:  scmRTOS
//*
//*     PROCESSOR: FR (Fujitsu)
//*
//*     TOOLKIT:   Softune Workbench V6 (Fujitsu)
//*
//*     PURPOSE:   Port Test File
//*
//*     Version:   3.00-beta
//*
//*     $Revision: 1 $
//*     $Date: 2006-12-15 12:51:04 +0500 (Пт, 15 дек 2006) $
//*
//*     Copyright (c) 2003-2006, Harry E. Zhurov
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
//********************************************************************************

//---------------------------------------------------------------------------
#include <scmRTOS.h>
#include <hwtools.h>

//---------------------------------------------------------------------------
//
//      Process types
//
typedef OS::process<OS::pr0, 256> TProc1;
typedef OS::process<OS::pr1, 256> TProc2;
typedef OS::process<OS::pr2, 256> TProc3;
//---------------------------------------------------------------------------
//
//      Process objects
//
TProc1  Proc1;
TProc2  Proc2;
TProc3  Proc3;
//---------------------------------------------------------------------------

OS::TEventFlag  ef;

OS::TEventFlag  ef_timer2;


//---------------------------------------------------------------------------
int main(void)
{

    HardwareInit();

    scmRTOS_OS_INT_SETUP();

    scmRTOS_SYSTIMER_SETUP();

    OS::Run();

    for(;;);
}

//---------------------------------------------------------------------------
OS_PROCESS void TProc1::Exec()
{
    for(;;)
    {
        ef_timer2.Wait();

        PDR5_P55 = 0;   // LED 1 Off
    }
}

//---------------------------------------------------------------------------
OS_PROCESS void TProc2::Exec()
{
    for(;;)
    {
        ef.Wait();

        PDR5_P56 = 0;   // LED 2 Off
    }
}

//---------------------------------------------------------------------------
OS_PROCESS void TProc3::Exec()
{
    PDR5_P56 = 1;   // LED 2 ON
    for(;;)
    {
#if 0
        Sleep(10);
        PDR5_P56 = 0;   // LED 2 Off
#endif
        Sleep(3);
        PDR5_P56 = 1;   // LED 2 ON

        ef.Signal();
    }
}

//---------------------------------------------------------------------------
//void OS::SystemTimerUserHook() { }

//---------------------------------------------------------------------------
void OS::IdleProcessUserHook() { }

//---------------------------------------------------------------------------
extern "C" __interrupt void ih_Timer2(void)
{
    OS::TISRW ISR;

    TMCSR2_UF = 0;      // Clear INT flag

    HWDCS_CL = 0x00;    // Reset hardware watchdog

    PDR5_P55 = 1;       // LED 1 ON

    ef_timer2.SignalISR();
}
//---------------------------------------------------------------------------