// *****************************************************************************
// *****************************************************************************
// Section: Constants
// *****************************************************************************
// *****************************************************************************


/* Constant Definitions */
#include <xc.h>

//Supported LOG2_BLOCK_LENGTH values of this project -> 6 to 9.
#define LOG2_BLOCK_LENGTH     8         /* = Number of "Butterfly" Stages in FFT processing */
#define FFT_BLOCK_LENGTH   (1 << LOG2_BLOCK_LENGTH)   /* = Number of frequency points in the FFT */
#define SAMPLING_RATE        10000    /* = Rate at which input signal was sampled */
                                        /* SAMPLING_RATE is used to calculate the frequency*/
                                        /* of the largest element in the FFT output vector*/


/**
 * If FFTTWIDCOEFFS_IN_PROGMEM is defined - Pre-generated FFT Twiddle factors are placed in Program memory
 * Else: FFT Twiddle factors are placed in RAM and is generated on the runtime using TwidFactorInit function
 */
//#define FFTTWIDCOEFFS_IN_PROGMEM    
                                   

/**
 * If PERFORM_REAL_FFT is defined - Program performs Real-FFT on input samples
 * Else: The program performs Complex FFT.
 */
//#define PERFORM_REAL_FFT