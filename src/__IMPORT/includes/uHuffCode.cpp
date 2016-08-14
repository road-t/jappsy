#include <malloc.h>
#include "core\uMemory.h"
#include "uHuffCode.h"

struct hufNodeStruc {
	unsigned long freq;
	int parent;
	int right;
	int left;
};

struct hufCodeStruc {
	unsigned char bitContainer;
	unsigned char bitCounter;
	int bit_count;
	int numOfRootNode;
	hufNodeStruc ourTree[511];
	unsigned char encodedCode[256];
	char* oBuf;
	unsigned long oPos;
	unsigned long origBytes;
	unsigned long activeSymbs;

	void buildHufTree();
	void outputBit(int bit);
	void compressByte(int node, int child);
	void encode(char *inBuf, unsigned long inSize, char *outBuf, unsigned long *outSize);
	void decode(char *inBuf, unsigned long inSize, char *outBuf, unsigned long *outSize);
};

// Построить дерево хуфмана и вернуть колличество родительских узлов
void hufCodeStruc::buildHufTree() {
	int nodeCounter = 256;
	int i;
	for (i = 0; i < nodeCounter; i++) {
		ourTree[i].parent = -1;
		ourTree[i].right = -1;
		ourTree[i].left = -1;
	}
	while (1) {
		int minFreq0 = -1;
		int minFreq1 = -1;
		for (i = 0; i < nodeCounter; i++) {
			if (i != minFreq0) {
				if (ourTree[i].freq > 0 && ourTree[i].parent == -1) {
					if (minFreq0 == -1 || ourTree[i].freq < ourTree[minFreq0].freq) {
						if (minFreq1 == -1 || ourTree[i].freq < ourTree[minFreq1].freq) minFreq1 = minFreq0;
						minFreq0 = i;
					} else if (minFreq1 == -1 || ourTree[i].freq < ourTree[minFreq1].freq) minFreq1 = i;
				}
			}
		}
		if (minFreq1 == -1)	{
			numOfRootNode = minFreq0;
			break;
		}
		// объединить два узла для формирования родительского узла
		ourTree[minFreq0].parent = nodeCounter;
		ourTree[minFreq1].parent = nodeCounter;
		ourTree[nodeCounter].freq = ourTree[minFreq0].freq + ourTree[minFreq1].freq;
		ourTree[nodeCounter].right = minFreq0;
		ourTree[nodeCounter].left = minFreq1;
		ourTree[nodeCounter].parent = -1;
		nodeCounter++;
	}
}

// Записать 1 бит данных в память
void hufCodeStruc::outputBit(int bit) {
	if (bitCounter == 8 || bit == -1) {
		if (bit == -1) bitContainer <<= (8-bitCounter);
		bitCounter = 0;
		if (oBuf)
			*(unsigned char*)(oBuf+oPos) = bitContainer;
		oPos++;
	}
	bitContainer <<= 1;
	bitContainer |= bit;
	bitCounter++;
}

// Сжать 1 байт данных
void hufCodeStruc::compressByte(int node, int child) {
	if (ourTree[node].parent != -1) {
		bit_count++;
		compressByte(ourTree[node].parent, node);
	}
	if (child != -1) {
		if (child == ourTree[node].right) outputBit(0);
		else if (child == ourTree[node].left) outputBit(1);
	}
}

void hufCodeStruc::encode(char *inBuf, unsigned long inSize, char *outBuf, unsigned long *outSize) {
	char c;
	unsigned char	uc;
	int				idx = 0;
	unsigned long	iPos = 0;
	origBytes		= 0;
	activeSymbs		= 0;
	bitContainer	= 0;
	bitCounter		= 0;
	numOfRootNode	= 0;
	oBuf = outBuf;
	oPos = 0;
	for (int i = 0; i < 511; i++)
		ourTree[i].freq = 0;
	while (iPos < inSize) {
		c = inBuf[iPos++];
		uc = static_cast<unsigned char>(c);
		if (ourTree[uc].freq == 0) activeSymbs++;
		ourTree[uc].freq++;
		origBytes++;
	}
	if (oBuf) *((unsigned long*)&(oBuf[oPos])) = origBytes;
	oPos += sizeof(unsigned long);
	if (oBuf) *((int*)&(oBuf[oPos])) = activeSymbs;
	oPos += sizeof(int);
	for (idx = 0; idx < 256; idx++) {
		if (ourTree[idx].freq > 0) {
			uc = static_cast<char>(idx);
			if (oBuf) oBuf[oPos] = uc;
			oPos++;
			if (oBuf) *((unsigned long*)&(oBuf[oPos])) = ourTree[idx].freq;
			oPos += sizeof(unsigned long);
		}
		encodedCode[idx] = 0;
	}
	buildHufTree();
	bit_count = 0;
	iPos = 0;
	while(iPos < inSize) {
		c = inBuf[iPos++];
		uc = static_cast<unsigned char>(c);
		compressByte(uc, -1);
		encodedCode[uc] = bit_count;
		bit_count = 0;
	}
	outputBit(-1);
	*outSize = oPos;
}

void hufCodeStruc::decode(char *inBuf, unsigned long inSize, char *outBuf, unsigned long *outSize) {
	unsigned long iPos = 0;
	oPos = 0;
	oBuf = outBuf;
	origBytes = *((unsigned long*)&(inBuf[iPos]));
	iPos += sizeof(unsigned long);
	activeSymbs = *((int*)&(inBuf[iPos]));
	iPos += sizeof(int);
	unsigned long temp1 = origBytes;
	int temp2 = activeSymbs;
	char c;
	unsigned char uc;
	for (int i = 0; i < 511; i++)
		ourTree[i].freq = 0;
	while (temp2--) {
		c = inBuf[iPos++];
		uc = static_cast<unsigned char>(c);
		ourTree[uc].freq = *((unsigned long*)&(inBuf[iPos]));
		iPos += sizeof(unsigned long);
	}
	buildHufTree();
	bitCounter = 0;
	while (temp1--) {
		int numOfTgtSymb;
		numOfTgtSymb = numOfRootNode;
		while (ourTree[numOfTgtSymb].right != -1) {
			if (bitCounter == 0) {
				if (iPos >= inSize) return;
				c = inBuf[iPos++];
				bitContainer = static_cast<unsigned char>(c);
				bitCounter = 8;
			}
			if (bitContainer & 0x80) numOfTgtSymb = ourTree[numOfTgtSymb].left;
			else numOfTgtSymb = ourTree[numOfTgtSymb].right;
			bitContainer <<= 1;
			bitCounter--;
		}
		if (oBuf) oBuf[oPos] = numOfTgtSymb;
		oPos++;
	}
	*outSize = oPos;
}

void* HUF_encode(const void* in, unsigned long insize, unsigned long* outsize) {
	hufCodeStruc huf;
	unsigned long size = 0;
	huf.encode((char*)in,insize,(char*)0,&size);
	void* out = (void*)0;
	if (size > 0) {
		out = memAlloc(void,out,size);
	}
	if (out) {
		huf.encode((char*)in,insize,(char*)out,outsize);
	}
	return out;
}

void* HUF_encode(const void* in, unsigned long insize, void* out, unsigned long* outsize) {
	hufCodeStruc huf;
	huf.encode((char*)in,insize,(char*)out,outsize);
	return out;
}

void* HUF_decode(const void* in, unsigned long insize, unsigned long* outsize) {
	hufCodeStruc huf = {0};
	unsigned long size = 0;
	huf.decode((char*)in,insize,(char*)0,&size);
	void* out = (void*)0;
	if (size > 0) {
		out = memAlloc(void,out,size);
	}
	if (out) {
		huf.decode((char*)in,insize,(char*)out,outsize);
	}
	return out;
}

void* HUF_decode(const void* in, unsigned long insize, void* out, unsigned long* outsize) {
	hufCodeStruc huf = {0};
	huf.decode((char*)in,insize,(char*)out,outsize);
	return out;
}

