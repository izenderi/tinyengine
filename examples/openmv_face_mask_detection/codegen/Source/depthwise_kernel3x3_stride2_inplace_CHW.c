/* This file is automatically generated */
/* ----------------------------------------------------------------------
 * Project:      TinyEngine
 * Description:  for sparse in-place 3x3 depth-wise convolution (HWC->CHW->HWC)
 * Target ISA:  ARMv7E-M
 * Author: wmchen@mit.edu
 * -------------------------------------------------------------------- */
#include "arm_nnsupportfunctions.h" //TODO: remove this in the future for self-contained
#include "tinyengine_function.h"
void depthwise_kernel3x3_stride2_inplace_kernel_CHW(
		const uint16_t output_y, const uint16_t output_x,
		const int32_t *bias, const int32_t *biasR, const q7_t *ksrc, const int32_t *multiplier,
		const int32_t *shift, q7_t *output, const int32_t output_offset,
		const int32_t activation_min, const int32_t activation_max,
		q7_t *cols_8b_iterptr, const uint16_t column_x, int channel_offset);
tinyengine_status depthwise_kernel3x3_stride2_inplace_CHW(q7_t *input, const uint16_t input_x, const uint16_t input_y,
				const uint16_t input_ch, const q7_t *kernel, const int32_t *bias, const int32_t *biasR,
				const int32_t *output_shift, const int32_t *output_mult,
				const int32_t output_offset, const int32_t input_offset,
				const int32_t output_activation_min,
				const int32_t output_activation_max, q7_t *output,
				const uint16_t output_x, const uint16_t output_y,
				const uint16_t output_ch, q15_t *runtime_buf, q7_t pad_value)
{

    uint16_t c,i,j;
	q7_t *cols_8b_start = (q7_t *)runtime_buf;
	q7_t* cols_8b = (q7_t* )cols_8b_start;

	//Set padding value
	q7_t PAD8 = pad_value;
	/* setup the padding regions for Im2col buffers */
	//top region: 8bit x (input_x + pad_w * 2) x pad_h: unroll by pad value
	for(i = 0; i < input_x + 2; i++){
		*cols_8b++ = PAD8;
	}

	//middle regions: left and right regions
	for(i = 0; i < input_y; i++){
		*cols_8b++ = PAD8;//left
		cols_8b += input_x; //skip middle
		*cols_8b++ = PAD8;//right
	}

	//bottom region: 8bit x (input_x + pad_w * 2) x pad_h: unroll by pad value
	for(i = 0; i < input_x + 2; i++){
		*cols_8b++ = PAD8;
	}

	const q7_t *src;
	const q7_t *ksrc = kernel;

	for (c = 0; c < input_ch; c++){        
        cols_8b = (q7_t*)(cols_8b_start + 1 * (input_x) + 2); //skip 1 rows
        src = input;
        for(i = 0; i < input_y; i++){
            cols_8b += 1;//skip front
            for(j = 0; j < input_x; j++){
                *cols_8b++ = *src;// + input_offset;
                src += input_ch;
            }
            cols_8b += 1;//skip end
        }
		q7_t *inplace_out = input;
		depthwise_kernel3x3_stride2_inplace_kernel_CHW(output_y, output_x, bias++, biasR++, ksrc, output_mult++, output_shift++, inplace_out, output_offset,output_activation_min, output_activation_max,cols_8b_start, input_x, input_ch);
		ksrc += 9;
		input++;
    }

}
void depthwise_kernel3x3_stride2_inplace_kernel_CHW(
		const uint16_t output_y, const uint16_t output_x,
		const int32_t *bias, const int32_t *biasR, const q7_t *ksrc, const int32_t *multiplier,
		const int32_t *shift, q7_t *output, const int32_t output_offset,
		const int32_t activation_min, const int32_t activation_max,
		q7_t *cols_8b_iterptr, const uint16_t column_x, int channel_offset)
{
    #define STRIDE 2
    int i, j;
    /* MACs for each output */
	for (i = 0; i < output_y; i++) {
		for (j = 0; j < output_x / 2; j++) {
			q7_t *cols_8b = cols_8b_iterptr;
            
            q31_t sum0 = bias[0] + biasR[0];
			q31_t sum1 = bias[0] + biasR[0];
                        
            /* computation */
			sum0 += cols_8b[0]*ksrc[0];
			sum1 += cols_8b[2]*ksrc[0];
			sum0 += cols_8b[1]*ksrc[1];
			sum1 += cols_8b[3]*ksrc[1];
			sum0 += cols_8b[2]*ksrc[2];
			sum1 += cols_8b[4]*ksrc[2];
            cols_8b += column_x + 2;
			sum0 += cols_8b[0]*ksrc[3];
			sum1 += cols_8b[2]*ksrc[3];
			sum0 += cols_8b[1]*ksrc[4];
			sum1 += cols_8b[3]*ksrc[4];
			sum0 += cols_8b[2]*ksrc[5];
			sum1 += cols_8b[4]*ksrc[5];
            cols_8b += column_x + 2;
			sum0 += cols_8b[0]*ksrc[6];
			sum1 += cols_8b[2]*ksrc[6];
			sum0 += cols_8b[1]*ksrc[7];
			sum1 += cols_8b[3]*ksrc[7];
			sum0 += cols_8b[2]*ksrc[8];
			sum1 += cols_8b[4]*ksrc[8];
           
            /* requantize */
            sum0 = arm_nn_requantize(sum0 + biasR[0], *multiplier, *shift);
            sum0 += output_offset;
            sum0 = MAX(sum0, activation_min);
            sum0 = MIN(sum0, activation_max);
            output[(i * output_x + j * 2) * channel_offset] = sum0;

            sum1 = arm_nn_requantize(sum1 + biasR[0], *multiplier, *shift);
            sum1 += output_offset;
            sum1 = MAX(sum1, activation_min);
            sum1 = MIN(sum1, activation_max);
            output[(i * output_x + (j * 2 + 1)) * channel_offset] = sum1;

            cols_8b_iterptr += STRIDE * 2;
        }
        if (output_x & 1) {
			q7_t * cols_8b = cols_8b_iterptr;
			q31_t sum = bias[0] + biasR[0];
			sum += cols_8b[0]*ksrc[0];
			sum += cols_8b[1]*ksrc[1];
			sum += cols_8b[2]*ksrc[2];
            cols_8b += column_x + 2;
			sum += cols_8b[0]*ksrc[3];
			sum += cols_8b[1]*ksrc[4];
			sum += cols_8b[2]*ksrc[5];
            cols_8b += column_x + 2;
			sum += cols_8b[0]*ksrc[6];
			sum += cols_8b[1]*ksrc[7];
			sum += cols_8b[2]*ksrc[8];

            sum = arm_nn_requantize(sum + biasR[0], *multiplier, *shift);
			sum += output_offset;
			sum = MAX(sum, activation_min);
			sum = MIN(sum, activation_max);
            output[(i * output_x + output_x - 1) * channel_offset] = sum;

			cols_8b_iterptr += STRIDE;
        }
        cols_8b_iterptr += 1 * 2 - (column_x & 1);
        cols_8b_iterptr += (STRIDE - 1) * (column_x + 1 * 2);
    }
}
