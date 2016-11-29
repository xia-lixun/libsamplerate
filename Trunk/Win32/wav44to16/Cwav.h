#pragma once



#include <cstdbool>
#include <cinttypes>
 



typedef struct MetaInfoStruct {

	//RIFF master header
	uint8_t riff_ckID[4];
	uint32_t riff_cksize;
	uint8_t riff_waveID[4];

	//FORMAT header
	uint8_t fmt_ckID[4];
	uint32_t fmt_cksize;
	uint16_t fmt_wFormatTag;
	uint16_t fmt_nChannels;
	uint32_t fmt_nSamplesPerSec;
	uint32_t fmt_nAvgbytesPerSec;
	uint16_t fmt_nBlockAlign;
	uint16_t fmt_wBitsPerSample;

	uint16_t fmt_cbSize;
	//EXTENDED FORMAT(ONLY FOR READ)
	uint16_t fmt_wValidBitsPerSample;
	uint32_t fmt_dwChannelMask;
	uint8_t fmt_SubFormat[16];

	//FACT header
	uint8_t fact_ckID[4];
	uint32_t fact_cksize;
	uint32_t fact_dwSampleLength; //number of samples per channel

	uint32_t unknown_cksize;
								  
	uint8_t data_ckID[4];//DATA
	uint32_t data_cksize;

} MetaInfo_t;



//application scenarios: 
// (a) Cwav a; a.GetFrameMatrix("file.wav"); f(FrameMatrix); Save2File_XXX();
// (b) Cwav b; b.SetFrameMatrix(1*fs, 2, 44100); f(FrameMatrix); Save2File_XXX();
class Cwav
{

public:
	Cwav();
	virtual ~Cwav();

	size_t ExtractMetaInfo(const char * FilePath);
	void PrintMetaInfo(void);
	
	int GetFrameLength(void) const;
	int GetNumChannel(void) const;
	int GetSampleRate(void) const;
	
	float * GetFrameMatrix(const char * FilePath);                     //compose a framematrix from file
	float * SetFrameMatrix(int nFrame, int nChannel, int nSampleRate); //compose a framematrix from param

	//write framematrix to files with slice param
	size_t Save2File_flt(const char * FilePath, double Start, double Stop);
	size_t Save2File_16b(const char * FilePath, double Start, double Stop);
	size_t Save2File_24b(const char * FilePath, double Start, double Stop);

	//write framematrix to files entirely
	size_t Save2File_flt(const char * FilePath);
	size_t Save2File_16b(const char * FilePath);
	size_t Save2File_24b(const char * FilePath);





private:
	size_t ExtractData_16b(const char * FilePath, float * FrameMat);
	size_t ExtractData_24b(const char * FilePath, float * FrameMat);
	size_t ExtractData_flt(const char * FilePath, float * FrameMat);

	void MakeMetaInfo(int nChannel, int nSampleRate, int nSample, int fmt);
	size_t SaveMetaInfo(const char * FilePath);


protected:
	MetaInfo_t MetaInfo;
	int NumChannel;
	int SampleRate;
	int NumFrame;

	float * FrameMatrix;
};

