
#include "Cwav.h"
#include <cstring>
#include <cstdio>
#include <cstdlib>





Cwav::Cwav()
{
	FrameMatrix = NULL;
}


Cwav::~Cwav()
{
	delete[] FrameMatrix;
	FrameMatrix = NULL;
}



size_t Cwav::ExtractMetaInfo(const char * FilePath) {

	    uint8_t id[4];
		size_t BytesMetaInfo = 0;

		FILE * f;
		errno_t Err = fopen_s(&f, FilePath, "rb");
		if (Err != 0) {
			printf("File open failure: %s (%d)\n", FilePath, Err);
		}

		//RIFF header
		BytesMetaInfo += fread(  MetaInfo.riff_ckID,    sizeof(uint8_t),  4, f);
		BytesMetaInfo += fread(&(MetaInfo.riff_cksize), sizeof(uint32_t), 1, f) * sizeof(uint32_t);
		BytesMetaInfo += fread(  MetaInfo.riff_waveID,  sizeof(uint8_t),  4, f);

		//FORMAT header
		BytesMetaInfo += fread(  MetaInfo.fmt_ckID,             sizeof(uint8_t),  4, f);
		BytesMetaInfo += fread(&(MetaInfo.fmt_cksize),          sizeof(uint32_t), 1, f) * sizeof(uint32_t);
		BytesMetaInfo += fread(&(MetaInfo.fmt_wFormatTag),      sizeof(uint16_t), 1, f) * sizeof(uint16_t);
		BytesMetaInfo += fread(&(MetaInfo.fmt_nChannels),       sizeof(uint16_t), 1, f) * sizeof(uint16_t);
		BytesMetaInfo += fread(&(MetaInfo.fmt_nSamplesPerSec),  sizeof(uint32_t), 1, f) * sizeof(uint32_t);
		BytesMetaInfo += fread(&(MetaInfo.fmt_nAvgbytesPerSec), sizeof(uint32_t), 1, f) * sizeof(uint32_t);
		BytesMetaInfo += fread(&(MetaInfo.fmt_nBlockAlign),     sizeof(uint16_t), 1, f) * sizeof(uint16_t);
		BytesMetaInfo += fread(&(MetaInfo.fmt_wBitsPerSample),  sizeof(uint16_t), 1, f) * sizeof(uint16_t);
	
		//cbSize field
		if (MetaInfo.fmt_cksize == 18 || MetaInfo.fmt_cksize == 40) {
			BytesMetaInfo += fread(&(MetaInfo.fmt_cbSize), sizeof(uint16_t), 1, f) * sizeof(uint16_t);
		}
		if (MetaInfo.fmt_cksize == 40) {
			BytesMetaInfo += fread(&(MetaInfo.fmt_wValidBitsPerSample), sizeof(uint16_t), 1, f)  * sizeof(uint16_t);
			BytesMetaInfo += fread(&(MetaInfo.fmt_dwChannelMask),       sizeof(uint32_t), 1, f)  * sizeof(uint32_t);
			BytesMetaInfo += fread(&(MetaInfo.fmt_SubFormat),           sizeof(uint8_t),  16, f);
		}

		//judge between FACT and DATA
		BytesMetaInfo += fread(id, sizeof(uint8_t), 4, f);
	
		//if FACT chunck
		memset(MetaInfo.fact_ckID, '-', sizeof(uint8_t) * 4);
		if (id[0] == 'f' && id[1] == 'a' && id[2] == 'c' && id[3] == 't') {
			memcpy(MetaInfo.fact_ckID, id, sizeof(uint8_t) * 4);
			BytesMetaInfo += fread(&(MetaInfo.fact_cksize),         sizeof(uint32_t), 1, f) * sizeof(uint32_t);
			BytesMetaInfo += fread(&(MetaInfo.fact_dwSampleLength), sizeof(uint32_t), 1, f) * sizeof(uint32_t);
		
			if (MetaInfo.fact_cksize > 4) {
				for (int k = 0; k < (int)(MetaInfo.fact_cksize - 4); ++k) {
					BytesMetaInfo += fread(id, sizeof(uint8_t), 1, f);
				}
			}
			BytesMetaInfo += fread(id, sizeof(uint8_t), 4, f);
		}

		while (id[0] != 'd' || id[1] != 'a' || id[2] != 't' || id[3] != 'a') { //unknown chunk that we need to skip
			BytesMetaInfo += fread(&(MetaInfo.unknown_cksize), sizeof(uint32_t), 1, f) * sizeof(uint32_t);
			uint8_t ByteSkip;
			for(uint32_t k = 0; k < MetaInfo.unknown_cksize; ++k)
				BytesMetaInfo += fread(&ByteSkip, sizeof(uint8_t), 1, f);
			BytesMetaInfo += fread(id, sizeof(uint8_t), 4, f);
		}

		if (id[0] == 'd' && id[1] == 'a' && id[2] == 't' && id[3] == 'a'){
			memcpy(MetaInfo.data_ckID, id, sizeof(uint8_t) * 4);

			//read rest of the DATA chunck
			BytesMetaInfo += fread(&(MetaInfo.data_cksize), sizeof(uint32_t), 1, f) * sizeof(uint32_t);
		}
		else {
			printf("Header Parse Error!\n");
		}
		fclose(f);

		//update parameter info
		NumChannel = (int)(MetaInfo.fmt_nChannels);
		SampleRate = (int)(MetaInfo.fmt_nSamplesPerSec);
		NumFrame = (int)(MetaInfo.data_cksize / MetaInfo.fmt_nBlockAlign);
		return BytesMetaInfo;
}

void Cwav::PrintMetaInfo(void) {

	printf("\n");
	printf("+---------------------------------------------------------------------------------------------------+\n");
	printf("| EXTENSIBLE FORMAT READ IS SUPPORTED                                                               |\n");
	printf("| EXTENSIBLE FORMAT WRITE IS NOT SUPPORTED                                                          |\n");
	printf("+----------------------------------------------RIFF-------------------------------------------------+\n");
	printf("|ckID                %c%c%c%c\n", MetaInfo.riff_ckID[0], MetaInfo.riff_ckID[1], MetaInfo.riff_ckID[2], MetaInfo.riff_ckID[3]);
	printf("|cksize              %d\n", MetaInfo.riff_cksize);
	printf("|WAVEID              %c%c%c%c\n", MetaInfo.riff_waveID[0], MetaInfo.riff_waveID[1], MetaInfo.riff_waveID[2], MetaInfo.riff_waveID[3]);
	printf("+---------------------------------------------FORMAT------------------------------------------------+\n");
	printf("|ckID                %c%c%c%c\n", MetaInfo.fmt_ckID[0], MetaInfo.fmt_ckID[1], MetaInfo.fmt_ckID[2], MetaInfo.fmt_ckID[3]);
	printf("|cksize              %d\n", MetaInfo.fmt_cksize);
	printf("|wFormatTag          %x\n", MetaInfo.fmt_wFormatTag);
	printf("|nChannels           %d\n", MetaInfo.fmt_nChannels);
	printf("|nSamplesPerSec      %d\n", MetaInfo.fmt_nSamplesPerSec);
	printf("|nAvgbytesPerSec     %d\n", MetaInfo.fmt_nAvgbytesPerSec);
	printf("|nBlockAlign         %d\n", MetaInfo.fmt_nBlockAlign);
	printf("|wBitsPerSample      %d\n", MetaInfo.fmt_wBitsPerSample);
	if (MetaInfo.fmt_cksize == 16) {
		printf("|cbSize              -\n");
	}
	else {
		printf("|cbSize              %d\n", MetaInfo.fmt_cbSize);
	}
	if (MetaInfo.fmt_cksize == 40) {
		printf("|wValidBitsPerSample %d\n", MetaInfo.fmt_wValidBitsPerSample);
		printf("|dwChannelMask       %d\n", MetaInfo.fmt_dwChannelMask);
		printf("|SubFormat           ");
		printf("0x%02X%02X ", MetaInfo.fmt_SubFormat[1], MetaInfo.fmt_SubFormat[0]);
		for (int j = 2; j < 16; j++) {
			printf("0x%02X ", MetaInfo.fmt_SubFormat[j]);
		}
		printf("\n");
	}
	else {
		printf("|wValidBitsPerSample -\n");
		printf("|dwChannelMask       -\n");
		printf("|SubFormat           -\n");
	}
	printf("+----------------------------------------------FACT-------------------------------------------------+\n");
	printf("|ckID                %c%c%c%c\n", MetaInfo.fact_ckID[0], MetaInfo.fact_ckID[1], MetaInfo.fact_ckID[2], MetaInfo.fact_ckID[3]);
	if (MetaInfo.fact_ckID[0] == '-') {
		printf("|cksize              -\n");
		printf("|dwSampleLength      -\n");
	}
	else {
		printf("|cksize              %d\n", MetaInfo.fact_cksize);
		printf("|dwSampleLength      %d\n", MetaInfo.fact_dwSampleLength);
	}
	printf("+----------------------------------------------DATA-------------------------------------------------+\n");
	printf("|ckID                %c%c%c%c\n", MetaInfo.data_ckID[0], MetaInfo.data_ckID[1], MetaInfo.data_ckID[2], MetaInfo.data_ckID[3]);
	printf("|cksize              %d\n", MetaInfo.data_cksize);
	printf("+---------------------------------------------------------------------------------------------------+\n");
}


int Cwav::GetFrameLength(void) const {
	return NumFrame;
}

int Cwav::GetNumChannel(void) const {
	return NumChannel;
}

int Cwav::GetSampleRate(void) const {
	return SampleRate;
}



// use getters to prepare buffers before invoking this method
size_t Cwav::ExtractData_flt(const char * FilePath, float * FrameMat) {

	// update the meta information
	size_t MetaInfoTotalBytes = ExtractMetaInfo(FilePath);
	uint8_t * flush = (uint8_t *)malloc(MetaInfoTotalBytes * sizeof(uint8_t));

	// read data out to buffer
	size_t EleRead;
	size_t BytesTotal = 0;
	float * EachFrame = new float[MetaInfo.fmt_nChannels];

	FILE * f;
	errno_t ERR = fopen_s(&f, FilePath, "rb");
		
	// flush the meta info block
	BytesTotal += fread(flush, sizeof(uint8_t), MetaInfoTotalBytes, f);
	free(flush);

	//read PCM data to buffers
	for (int i = 0; i < NumFrame; i++) {
		EleRead = fread(EachFrame, sizeof(float), MetaInfo.fmt_nChannels, f);
		BytesTotal += EleRead * sizeof(float);
		if (EleRead != MetaInfo.fmt_nChannels) {
			printf("Data frame read error: %d out of %d\n", i, NumFrame);
		}	
		for (int j = 0; j < MetaInfo.fmt_nChannels; j++) {
			FrameMat[i * MetaInfo.fmt_nChannels + j] = EachFrame[j];
		}
	}

	fclose(f);
	delete[] EachFrame;
	printf("total: %d bytes\n", BytesTotal);
	return BytesTotal;
}

size_t Cwav::ExtractData_16b(const char * FilePath, float * FrameMat) {
	// update the meta information
	size_t MetaInfoTotalBytes = ExtractMetaInfo(FilePath);
	uint8_t * flush = (uint8_t *)malloc(MetaInfoTotalBytes * sizeof(uint8_t));

	// read data out to buffer
	size_t BytesTotal = 0;
	int16_t * Sample = new int16_t[MetaInfo.fmt_nChannels];
	float * EachFrame = new float [MetaInfo.fmt_nChannels];

	FILE * f;
	errno_t ERR = fopen_s(&f, FilePath, "rb");

	// flush the meta info block
	BytesTotal += fread(flush, sizeof(uint8_t), MetaInfoTotalBytes, f);
	free(flush);
	//printf("bytes flushed: %d\n", BytesTotal);
	//printf("number of channels: %d\n", MetaInfo.fmt_nChannels);
	//printf("number of frames %d, channels %d\n", NumFrame, NumChannel);
	
	//read PCM data to buffers
	//there are 2 cases: mono and stereo
	for (int i = 0; i < NumFrame; i++) {
		BytesTotal += fread(Sample, sizeof(int16_t), MetaInfo.fmt_nChannels, f) * sizeof(int16_t);
		for (int j = 0; j < MetaInfo.fmt_nChannels; j++) {
			EachFrame[j] = (float)((double)Sample[j] / (double)32768);
			FrameMat[i * MetaInfo.fmt_nChannels + j] = EachFrame[j];
		}
		//if(i >= 0 && i < 8) printf("%d  %d\n", Sample[0], Sample[1]);
	}
	fclose(f);
	delete[] EachFrame;
	delete[] Sample;
	printf("total: %d bytes\n", BytesTotal);
	return BytesTotal;
}

size_t Cwav::ExtractData_24b(const char * FilePath, float * FrameMat) {
	// update the meta information
	size_t MetaInfoTotalBytes = ExtractMetaInfo(FilePath);
	uint8_t * flush = (uint8_t *)malloc(MetaInfoTotalBytes * sizeof(uint8_t));

	// read data out to buffer
	size_t BytesTotal = 0;
	uint8_t Sample[3];
	float * EachFrame = new float[MetaInfo.fmt_nChannels];

	FILE * f;
	errno_t ERR = fopen_s(&f, FilePath, "rb");

	// flush the meta info block
	BytesTotal += fread(flush, sizeof(uint8_t), MetaInfoTotalBytes, f);
	free(flush);

	//read PCM data to buffers
	//there are 2 cases: mono and stereo
	for (int i = 0; i < NumFrame; i++) {
		for (int j = 0; j < MetaInfo.fmt_nChannels; j++) {
			BytesTotal += fread(Sample, sizeof(uint8_t), 3, f);
			int32_t Word24b = 0x0;
			for (int k = 2; k >= 0; k--) {
				Word24b = (Word24b | Sample[k]) << 8;
			}
			EachFrame[j] = (float)((double)Word24b / (double)0x80000000);
		}
		//if (i >= 0 && i < 8) printf("%f  %f\n", EachFrame[0], EachFrame[1]);
		for (int j = 0; j < MetaInfo.fmt_nChannels; j++) {
			FrameMat[i * MetaInfo.fmt_nChannels + j] = EachFrame[j];
		}
	}

	fclose(f);
	delete[] EachFrame;
	return BytesTotal;
	//printf("total: %d bytes\n", BytesTotal);
}



float * Cwav::GetFrameMatrix(const char * FilePath) {

	// wFormatTag == 0x0001  PCM
	//               0x0003  IEEE_FLOAT
	//               0x0006  A-LAW
	//               0x0007  u-LAW
	//               0xFFFE  EXTENSIBLE
	//
	// nBlockAlign     4  / 6  / 8
	// wBitsPerSample  16 / 24 / 32
	if (FrameMatrix != NULL) {
		return NULL;	
	}
	size_t MetaInfoBytesAll = ExtractMetaInfo(FilePath);
	printf("wav header: %d bytes\n", MetaInfoBytesAll);
	//PrintMetaInfo();
	

	// allocation of space for frame matrix
	FrameMatrix = new float[NumFrame * NumChannel];
	if (FrameMatrix == NULL) printf("alloc error!\n");	
	
	// dispatch tree based on header info
	size_t BytesRead = 0;
	if (MetaInfo.fmt_wFormatTag == 0x0003) {
		BytesRead = ExtractData_flt(FilePath, FrameMatrix);
	}
	else if (MetaInfo.fmt_wFormatTag == 0x0001) {
		if (MetaInfo.fmt_wBitsPerSample == 16) {
			BytesRead = ExtractData_16b(FilePath, FrameMatrix);
		}
		else if (MetaInfo.fmt_wBitsPerSample == 24) {
			BytesRead = ExtractData_24b(FilePath, FrameMatrix);
		}
		else {
			printf("PCM Error: bits/sample == %d not supported\n", MetaInfo.fmt_wBitsPerSample);
		}
	}
	else if ((MetaInfo.fmt_wFormatTag == 0xFFFE) && (MetaInfo.fmt_SubFormat[1] == 0x00) && (MetaInfo.fmt_SubFormat[0] == 0x03)) {
		BytesRead = ExtractData_flt(FilePath, FrameMatrix);
	}
	else if ((MetaInfo.fmt_wFormatTag == 0xFFFE) && (MetaInfo.fmt_SubFormat[1] == 0x00) && (MetaInfo.fmt_SubFormat[0] == 0x01)) {
		if (MetaInfo.fmt_wBitsPerSample == 16) {
			BytesRead = ExtractData_16b(FilePath, FrameMatrix);
		}
		else if (MetaInfo.fmt_wBitsPerSample == 24) {
			BytesRead = ExtractData_24b(FilePath, FrameMatrix);
		}
		else {
			printf("PCM Error: bits/sample == %d not supported\n", MetaInfo.fmt_wBitsPerSample);
		}
	}
	else {
		printf("Read Error: A-Law / u-Law not supported: %x \n", MetaInfo.fmt_wFormatTag);
	}
	printf("bytes read from file: %ld\n", BytesRead);
	return FrameMatrix;
}


float * Cwav::SetFrameMatrix(int nFrame, int nChannel, int nSampleRate)
{
	if (FrameMatrix != NULL) {
		return NULL;
	}
	NumChannel = nChannel;
	SampleRate = nSampleRate;
	NumFrame = nFrame;

	// allocation of space for frame matrix
	FrameMatrix = new float[NumFrame * NumChannel];
	if (FrameMatrix == NULL) {
		printf("alloc error!\n");
	}
	else {
		std::memset(FrameMatrix, 0, sizeof(float) * NumFrame * NumChannel);
	}
	return FrameMatrix;
}



// fmt == 16, 16-bit PCM
// fmt == 24, 24-bit PCM
// fmt == 32, IEEE float
void Cwav::MakeMetaInfo(int nChannel, int nSampleRate, int nSample, int fmt) {
	
	// RIFF MASTER CHUNK 
	MetaInfo.riff_ckID[0] = 'R';
	MetaInfo.riff_ckID[1] = 'I';
	MetaInfo.riff_ckID[2] = 'F';
	MetaInfo.riff_ckID[3] = 'F';
	MetaInfo.riff_cksize = 0;     //update place holder
									 
	MetaInfo.riff_waveID[0] = 'W';
	MetaInfo.riff_waveID[1] = 'A';
	MetaInfo.riff_waveID[2] = 'V';
	MetaInfo.riff_waveID[3] = 'E';

		// FORMAT CHUNK
		MetaInfo.fmt_ckID[0] = 'f';
		MetaInfo.fmt_ckID[1] = 'm';
		MetaInfo.fmt_ckID[2] = 't';
		MetaInfo.fmt_ckID[3] = ' ';
		MetaInfo.fmt_cksize = 18;

		if (fmt == 32) {
			MetaInfo.fmt_wFormatTag = 0x0003;
		}
		else if (fmt == 16 || fmt == 24) {
			MetaInfo.fmt_wFormatTag = 0x0001;
		}
		else {
			MetaInfo.fmt_wFormatTag = 0x0000;
			printf("Error: we don't support formats beyond PCM and FLOAT!\n");
		}
		
		MetaInfo.fmt_nChannels = nChannel;
		MetaInfo.fmt_nSamplesPerSec = nSampleRate;
		MetaInfo.fmt_nAvgbytesPerSec = nSampleRate * nChannel * (fmt/8);
		MetaInfo.fmt_nBlockAlign = nChannel * (fmt/8);
		MetaInfo.fmt_wBitsPerSample = fmt;
		MetaInfo.fmt_cbSize = 0;

			// FACT CHUNK 
			MetaInfo.fact_ckID[0] = 'f';
			MetaInfo.fact_ckID[1] = 'a';
			MetaInfo.fact_ckID[2] = 'c';
			MetaInfo.fact_ckID[3] = 't';
			MetaInfo.fact_cksize = 4;

			MetaInfo.fact_dwSampleLength = 0; //update place holder

				// DATA CHUNK 
				MetaInfo.data_ckID[0] = 'd';
				MetaInfo.data_ckID[1] = 'a';
				MetaInfo.data_ckID[2] = 't';
				MetaInfo.data_ckID[3] = 'a';
				MetaInfo.data_cksize = 0;     //update place holder   

	//fill place holder information
	MetaInfo.riff_cksize = 4 + (8 + 18) + (8 + 4) + (8 + ((fmt/8) * nChannel * nSample));
	MetaInfo.fact_dwSampleLength = nSample;
	MetaInfo.data_cksize = (fmt/8) * nChannel * nSample;

}



size_t Cwav::SaveMetaInfo(const char * FilePath) {

	size_t BytesWritten = 0;
	FILE * f;
	errno_t Err = fopen_s(&f, FilePath, "wb");

	//RIFF header
	BytesWritten += fwrite(MetaInfo.riff_ckID,      sizeof(uint8_t),  4, f);
	BytesWritten += fwrite(&(MetaInfo.riff_cksize), sizeof(uint32_t), 1, f) * sizeof(uint32_t);
	BytesWritten += fwrite(MetaInfo.riff_waveID,    sizeof(uint8_t),  4, f);

	//FORMAT header
	BytesWritten += fwrite(MetaInfo.fmt_ckID,          sizeof(uint8_t),  4, f);
	BytesWritten += fwrite(&(MetaInfo.fmt_cksize),     sizeof(uint32_t), 1, f) * sizeof(uint32_t);
	BytesWritten += fwrite(&(MetaInfo.fmt_wFormatTag), sizeof(uint16_t), 1, f) * sizeof(uint16_t);

	BytesWritten += fwrite(&(MetaInfo.fmt_nChannels),       sizeof(uint16_t), 1, f) * sizeof(uint16_t);
	BytesWritten += fwrite(&(MetaInfo.fmt_nSamplesPerSec),  sizeof(uint32_t), 1, f) * sizeof(uint32_t);
	BytesWritten += fwrite(&(MetaInfo.fmt_nAvgbytesPerSec), sizeof(uint32_t), 1, f) * sizeof(uint32_t);
	BytesWritten += fwrite(&(MetaInfo.fmt_nBlockAlign),     sizeof(uint16_t), 1, f) * sizeof(uint16_t);
	BytesWritten += fwrite(&(MetaInfo.fmt_wBitsPerSample),  sizeof(uint16_t), 1, f) * sizeof(uint16_t);
	BytesWritten += fwrite(&(MetaInfo.fmt_cbSize),          sizeof(uint16_t), 1, f) * sizeof(uint16_t);

	//FACT header
	BytesWritten += fwrite(MetaInfo.fact_ckID,              sizeof(uint8_t),  4, f);
	BytesWritten += fwrite(&(MetaInfo.fact_cksize),         sizeof(uint32_t), 1, f) * sizeof(uint32_t);
	BytesWritten += fwrite(&(MetaInfo.fact_dwSampleLength), sizeof(uint32_t), 1, f) * sizeof(uint32_t);

	//DATA
	BytesWritten += fwrite(MetaInfo.data_ckID,      sizeof(uint8_t),  4, f);
	BytesWritten += fwrite(&(MetaInfo.data_cksize), sizeof(uint32_t), 1, f) * sizeof(uint32_t);

	fclose(f);
	return BytesWritten;
}




size_t Cwav::Save2File_flt(const char * FilePath)
{
	size_t BytesWritten = 0;
	MakeMetaInfo(NumChannel, SampleRate, NumFrame, 32);
	BytesWritten = SaveMetaInfo(FilePath);

	//add data body
	float * data = new float[NumChannel];
	FILE * f;
	errno_t Err = fopen_s(&f, FilePath, "ab");
	for (int i = 0; i < NumFrame; i++) {
		for (int j = 0; j < NumChannel; j++) {
			data[j] = FrameMatrix[i * NumChannel + j];
		}
		BytesWritten += fwrite(data, sizeof(float), NumChannel, f) * sizeof(float);
	}
	fclose(f);
	delete[] data;
	return BytesWritten;
}

size_t Cwav::Save2File_16b(const char * FilePath)
{
	size_t BytesWritten = 0;
	MakeMetaInfo(NumChannel, SampleRate, NumFrame, 16);
	BytesWritten = SaveMetaInfo(FilePath);

	//add data body
	int16_t * data = new int16_t[NumChannel];
	FILE * f;
	errno_t Err = fopen_s(&f, FilePath, "ab");
	for (int i = 0; i < NumFrame; i++) {
		for (int j = 0; j < NumChannel; j++) {
			data[j] = (int16_t)(FrameMatrix[i * NumChannel + j] * 32768.0);
		}
		BytesWritten += fwrite(data, sizeof(int16_t), NumChannel, f) * sizeof(int16_t);
	}
	fclose(f);
	delete[] data;
	return BytesWritten;
}

size_t Cwav::Save2File_24b(const char * FilePath)
{
	size_t BytesWritten = 0;
	MakeMetaInfo(NumChannel, SampleRate, NumFrame, 24);
	BytesWritten = SaveMetaInfo(FilePath);

	//add data body
	int32_t data;
	FILE * f;
	errno_t Err = fopen_s(&f, FilePath, "ab");
	for (int i = 0; i < NumFrame; i++) {
		for (int j = 0; j < NumChannel; j++) {
			data = (int32_t)(FrameMatrix[i * NumChannel + j] * 8388608.0);
			BytesWritten += fwrite(&data, sizeof(uint8_t), 3, f);
		}
	}
	fclose(f);
	return BytesWritten;
}





// the trimmed slice has the same sample rate and number of channels as the input wav file
// so we don't use channel or sample rate info here
size_t Cwav::Save2File_16b(const char * FilePath, double Start, double Stop) {

	size_t BytesWritten = 0;

	// calculate slice to be saved
	// FrameMatrix[IdxStart][Ch] .. FrameMatrix[IdxStop-1][ch]
	int IdxStart = (int)(Start * (double)SampleRate);
	int IdxStop = (int)(Stop * (double)SampleRate);
	if (IdxStart >= IdxStop || IdxStop > NumFrame) {
		printf("Error: illegal time stamp found!\n");
	}

	MakeMetaInfo(NumChannel, SampleRate, IdxStop - IdxStart, 16);
	BytesWritten = SaveMetaInfo(FilePath);

	//add data body
	int16_t * data = new int16_t[NumChannel];
	FILE * f;
	errno_t Err = fopen_s(&f, FilePath, "ab");
	for (int i = IdxStart; i < IdxStop; i++) {
		for (int j = 0; j < NumChannel; j++) {
			data[j] = (int16_t)(FrameMatrix[i * NumChannel + j] * 32768.0);
		}
		BytesWritten += fwrite(data, sizeof(int16_t), NumChannel, f) * sizeof(int16_t);
	}
	fclose(f);
	delete[] data;
	return BytesWritten;
}


size_t Cwav::Save2File_24b(const char * FilePath, double Start, double Stop) {

	size_t BytesWritten = 0;

	// calculate slice to be saved
	// FrameMatrix[IdxStart][Ch] .. FrameMatrix[IdxStop-1][ch]
	int IdxStart = (int)(Start * (double)SampleRate);
	int IdxStop = (int)(Stop * (double)SampleRate);
	if (IdxStart >= IdxStop || IdxStop > NumFrame) {
		printf("Error: illegal time stamp found!\n");
	}

	MakeMetaInfo(NumChannel, SampleRate, IdxStop - IdxStart, 24);
	BytesWritten = SaveMetaInfo(FilePath);

	//add data body
	int32_t data;
	FILE * f;
	errno_t Err = fopen_s(&f, FilePath, "ab");
	for (int i = IdxStart; i < IdxStop; i++) {
		for (int j = 0; j < NumChannel; j++) {
			data = (int32_t)(FrameMatrix[i * NumChannel + j] * 8388608.0);
			BytesWritten += fwrite(&data, sizeof(uint8_t), 3, f);
		}
	}
	fclose(f);
	return BytesWritten;
}


size_t Cwav::Save2File_flt(const char * FilePath, double Start, double Stop) {

	size_t BytesWritten = 0;
	
	// calculate slice to be saved
	// FrameMatrix[IdxStart][Ch] .. FrameMatrix[IdxStop-1][ch]
	int IdxStart = (int)(Start * (double)SampleRate);
	int IdxStop = (int)(Stop * (double)SampleRate);
	if (IdxStart >= IdxStop || IdxStop > NumFrame) {
		printf("Error: illegal time stamp found!\n");
	}

	MakeMetaInfo(NumChannel, SampleRate, IdxStop - IdxStart, 32);
	BytesWritten = SaveMetaInfo(FilePath);

	//add data body
	float * data = new float[NumChannel];
	FILE * f;
	errno_t Err = fopen_s(&f, FilePath, "ab");
	for (int i = IdxStart; i < IdxStop; i++) {
		for (int j = 0; j < NumChannel; j++) {
			data[j] = FrameMatrix[i * NumChannel + j];
		}
		BytesWritten += fwrite(data, sizeof(float), NumChannel, f) * sizeof(float);
	}
	fclose(f);
	delete[] data;
	return BytesWritten;
}
