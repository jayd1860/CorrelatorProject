
struct AnalogData
{
	AnalogData(int segSize, int nCh0)
	{
		nCh = nCh0;
		maxsize = segSize / 6;

		t = new unsigned long long[maxsize];
		memset(t, 0, sizeof(unsigned long long) * maxsize);

		amp = new unsigned long long*[nCh];
		for(int ii=0; ii < nCh; ii++)
		{
			amp[ii] = new unsigned long long[maxsize];
			memset(amp[ii], 0, sizeof(unsigned long long) * maxsize);
		}

		chIdxOrder = new int[nCh];
		chIdxOrder[0] = 4;  // get data for ADC 1 from word 4
		chIdxOrder[1] = 2;  // get data for ADC 2 from word 2
		chIdxOrder[2] = 5;  // get data for ADC 3 from word 5
		chIdxOrder[3] = 3;  // get data for ADC 4 from word 3

		count = 0;
	}

	int nCh;
	unsigned long long* t;
	unsigned long long** amp;
	unsigned long count;
	unsigned long countTotal;
	int* chIdxOrder;
	int maxsize;

};



struct LightIntensityData
{
	LightIntensityData(int segSize, int nCh0)
	{
		nCh = nCh0;
		maxsize = segSize;

		arrtimes = new unsigned long long**[nCh];
		for(int ii=0; ii < nCh; ii++)
		{
			arrtimes[ii] = new unsigned long long*[2];
			arrtimes[ii][0] = new unsigned long long[maxsize];
			arrtimes[ii][1] = new unsigned long long[maxsize];

			memset(arrtimes[ii][0], 0, sizeof(unsigned long long) * maxsize);
			memset(arrtimes[ii][1], 0, sizeof(unsigned long long) * maxsize);
		}
		countTotal = new unsigned long[nCh];
		memset(countTotal, 0, nCh * sizeof(unsigned long));

		countTotalDropped = new unsigned long[nCh];
		memset(countTotalDropped, 0, nCh * sizeof(unsigned long));

		countThreshold = 1e6;
	}


	int nCh;
	unsigned long long*** arrtimes;
	unsigned long* countTotal;
	unsigned long* countTotalDropped;
	int countThreshold;
	int maxsize;
};



class Fx3DataParser
{
public:
	Fx3DataParser(int segSize);
	~Fx3DataParser();

	void parseData(unsigned short* dataraw, int n);
	void ClearBuffers();

public:
	int nCh_dcs;
	int nCh_adc;
	unsigned long   maxsize;
	unsigned long   macrotime;
	unsigned long long   macrotimeADC;
	unsigned long   macrotimeADCPrev;
	unsigned long   macrotimeDCS;
	unsigned short* word;
	unsigned long long* wrapcount;
	LightIntensityData* dataDCS;
	AnalogData*         dataADC;
};


