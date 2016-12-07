#pragma once


//resample_fs = fs * P / Q
#define TARGET_SAMPLE_RATE  (16000)  //Hz
#define SOURCE_SAMPLE_RATE  (441000)  //Hz
//#define SOURCE_SAMPLE_RATE  (48000)  //Hz

const double FirCoeffs[] =
{
#include "FirCoeff44To16.dat"
//#include "FirCoeff48To16.dat"
};