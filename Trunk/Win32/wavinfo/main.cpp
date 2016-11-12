#include "../wav44to16/Cwav.h"
#include <stdio.h>



int main(int argc, char *argv[])
{
	Cwav w;
	w.ExtractMetaInfo(argv[1]);
	w.PrintMetaInfo();

	float * p = w.GetFrameMatrix(argv[1]);
	//printf("%f, %f, %f, %f\n%f, %f, %f, %f\n", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7] );
	printf("sample rate: %d Hz\n", w.GetSampleRate());
	printf("channels: %d\n", w.GetNumChannel());
	printf("length: %f seconds\n[OK]\n", (float)w.GetFrameLength() / (float)w.GetSampleRate());

	return 0;
}