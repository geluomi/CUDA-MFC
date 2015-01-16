
#include <vector>
#include <string>
#include <math.h>	
#include <stdio.h>
#include <stdlib.h>
using namespace std;

#define  PI 3.1415926
#define  MAXBLOBNUMBER 1024
#define  HORLINE          1      //-15~+15
#define  VERLINE          2      //-75~+75 
#define  UPSLOPELINE      3      //+15~+75
#define  DOWNSLOPELINE    4      //-75~-15  
#define  CURVELINE        5    
#define ANGLECLASS_THRESHOLD     75 

struct Coord {
	int x;
	int y;
};

struct Element {
	//vector<struct Element> neighbs;
	struct Coord coord;
};
typedef struct Positionxy{
	int x;
	int y;
} Positionxy;


struct LineAttributeBlob {
	int  sortcount;
	int  linetype;
	int  slopetype;
	bool checkflag;
	bool proxmityflag;
	bool colineflag;
	bool cocurveflag;
	bool Parallelflag;
	bool crossflag;
	bool directflag;

	unsigned int elements_number;
	vector<struct Element> elements;

	int    leftx, lefty;   //下面四个点只是blob区域的最大外接矩形的个点
	int    topx, topy;
	int    rightx, righty;
	int    bottomx, bottomy;
	Coord  startP;
	Coord  endP;
	Coord  centerP;
	double slope;
	double intercept;
	double SegmentSlope;

	double startslope;
	double startintercept;
	double endslope;
	double endintercept;

	int    length;
	int    linewidth;
	int    BlobNumber;
};


class  Edgeanalysis{

protected:

	unsigned char   *m_image;
	int              Blobtotal;
	int              Blobcount;

	Positionxy       Queuelist[16384];
	int              Distlist[16384];
	double           Slopelist[16384];
	double           Distslopelist[16384];
	int              Indexlist[16394];
	int              Qfront, Qrear, Qcount;
	static const int MaxQsize;               //=1024;
	int              pixelcount;
	struct Element   element;
	int              selx[49], sely[49];
	int              tempx, tempy, tempdist;
public:

	Edgeanalysis();
	~Edgeanalysis();

	void init(int w, int h);

	void    clear_blob(LineAttributeBlob *blob, int maxblobnumber);

	void Mark_BlackEdgever(unsigned char *image, unsigned char *markbmpver, int msize);
	void Mark_BlackEdgehor(unsigned char *image, unsigned char *markbmphor, int msize);
	void Mark_WhiteEdgehor(unsigned char *image, unsigned char *markbmphor, int msize);
	void Mark_WhiteEdgever(unsigned char *image, unsigned char *markbmpver, int msize);
	int     search_lineblob(unsigned char *image, LineAttributeBlob *blob, int numthreshold);
	void    CopyBlob(LineAttributeBlob *srcblob, LineAttributeBlob *dstblob, int sortcount);
	int     DeisionDir(double slope);
	void    cal_blobattri(LineAttributeBlob *eachblob);
	double  Bottom_Perceptual(unsigned char *GrayImage, LineAttributeBlob *CurveBlob, unsigned char *maskimage, bool blackorwhite, bool gpuorcpu, int w, int h);
	int     chaincode_comp(unsigned char *chainmask, Coord left, Coord right, Coord top, Coord bottom, unsigned char *maskimage);
	int     chaincode_comph(unsigned char *chainmask, Coord left, Coord right, Coord top, Coord bottom, unsigned char *maskimage);

public:
	int m_width, m_height;
	unsigned char *BlackHorEdge;
	unsigned char *BlackVerEdge;
	unsigned char *WhiteHorEdge;
	unsigned char *WhiteVerEdge;
	unsigned char *WhiteHorEdger;
	unsigned char *WhiteVerEdger;
	unsigned char *BlackVerEdger;
	unsigned char *BlackHorEdger;
	FILE *fp_chain;
	std::vector<unsigned char>clockcode;                //因为在函数中直接用到，所以最好申明为类成员函数
	std::vector<unsigned char>anticlockcode;
	std::vector<unsigned char>anticlockcode1;
	std::vector<unsigned char>chaincode;
	std::vector<struct Coord >chainposition;
	LineAttributeBlob HorBlob[MAXBLOBNUMBER];
	LineAttributeBlob VerBlob[MAXBLOBNUMBER];







};
