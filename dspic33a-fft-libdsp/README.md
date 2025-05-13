<picture>
    <source media="(prefers-color-scheme: dark)" srcset="../images/microchip_logo_white_red.png">
	<source media="(prefers-color-scheme: light)" srcset="../images/microchip_logo_black_red.png">
    <img alt="Microchip Logo." src="../images/microchip_logo_black_red.png">
</picture>


##  FFT Implementation using DSP Library

## Description:

Microchip's dsPIC� Digital Signal Controllers feature a DSP Engine in the CPU that is capable of executing a Fast Fourier Transform (FFT) 
with great efficiency (high speed and low RAM usage). The on-chip features enabling the FFT implementation include, bit-reversed addressing,
Multiply-accumulate (MAC) type instructions and the ability to store and retrieve constants stored in Program memory.

Microchip provides a DSP functions library that provides in-place FFT functions.

This code example demonstrates how the DSP library functions can be used on dsPIC33A family of devices to perform an FFT operation on an input signal (vector) and find the spectral component with the highest energy. 

## Configuration
This project may be easily reconfigured to different FFT sizes, perform Real/Complex FFTs or store Twiddle Factors in either Program memory or RAM, simply by modifying the header file, [fft.h](firmware/src/fft.h).

### Configuring FFT Size:

The code example is reconfigurable to perfrom an FFT of any size, including common sizes of 64, 128, 256 and 512 points.  
- Change `FFT_BLOCK_LENGTH` to either 64, 128, 256 or 512

### Configuring Placement of Twiddle Factors
The code example also allows the user to place the FFT coefficients (known as Twiddle Factors) in RAM or in Program Flash Memory.
- If you would like to store Twiddle Factors coefficients in RAM instead of Program Memory comment out the line - `#define FFTTWIDCOEFFS_IN_PROGMEM`

> Twiddle factors can be pre-generated and stored in program memory to conserve RAM space. However, this approach incurs the cost of additional execution cycles.

### Configuring project to perform Real/Complex FFT
The code example also lets users to select between Real/Complex FFT algorithms
- Including `#define PERFORM_REAL_FFT` in the code, performs Real FFT.
- Commenting out `#define PERFORM_REAL_FFT` in the code, performs Complex FFT.


> `FFTReal` function performs a Fast Fourier Transform (FFT) exclusively on real data. In contrast, the `FFTComplex` function can handle both real data (with an imaginary part equal to zero) and complex data.
>
> When performing an FFT on real data, the result is a complex vector where the first half of the vector is the conjugate of the second half. Therefore, only the first N/2+1 outputs are required.
>
> `FFTReal` operations are generally faster compared to complex FFT functions.

## Operation

The FFT is performed in the following steps:

1. Initialization: 

   i. Generate Twiddle Factor Coefficients and store them in X-RAM. The twiddle factors can be generated using the following function:
      ```C
      #ifndef FFTTWIDCOEFFS_IN_PROGMEM					/* Generate TwiddleFactor Coefficients */
           TwidFactorInit (LOG2_BLOCK_LENGTH, &twiddleFactors[0], 0);	/* We need to do this only once at start-up */
      #endif
      ```

      Alternatively, you may also use the pre-generated twiddle factors stored in Program Flash.

   ii. Initialize the input vector in Y-memory at an address aligned to `2 x 4 x FFT_BLOCK_LENGTH` as shown below.
      
      ```C
      fractcomplex sigCmpx[FFT_BLOCK_LENGTH] __attribute__ ((space(ymemory), aligned (FFT_BLOCK_LENGTH * 2 *4)));
      ```
      
   iii. Scale the input signal to lie within the range _[-0.5, +0.5]_. For fixed point fractional input data, this translates to input samples in 
   the range _[0xC0000000,0x3FFFFFFF]_. The scaling is achieved by simply right-shifting the input samples by 1 bit, assuming the input samples lie 
   in the fixed point range _[0x80000000,0x7FFFFFFF]_ or _[-1,+1)_.

   iv. Convert the real input signal vector to a complex vector by placing zeros in every other location to signify a complex input whose
 imaginary part is _0x00000000_.


2. Butterfly computation and Bit-Reversed Re-ordering: 
   
   This is achieved by performing a call to the FFTComplexIP() function. The output array is re-ordered to be in bit-reversed order of the addresses.
   ```C
   /* Perform FFT operation */
   #ifdef PERFORM_REAL_FFT
       /* BitReverseComplex is called within FFTRealIP function.*/
       FFTRealIP(LOG2_BLOCK_LENGTH, &sigCmpx[0], (fractcomplex*) & twiddleFactors[0]);
   #else
       FFTComplexIP(LOG2_BLOCK_LENGTH, &sigCmpx[0], (fractcomplex*) & twiddleFactors[0], 0);

       /* Store output samples in bit-reversed order of their addresses */
       BitReverseComplex(LOG2_BLOCK_LENGTH, (fractcomplex*) &sigCmpx[0]);
   #endif
   ```


3. SquareMagnitude computation: We then need to compute the magnitude of each complex element in the output vector, so that we can estimate 
the energy in each spectral component/frequency bin. This is achieved by a call to a DSP Library's routine, SquareMagnitudeCplx(). 
   ```C
   /* Compute the square magnitude of the complex FFT output array so we have a Real output vetor */
       SquareMagnitudeCplx(FFT_BLOCK_LENGTH/2, &sigCmpx[0], output);
   ```


4. Peak-picking: We then find the frequency component with the largest energy by using the VectorMax() routine in the DSP library.
   ```C
   /* Find the frequency Bin ( = index into the SigCmpx[] array) that has the largest energy i.e., the largest spectral component */
       VectorMax(FFT_BLOCK_LENGTH/2, output, &peakFrequencyBin);
   ```


5. Frequency Calculation: The value of the spectral component with the highest energy, in Hz, is calculated by multiplying the array index of 
the largest element in the output array with the spectral (bin) resolution as follows - 
   ```C
   /* Compute the frequency (in Hz) of the largest spectral component */
       peakFrequency = (uint32_t) peakFrequencyBin*((float)SAMPLING_RATE/FFT_BLOCK_LENGTH);
   ```


### Input

The input signal used in the example will be 256 points of a Square wave signal of frequency 1KHz sampled at 10 KHz.

### Output
The FFT operation is performed on the input signal, in-place. This means that the output of the FFT resides in the same RAM locations where the
 input signal used to reside. 

Observe output variable `peakFrequency` in debug mode. Value should be almost near to 1Khz



## Hardware Used

- Curiosity Platform Development Board (https://www.microchip.com/en-us/development-tool/ev74h48a)
- dsPIC33AK128MC106 General Purpose Dual In-Line Module (DIM) (https://www.microchip.com/en-us/development-tool/ev74h48a)
	
	
## Software Used 

- MPLAB® X IDE v6.20 or newer (https://www.microchip.com/mplabx)
- MPLAB® XC-DSC v3.20 or newer (https://www.microchip.com/xc)
- MPLAB® XC-DSC DSP Library. (Shttps://www.microchip.com/en-us/software-library/dsp-library-for-dspic33a-dscs)