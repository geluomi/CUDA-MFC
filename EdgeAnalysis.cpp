#include "stdafx.h"
#include "EdgeAnalysis.h"
#include "cuda_converter.h"
#include "windows.h"
cuda_converter  cudaconverter;            //新建一个CUDA类实例，便于调用CUDA函数
cuda_converter  cudachaincode;               //两个不同的类对象分别对应十字模板并行和链码分析并行
#define WIDTH      720
#define HEIGHT     576
const int Edgeanalysis::MaxQsize = 1024;



Edgeanalysis::Edgeanalysis()
{

}


Edgeanalysis::~Edgeanalysis()
{

	delete BlackVerEdge;
	delete BlackVerEdger;
	delete BlackHorEdger;
	delete BlackHorEdge;

	delete WhiteHorEdge;
	delete WhiteVerEdge;
	delete WhiteHorEdger;
	delete WhiteVerEdger;
	
}

LONGLONG atime;
LONGLONG gtime;
//测试开始时间函数
void starttime(LONGLONG* start)
{
	QueryPerformanceCounter((LARGE_INTEGER *)start);
}

double stoptime(LONGLONG* start, char* title)
{
	//char test[100];
	LONGLONG stop, persecond;
	double timeuse;

	QueryPerformanceFrequency((LARGE_INTEGER *)&persecond);
	QueryPerformanceCounter((LARGE_INTEGER *)&stop);

	timeuse = (double)(stop - *start) / (double)persecond;

	return timeuse;

}
void Edgeanalysis::init(int w, int h)
{
	m_height = h;
	m_width = w;
	int size = m_width*m_height;


	BlackVerEdge = new unsigned char[size];
	BlackVerEdger = new unsigned char[size];
	BlackHorEdger = new unsigned char[size];
	BlackHorEdge = new unsigned char[size];

	WhiteHorEdge  = new unsigned char[size];
	WhiteVerEdge  = new unsigned char[size];
	WhiteHorEdger = new unsigned char[size];
	WhiteVerEdger = new unsigned char[size];
	
}



void Edgeanalysis::Mark_BlackEdgever(unsigned char *image, unsigned char *markbmpver, int msize)
{
	//水平模板 针对垂直线
	unsigned char *R = markbmpver;
	unsigned char *Y = image;
	int i, j, Y0, s1, s2, sn, snedge, sum1, sum2, sntemp1, sntemp2;
	int T = 4;
	int masksize = msize;

	for (i = 0; i<m_height; i++)
	{
		for (j = 0; j<masksize / 2; j++)
		{
			R[i*m_width + j] = 0;
		}
		for (int j2 = m_width - masksize / 2; j2<m_width; j2++)
		{
			R[i*m_width + j2] = 0;
		}
	}
	for (i = 0; i<m_height; i++)
	{
		for (j = masksize / 2; j<m_width - masksize / 2; j++)
		{
			Y0 = Y[i*m_width + j];
			s1 = 0;
			s2 = 0;
			sum1 = 0;
			sum2 = 0;
			sntemp1 = 0;
			sntemp2 = 0;

			for (int k = -masksize / 2; k<0; k++)
			{
				s1 = s1 + Y[i*m_width + j + masksize / 2] - Y0;
				s2 = s2 + Y[i*m_width + j - masksize / 2] - Y0;
				sum1 = sum1 + Y[i*m_width + j + masksize / 2];
				sum2 = sum2 + Y[i*m_width + j - masksize / 2];

				if (abs(Y[i*m_width + j + masksize / 2] - Y0) <= T)
				{
					sntemp1 = sntemp1 + 1;
				}
				if (abs(Y[i*m_width + j - masksize / 2] - Y0) <= T)
				{
					sntemp1 = sntemp1 + 1;
				}
				if (abs(Y[i*m_width + j + masksize / 2] - Y0) <= T)
				{
					sntemp2 = sntemp2 + 1;
				}
				if (abs(Y[i*m_width + j - masksize / 2] - Y0) <= T)
				{
					sntemp2 = sntemp2 + 1;
				}
			}
			sn = sntemp1;
			snedge = sntemp2;

			if (s1>0 && s2>0 && sn<masksize / 2)         //线对象
			{
				R[i*m_width + j] = 1;
			}
			//else if (sum1>sum2 && snedge==masksize/2)  //左边缘
			//{ 
			//	R[i*m_width+j]=2;
			//}
			//else if (sum1<sum2 && snedge==masksize/2)  //右边缘
			//{
			//	R[i*m_width+j]=3;
			//}
			else
			{
				R[i*m_width + j] = 0;
			}
		}
	}
}
void Edgeanalysis::Mark_BlackEdgehor(unsigned char *image, unsigned char *markbmphor, int msize)
{
	int i, j, Y0, s1, s2, sn, snedge, sum1, sum2, sntemp1, sntemp2;
	int T = 4;
	int masksize = msize;

	//	int masksize=13;                //masksize大于线对象宽度的两倍
	//	int masksize2=9;

	unsigned char *R = markbmphor;
	unsigned char *Y = image;

	// 循环两次方式

	// 垂直模板 针对水平线

	for (j = 0; j < m_width; j++)
	{
		for (i = 0; i < masksize / 2; i++)
		{
			R[i*m_width + j] = 0;
		}
		for (i = m_height - masksize / 2; i < m_height; i++)
		{
			R[i*m_width + j] = 0;
		}
	}

	for (i = masksize / 2; i < m_height - masksize / 2; i++)
	{
		for (j = 0; j < m_width; j++)
		{
			Y0 = Y[i*m_width + j];
			s1 = 0;
			s2 = 0;
			sum1 = 0;
			sum2 = 0;
			sntemp1 = 0;
			sntemp2 = 0;

			for (int k = -masksize / 2; k < 0; k++)
			{
				s1 = s1 + Y[(i + masksize / 2)*m_width + j] - Y0;
				s2 = s2 + Y[(i - masksize / 2)*m_width + j] - Y0;
				sum1 = sum1 + Y[(i + masksize / 2)*m_width + j];
				sum2 = sum2 + Y[(i - masksize / 2)*m_width + j];

				if (abs(Y[(i + masksize / 2)*m_width + j] - Y0) <= T)
				{
					sntemp1 = sntemp1 + 1;
				}
				if (abs(Y[(i - masksize / 2)*m_width + j] - Y0) <= T)
				{
					sntemp1 = sntemp1 + 1;
				}
				if (abs(Y[(i + masksize / 2)*m_width + j] - Y0) <= T)
				{
					sntemp2 = sntemp2 + 1;
				}
				if (abs(Y[(i - masksize / 2)*m_width + j] - Y0) <= T)
				{
					sntemp2 = sntemp2 + 1;
				}
			}
			sn = sntemp1;
			snedge = sntemp2;

			if (s1 > 0 && s2 > 0 && sn<masksize / 2)         //条带对象
			{
				R[i*m_width + j] = 1;
			}
			//else if (sum1>sum2 && snedge == masksize / 2)  //上边缘
			//{
			//	R[i*m_width + j] = 2;
			//}
			//else if (sum1 < sum2 && snedge == masksize / 2)  //下边缘
			//{
			//	R[i*m_width + j] = 3;
			//}
			else
			{
				R[i*m_width + j] = 0;
			}
		}
	}
}


void Edgeanalysis::Mark_WhiteEdgehor(unsigned char *image, unsigned char *markbmphor, int msize)
{
	int i, j, Y0, s1, s2, sn, snedge, sum1, sum2, sntemp1, sntemp2;
	int T = 4;
	int masksize = msize;

	//	int masksize=13;                //masksize大于线对象宽度的两倍
	//	int masksize2=9;

	unsigned char *R = markbmphor;
	unsigned char *Y = image;

	// 循环两次方式

	// 垂直模板 针对水平线

	for (j = 0; j < m_width; j++)
	{
		for (i = 0; i < masksize / 2; i++)
		{
			R[i*m_width + j] = 0;
		}
		for (i = m_height - masksize / 2; i < m_height; i++)
		{
			R[i*m_width + j] = 0;
		}
	}

	for (i = masksize / 2; i < m_height - masksize / 2; i++)
	{
		for (j = 0; j < m_width; j++)
		{
			Y0 = Y[i*m_width + j];
			s1 = 0;
			s2 = 0;
			sum1 = 0;
			sum2 = 0;
			sntemp1 = 0;
			sntemp2 = 0;

			for (int k = -masksize / 2; k < 0; k++)
			{
				s1 = s1 + Y0 - Y[(i + masksize / 2)*m_width + j];
				s2 = s2 + Y0 - Y[(i - masksize / 2)*m_width + j];
				sum1 = sum1 + Y[(i + masksize / 2)*m_width + j];
				sum2 = sum2 + Y[(i - masksize / 2)*m_width + j];

				if (abs(Y[(i + masksize / 2)*m_width + j] - Y0) <= T)
				{
					sntemp1 = sntemp1 + 1;
				}
				if (abs(Y[(i - masksize / 2)*m_width + j] - Y0) <= T)
				{
					sntemp1 = sntemp1 + 1;
				}
				if (abs(Y[(i + masksize / 2)*m_width + j] - Y0) <= T)
				{
					sntemp2 = sntemp2 + 1;
				}
				if (abs(Y[(i - masksize / 2)*m_width + j] - Y0) <= T)
				{
					sntemp2 = sntemp2 + 1;
				}
			}
			sn = sntemp1;
			snedge = sntemp2;

			if (s1 > 0 && s2 > 0 && sn<masksize / 2)         //条带对象
			{
				R[i*m_width + j] = 1;
			}
			//else if (sum1>sum2 && snedge == masksize / 2)  //上边缘
			//{
			//	R[i*m_width + j] = 2;
			//}
			//else if (sum1 < sum2 && snedge == masksize / 2)  //下边缘
			//{
			//	R[i*m_width + j] = 3;
			//}
			else
			{
				R[i*m_width + j] = 0;
			}
		}
	}
}

void Edgeanalysis::Mark_WhiteEdgever(unsigned char *image, unsigned char *markbmpver, int msize)
{
	//水平模板 针对垂直线
	int i, j, Y0, s1, s2, sn, snedge, sum1, sum2, sntemp1, sntemp2;
	int T = 4;
	int masksize = msize;

	//	int masksize=13;                //masksize大于线对象宽度的两倍
	//	int masksize2=9;

	unsigned char *R = markbmpver;
	unsigned char *Y = image;

	for (i = 0; i<m_height; i++)
	{
		for (j = 0; j<masksize / 2; j++)
		{
			R[i*m_width + j] = 0;
		}
		for (int j2 = m_width - masksize / 2; j2<m_width; j2++)
		{
			R[i*m_width + j2] = 0;
		}
	}
	for (i = 0; i<m_height; i++)
	{
		for (j = masksize / 2; j<m_width - masksize / 2; j++)
		{
			Y0 = Y[i*m_width + j];
			s1 = 0;
			s2 = 0;
			sum1 = 0;
			sum2 = 0;
			sntemp1 = 0;
			sntemp2 = 0;

			for (int k = -masksize / 2; k<0; k++)
			{
				s1 = s1 + Y0 - Y[i*m_width + j + masksize / 2];
				s2 = s2 + Y0 - Y[i*m_width + j - masksize / 2];
				sum1 = sum1 + Y[i*m_width + j + masksize / 2];
				sum2 = sum2 + Y[i*m_width + j - masksize / 2];

				if (abs(Y[i*m_width + j + masksize / 2] - Y0) <= T)
				{
					sntemp1 = sntemp1 + 1;
				}
				if (abs(Y[i*m_width + j - masksize / 2] - Y0) <= T)
				{
					sntemp1 = sntemp1 + 1;
				}
				if (abs(Y[i*m_width + j + masksize / 2] - Y0) <= T)
				{
					sntemp2 = sntemp2 + 1;
				}
				if (abs(Y[i*m_width + j - masksize / 2] - Y0) <= T)
				{
					sntemp2 = sntemp2 + 1;
				}
			}
			sn = sntemp1;
			snedge = sntemp2;

			if (s1>0 && s2>0 && sn<masksize / 2)         //线对象
			{
				R[i*m_width + j] = 1;
			}
			//else if (sum1>sum2 && snedge==masksize/2)  //左边缘
			//{ 
			//	R[i*m_width+j]=2;
			//}
			//else if (sum1<sum2 && snedge==masksize/2)  //右边缘
			//{
			//	R[i*m_width+j]=3;
			//}
			else
			{
				R[i*m_width + j] = 0;
			}
		}
	}
}

void Edgeanalysis::clear_blob(LineAttributeBlob *blob, int maxblobnumber)
{
	int k;
	if (maxblobnumber)
	{
		for (k = 0; k < maxblobnumber; k++)
		{
			blob[k].elements_number = 0;
			blob[k].elements.clear();
			blob[k].checkflag = 0;
			blob[k].sortcount = 0;
			blob[k].linetype = 0;
			blob[k].slope = 0;
			blob[k].intercept = 0;
			blob[k].length = 0;

			blob[k].leftx = m_width - 1;
			blob[k].lefty = m_height - 1;

			blob[k].topx = m_width - 1;
			blob[k].topy = m_height - 1;

			blob[k].rightx = 0;
			blob[k].righty = 0;

			blob[k].bottomx = 0;
			blob[k].bottomy = 0;

		}
	}
}


int Edgeanalysis::search_lineblob(unsigned char *image, LineAttributeBlob *blob, int numthreshold)
{
	int x, y, k;

	m_image = image;

	if (m_image == 0)       //not initialized
		return -1;

	//search blob
	x = 0;
	for (y = 0; y < m_height; y++)
	{
		m_image[y*m_width + x] = 0;
	}
	x = m_width - 1;
	for (y = 0; y < m_height; y++)
	{
		m_image[y*m_width + x] = 0;
	}
	y = 0;
	for (x = 0; x < m_width; x++)
	{
		m_image[y*m_width + x] = 0;
	}
	y = m_height - 1;
	for (x = 0; x < m_width; x++)
	{
		m_image[y*m_width + x] = 0;
	}

	Blobcount = 0;
	Blobtotal = 0;

	clear_blob(blob, MAXBLOBNUMBER);

	//define queue structure
	for (y = 1; y < m_height - 1; y++)
	{
		for (x = 1; x < m_width - 1; x++)
		{
			if (Blobcount < MAXBLOBNUMBER)
			{
				if (m_image[y*m_width + x] == 1)
				{
					//种子点清除
					m_image[y*m_width + x] = 0;

					Qfront = 0;
					Qrear = 0;
					Qcount = 0;

					Queuelist[0].x = x;
					Queuelist[0].y = y;
					Qcount++;
					Qrear = (Qrear + 1) % MaxQsize;

					element.coord.x = x;
					element.coord.y = y;
					blob[Blobcount].elements.push_back(element);

					blob[Blobcount].elements_number++;

					if (blob[Blobcount].leftx > x)
					{
						blob[Blobcount].leftx = x;
						blob[Blobcount].lefty = y;
					}

					if (blob[Blobcount].topy > y)
					{
						blob[Blobcount].topy = y;
						blob[Blobcount].topx = x;
					}

					if (blob[Blobcount].rightx < x)
					{
						blob[Blobcount].rightx = x;
						blob[Blobcount].righty = y;
					}

					if (blob[Blobcount].bottomy < y)
					{
						blob[Blobcount].bottomy = y;
						blob[Blobcount].bottomx = x;
					}

					pixelcount = 1;

					do{
						tempx = Queuelist[Qfront].x;
						tempy = Queuelist[Qfront].y;
						Qcount--;

						Qfront = (Qfront + 1) % MaxQsize;

						//--8
						selx[0] = tempx + 1;
						sely[0] = tempy;

						selx[1] = tempx + 1;
						sely[1] = tempy + 1;

						selx[2] = tempx;
						sely[2] = tempy + 1;

						selx[3] = tempx - 1;
						sely[3] = tempy + 1;

						selx[4] = tempx - 1;
						sely[4] = tempy;

						selx[5] = tempx - 1;
						sely[5] = tempy - 1;

						selx[6] = tempx;
						sely[6] = tempy - 1;

						selx[7] = tempx + 1;
						sely[7] = tempy - 1;

						for (k = 0; k < 8; k++)
						{
							if ((sely[k] > -1 && sely[k] < m_height) & (selx[k] <m_width && selx[k] >-1))
							{
								if (m_image[sely[k] * m_width + selx[k]] == 1)
								{
									//清除该点
									m_image[sely[k] * m_width + selx[k]] = 0;

									//加入队列尾部
									Queuelist[Qrear].x = selx[k];
									Queuelist[Qrear].y = sely[k];
									Qrear = (Qrear + 1) % MaxQsize;
									Qcount++;

									//该点进入Blob

									element.coord.x = selx[k];
									element.coord.y = sely[k];
									blob[Blobcount].elements.push_back(element);
									blob[Blobcount].elements_number++;


									if (blob[Blobcount].leftx > selx[k])
									{
										blob[Blobcount].leftx = selx[k];
										blob[Blobcount].lefty = sely[k];
									}

									if (blob[Blobcount].topy > sely[k])
									{
										blob[Blobcount].topy = sely[k];
										blob[Blobcount].topx = selx[k];
									}

									if (blob[Blobcount].rightx < selx[k])
									{
										blob[Blobcount].rightx = selx[k];
										blob[Blobcount].righty = sely[k];
									}

									if (blob[Blobcount].bottomy < sely[k])
									{
										blob[Blobcount].bottomy = sely[k];
										blob[Blobcount].bottomx = selx[k];
									}
								}
							}
						}

					} while (Qcount != 0);

					//处理完一个blob  m_width/4   m_height/4

					int currblobnumber = blob[Blobcount].elements_number;

					if (currblobnumber < numthreshold)
					{
						blob[Blobcount].elements_number = 0;  //删除blob
						blob[Blobcount].elements.clear();

						blob[Blobcount].leftx = m_width - 1;
						blob[Blobcount].lefty = m_height - 1;

						blob[Blobcount].topx = m_width - 1;
						blob[Blobcount].topy = m_height - 1;

						blob[Blobcount].rightx = 0;
						blob[Blobcount].righty = 0;

						blob[Blobcount].bottomx = 0;
						blob[Blobcount].bottomy = 0;
					}
					else
					{
						//保留该blob

						blob[Blobcount].sortcount = Blobcount;
						cal_blobattri(&blob[Blobcount]);

						blob[Blobcount].checkflag = 1;
						blob[Blobcount].proxmityflag = 1;
						blob[Blobcount].colineflag = 1;
						blob[Blobcount].cocurveflag = 1;
						blob[Blobcount].Parallelflag = 1;
						blob[Blobcount].crossflag = 1;

						Blobcount = Blobcount + 1;
					}
				}
			}
		}
	}
	return Blobcount;
}


void   Edgeanalysis::cal_blobattri(LineAttributeBlob *eachblob)
{
	Coord left, right, top, bottom;

	std::vector<struct Element>::iterator iter;
	struct Element tmpElement;

	int blobnumber = eachblob->elements_number;

	//计算外接矩形四个顶点坐标及中心点坐标
	long xsum = 0;
	long ysum = 0;
	left.x = m_width;
	right.x = 0;
	top.y = m_height;
	bottom.y = 0;
	for (iter = eachblob->elements.begin(); iter != eachblob->elements.end(); ++iter)
	{
		tmpElement = *iter;
		int tempx = tmpElement.coord.x;
		int tempy = tmpElement.coord.y;
		if (tempx  < left.x)
		{
			left.x = tempx;
			left.y = tempy;
		}
		if (tempx > right.x)
		{
			right.x = tempx;
			right.y = tempy;
		}
		if (tempy < top.y)
		{
			top.y = tempy;
			top.x = tempx;
		}
		if (tempy > bottom.y)
		{
			bottom.y = tempy;
			bottom.x = tempx;
		}

		xsum = tmpElement.coord.x + xsum;
		ysum = tmpElement.coord.y + ysum;
	}

	eachblob->leftx = left.x;
	//eachblob->lefty   = left.y;
	eachblob->rightx = right.x;
	//eachblob->righty  = right.y;
	//eachblob->topx    = top.x;
	eachblob->topy = top.y;
	//eachblob->bottomx = bottom.x;
	eachblob->bottomy = bottom.y;

	eachblob->centerP.x = (int)((double)xsum / blobnumber + 0.5);
	eachblob->centerP.y = (int)((double)ysum / blobnumber + 0.5);

	//计算线段的斜率或角度与截距
	double   SumX, SumY, SumXY, SumX2;

	SumX = 0;
	SumX2 = 0;
	SumY = 0;
	SumXY = 0;

	for (iter = eachblob->elements.begin(); iter != eachblob->elements.end(); ++iter)
	{
		tmpElement = *iter;
		int tempx = tmpElement.coord.x;
		int tempy = tmpElement.coord.y;

		SumX += tempx;
		SumX2 += tempx*tempx;
		SumY += tempy;
		SumXY += tempx*tempy;
	}

	double tempslope;
	double tempintercept;

	if (blobnumber*SumX2 == SumX*SumX)
	{
		eachblob->slope = 90;
		tempintercept = eachblob->centerP.x;
	}
	else
	{
		tempslope = (double)(blobnumber*SumXY - SumX*SumY) / (blobnumber*SumX2 - SumX*SumX);
		eachblob->slope = 180 * atan(tempslope) / PI;
		tempintercept = (double)(eachblob->centerP.y - eachblob->centerP.x*tempslope);
	}
	eachblob->intercept = tempintercept;

	if (eachblob->slope >= -15 && eachblob->slope <= 15)
		eachblob->linetype = HORLINE;
	else if (eachblob->slope > 15 && eachblob->slope <= 75)
		eachblob->linetype = UPSLOPELINE;
	else if (eachblob->slope >= -75 && eachblob->slope < -15)
		eachblob->linetype = DOWNSLOPELINE;
	else if (eachblob->slope <= -75 || eachblob->slope > 75)
		eachblob->linetype = VERLINE;

	eachblob->slopetype = DeisionDir(eachblob->slope);

	int minvalue, maxvalue;
	if (eachblob->slope >= -ANGLECLASS_THRESHOLD && eachblob->slope <= ANGLECLASS_THRESHOLD)
	{
		//计算水平方向线段两端点斜率或角度与截距
		minvalue = m_width;
		maxvalue = 0;

		unsigned char *tmpmask = new unsigned char[m_width*m_height];
		memset(tmpmask, 0, m_width*m_height);

		for (iter = eachblob->elements.begin(); iter != eachblob->elements.end(); ++iter)
		{
			tmpElement = *iter;
			int tempx = tmpElement.coord.x;
			int tempy = tmpElement.coord.y;

			tmpmask[tempy*m_width + tempx] = 1;
		}

		int kx = eachblob->centerP.x;
		for (int ky = 1; ky < m_height - 1; ky++)
		{
			if (tmpmask[(ky - 1)*m_width + kx] == 0 && tmpmask[ky*m_width + kx] == 1)
				minvalue = ky;
			if (tmpmask[ky*m_width + kx] == 1 && tmpmask[(ky + 1)*m_width + kx] == 0)
				maxvalue = ky;

		}
		//计算线段的平均宽度与长度
		eachblob->linewidth = (int)(abs((maxvalue - minvalue + 1)*sin((90 - eachblob->slope)*PI / 180)) + 0.5);
		eachblob->length = (int)(sqrtf((left.x - right.x)*(left.x - right.x) + (left.y - right.y)*(left.y - right.y)) + 0.5);

		int linelength = right.x - left.x;
		int lineseg = (int)(linelength / 4 + 0.5);

		double tmpslope[4], tmpintercept[4];
		int    tmpblobnumber;
		Coord  tmpcenter;

		for (int k = 0; k < 4; k++)
		{
			SumX = 0;
			SumX2 = 0;
			SumY = 0;
			SumXY = 0;

			xsum = 0;
			ysum = 0;
			tmpblobnumber = 0;
			for (iter = eachblob->elements.begin(); iter != eachblob->elements.end(); ++iter)
			{
				tmpElement = *iter;
				int tempx = tmpElement.coord.x;
				int tempy = tmpElement.coord.y;

				if (tempx >= (left.x + k*lineseg) && tempx <= (left.x + (k + 1)*lineseg))
				{
					SumX += tempx;
					SumX2 += tempx*tempx;
					SumY += tempy;
					SumXY += tempx*tempy;

					xsum = tempx + xsum;
					ysum = tempy + ysum;

					tmpblobnumber = tmpblobnumber + 1;
				}
			}
			tmpcenter.x = (int)((double)xsum / tmpblobnumber + 0.5);
			tmpcenter.y = (int)((double)ysum / tmpblobnumber + 0.5);

			double tempintercept;
			if (tmpblobnumber*SumX2 == SumX*SumX)
			{
				tmpslope[k] = 90;
				tmpintercept[k] = tmpcenter.x;
			}
			else
			{
				double tempslope = (double)(tmpblobnumber*SumXY - SumX*SumY) / (tmpblobnumber*SumX2 - SumX*SumX);
				tmpslope[k] = 180 * atan(tempslope) / PI;
				tempintercept = (double)(tmpcenter.y - tmpcenter.x*tempslope);
				tmpintercept[k] = tempintercept;
			}
		}

		eachblob->startslope = tmpslope[0];
		eachblob->startintercept = tmpintercept[0];
		eachblob->endslope = tmpslope[3];
		eachblob->endintercept = tmpintercept[3];

		delete  tmpmask;

	}
	else
	{
		//计算垂直方向线段两端点斜率或角度与截距
		minvalue = m_height;
		maxvalue = 0;

		unsigned char *tmpmask = new unsigned char[m_width*m_height];
		memset(tmpmask, 0, m_width*m_height);

		for (iter = eachblob->elements.begin(); iter != eachblob->elements.end(); ++iter)
		{
			tmpElement = *iter;
			int tempx = tmpElement.coord.x;
			int tempy = tmpElement.coord.y;

			tmpmask[tempy*m_width + tempx] = 1;
		}
		int ky = eachblob->centerP.y;
		for (int kx = 1; kx < m_width - 1; kx++)
		{
			if (tmpmask[ky*m_width + kx - 1] == 0 && tmpmask[ky*m_width + kx] == 1)
				minvalue = kx;
			if (tmpmask[ky*m_width + kx] == 1 && tmpmask[ky*m_width + kx + 1] == 0)
				maxvalue = kx;

		}
		//计算线段的平均宽度与长度
		eachblob->linewidth = (int)(abs((maxvalue - minvalue + 1)*sin((eachblob->slope)*PI / 180)) + 0.5);
		eachblob->length = (int)(sqrtf((top.x - bottom.x)*(top.x - bottom.x) + (top.y - bottom.y)*(top.y - bottom.y)) + 0.5);

		int linelength = bottom.y - top.y;
		int lineseg = (int)(linelength / 4 + 0.5);

		double tmpslope[4], tmpintercept[4];
		int    tmpblobnumber;
		Coord  tmpcenter;

		for (int k = 0; k < 4; k++)
		{
			SumX = 0;
			SumX2 = 0;
			SumY = 0;
			SumXY = 0;

			xsum = 0;
			ysum = 0;
			tmpblobnumber = 0;
			for (iter = eachblob->elements.begin(); iter != eachblob->elements.end(); ++iter)
			{
				tmpElement = *iter;
				int tempx = tmpElement.coord.x;
				int tempy = tmpElement.coord.y;

				if (tempy >= (top.y + k*lineseg) && tempy <= (top.y + (k + 1)*lineseg))
				{
					SumX += tempx;
					SumX2 += tempx*tempx;
					SumY += tempy;
					SumXY += tempx*tempy;

					xsum = tempx + xsum;
					ysum = tempy + ysum;

					tmpblobnumber = tmpblobnumber + 1;
				}
			}
			tmpcenter.x = (int)((double)xsum / tmpblobnumber + 0.5);
			tmpcenter.y = (int)((double)ysum / tmpblobnumber + 0.5);

			double tempintercept;
			if (tmpblobnumber*SumX2 == SumX*SumX)
			{
				tmpslope[k] = 90;
				tmpintercept[k] = tmpcenter.x;
			}
			else
			{
				double tempslope = (double)(tmpblobnumber*SumXY - SumX*SumY) / (tmpblobnumber*SumX2 - SumX*SumX);
				tmpslope[k] = 180 * atan(tempslope) / PI;
				tempintercept = (double)(tmpcenter.y - tmpcenter.x*tempslope);
				tmpintercept[k] = tempintercept;
			}
		}

		eachblob->startslope = tmpslope[0];
		eachblob->startintercept = tmpintercept[0];
		eachblob->endslope = tmpslope[3];
		eachblob->endintercept = tmpintercept[3];

		delete  tmpmask;
	}
}


int Edgeanalysis::DeisionDir(double slope)
{
	int slopetype;
	if (slope >= -15 && slope < 15)
		slopetype = 0;
	else if (slope >= 15 && slope < 45)
		slopetype = 1;
	else if (slope >= 45 && slope < 75)
		slopetype = 2;
	else if (slope >= -75 && slope < -45)
		slopetype = 3;
	else if (slope >= -45 && slope < -15)
		slopetype = 4;
	else if (slope >= 75 || slope < -75)
		slopetype = 5;
	return slopetype;
}



double Edgeanalysis::Bottom_Perceptual(unsigned char *GrayImage, LineAttributeBlob *CurveBlob, unsigned char *maskimage, bool blackorwhite, bool gpuorcpu, int w, int h)
{
	int   k, i;
	int   curvenumber;
	double timeuse_line;                 //十字模板算法计时
	double timeuse_chain;                //链码算法计时
	unsigned char *chain_img;
	chain_img = (unsigned char *)malloc(sizeof(unsigned char)*w*h);   // 为每一个blob初始化的mask空间
	std::vector<struct Element>::iterator iter;
	struct Element tmpElement;
	cudaconverter.InitCUDA();             //CUDA的初始化测试函数
	LineAttributeBlob TempBlob[MAXBLOBNUMBER];
	int TempNumber;
	int HorNumber;
	int VerNumber;
	int tempval;
	char title[] = "converter filter";
	int maxlinewidth = 4;
	int numthreshold = 6;
	curvenumber = 0;
	TempNumber = 0;
	VerNumber = 0;
	HorNumber = 0;
	Coord left_bl;
	Coord right_bl;
	Coord top_bl;
	Coord bottom_bl;
	int size = w*h;
	fp_chain = fopen("D:\\chaincode.log", "w");
	memset(chain_img, 0, sizeof(unsigned char)*w*h);              //填充前先置0
	// allocate device memory
	cudaconverter.MallocMemA(size);
	cudaconverter.TranslateInput(w, h, GrayImage);

	starttime(&atime);
	if (blackorwhite)  // 1: black; 0: white
	{
		if (gpuorcpu){    //1:gpu 0:cpu
			/*cudaconverter.MallocMemA(size);
			cudaconverter.TranslateInput(w, h, GrayImage);*/
			cudaconverter.blackverkernel(w, h);
			memset(BlackVerEdger, 0, w*h);
			memset(BlackHorEdger, 0, w*h);
			cudaconverter.TranslateOutput(w, h, BlackVerEdger, BlackHorEdger);
			timeuse_line = stoptime(&atime, title);
			clear_blob(VerBlob, MAXBLOBNUMBER);
			VerNumber = search_lineblob(BlackVerEdger, VerBlob, numthreshold);
			clear_blob(HorBlob, MAXBLOBNUMBER);
			HorNumber = search_lineblob(BlackHorEdger, HorBlob, numthreshold);
			/*cudaconverter.ReleaseMem();*/
		}
		else
		{
			memset(BlackVerEdge, 0, w*h);
			memset(BlackHorEdge, 0, w*h);
			Mark_BlackEdgever(GrayImage, BlackVerEdge, 2 * maxlinewidth + 1);
			Mark_BlackEdgehor(GrayImage, BlackHorEdge, 2 * maxlinewidth + 1);
			timeuse_line = stoptime(&atime, title);
			clear_blob(VerBlob, MAXBLOBNUMBER);
			VerNumber = search_lineblob(BlackVerEdge, VerBlob, numthreshold);
			clear_blob(HorBlob, MAXBLOBNUMBER);
			HorNumber = search_lineblob(BlackHorEdge, HorBlob, numthreshold);


		}
		cudaconverter.ReleaseMem();
	}
	else
	{
		if (gpuorcpu){
			cudaconverter.whiteverkernel(w, h);
			memset(WhiteVerEdger, 0, w*h);
			memset(WhiteHorEdger, 0, w*h);
			cudaconverter.TranslateOutput(w, h, WhiteVerEdger, WhiteHorEdger);
			timeuse_line = stoptime(&atime, title);
			clear_blob(VerBlob, MAXBLOBNUMBER);
			VerNumber = search_lineblob(WhiteVerEdger, VerBlob, numthreshold);
			clear_blob(HorBlob, MAXBLOBNUMBER);
			HorNumber = search_lineblob(WhiteHorEdger, HorBlob, numthreshold);


		}
		else
		{
			memset(WhiteVerEdge, 0, w*h);
			memset(WhiteHorEdge, 0, w*h);
			Mark_WhiteEdgehor(GrayImage, WhiteHorEdge, 2 * maxlinewidth + 1);//_new
			Mark_WhiteEdgever(GrayImage, WhiteVerEdge, 2 * maxlinewidth + 1);
			timeuse_line = stoptime(&atime, title);
			clear_blob(HorBlob, MAXBLOBNUMBER);
			HorNumber = search_lineblob(WhiteHorEdge, HorBlob, numthreshold);
			clear_blob(VerBlob, MAXBLOBNUMBER);
			VerNumber = search_lineblob(WhiteVerEdge, VerBlob, numthreshold);
		}
		cudaconverter.ReleaseMem();
	}
#if 1
	starttime(&gtime);
	cudachaincode.MallocMemM(w*h);
	if (VerNumber)
	{
		int maxlinewidth =100;
		int num = 0;

		for (int i = 0; i < VerNumber; i++)
		{
			int len = VerBlob[i].elements_number;
			if (len<100)
			{
				++num;
			}
		}
		//unsigned char *h_ptr,
		int *h_ptr, *chain_position;
		h_ptr          = (int*)malloc((2 * maxlinewidth * num*sizeof(int)));  //因为有left,right等结构体变量作为参数，所以没有必要也写进h_ptr中
		memset(h_ptr,0,sizeof(int)*(2*maxlinewidth*num));
		chain_position = (int*)malloc((2 * maxlinewidth * num*sizeof(int)));
		memset(chain_position, 0, sizeof(int)*(2 * maxlinewidth*num));
		int kn = 0;        //kn代表h_ptr的行索引，因为长度满足条件的blob的索引和行索引不一致，所以需要另外的自动记数变量kn
		for (int k = 0; k < VerNumber; k++)        //将vector blob中的元素转移进二元数组ptr
		{
			if (VerBlob[k].elements_number<100){
				++kn;
				for (iter = VerBlob[k].elements.begin(); iter != VerBlob[k].elements.end(); iter++)
				{
					tmpElement = *iter;
					int cn = iter - VerBlob[k].elements.begin();
					int tempx = tmpElement.coord.x;
					int tempy = tmpElement.coord.y;
					//maskimage[tempy*m_width + tempx] = 1;
					h_ptr[2 * (kn*maxlinewidth + cn)] = tempx;
					h_ptr[2 * (kn*maxlinewidth + cn) + 1] = tempy;   //将坐标值遍历写进ptr矩阵中

				}
			}
			
		}
		
	
		
		cudachaincode.MallocMemP(2*maxlinewidth*num);             //初始化d_ptr
		cudachaincode.TranslateInputB(maxlinewidth, num, h_ptr);

	
			//链码分析函数
	    cudachaincode.chaincodekernel(maxlinewidth,num);
		
		cudachaincode.TranslateOutputP(maxlinewidth*num*2,chain_position);
		cudachaincode.ReleaseMemP();
	
		free(h_ptr);
		free(chain_position);
	}
	timeuse_chain = stoptime(&gtime, title);
#if 0
	if (HorNumber)
	{
		int maxlinewidth;
		int num = HorNumber;
		maxlinewidth = HorBlob[0].elements_number;

		for (int i = 0; i < num; i++)
		{
			if (maxlinewidth < HorBlob[i].elements_number)
			{
				maxlinewidth = HorBlob[i].elements_number;
			}
		}
		//unsigned char *h_ptr,
		int *h2_ptr, *chain2_position;
		h2_ptr = (int*)malloc((2 * maxlinewidth * num*sizeof(int)));  //因为有left,right等结构体变量作为参数，所以没有必要也写进h_ptr中
		memset(h2_ptr, 0, sizeof(int)*(2 * maxlinewidth*num));
		chain2_position = (int*)malloc((2 * maxlinewidth * num*sizeof(int)));
		memset(chain2_position, 0, sizeof(int)*(2 * maxlinewidth*num));
		for (int k = 0; k < num; k++)        //将vector blob中的元素转移进二元数组ptr
		{

			for (iter = HorBlob[k].elements.begin(); iter != HorBlob[k].elements.end(); iter++)
			{
				tmpElement = *iter;
				int cn = iter - HorBlob[k].elements.begin();
				int tempx = tmpElement.coord.x;
				int tempy = tmpElement.coord.y;
				//maskimage[tempy*m_width + tempx] = 1;
				h2_ptr[2 * (k*maxlinewidth + cn)] = tempx;
				h2_ptr[2 * (k*maxlinewidth + cn) + 1] = tempy;   //将坐标值遍历写进ptr矩阵中

			}

		}



		cudachaincode.MallocMemH(2 * maxlinewidth*num);             //初始化d_ptr
		cudachaincode.TranslateInputH(maxlinewidth, num, h2_ptr);


		//链码分析函数
		cudachaincode.chaincodehkernel(maxlinewidth, num);
		//chaincode_comph(chain_img, left_bl, right_bl, bottom_bl, top_bl, maskimage);  //maskimage 直接作为输出结果图像，被循环填充（可叠加
		cudachaincode.TranslateOutputH(maxlinewidth*num * 2, chain2_position);
		cudachaincode.ReleaseMemH();

		/*for (int i = 0; i < num*maxlinewidth; i++)
		{
		if (chain_position[2*i]>0)
		{
		int x = chain_position[2*i];
		int y = chain_position[2*i + 1];
		fprintf(fp_chain, "chaincode-function-xy  %10d,%10d\n", x, y);

		}
		}*/
		free(h2_ptr);
		free(chain2_position);
	}
#endif
	cudachaincode.TranslateOutputM(w*h,maskimage);
	cudachaincode.ReleaseMemM();
#endif
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////vector cpu端函数
#if 0
    starttime(&gtime);
    if (VerNumber)
	  {
		memset(chain_img, 0, sizeof(unsigned char)*w*h);                      //chain_img已被使用过一次，第二次使用前要确保都为空值
		for (k = 0; k < VerNumber; k++)
		{
			/*CopyBlob(&VerBlob[k], &TempBlob[TempNumber], TempNumber);
			TempNumber++;*/
			for (iter = VerBlob[k].elements.begin(); iter != VerBlob[k].elements.end(); iter++)
			{
				tmpElement = *iter;

				int tempx = tmpElement.coord.x;
				int tempy = tmpElement.coord.y;
				chain_img[tempy*m_width + tempx] = 1;//chain_img[tempy*m_width + tempx] = 1;
			}
			bottom_bl.x = VerBlob[k].bottomx;
			bottom_bl.y = VerBlob[k].bottomy;

			left_bl.x = VerBlob[k].leftx;
			left_bl.y = VerBlob[k].lefty;

			top_bl.x = VerBlob[k].topx;
			top_bl.y = VerBlob[k].topy;

			right_bl.x = VerBlob[k].rightx;
			right_bl.y = VerBlob[k].righty;
			//链码分析函数
			chaincode_comp(chain_img, left_bl, right_bl, bottom_bl, top_bl, maskimage);  //maskimage 直接作为输出结果图像

		}
	}
	timeuse_chain = stoptime(&gtime, title);
#if 0
	if (HorNumber)
	{
		memset(chain_img, 0, sizeof(unsigned char)*w*h);                      //chain_img已被使用过一次，第二次使用前要确保都为空值
		for (k = 0; k < HorNumber; k++)
		{
			/*CopyBlob(&VerBlob[k], &TempBlob[TempNumber], TempNumber);
			TempNumber++;*/
			for (iter = HorBlob[k].elements.begin(); iter != HorBlob[k].elements.end(); iter++)
			{
				tmpElement = *iter;

				int tempx = tmpElement.coord.x;
				int tempy = tmpElement.coord.y;
				chain_img[tempy*m_width + tempx] = 1;//chain_img[tempy*m_width + tempx] = 1;
			}
			bottom_bl.x = HorBlob[k].bottomx;
			bottom_bl.y = HorBlob[k].bottomy;

			left_bl.x = HorBlob[k].leftx;
			left_bl.y = HorBlob[k].lefty;

			top_bl.x = HorBlob[k].topx;
			top_bl.y = HorBlob[k].topy;

			right_bl.x = HorBlob[k].rightx;
			right_bl.y = HorBlob[k].righty;
			//链码分析函数
			chaincode_comph(chain_img, left_bl, right_bl, bottom_bl, top_bl, maskimage);  //maskimage 直接作为输出结果图像

		}
	}
#endif
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	for (int j=0;j<TempNumber;j++)
	{
		CopyBlob(&TempBlob[j],&CurveBlob[curvenumber],curvenumber);
		for (iter=CurveBlob[curvenumber].elements.begin();iter!=CurveBlob[curvenumber].elements.end();iter++)
		{
			tmpElement=*iter;
			int tempx=tmpElement.coord.x;
			int tempy=tmpElement.coord.y;
			//maskimage[tempy*m_width+tempx]=1;
		}
		curvenumber++;
	}
	return curvenumber;
#endif
//////////////////////////////////////////链码GPU算法的CPU验证 要为新增的实参*chaincode分配存储空间？目前先验证水平线的链码算法
#if 1
	starttime(&gtime);                                     //同时if 1同一函数的两个版本时，有些内部变量（h_ptr,chain_position）会因为名字相同在释放内存的时候会产生冲突
	if (VerNumber)
	{
		int maxlinewidth;
		int num = VerNumber;
		maxlinewidth = VerBlob[0].elements_number;

		for (int i = 0; i < num; i++)
		{
			if (maxlinewidth < VerBlob[i].elements_number)
			{
				maxlinewidth = VerBlob[i].elements_number;
			}
		}
		//unsigned char *h_ptr,
		int *h2_ptr, *chain2_position;
		unsigned char *chain2_code;
		h2_ptr = (int*)malloc((2 * maxlinewidth * num*sizeof(int)));  //因为有left,right等结构体变量作为参数，所以没有必要也写进h_ptr中
		memset(h2_ptr, 0, sizeof(int)*(2 * maxlinewidth*num));
		chain2_position = (int*)malloc((2 * maxlinewidth * num*sizeof(int)));        //几个中间存储区和GPU类似，没必要更改
		memset(chain2_position, 0, sizeof(int)*(2 * maxlinewidth*num));
		chain2_code = (unsigned char*)malloc((maxlinewidth*num*sizeof(unsigned char)));        //方向占的存储空间只为坐标的一半
		memset(chain2_code, 0, sizeof(unsigned char)*(maxlinewidth*num));
		for (int k = 0; k < num; k++)        //将vector blob中的元素转移进二元数组ptr
		{

			for (iter = VerBlob[k].elements.begin(); iter != VerBlob[k].elements.end(); iter++)
			{
				tmpElement = *iter;
				int cn = iter - VerBlob[k].elements.begin();
				int tempx = tmpElement.coord.x;
				int tempy = tmpElement.coord.y;
				//maskimage[tempy*m_width + tempx] = 1;
				h2_ptr[2 * (k*maxlinewidth + cn)] = tempx;
				h2_ptr[2 * (k*maxlinewidth + cn) + 1] = tempy;   //将坐标值遍历写进ptr矩阵中

			}

		}
		/////////////////////////输入参数对应 maxlinewidth:w,num:h,h_ptr:blobmask
		chaincode_compcu(h2_ptr,maskimage,chain2_code,chain2_position,maxlinewidth,num);

		free(h2_ptr);
		free(chain2_code);
		free(chain2_position);
	}
	timeuse_chain = stoptime(&gtime, title);
#endif


///////////////////////////////////////////////////////////////////////////////////////

	free(chain_img);
	fclose(fp_chain);
	return timeuse_chain;//timeuse;
}







void Edgeanalysis::CopyBlob(LineAttributeBlob *srcblob, LineAttributeBlob *dstblob, int sortcount)
{
	std::vector<struct Element>::iterator iter;
	struct Element tmpElement;

	dstblob->elements.clear();
	dstblob->elements_number = 0;
	for (iter = srcblob->elements.begin(); iter != srcblob->elements.end(); ++iter)
	{

		tmpElement = *iter;

		int tempx = tmpElement.coord.x;
		int tempy = tmpElement.coord.y;

		dstblob->elements.push_back(tmpElement);
		dstblob->elements_number++;
	}

	dstblob->leftx = srcblob->leftx;
	dstblob->lefty = srcblob->lefty;
	dstblob->topx = srcblob->topx;
	dstblob->topy = srcblob->topy;
	dstblob->rightx = srcblob->rightx;
	dstblob->righty = srcblob->righty;
	dstblob->bottomx = srcblob->bottomx;
	dstblob->bottomy = srcblob->bottomy;
	dstblob->centerP.x = srcblob->centerP.x;
	dstblob->centerP.y = srcblob->centerP.y;
	dstblob->sortcount = sortcount;
	dstblob->linetype = srcblob->linetype;
	dstblob->checkflag = srcblob->checkflag;
	dstblob->Parallelflag = srcblob->Parallelflag;
	dstblob->proxmityflag = srcblob->proxmityflag;
	dstblob->colineflag = srcblob->colineflag;
	dstblob->cocurveflag = srcblob->cocurveflag;
	dstblob->crossflag = srcblob->crossflag;
	dstblob->length = srcblob->length;
	dstblob->slope = srcblob->slope;
	dstblob->intercept = srcblob->intercept;
	dstblob->BlobNumber = srcblob->BlobNumber;

	dstblob->startslope = srcblob->startslope;
	dstblob->SegmentSlope = srcblob->SegmentSlope;
	dstblob->startintercept = srcblob->startintercept;
	dstblob->endslope = srcblob->endslope;
	dstblob->endintercept = srcblob->endintercept;
	dstblob->slopetype = srcblob->slopetype;
	dstblob->linewidth = srcblob->linewidth;
}

int Edgeanalysis::chaincode_comp(unsigned char *chainmask, Coord left, Coord right, Coord top, Coord bottom, unsigned char *maskimage)
{


	int i, j, k;
	double distance, clockdist, anticlockdist, anticlockdist1;
	Coord start, end, cur, prev;
	Coord clockstart, anticlockstart, anticlockstart1;
	bool burrflag;

	//chain code analysis	
	int clock[8][2] = { { 1, 0 }, { 1, -1 }, { 1, -1 }, { 1, -1 }, { 1, -1 }, { -1, 1 }, { 0, 1 }, { 1, 1 } };           //跳过2，3，4方向，避免搜索原路返回
	int anticlock[8][2] = { { 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 1 }, { -1, 0 }, { -1, -1 }, { 0, -1 }, { 1, -1 } };
	int anticlock1[8][2] = { { -1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 1 }, { 1, 0 }, { 1, -1 }, { 0, -1 }, { -1, -1 } };

	unsigned char *antichainmask = new unsigned char[m_width*m_height];
	unsigned char *antichainmask1 = new unsigned char[m_width*m_height];

	clockcode.clear();
	anticlockcode.clear();
	anticlockcode1.clear();
	chaincode.clear();
	chainposition.clear();

	if (right.x - left.x >= top.y - bottom.y)
	{
		start.x = left.x;
		start.y = left.y;
		end.x = right.x;
		end.y = right.y;
	}
	else
	{
		start.x = bottom.x;
		start.y = bottom.y;
		end.x = top.x;
		end.y = top.y;
	}

	if (left.x == m_width || top.y == m_height)
	{
		start.x = 0;
		start.y = 0;
		end.x = 0;
		end.y = 0;
		return -1;
	}
	//maoci
	bool padflag, proflag;
	do{
		proflag = true;

		for (j = bottom.y - 1; j < top.y + 1; j++)
		{
			for (i = left.x - 1; i < right.x + 1; i++)
			{
				if ((i != start.x && j != start.y) || (i != end.x && j != end.y))
				{
					if (chainmask[j*m_width + i] == 1)
					{
						int boundvalue;
						int  f1, f2, f3, f4, f5, f6, f7, f8;
						if (((i + 1) == start.x && (j + 0) == start.y) || ((i + 1) == end.x && (j + 0) == end.y))
							f1 = 10;
						else
							f1 = chainmask[(j + 0)*m_width + (i + 1)];

						if (((i + 1) == start.x && (j + 1) == start.y) || ((i + 1) == end.x && (j + 1) == end.y))
							f2 = 10;
						else
							f2 = chainmask[(j + 1)*m_width + (i + 1)];

						if (((i + 0) == start.x && (j + 1) == start.y) || ((i + 0) == end.x && (j + 1) == end.y))
							f3 = 10;
						else
							f3 = chainmask[(j + 1)*m_width + (i + 0)];

						if (((i - 1) == start.x && (j + 1) == start.y) || ((i - 1) == end.x && (j + 1) == end.y))
							f4 = 10;
						else
							f4 = chainmask[(j + 1)*m_width + (i - 1)];

						if (((i - 1) == start.x && (j + 0) == start.y) || ((i - 1) == end.x && (j + 0) == end.y))
							f5 = 10;
						else
							f5 = chainmask[(j + 0)*m_width + (i - 1)];

						if (((i - 1) == start.x && (j - 1) == start.y) || ((i - 1) == end.x && (j - 1) == end.y))
							f6 = 10;
						else
							f6 = chainmask[(j - 1)*m_width + (i - 1)];

						if (((i + 0) == start.x && (j - 1) == start.y) || ((i + 0) == end.x && (j - 1) == end.y))
							f7 = 10;
						else
							f7 = chainmask[(j - 1)*m_width + (i + 0)];

						if (((i + 1) == start.x && (j - 1) == start.y) || ((i + 1) == end.x && (j - 1) == end.y))
							f8 = 10;
						else
							f8 = chainmask[(j - 1)*m_width + (i + 1)];

						boundvalue = f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8;

						if (boundvalue == 1 || boundvalue == 0)
						{
							proflag = false;
							chainmask[j*m_width + i] = 0;
						}
					}
				}
			}
		}
		if (proflag == true)
			padflag = false;
		else
			padflag = true;

	} while (padflag == true);

	for (j = bottom.y - 1; j < top.y + 1; j++)
	{
		for (i = left.x - 1; i < right.x + 1; i++)
		{
			if (chainmask[j*m_width + i] == 1)
			{
				antichainmask[j*m_width + i] = 1;
				antichainmask1[j*m_width + i] = 1;

				//maskimage[j*m_width+i] = 2;
				fprintf(fp_chain, "maskcode-function-xy  %10d,%10d\n", i, j);
			}
		}
	}

	//step1:顺时针方向搜索链码
	if (right.x - left.x >= top.y - bottom.y)
	{
		start.x = left.x;
		start.y = left.y;
		end.x = right.x;
		end.y = right.y;
	}
	else
	{
		start.x = bottom.x;
		start.y = bottom.y;
		end.x = top.x;
		end.y = top.y;
	}

	if (left.x == m_width || top.y == m_height)
	{
		start.x = 0;
		start.y = 0;
		end.x = 0;
		end.y = 0;
		return -1;
	}
	distance = sqrtf((end.x - start.x)*(end.x - start.x) + (end.y - start.y)*(end.y - start.y));

	//初始链码
	clockstart.x = start.x;
	clockstart.y = start.y;

	chainmask[start.y*m_width + start.x] = 0;
	clockcode.push_back(0);

	do{
		for (k = 0; k < 8; k++)
		{
			burrflag = false;
			cur.x = start.x + clock[k][0];
			cur.y = start.y + clock[k][1];

			if ((cur.x > -1) && (cur.x < m_width) && (cur.y > -1) && (cur.y < m_height))
			{
				if (chainmask[cur.y*m_width + cur.x] == 1)
				{
					chainmask[cur.y*m_width + cur.x] = 0;
					clockcode.push_back(k);
					start.x = cur.x;
					start.y = cur.y;
					burrflag = true;
					break;
				}
			}
		}
		if (burrflag)
		{
			int distx = abs(start.x - end.x);
			int disty = abs(start.y - end.y);

			if (distx < 1 && disty <1)      //(StartP.x == right.x) && (StartP.y == right.y)
				burrflag = true;
			else
				burrflag = false;
		}
		else
			burrflag = true;

	} while (burrflag == false);
	clockdist = sqrtf((clockstart.x - start.x)*(clockstart.x - start.x) + (clockstart.y - start.y)*(clockstart.y - start.y));

	//step2:逆时针方向搜索链码
	if (right.x - left.x >= top.y - bottom.y)
	{
		start.x = left.x;
		start.y = left.y;
		end.x = right.x;
		end.y = right.y;
	}
	else
	{
		start.x = bottom.x;
		start.y = bottom.y;
		end.x = top.x;
		end.y = top.y;
	}

	if (left.x == m_width || top.y == m_height)
	{
		start.x = 0;
		start.y = 0;
		end.x = 0;
		end.y = 0;
		return -1;
	}

	anticlockstart.x = start.x;
	anticlockstart.y = start.y;

	antichainmask[start.y*m_width + start.x] = 0;
	anticlockcode.push_back(0);
	do{
		for (k = 0; k < 8; k++)
		{
			burrflag = false;
			cur.x = start.x + anticlock[k][0];
			cur.y = start.y + anticlock[k][1];

			if ((cur.x > -1) && (cur.x < m_width) && (cur.y > -1) && (cur.y < m_height))
			{
				if (antichainmask[cur.y*m_width + cur.x] == 1)
				{
					antichainmask[cur.y*m_width + cur.x] = 0;
					anticlockcode.push_back(k);
					start.x = cur.x;
					start.y = cur.y;
					burrflag = true;
					break;
				}
			}
		}
		if (burrflag)
		{
			int distx = abs(start.x - end.x);
			int disty = abs(start.y - end.y);

			if (distx < 1 && disty <1)      //(StartP.x == right.x) && (StartP.y == right.y)
				burrflag = true;
			else
				burrflag = false;
		}
		else
			burrflag = true;

	} while (burrflag == false);
	anticlockdist = sqrtf((anticlockstart.x - start.x)*(anticlockstart.x - start.x) + (anticlockstart.y - start.y)*(anticlockstart.y - start.y));

	//step3:延迟方向搜索链码
	if (right.x - left.x >= top.y - bottom.y)
	{
		start.x = left.x;
		start.y = left.y;
		end.x = right.x;
		end.y = right.y;
	}
	else
	{
		start.x = bottom.x;
		start.y = bottom.y;
		end.x = top.x;
		end.y = top.y;
	}

	if (left.x == m_width || top.y == m_height)
	{
		start.x = 0;
		start.y = 0;
		end.x = 0;
		end.y = 0;
		return -1;
	}

	anticlockstart1.x = start.x;
	anticlockstart1.y = start.y;

	antichainmask1[start.y*m_width + start.x] = 0;
	anticlockcode1.push_back(0);
	do{
		for (k = 0; k < 8; k++)
		{
			burrflag = false;
			cur.x = start.x + anticlock1[k][0];
			cur.y = start.y + anticlock1[k][1];

			if ((cur.x > -1) && (cur.x < m_width) && (cur.y > -1) && (cur.y < m_height))
			{
				if (antichainmask1[cur.y*m_width + cur.x] == 1)
				{
					antichainmask1[cur.y*m_width + cur.x] = 0;
					anticlockcode1.push_back(k);
					start.x = cur.x;
					start.y = cur.y;
					burrflag = true;
					break;
				}
			}
		}
		if (burrflag)
		{
			int distx = abs(start.x - end.x);
			int disty = abs(start.y - end.y);

			if (distx < 1 && disty <1)      //(StartP.x == right.x) && (StartP.y == right.y)
				burrflag = true;
			else
				burrflag = false;
		}
		else
			burrflag = true;

	} while (burrflag == false);
	anticlockdist1 = sqrtf((anticlockstart1.x - start.x)*(anticlockstart1.x - start.x) + (anticlockstart1.y - start.y)*(anticlockstart1.y - start.y));

	//step4:判决最优链码
	int retflag ;
	int clocksize = clockcode.size();	//当前链码长度
	int anticlocksize = anticlockcode.size();
	int anticlocksize1 = anticlockcode1.size();

	fprintf(fp_chain, "chaincode-algorithm-xy  %10d, %10d, %10d, %3.3f\n", clocksize, anticlocksize, anticlocksize1, distance);
	if (right.x - left.x >= top.y - bottom.y)
	{
		start.x = left.x;
		start.y = left.y;
		end.x = right.x;
		end.y = right.y;
	}
	else
	{
		start.x = bottom.x;
		start.y = bottom.y;
		end.x = top.x;
		end.y = top.y;
	}
	maskimage[start.y*m_width + start.x] = 1;

	if (clocksize >= anticlocksize)
	{
		if (clocksize >= anticlocksize1)
		{
			retflag = 1;
			for (k = 0; k < clocksize; k++)
			{
				int jj = clockcode.at(k);
				chaincode.push_back(jj);
				start.x = start.x + clock[jj][0];
				start.y = start.y + clock[jj][1];
				chainposition.push_back(start);
				maskimage[start.y*m_width + start.x] = 1;
				fprintf(fp_chain, "chaincode-function-xy  %10d,%10d, %10d\n", start.x, start.y, jj);
			}
		}
		else
		{
			retflag = 2;
			for (k = 0; k < anticlocksize1; k++)
			{
				int jj = anticlockcode1.at(k);
				chaincode.push_back(jj);
				start.x = start.x + anticlock1[jj][0];
				start.y = start.y + anticlock1[jj][1];
				chainposition.push_back(start);
				maskimage[start.y*m_width + start.x] = 2;
				fprintf(fp_chain, "chaincode-function-xy  %10d,%10d, %10d\n", start.x, start.y, jj);
			}
		}
	}
	else
	{
		if (anticlocksize >= anticlocksize1)
		{
			retflag = 0;
			for (k = 0; k < anticlocksize; k++)
			{
				int jj = anticlockcode.at(k);
				chaincode.push_back(jj);
				start.x = start.x + anticlock[jj][0];
				start.y = start.y + anticlock[jj][1];
				chainposition.push_back(start);
				maskimage[start.y*m_width + start.x] = 3;
				fprintf(fp_chain, "chaincode-function-xy  %10d,%10d, %10d\n", start.x, start.y, jj);
			}
		}
		else
		{
			retflag = 2;
			for (k = 0; k < anticlocksize1; k++)
			{
				int jj = anticlockcode1.at(k);
				chaincode.push_back(jj);
				start.x = start.x + anticlock1[jj][0];
				start.y = start.y + anticlock1[jj][1];
				chainposition.push_back(start);
				maskimage[start.y*m_width + start.x] = 2;
				fprintf(fp_chain, "chaincode-function-xy  %10d,%10d, %10d\n", start.x, start.y, jj);         //多考虑了一种情况？
			}
		}
	}

	/*clockcode.clear();
	anticlockcode.clear();
	anticlockcode1.clear();
	chaincode.clear();
	chainposition.clear();*/
	delete antichainmask;
	delete antichainmask1;

	return retflag;






}
int Edgeanalysis::chaincode_comph(unsigned char *chainmask, Coord left, Coord right, Coord top, Coord bottom, unsigned char *maskimage)
{


	int i, j, k;
	double distance, clockdist, anticlockdist, anticlockdist1;
	Coord start, end, cur, prev;
	Coord clockstart, anticlockstart, anticlockstart1;
	bool burrflag;

	//chain code analysis	
	int clock[8][2] = { { 1, 0 }, { 1, -1 }, { 1, -1 }, { 1, -1 }, { 1, -1 }, { -1, 1 }, { 0, 1 }, { 1, 1 } };          //跳过2，3，4方向，避免搜索原路返回
	int anticlock[8][2] = { { 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 1 }, { -1, 0 }, { -1, -1 }, { 0, -1 }, { 1, -1 } };
	int anticlock1[8][2] = { { -1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 1 }, { 1, 0 }, { 1, -1 }, { 0, -1 }, { -1, -1 } };

	unsigned char *antichainmask = new unsigned char[m_width*m_height];
	unsigned char *antichainmask1 = new unsigned char[m_width*m_height];

	clockcode.clear();
	anticlockcode.clear();
	anticlockcode1.clear();
	chaincode.clear();
	chainposition.clear();

	if (right.x - left.x >= top.y - bottom.y)
	{
		start.x = left.x;
		start.y = left.y;
		end.x = right.x;
		end.y = right.y;
	}
	else
	{
		start.x = bottom.x;
		start.y = bottom.y;
		end.x = top.x;
		end.y = top.y;
	}

	if (left.x == m_width || top.y == m_height)
	{
		start.x = 0;
		start.y = 0;
		end.x = 0;
		end.y = 0;
		return -1;
	}
	//maoci
	bool padflag, proflag;
	do{
		proflag = true;

		for (j = bottom.y - 1; j < top.y + 1; j++)
		{
			for (i = left.x - 1; i < right.x + 1; i++)
			{
				if ((i != start.x && j != start.y) || (i != end.x && j != end.y))
				{
					if (chainmask[j*m_width + i] == 1)
					{
						int boundvalue;
						int  f1, f2, f3, f4, f5, f6, f7, f8;
						if (((i + 1) == start.x && (j + 0) == start.y) || ((i + 1) == end.x && (j + 0) == end.y))
							f1 = 10;
						else
							f1 = chainmask[(j + 0)*m_width + (i + 1)];

						if (((i + 1) == start.x && (j + 1) == start.y) || ((i + 1) == end.x && (j + 1) == end.y))
							f2 = 10;
						else
							f2 = chainmask[(j + 1)*m_width + (i + 1)];

						if (((i + 0) == start.x && (j + 1) == start.y) || ((i + 0) == end.x && (j + 1) == end.y))
							f3 = 10;
						else
							f3 = chainmask[(j + 1)*m_width + (i + 0)];

						if (((i - 1) == start.x && (j + 1) == start.y) || ((i - 1) == end.x && (j + 1) == end.y))
							f4 = 10;
						else
							f4 = chainmask[(j + 1)*m_width + (i - 1)];

						if (((i - 1) == start.x && (j + 0) == start.y) || ((i - 1) == end.x && (j + 0) == end.y))
							f5 = 10;
						else
							f5 = chainmask[(j + 0)*m_width + (i - 1)];

						if (((i - 1) == start.x && (j - 1) == start.y) || ((i - 1) == end.x && (j - 1) == end.y))
							f6 = 10;
						else
							f6 = chainmask[(j - 1)*m_width + (i - 1)];

						if (((i + 0) == start.x && (j - 1) == start.y) || ((i + 0) == end.x && (j - 1) == end.y))
							f7 = 10;
						else
							f7 = chainmask[(j - 1)*m_width + (i + 0)];

						if (((i + 1) == start.x && (j - 1) == start.y) || ((i + 1) == end.x && (j - 1) == end.y))
							f8 = 10;
						else
							f8 = chainmask[(j - 1)*m_width + (i + 1)];

						boundvalue = f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8;

						if (boundvalue == 1 || boundvalue == 0)
						{
							proflag = false;
							chainmask[j*m_width + i] = 0;
						}
					}
				}
			}
		}
		if (proflag == true)
			padflag = false;
		else
			padflag = true;

	} while (padflag == true);

	for (j = bottom.y - 1; j < top.y + 1; j++)
	{
		for (i = left.x - 1; i < right.x + 1; i++)
		{
			if (chainmask[j*m_width + i] == 1)
			{
				antichainmask[j*m_width + i] = 1;
				antichainmask1[j*m_width + i] = 1;

				//maskimage[j*m_width+i] = 2;
				fprintf(fp_chain, "maskcode-function-xy  %10d,%10d\n", i, j);
			}
		}
	}

	//step1:顺时针方向搜索链码
	if (right.x - left.x >= top.y - bottom.y)
	{
		start.x = left.x;
		start.y = left.y;
		end.x = right.x;
		end.y = right.y;
	}
	else
	{
		start.x = bottom.x;
		start.y = bottom.y;
		end.x = top.x;
		end.y = top.y;
	}

	if (left.x == m_width || top.y == m_height)
	{
		start.x = 0;
		start.y = 0;
		end.x = 0;
		end.y = 0;
		return -1;
	}
	distance = sqrtf((end.x - start.x)*(end.x - start.x) + (end.y - start.y)*(end.y - start.y));

	//初始链码
	clockstart.x = start.x;
	clockstart.y = start.y;

	chainmask[start.y*m_width + start.x] = 0;
	clockcode.push_back(0);

	do{
		for (k = 0; k < 8; k++)
		{
			burrflag = false;
			cur.x = start.x + clock[k][1];
			cur.y = start.y + clock[k][0];

			if ((cur.x > -1) && (cur.x < m_width) && (cur.y > -1) && (cur.y < m_height))
			{
				if (chainmask[cur.y*m_width + cur.x] == 1)
				{
					chainmask[cur.y*m_width + cur.x] = 0;
					clockcode.push_back(k);
					start.x = cur.x;
					start.y = cur.y;
					burrflag = true;
					break;
				}
			}
		}
		if (burrflag)
		{
			int distx = abs(start.x - end.x);
			int disty = abs(start.y - end.y);

			if (distx < 1 && disty <1)      //(StartP.x == right.x) && (StartP.y == right.y)
				burrflag = true;
			else
				burrflag = false;
		}
		else
			burrflag = true;

	} while (burrflag == false);
	clockdist = sqrtf((clockstart.x - start.x)*(clockstart.x - start.x) + (clockstart.y - start.y)*(clockstart.y - start.y));

	//step2:逆时针方向搜索链码
	if (right.x - left.x >= top.y - bottom.y)
	{
		start.x = left.x;
		start.y = left.y;
		end.x = right.x;
		end.y = right.y;
	}
	else
	{
		start.x = bottom.x;
		start.y = bottom.y;
		end.x = top.x;
		end.y = top.y;
	}

	if (left.x == m_width || top.y == m_height)
	{
		start.x = 0;
		start.y = 0;
		end.x = 0;
		end.y = 0;
		return -1;
	}

	anticlockstart.x = start.x;
	anticlockstart.y = start.y;

	antichainmask[start.y*m_width + start.x] = 0;
	anticlockcode.push_back(0);
	do{
		for (k = 0; k < 8; k++)
		{
			burrflag = false;
			cur.x = start.x + anticlock[k][1];
			cur.y = start.y + anticlock[k][0];

			if ((cur.x > -1) && (cur.x < m_width) && (cur.y > -1) && (cur.y < m_height))
			{
				if (antichainmask[cur.y*m_width + cur.x] == 1)
				{
					antichainmask[cur.y*m_width + cur.x] = 0;
					anticlockcode.push_back(k);
					start.x = cur.x;
					start.y = cur.y;
					burrflag = true;
					break;
				}
			}
		}
		if (burrflag)
		{
			int distx = abs(start.x - end.x);
			int disty = abs(start.y - end.y);

			if (distx < 1 && disty <1)      //(StartP.x == right.x) && (StartP.y == right.y)
				burrflag = true;
			else
				burrflag = false;
		}
		else
			burrflag = true;

	} while (burrflag == false);
	anticlockdist = sqrtf((anticlockstart.x - start.x)*(anticlockstart.x - start.x) + (anticlockstart.y - start.y)*(anticlockstart.y - start.y));

	//step3:延迟方向搜索链码
	if (right.x - left.x >= top.y - bottom.y)
	{
		start.x = left.x;
		start.y = left.y;
		end.x = right.x;
		end.y = right.y;
	}
	else
	{
		start.x = bottom.x;
		start.y = bottom.y;
		end.x = top.x;
		end.y = top.y;
	}

	if (left.x == m_width || top.y == m_height)
	{
		start.x = 0;
		start.y = 0;
		end.x = 0;
		end.y = 0;
		return -1;
	}

	anticlockstart1.x = start.x;
	anticlockstart1.y = start.y;

	antichainmask1[start.y*m_width + start.x] = 0;
	anticlockcode1.push_back(0);
	do{
		for (k = 0; k < 8; k++)
		{
			burrflag = false;
			cur.x = start.x + anticlock1[k][1];
			cur.y = start.y + anticlock1[k][0];

			if ((cur.x > -1) && (cur.x < m_width) && (cur.y > -1) && (cur.y < m_height))
			{
				if (antichainmask1[cur.y*m_width + cur.x] == 1)
				{
					antichainmask1[cur.y*m_width + cur.x] = 0;
					anticlockcode1.push_back(k);
					start.x = cur.x;
					start.y = cur.y;
					burrflag = true;
					break;
				}
			}
		}
		if (burrflag)
		{
			int distx = abs(start.x - end.x);
			int disty = abs(start.y - end.y);

			if (distx < 1 && disty <1)      //(StartP.x == right.x) && (StartP.y == right.y)
				burrflag = true;
			else
				burrflag = false;
		}
		else
			burrflag = true;

	} while (burrflag == false);
	anticlockdist1 = sqrtf((anticlockstart1.x - start.x)*(anticlockstart1.x - start.x) + (anticlockstart1.y - start.y)*(anticlockstart1.y - start.y));

	//step4:判决最优链码
	int retflag;
	int clocksize = clockcode.size();	//当前链码长度
	int anticlocksize = anticlockcode.size();
	int anticlocksize1 = anticlockcode1.size();

	fprintf(fp_chain, "chaincode-algorithm-xy  %10d, %10d, %10d, %3.3f\n", clocksize, anticlocksize, anticlocksize1, distance);
	if (right.x - left.x >= top.y - bottom.y)
	{
		start.x = left.x;
		start.y = left.y;
		end.x = right.x;
		end.y = right.y;
	}
	else
	{
		start.x = bottom.x;
		start.y = bottom.y;
		end.x = top.x;
		end.y = top.y;
	}
	maskimage[start.y*m_width + start.x] = 1;

	if (clocksize >= anticlocksize)
	{
		if (clocksize >= anticlocksize1)
		{
			retflag = 1;
			for (k = 0; k < clocksize; k++)
			{
				int jj = clockcode.at(k);
				chaincode.push_back(jj);
				start.x = start.x + clock[jj][1];
				start.y = start.y + clock[jj][0];
				chainposition.push_back(start);
				maskimage[start.y*m_width + start.x] = 1;
				fprintf(fp_chain, "chaincode-function-xy  %10d,%10d, %10d\n", start.x, start.y, jj);
			}
		}
		else
		{
			retflag = 2;
			for (k = 0; k < anticlocksize1; k++)
			{
				int jj = anticlockcode1.at(k);
				chaincode.push_back(jj);
				start.x = start.x + anticlock1[jj][1];
				start.y = start.y + anticlock1[jj][0];
				chainposition.push_back(start);
				maskimage[start.y*m_width + start.x] = 2;
				fprintf(fp_chain, "chaincode-function-xy  %10d,%10d, %10d\n", start.x, start.y, jj);
			}
		}
	}
	else
	{
		if (anticlocksize >= anticlocksize1)
		{
			retflag = 0;
			for (k = 0; k < anticlocksize; k++)
			{
				int jj = anticlockcode.at(k);
				chaincode.push_back(jj);
				start.x = start.x + anticlock[jj][1];
				start.y = start.y + anticlock[jj][0];
				chainposition.push_back(start);
				maskimage[start.y*m_width + start.x] = 3;
				fprintf(fp_chain, "chaincode-function-xy  %10d,%10d, %10d\n", start.x, start.y, jj);
			}
		}
		else
		{
			retflag = 2;
			for (k = 0; k < anticlocksize1; k++)
			{
				int jj = anticlockcode1.at(k);
				chaincode.push_back(jj);
				start.x = start.x + anticlock1[jj][1];
				start.y = start.y + anticlock1[jj][0];
				chainposition.push_back(start);
				maskimage[start.y*m_width + start.x] = 2;
				fprintf(fp_chain, "chaincode-function-xy  %10d,%10d, %10d\n", start.x, start.y, jj);         //多考虑了一种情况？
			}
		}
	}

	/*clockcode.clear();
	anticlockcode.clear();
	anticlockcode1.clear();
	chaincode.clear();
	chainposition.clear();*/
	delete antichainmask;
	delete antichainmask1;

	return retflag;






}


int Edgeanalysis::chaincode_compcu(int *blobmask, unsigned char *maskimage, unsigned char *chaincode,int *chainposition, int w, int h)           //使用数组形式的链码排序函数
{
	for (int n = 0; n < h;n++){                                         //最外层的循环相当于遍历blobmask的每一行
		bool burrflag = false;
		//chain code analysis	
		unsigned char clock[8][2] = { { 1, 0 }, { 1, -1 }, { 1, -1 }, { 1, -1 }, { 1, -1 }, { -1, 1 }, { 0, 1 }, { 1, 1 } };           //跳过2，3，4方向，避免搜索原路返回
		unsigned char anticlock[8][2] = { { 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 1 }, { -1, 0 }, { -1, -1 }, { 0, -1 }, { 1, -1 } };
		unsigned char anticlock1[8][2] = { { -1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 1 }, { 1, 0 }, { 1, -1 }, { 0, -1 }, { -1, -1 } };
		//重排序后的blobmask，startx和starty在起点处，按x为主序排列（符合8邻域内的x的条件 差为1，-1，0）

		//链表跟踪值会超出寄存器容量吗？
		//如果可以直接new分配存储空间的话，全零初始化显得没有必要，因为有自带的计数器
		unsigned char *clockcode = new unsigned char[w];             //用chaincode和clockcode两个存储空间来比较3个方向并将结果输出到chaincode（不合适，其他线程也会对这个chaincode进行写操作，会出错，还是先检查BUG，以及归类大BLOB和小BLOB比较合适）
		unsigned char *anticlockcode = new unsigned char[w];          //不基于图像的搜索（循环遍历）很繁琐，可以考虑在后期加上预先的搜索函数（双循环，找当前坐标的下一个相邻坐标）
		unsigned char *anticlockcode1 = new unsigned char[w];
		int *tempmask = new int[2 * w];
		int *antichainmask = new int[2 * w];
		int *antichainmask1 = new int[2 * w];


		//这里chainposition因为是作为输出变量，不是函数内变量，所以不用new初始化，直接写入数据即可，而记录方向的clockcode等变量是为了后续的比较长度等等
		// 对应CPU版本中的vector模型，使用thrust库来对应，希望接口不会出问题,因为对应每一个blob，直接移到执行函数内部,因为thrust和fprintf一样是host函数，所以此处用不了

		//因为CPU中用到了vector向量，所以不急着实施，先看看输出图像效果怎样（链表分析函数是否顺利运行）
		int rx, ry, lx, ly, tx, ty, bx, by;  //分别储存四个坐标（八个数值）
		int startx, starty, endx, endy;            //把所以涉及坐标的变量都变成short类型，不然会导致int转换到short时的精度缺失
		int *blobmaskcu            = NULL;
		int *chainpositioncu       = NULL;
		unsigned char *chaincodecu = NULL;
		blobmaskcu = &blobmask[2*n*w];                        //三个数据结构，一个输入，两个输出，分别对应每一行的行首
		chainpositioncu = &chainposition[2*n*w];
		chaincodecu = &chaincode[n*w];
		rx = lx = tx = bx = blobmaskcu[0];            //每一行一共有2w个元素（x,y坐标），所以坐标后缀应为2*n*w
		ry = ly = ty = by = blobmaskcu[1];
		for (int i = 0; i < w; i++)
		{
			if ((blobmaskcu[2 * i] > 0) && (blobmaskcu[2 * i + 1] > 0)){
				if (blobmaskcu[2 * i] > rx)
				{
					rx = blobmaskcu[2 * i];
					ry = blobmaskcu[2 * i + 1];
				}
				if (blobmaskcu[2 * i] < lx)
				{
					lx = blobmaskcu[2 * i];
					ly = blobmaskcu[2 * i + 1];
				}
				if (blobmaskcu[2 * i + 1] > ty)
				{
					tx = blobmaskcu[2 * i];
					ty = blobmaskcu[2 * i + 1];
				}
				if (blobmaskcu[2 * i + 1] < by)
				{
					bx = blobmaskcu[2 * i];
					by = blobmaskcu[2 * i + 1];
				}
			}
		}
		if (rx - lx >= ty - by)
		{
			startx = lx;
			starty = ly;
			endx = rx;
			endy = ry;
		}
		else
		{
			startx = bx;
			starty = by;
			endx = tx;
			endy = ty;
		}

		if (lx == WIDTH || ty == HEIGHT)
		{
			startx = 0;
			starty = 0;
			endx = 0;
			endy = 0;
			return -1;
		}
		/*tempmask[0] = startx;
		tempmask[1] = starty;*/

		//c:对应于每一行的blob的实际长度
		//默认前提：i=w-1，m=w-2时，最大blob的终点（blobmask[2*i]）在次点的邻域内（被包含进去）
		int c = 0;
		for (int m = 0; m < w; m++){
			if (blobmaskcu[2 * m] > 0){
				int mx = (blobmaskcu[2 * m]);
				int my = (blobmaskcu[2 * m + 1]);//作为被比较的点也得放进tempmask数组中
				tempmask[2 * c]     = mx;
				tempmask[2 * c + 1] = my;
				/*int x = tempmask[2 * c];
				int y = tempmask[2 * c + 1];
				maskimage[y*WIDTH + x] = 1;*/
				c++;
			}
		}

		//因为退出循环前c还加了一次，所以c应该代表的是元素个数
		for (int p = 0; p < c; p++)
		{

			int x = tempmask[2 * p];
			int y = tempmask[2 * p + 1];
			antichainmask[2 * p]  = x;
			antichainmask1[2 * p] = x;
			antichainmask[2 * p + 1]  = y;
			antichainmask1[2 * p + 1] = y;
			//maskimage[y*WIDTH + x] = 1;

		}

		int count, anticount, antcount;     //相当于clock.size()记数
		int curx, cury;                    //记录目前搜索的坐标位置
		//初始链码
		/*clockstart.x = start.x;
		clockstart.y = start.y;*/
		if (rx - lx >= ty - by)
		{
			startx = lx;
			starty = ly;
			endx = rx;
			endy = ry;
		}
		else
		{
			startx = bx;
			starty = by;
			endx = tx;
			endy = ty;
		}

		if (lx == WIDTH || ty == HEIGHT)
		{
			startx = 0;
			starty = 0;
			endx = 0;
			endy = 0;
			return -1;
		}
		for (int i = 0; i < c; i++){
			if ((tempmask[2 * i] == startx) && (tempmask[2 * i + 1] == starty)){
				//maskimage[y*WIDTH + x] = 1;
				tempmask[2 * i] = 0;
				tempmask[2 * i + 1] = 0;   //开头的两个都得赋值为0
				break;
			}
		}
		clockcode[0] = 0;
		count = 1;

		for (int q = 0; q < c; q++)         //搜索到的点个数肯定小于c,用最外层的for循环来取代可能造成死循环的do-while结构
		{
			if (burrflag == false){
				for (int k = 0; k < 8; k++)
				{
					burrflag = false;
					curx = startx + clock[k][0];                   //ushort和uchar之间的加法不需要强制类型转换?根据产生的结果来判断是否需要强制转换
					cury = starty + clock[k][1];
					if ((curx > -1) && (curx < WIDTH) && (cury > -1) && (cury < HEIGHT))
					{
						for (int i = 0; i < c; i++)
						{
							if ((tempmask[2 * i] == curx) && (tempmask[2 * i + 1] == cury))
							{
								//maskimage[cury*WIDTH + curx] = 1;       //直接测试第一个函数是否能成功
								tempmask[2 * i] = tempmask[2 * i + 1] = 0;
								clockcode[count] = k;
								count++;
								startx = curx;
								starty = cury;
								burrflag = true;
								goto verify;           //break只能跳出最内层的循环，而break总是与if成对出现，来跳出循环
							}
						}

					}
				}
			verify:
				if (burrflag)
				{
					int distx = abs(startx - endx);           //不同类型的变量转换很繁琐
					int disty = abs(starty - endy);

					if (distx < 1 && disty < 1)      //(StartP.x == right.x) && (StartP.y == right.y)
						burrflag = true;
					else
						burrflag = false;
				}
				else
					burrflag = true;
			}
		}
		//while (burrflag == false);
		//clockdist = sqrtf((clockstart.x - start.x)*(clockstart.x - start.x) + (clockstart.y - start.y)*(clockstart.y - start.y));

		//step2:逆时针方向搜索链码

		if (rx - lx >= ty - by)
		{
			startx = lx;
			starty = ly;
			endx = rx;
			endy = ry;
		}
		else
		{
			startx = bx;
			starty = by;
			endx = tx;
			endy = ty;
		}

		if (lx == WIDTH || ty == HEIGHT)
		{
			startx = 0;
			starty = 0;
			endx = 0;
			endy = 0;
			return -1;
		}
		///*anticlockstart.x = start.x;
		//anticlockstart.y = start.y;*/
		//三个搜索函数必须注意三个变量 clockcode,count,tempmask分别对应改变
		for (int i = 0; i < c; i++){
			if ((antichainmask[2 * i] == startx) && (antichainmask[2 * i + 1] == starty)){
				antichainmask[2 * i] = 0;
				antichainmask[2 * i + 1] = 0;   //开头的两个都得赋值为0
				break;
			}
		}
		anticlockcode[0] = 0;
		anticount = 1;
		burrflag = false;
		for (int q = 0; q < c; q++)         //搜索到的点个数肯定小于c,用最外层的for循环来取代可能造成死循环的do-while结构
		{
			if (burrflag == false){
				for (int k = 0; k < 8; k++)
				{
					burrflag = false;
					curx = startx + anticlock[k][0];
					cury = starty + anticlock[k][1];
					if ((curx > -1) && (curx < WIDTH) && (cury > -1) && (cury < HEIGHT))
					{
						for (int i = 0; i < c; i++)
						{
							if ((antichainmask[2 * i] == curx) && (antichainmask[2 * i + 1] == cury))
							{
								//maskimage[cury*WIDTH + curx] = 1;       //直接测试第一个函数是否能成功
								antichainmask[2 * i] = antichainmask[2 * i + 1] = 0;
								anticlockcode[anticount] = k;
								anticount++;
								startx = curx;
								starty = cury;
								burrflag = true;
								goto antiverify;           //break只能跳出最内层的循环，而break总是与if成对出现，来跳出循环
							}
						}

					}
				}
			antiverify:
				if (burrflag)
				{
					int distx = abs(startx - endx);
					int disty = abs(starty - endy);

					if (distx < 1 && disty < 1)      //(StartP.x == right.x) && (StartP.y == right.y)
						burrflag = true;
					else
						burrflag = false;
				}
				else
					burrflag = true;
			}
		}

		//} while (burrflag == false);
		////anticlockdist = sqrtf((anticlockstart.x - start.x)*(anticlockstart.x - start.x) + (anticlockstart.y - start.y)*(anticlockstart.y - start.y));

		////step3:延迟方向搜索链码

		if (rx - lx >= ty - by)
		{
			startx = lx;
			starty = ly;
			endx = rx;
			endy = ry;
		}
		else
		{
			startx = bx;
			starty = by;
			endx = tx;
			endy = ty;
		}

		if (lx == WIDTH || ty == HEIGHT)
		{
			startx = 0;
			starty = 0;
			endx = 0;
			endy = 0;
			return -1;
		}
		///*anticlockstart1.x = start.x;
		//anticlockstart1.y = start.y;*/

		//antichainmask1[0] = 0;
		//antichainmask1[1] = 0;
		//anticlockcode1[0] = 0;
		//antcount = 1;
		for (int i = 0; i < c; i++){
			if ((antichainmask1[2 * i] == startx) && (antichainmask1[2 * i + 1] == starty)){
				antichainmask1[2 * i] = 0;
				antichainmask1[2 * i + 1] = 0;   //开头的两个都得赋值为0
				break;
			}
		}
		anticlockcode1[0] = 0;
		antcount = 1;
		burrflag = false;
		for (int q = 0; q < c; q++)         //搜索到的点个数肯定小于c,用最外层的for循环来取代可能造成死循环的do-while结构
		{
			if (burrflag == false){
				for (int k = 0; k < 8; k++)
				{
					burrflag = false;
					curx = startx + anticlock1[k][0];
					cury = starty + anticlock1[k][1];
					if ((curx > -1) && (curx < WIDTH) && (cury > -1) && (cury < HEIGHT))
					{
						for (int i = 0; i < c; i++)
						{
							if ((antichainmask1[2 * i] == curx) && (antichainmask1[2 * i + 1] == cury))
							{
								//maskimage[cury*WIDTH + curx] = 1;       //直接测试第一个函数是否能成功
								antichainmask1[2 * i] = antichainmask1[2 * i + 1] = 0;
								anticlockcode1[antcount] = k;
								antcount++;
								startx = curx;
								starty = cury;
								burrflag = true;
								goto antverify;           //break只能跳出最内层的循环，而break总是与if成对出现，来跳出循环
							}
						}

					}
				}
			antverify:
				if (burrflag)
				{
					int distx = abs(startx - endx);
					int disty = abs(starty - endy);

					if (distx < 1 && disty < 1)      //(StartP.x == right.x) && (StartP.y == right.y)
						burrflag = true;
					else
						burrflag = false;
				}
				else
					burrflag = true;
			}
		}


		//} while (burrflag == false);
		//anticlockdist1 = sqrtf((anticlockstart1.x - start.x)*(anticlockstart1.x - start.x) + (anticlockstart1.y - start.y)*(anticlockstart1.y - start.y));

		//step4:判决最优链码
		int retflag = 1;
		//int clocksize = clockcode.size();	//所有标准库对应的函数都能在thrust中使用？重载是个好东西
		//int anticlocksize = anticlockcode.size();
		//int anticlocksize1 = anticlockcode1.size();

		//fprintf(fp_chain, "chaincode-algorithm-xy  %10d, %10d, %10d, %3.3f\n", clocksize, anticlocksize, anticlocksize1);
		if (rx - lx >= ty - by)
		{
			startx = lx;
			starty = ly;
			endx = rx;
			endy = ry;
		}
		else
		{
			startx = bx;
			starty = by;
			endx = tx;
			endy = ty;
		}

		if (lx == WIDTH || ty == HEIGHT)
		{
			startx = 0;
			starty = 0;
			endx = 0;
			endy = 0;
			return -1;
		}
		maskimage[(starty)*WIDTH + (startx)] = 1;                //直接通过反坐标系转换映射到大图maskimage中，而不是对应每个blob的小图temp
		chainpositioncu[0] = startx;                               //short转换为int不需要强制类型转换
		chainpositioncu[1] = starty;
		//不用将起点的坐标写入chainposition吗？
		if (count >= anticount)
		{




			if (count >= antcount)
			{
				retflag = 1;
				for (int k = 0; k < count; k++)
				{
					int jj = clockcode[k];  //at()有边界检查，而operator没有，有边界溢出可能
					chaincodecu[k] = jj;
					startx = startx + clock[jj][0];
					starty = starty + clock[jj][1];
					chainpositioncu[2 * (k + 1) + 0] = startx;
					chainpositioncu[2 * (k + 1) + 1] = starty;
					maskimage[(starty)*WIDTH + (startx)] = 1;
					//fprintf(fp_chain, "chaincode-function-xy  %10d,%10d, %10d\n", start.x, start.y, jj);
				}
			}
			else
			{
				retflag = 2;
				for (int k = 0; k < antcount; k++)
				{
					int jj = anticlockcode1[k];
					chaincodecu[k] = jj;
					startx = startx + anticlock1[jj][0];
					starty = starty + anticlock1[jj][1];
					//chainposition.push_back(start);
					maskimage[(starty)*WIDTH + (startx)] = 2;
					chainpositioncu[2 * (k + 1) + 0] = startx;
					chainpositioncu[2 * (k + 1) + 1] = starty;
				}
			}
		}
		else
		{
			if (anticount >= antcount)
			{
				retflag = 0;
				for (int k = 0; k < anticount; k++)
				{
					int jj = anticlockcode[k];
					chaincodecu[k] = jj;
					startx = startx + anticlock[jj][0];
					starty = starty + anticlock[jj][1];
					//chainposition.push_back(start);
					maskimage[(starty)*WIDTH + (startx)] = 3;
					chainpositioncu[2 * (k + 1) + 0] = startx;
					chainpositioncu[2 * (k + 1) + 1] = starty;
				}
			}
			else
			{
				retflag = 2;
				for (int k = 0; k < antcount; k++)
				{
					int jj = anticlockcode1[k];
					chaincodecu[k] = jj;
					startx = startx + anticlock1[jj][0];
					starty = starty + anticlock1[jj][1];
					//chainposition.push_back(start);
					maskimage[(starty)*WIDTH + (startx)] = 2;
					chainpositioncu[2 * (k + 1) + 0] = startx;
					chainpositioncu[2 * (k + 1) + 1] = starty;
				}
			}
		}

		/*clockcode.clear();
		anticlockcode.clear();
		anticlockcode1.clear();
		chaincode.clear();
		chainposition.clear();*/
		delete tempmask;
		delete antichainmask;
		delete antichainmask1;
		delete clockcode;
		delete anticlockcode;
		delete anticlockcode1;
		//delete chaincode;        //如果要观察chaincode的话，应当copy保存至一个数组中输出，以便后续传到host端
		return retflag;

	}








}
