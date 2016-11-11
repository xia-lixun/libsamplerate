#include <vector>
#include <string>
#include <cstring>
#include <cassert>
#include <iostream>
#include "Cwav.h"
#include "Coeffs.h"



int gcd(int a, int b) {
	return b == 0 ? a : gcd(b, a % b);
}



int main(int argc, char *argv[])
{
	//filter length
	const size_t FilterLength = sizeof(FirCoeffs) / sizeof(FirCoeffs[0]);
	

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

	std::cout << "+";
	for (size_t chan = 0; chan < channels; ++chan)
	{
		std::cout << chan;

		//extract the track from interleaved frame matrix	
		for (size_t i = 0; i < w44samples; ++i)
			w44track[i] = w44x[i*channels + chan];

		//apply filtering to w44track and store the result to w16track
		size_t count = 0;
		size_t offset = 0;
		while (offset < w44samples * numera)
		{
			size_t nn = offset / numera;
			float pd = 0.f;
			for (int i = nn; (offset - i*numera < FilterLength) && (i >= 0); --i)
				pd += w44track[i] * FirCoeffs[offset - i*numera];
			w16track[count++] = pd;
			offset += denorm;
		}

		Cwav w16;
		float *w16x = w16.SetFrameMatrix((int)count, 1, TARGET_SAMPLE_RATE);
		memcpy(w16x, &w16track[0], sizeof(float) * count);
		
		std::string path("MicIn_" + std::to_string(chan) + ".wav");
		w16.Save2File_flt(path.c_str());

	}
	std::cout << "+\n";



	return 0;
}