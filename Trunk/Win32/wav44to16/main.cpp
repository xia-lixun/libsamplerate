#include <vector>
#include <cstring>
#include "Cwav.h"
#include "Coeffs.h"





int main(int argc, char *argv[])
{
	const size_t filterLength = sizeof(FirCoeffs) / sizeof(FirCoeffs[0]);
	
	size_t samples;
	std::vector<float> audio;
	{
		Cwav w44;
		w44.ExtractMetaInfo(argv[1]);
		w44.PrintMetaInfo();
		float *pw44 = w44.GetFrameMatrix(argv[1]);

		samples = w44.GetFrameLength();
		audio.resize(samples * 160);
		std::fill(std::begin(audio), std::end(audio), 0.f);

		
		for(int i = 0; i < samples; ++i)
			audio[]
	}
	
	
	
	std::vector<float> fifo(filterLength);
	
	std::fill(std::begin(fifo), std::end(fifo), 0.f);

	//pushing samples to fifo and do inner product
	for(int i = 0; i < wi.GetFrameLength(); ++i)
	{
		memmove(&fifo[0], &fifo[1], sizeof(float) * (filterLength - 1));
		fifo[filterLength - 1] = pwi[i];
		
	}
		
	



	return 0;
}