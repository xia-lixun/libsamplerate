#include <vector>
#include <string>
#include <cstring>
#include <cassert>
#include <iostream>
#include <cmath>
#include <windows.h>

#include "Cwav.h"
#include "Coeffs.h"


//w44to16.exe speech_micarray_n8_44k.wav "D:\\Cortana\\Input\\"
//@todo: use compensated accumulation for inner product
//@todo: if input is @16000Hz, use pass-through
//@todo: if input is @48000Hz, use 3:1 filter


int gcd(int a, int b) {
	return b == 0 ? a : gcd(b, a % b);
}


double error;
void reset_accumulate(double& sigma)
{
	sigma = 0.0f;
	error = 0.0f;
}

void accumulate(double& sigma, double increment)
{
	double mInputCompensated;
	double mSumConverge;

	mInputCompensated = increment - error;
	mSumConverge = sigma + mInputCompensated;
	error = (mSumConverge - sigma) - mInputCompensated;
	sigma = mSumConverge;
}






int main(int argc, char *argv[])
{
	//filter length
	const size_t FilterLength = sizeof(FirCoeffs) / sizeof(FirCoeffs[0]);
	std::cout << "sizeof(size_t) = " << sizeof(size_t) << std::endl;
	std::string DestRoot(argv[2]);
	bool unscaled = (argc > 3) ? true : false;


	//source info
	Cwav w44;
	
	w44.ExtractMetaInfo(argv[1]);
	w44.PrintMetaInfo();

	assert(w44.GetSampleRate() == SOURCE_SAMPLE_RATE);
	const size_t channels = w44.GetNumChannel();
	const size_t w44samples = w44.GetFrameLength();

	const size_t divisor = (size_t)gcd(SOURCE_SAMPLE_RATE, TARGET_SAMPLE_RATE);
	const size_t numera = TARGET_SAMPLE_RATE / divisor; //numera == 160
	const size_t denorm = SOURCE_SAMPLE_RATE / divisor; //denorm == 441
	const size_t w16samples = (w44samples * numera) / denorm + 1;


	//iterate over all pcm tracks of the source file
	std::vector<float> w44track(w44samples);
	std::vector<float> w16track(w16samples);
	float *w44x = w44.GetFrameMatrix(argv[1]);
	double pd;
	

	for (size_t chan = 0; chan < channels; ++chan)
	{
		std::cout << "channel-" << chan << ":  ";

		//extract the track from interleaved frame matrix
		for (size_t i = 0; i < w44samples; ++i) 
		{
			w44track[i] = w44x[i*channels + chan];
		}


		//apply filtering to w44track and store the result to w16track
		size_t count = 0;
		size_t offset = 0;
		while (offset < w44samples * numera)
		{
			size_t nn = offset / numera;
			//float pd = 0.f;
			reset_accumulate(pd);
			for (int64_t i = nn; (offset - i*numera < FilterLength) && (i >= 0); --i)
				accumulate(pd, (double)w44track[i] * FirCoeffs[offset - i*numera]);
				//pd += w44track[i] * FirCoeffs[offset - i*numera];
			w16track[count++] = (float)pd;
			offset += denorm;
		}
		std::cout << count << " frames  [" << (float)count / (float)TARGET_SAMPLE_RATE << " seconds] processed\n";


		//find the max(abs(x)) for amplitude normalization
		double maxima = 0.0;
		double mabs;
		float gain;
		double sigma;
		double md;

		//find mu and peak of the w16 track
		reset_accumulate(sigma);
		for (const auto& m : w16track)
		{
			md = (double)m;
			accumulate(sigma, md);
			mabs = abs(md);
			if (mabs > maxima)
				maxima = mabs;
		}
		sigma = sigma / (double)count;
		std::cout << "mu:  " << sigma << "  ";


		//calculate gain and application
		if (unscaled)
			gain = 1.f;
		else
			gain = (float)(0.99 / (maxima + 1e-12));
		

		//apply gain and calculate rms value
		reset_accumulate(sigma);
		for (auto& m : w16track)
		{
			m = m * gain;
			md = (double)m * (double)m;
			accumulate(sigma, md);
		}
		std::cout << "rms:  " << 20.0 * log10(sqrt(sigma / ((double)count + 1e-12))) << " dB\n";


		Cwav w16;
		float *w16x = w16.SetFrameMatrix((int)count, 1, TARGET_SAMPLE_RATE);
		memcpy(w16x, &w16track[0], sizeof(float) * count);
		
		std::string DestPath(DestRoot + "CH" + std::to_string(chan) + "\\");
		CreateDirectory(DestPath.c_str(), NULL);
		std::string path(DestPath + "MicIn.wav");
		
		w16.Save2File_flt(path.c_str());
	}



	return 0;
}