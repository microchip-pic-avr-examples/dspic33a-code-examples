/*******************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    main_fft_example.c

  Summary:
    This file is used to call the FFT related functions.

  Description:
    This file includes the function calls for FFTComplexIP, BitReverseComplex,
    SquareMagnitudeCplx functions that are central to the computation of the FFT
    of an input signal and then the peak frequency is calculated.
 *******************************************************************************/


/*
;*****************************************************************************
;                                                                            *
;                       Software License Agreement                           *
;*****************************************************************************
;*****************************************************************************
;© [2025] Microchip Technology Inc. and its subsidiaries.                    *
;                                                                            *
;   Subject to your compliance with these terms, you may use Microchip       *
;   software and any derivatives exclusively with Microchip products.        *
;    You are responsible for complying with 3rd party license terms          *
;    applicable to your use of 3rd party software (including open source     *
;    software) that may accompany Microchip software. SOFTWARE IS ?AS IS.?   *
;    NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS     *
;    SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,         *
;    MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT       *
;    WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,           *
;    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY        *
;    KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF        *
;    MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE        *
;    FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP?S          *
;    TOTAL LIABILITY ON ALL CLAIMS RELATED TO THE SOFTWARE WILL NOT          *
;    EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR       *
;   THIS SOFTWARE.                                                           *
;*****************************************************************************
 */


 /*****************************************************************************
 *****************************************************************************
                            Section: Included Files
 *****************************************************************************
 *****************************************************************************/

#include <stdint.h>
#include "fft.h"
#include <dsp.h>


 /****************************************************************************
 *****************************************************************************
                            Section:  Extern definitions 
 *****************************************************************************
 *****************************************************************************/
/**
 *  Typically, the input to an FFT  is a complex array containing samples
 * of an input signal. For this example, we will provide the input signal in an
 * array declared in Y-data space.
 */
#ifdef PERFORM_REAL_FFT
extern fractional sigCmpx[FFT_BLOCK_LENGTH + 1] _YDATA(FFT_BLOCK_LENGTH * 2 * 4); /*  */
#else
extern fractcomplex sigCmpx[FFT_BLOCK_LENGTH] _YDATA(FFT_BLOCK_LENGTH * 2 * 4); /*  */
#endif

/* Declare Twiddle Factor array in X-space*/
#ifndef FFTTWIDCOEFFS_IN_PROGMEM
fractcomplex twiddleFactors[FFT_BLOCK_LENGTH / 2] _XDATA(4*2);
#else
extern const fractcomplex twiddleFactors[FFT_BLOCK_LENGTH / 2] /* Twiddle Factor array in Program memory */
__attribute__((space(prog), aligned(FFT_BLOCK_LENGTH * 2)));
#endif
extern fractional input[FFT_BLOCK_LENGTH];


 /*****************************************************************************
 *****************************************************************************
                    Section: File Scope or Global Constants
 *****************************************************************************
 *****************************************************************************/
fractional output[FFT_BLOCK_LENGTH / 2];
int peakFrequencyBin = 0;   // Declare post-FFT variables to compute the
uint32_t peakFrequency = 0; // frequency of the largest spectral component


/******************************************************************************
 * Function:        int main(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Performs FFT of input signal and finds frequency of largest
 *                  spectrum component
 *****************************************************************************/
#ifdef TEST_MODE
unsigned char test_flag;
#endif
#ifdef TEST_MODE
int ce_main(void)
#else
int main(void)
#endif
{

#ifdef TEST_MODE
    test_flag = 0;
#endif
    int i = 0;
#ifndef FFTTWIDCOEFFS_IN_PROGMEM					
    /* Generate TwiddleFactor Coefficients */
    TwidFactorInit(LOG2_BLOCK_LENGTH, &twiddleFactors[0], 0); /* We need to do this only once at start-up */
#endif

    /* The FFT function requires input data */
    /* to be in the fractional fixed-point range [-0.5, +0.5]*/
    /* So, we shift all data samples by 1 bit to the right. */
    for (i = 0; i < FFT_BLOCK_LENGTH; i++) { 
#ifdef PERFORM_REAL_FFT
        /* Assign real values*/
        sigCmpx[i] = input[i] >> 1;
#else
        /* Assign real values to complex structure. Hence, .imag component is made 0.*/
        sigCmpx[i].real = input[i] >> 1; 
        sigCmpx[i].imag = 0; 
#endif
    }

/* Perform FFT operation */
#ifdef PERFORM_REAL_FFT
    /* BitReverseComplex is called within FFTRealIP function.*/
    FFTRealIP(LOG2_BLOCK_LENGTH, &sigCmpx[0], (fractcomplex*) & twiddleFactors[0]);
#else
    FFTComplexIP(LOG2_BLOCK_LENGTH, &sigCmpx[0], (fractcomplex*) & twiddleFactors[0], 0);

    /* Store output samples in bit-reversed order of their addresses */
    BitReverseComplex(LOG2_BLOCK_LENGTH, (fractcomplex*) &sigCmpx[0]);
#endif
    
    /* Compute the square magnitude of the complex FFT output array so we have a Real output vector */
    
    /* The FFT of a real valued input (with sigCmpx.imag = 0) is a complex valued output 
     * with 1st half of output being conjugate of 2nd half.
     * Hence, only 1st half of output is being considered for SquareMagnitudeCplx, VectorMax */
    SquareMagnitudeCplx(FFT_BLOCK_LENGTH / 2, (fractcomplex*) &sigCmpx[0], output);

    /* Find the frequency Bin ( = index into the SigCmpx[] array) that has the largest energy*/
    /* i.e., the largest spectral component */
    VectorMax(FFT_BLOCK_LENGTH / 2, output, &peakFrequencyBin);

    /* Compute the frequency (in Hz) of the largest spectral component */
    peakFrequency = (uint32_t) peakFrequencyBin * ((float) SAMPLING_RATE / FFT_BLOCK_LENGTH);

#ifdef TEST_MODE
    while (1) {
        if ((peakFrequency >= 950) && (peakFrequency <= 1050)) {
            test_flag = 1;
            return 0;
        } else {
            test_flag = 0;
            return 1;
        }
    }
#else

    while (1); /* Place a breakpoint here and observe the watch window variables */
#endif

}

/*
 *  Default Interrupt Routine : Default ISR for all undefined interrupts/traps.
 */
void __attribute__((interrupt)) _DefaultInterrupt() {
    while (1);
}
/*******************************************************************************
 End of File
 */