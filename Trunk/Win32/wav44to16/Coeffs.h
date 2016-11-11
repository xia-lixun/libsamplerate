#pragma once


//resample_fs = fs * P / Q
#define TARGET_SAMPLE_RATE  (16000)  //Hz
#define SOURCE_SAMPLE_RATE  (44100)  //Hz

const float FirCoeffs[] =
{
#include "FirCoeff44To16.dat"
};