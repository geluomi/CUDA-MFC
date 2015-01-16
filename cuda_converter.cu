//先测试竖直黑线 水平模板
#include "cuda_converter.h"
#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>
#include "device_launch_parameters.h"
#include "device_functions.h"


#define BLOCKSIZE1 64
#define BLOCKSIZE2 48
#define BLOCKSIZE3 16
#define WIDTH      720
#define HEIGHT     576
typedef unsigned char     BYTE;
typedef unsigned int      uint;



cuda_converter::cuda_converter(void)
{
}

cuda_converter::~cuda_converter(void)
{
}
int cuda_converter::InitCUDA()
{
	/************************************************************************/
	/* Init CUDA                                                            */
	/************************************************************************/

	int count = 0;
	int i = 0;

	cudaGetDeviceCount(&count);
	if (count == 0) {
		fprintf(stderr, "There is no device.\n");
		return false;
	}

	for (i = 0; i < count; i++) {
		cudaDeviceProp prop;
		if (cudaGetDeviceProperties(&prop, i) == cudaSuccess) {
			if (prop.major >= 1) {
				break;
			}
		}
	}
	if (i == count) {
		fprintf(stderr, "There is no device supporting CUDA.\n");
		return false;
	}
	cudaSetDevice(i);

	printf("CUDA initialized.\n");
	return true;

}


int cuda_converter::TranslateOutput(int width, int height, unsigned char *out_data, unsigned char *ou_data)  //out_data:d_result   直接输出四个独立的二值化标记图
{
	int size = width * height;
	cudaMemcpy(out_data, d_blackver, sizeof(unsigned char)*size, cudaMemcpyDeviceToHost);
	cudaMemcpy(ou_data, d_blackhor, sizeof(unsigned char)*size, cudaMemcpyDeviceToHost);
	return 1;
}

int cuda_converter::TranslateOutputB(int width, int height, unsigned char *out_data, unsigned char *ou_data)  //out_data:d_result   直接输出四个独立的二值化标记图
{
	int size = width * height;
	cudaMemcpy(out_data, d_whitever, sizeof(unsigned char)*size, cudaMemcpyDeviceToHost);
	cudaMemcpy(ou_data, d_whitehor, sizeof(unsigned char)*size, cudaMemcpyDeviceToHost);
	return 1;
}
int cuda_converter::TranslateInput(int width, int height, unsigned char *pImage)        //输入是灰度图Y矩阵,所以不必乘以3
{
	cudaMalloc((void**)&d_yuv, sizeof(unsigned char)*width*height);
	cudaMemcpy(d_yuv, pImage, sizeof(unsigned char)*width*height, cudaMemcpyHostToDevice);

	return 0;
}

///////////////////////////////////////////////////////blob gpu mem
int cuda_converter::TranslateInputB(int width, int height, int *pImage)        //输入是灰度图Y矩阵,所以不必乘以3
{
	cudaMalloc((void**)&d_ptr, sizeof(int)*2*width*height);               //已经对d_ptr赋值，所以MallocMemP中没必要再赋值一遍
	cudaMemcpy(d_ptr, pImage,  sizeof(int)*2*width*height, cudaMemcpyHostToDevice);

	return 0;
}

void cuda_converter::MallocMemP(unsigned int len)  //len=maxlinewidth*num*2 lenn=720*576
{
	int m_ret_len = len;

	//cudaMalloc((void**)&d_ptr,    sizeof(unsigned char)* m_ret_len); //output 黑色垂直线标记图
	cudaMalloc((void**)&d_outptr, sizeof(int)* m_ret_len); //对应于d_ptr的*chainposition,和d_ptr一样大小
	cudaMemset(d_outptr, 0, sizeof(int)*m_ret_len);                              //为了便于从输出中提取信息，chainposition数组应在一开始全0初始化
}


void cuda_converter::MallocMemM(unsigned int len)  //len=maxlinewidth*num*2 lenn=720*576
{
	int m_ret_len = len;
	cudaMalloc((void**)&d_maskimg, sizeof(unsigned char)* m_ret_len); //输出：相当于CPU端的maskimage
	cudaMemset(d_maskimg, 0, sizeof(unsigned char)*m_ret_len);
}
int cuda_converter::TranslateOutputM(int len, unsigned char *out_data)        //输入是灰度图Y矩阵,所以不必乘以3
{
	int m_ret_len = len;
	cudaMemcpy(out_data, d_maskimg, sizeof(unsigned char)*m_ret_len, cudaMemcpyDeviceToHost);
	return 1;
}
int cuda_converter::ReleaseMemM()
{

	cudaFree(d_maskimg);
	return 1;
}
///////////////////////对应horizontal水平线的并行函数///////////////////////////////////////////////
int cuda_converter::TranslateInputH(int width, int height, int *pImage)        //输入是灰度图Y矩阵,所以不必乘以3
{
	cudaMalloc((void**)&d2_ptr, sizeof(int) * 2 * width*height);               //已经对d_ptr赋值，所以MallocMemP中没必要再赋值一遍
	cudaMemcpy(d2_ptr, pImage, sizeof(int) * 2 * width*height, cudaMemcpyHostToDevice);

	return 1;
}

void cuda_converter::MallocMemH(unsigned int len)
{
	int m_ret_len = len;
	cudaMalloc((void**)&d2_outptr, sizeof(int)* m_ret_len); //对应于d_ptr的*chainposition,和d_ptr一样大小
	cudaMemset(d2_outptr, 0, sizeof(int)*m_ret_len);

}
int cuda_converter::TranslateOutputH(int len, int *out_data)    //len=maxlinewidth*num*2 lenn=720*576
{
	int m_ret_len = len;
	cudaMemcpy(out_data, d2_outptr, sizeof(int)*m_ret_len, cudaMemcpyDeviceToHost);
	return 1;
}

int cuda_converter::ReleaseMemH()
{

	cudaFree(d2_ptr);
	cudaFree(d2_outptr);
	return 1;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////
int cuda_converter::ReleaseMemP()
{

	cudaFree(d_ptr);
	cudaFree(d_outptr);
	return 1;
}
int cuda_converter::TranslateOutputP(int len, int *out_data)    //len=maxlinewidth*num*2 lenn=720*576
{
	int m_ret_len = len;
	cudaMemcpy(out_data , d_outptr,  sizeof(int)*m_ret_len, cudaMemcpyDeviceToHost);
	return 1;
}

__device__ int chaincode(int *blobmask, unsigned char *maskimage, int *chainposition ,int w, int h)           //如果chaincode很复杂，那就必须独立写成一个kernel，这样就关系到d_temp的传输了，从fill()中输出，显存释放函数cudaFree可能不能和初始化函数
{


	//在同一个函数体内，这样的话是否会造成矛盾  w:maxlinewidth h:num
	bool burrflag = false;
	//chain code analysis	
	int clock[8][2] = { { 1, 0 }, { 1, -1 }, { 1, -1 }, { 1, -1 }, { 1, -1 }, { -1, 1 }, { 0, 1 }, { 1, 1 } };           //跳过2，3，4方向，避免搜索原路返回
	int anticlock[8][2] = { { 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 1 }, { -1, 0 }, { -1, -1 }, { 0, -1 }, { 1, -1 } };
	int anticlock1[8][2] = { { -1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 1 }, { 1, 0 }, { 1, -1 }, { 0, -1 }, { -1, -1 } };
	//重排序后的blobmask，startx和starty在起点处，按x为主序排列（符合8邻域内的x的条件 差为1，-1，0）

	//链表跟踪值会超出寄存器容量吗？
	unsigned char *chaincode      = new unsigned char[w];            //如果可以直接new分配存储空间的话，全零初始化显得没有必要，因为有自带的计数器
	unsigned char *clockcode      = new unsigned char[w];
	unsigned char *anticlockcode  = new unsigned char[w];
	unsigned char *anticlockcode1 = new unsigned char[w];
	int *tempmask       = new int[2*w];
	int *antichainmask  = new int[2*w];
	int *antichainmask1 = new int[2*w];


	//这里chainposition因为是作为输出变量，不是函数内变量，所以不用new初始化，直接写入数据即可，而记录方向的clockcode等变量是为了后续的比较长度等等
	// 对应CPU版本中的vector模型，使用thrust库来对应，希望接口不会出问题,因为对应每一个blob，直接移到执行函数内部,因为thrust和fprintf一样是host函数，所以此处用不了

	//因为CPU中用到了vector向量，所以不急着实施，先看看输出图像效果怎样（链表分析函数是否顺利运行）
	int rx, ry, lx, ly, tx, ty, bx, by;  //分别储存四个坐标（八个数值）
	int startx, starty, endx, endy;
	rx = lx = tx = bx = blobmask[0];
	ry = ly = ty = by = blobmask[1];
	for (int i = 0; i < w; i++)
	{
		if ((blobmask[2 * i] > 0) && (blobmask[2 * i + 1] > 0)){
			if (blobmask[2 * i] > rx)
			{
				rx = blobmask[2 * i];
				ry = blobmask[2 * i + 1];
			}
			if (blobmask[2 * i] < lx)
			{
				lx = blobmask[2 * i];
				ly = blobmask[2 * i + 1];
			}
			if (blobmask[2 * i + 1] > ty)
			{
				tx = blobmask[2 * i];
				ty = blobmask[2 * i + 1];
			}
			if (blobmask[2 * i + 1] < by)
			{
				bx = blobmask[2 * i];
				by = blobmask[2 * i + 1];
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
		if (blobmask[2 * m] > 0){
			int mx = blobmask[2 * m];
			int my = blobmask[2 * m + 1];//作为被比较的点也得放进tempmask数组中
			tempmask[2 * c] = mx;
			tempmask[2 * c + 1] = my;
			/*int x = tempmask[2 * c];
			int y = tempmask[2 * c + 1];
			maskimage[y*WIDTH + x] = 1;*/
			c++;
		}
	}
	
    //因为退出循环前c还加了一次，所以c应该代表的是元素个数
	for (int p = 0; p < c;p++)
	{
		
			int x = tempmask[2 * p];
			int y = tempmask[2 * p + 1];
			antichainmask[2 * p] = x;
			antichainmask1[2 * p] = x;
			antichainmask[2 * p + 1] = y;
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
			int x = tempmask[2 * i];
			int y = tempmask[2 * i + 1];
			//maskimage[y*WIDTH + x] = 1;
			tempmask[2*i] = 0;
			tempmask[2*i+1] = 0;   //开头的两个都得赋值为0
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
				curx = startx + clock[k][0];
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
			int x = antichainmask[2 * i];
			int y = antichainmask[2 * i + 1];
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
			int x = antichainmask1[2 * i];
			int y = antichainmask1[2 * i + 1];
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
	chainposition[0] = startx;
	chainposition[1] = starty;
	//不用将起点的坐标写入chainposition吗？
	if (count >= anticount)
	{
		if (count >= antcount)
		{
			retflag = 1;
			for (int k = 0; k < count; k++)
			{
				int jj = clockcode[k];  //at()有边界检查，而operator没有，有边界溢出可能
				chaincode[k] = jj;
				startx = startx + clock[jj][0];
				starty = starty + clock[jj][1];
				chainposition[2 * (k + 1) + 0] = startx;
				chainposition[2 * (k + 1) + 1] = starty;
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
				chaincode[k] = jj;
				startx = startx + anticlock1[jj][0];
				starty = starty + anticlock1[jj][1];
				//chainposition.push_back(start);
				maskimage[(starty)*WIDTH + (startx)] = 2;
				chainposition[2 * (k + 1) + 0] = startx;
				chainposition[2 * (k + 1) + 1] = starty;
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
				chaincode[k] = jj;
				startx = startx + anticlock[jj][0];
				starty = starty + anticlock[jj][1];
				//chainposition.push_back(start);
				maskimage[(starty)*WIDTH + (startx)] = 3;
				chainposition[2 * (k + 1) + 0] = startx;
				chainposition[2 * (k + 1) + 1] = starty;
			}
		}
		else
		{
			retflag = 2;
			for (int k = 0; k < antcount; k++)
			{
				int jj = anticlockcode1[k];
				chaincode[k] = jj;
				startx = startx + anticlock1[jj][0];
				starty = starty + anticlock1[jj][1];
				//chainposition.push_back(start);
				maskimage[(starty)*WIDTH + (startx)] = 2;
				chainposition[2 * (k + 1) + 0] = startx;
				chainposition[2 * (k + 1) + 1] = starty;
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
	delete chaincode;        //如果要观察chaincode的话，应当copy保存至一个数组中输出，以便后续传到host端
	return retflag;
}

__global__ void
d_chaincode_global(int *id, unsigned char *maskimage, int *od,int w, int h){
	unsigned int y = blockIdx.x*blockDim.x + threadIdx.x;
	if (y<h){
		chaincode(&id[2 * y*w], maskimage, &od[2 * y*w],w,h);             //因为每一行实际长度为2*maxlinewidth，所以每一行的行首索引应该是2*y*w
	}
}

double cuda_converter::chaincodekernel(int width, int height)
{
	// var for kernel computation timing
	double dKernelTime;

	//make sure input d_rgb was OK

	//maybe donnot need cudaThreadSynchronize() 

	d_chaincode_global <<<  (height + BLOCKSIZE2 - 1) / BLOCKSIZE2, BLOCKSIZE2, 0 >>>(d_ptr, d_maskimg, d_outptr, width, height);

	return dKernelTime;

}
///////////////////////////////////////////////////////////////////////////
__device__ int chaincodeh(int *blobmask, unsigned char *maskimage, int *chainposition, int w, int h)           //如果chaincode很复杂，那就必须独立写成一个kernel，这样就关系到d_temp的传输了，从fill()中输出，显存释放函数cudaFree可能不能和初始化函数
{


	//在同一个函数体内，这样的话是否会造成矛盾  w:maxlinewidth h:num
	bool burrflag = false;
	//chain code analysis	
	int clock[8][2] = { { 1, 0 }, { 1, -1 }, { 1, -1 }, { 1, -1 }, { 1, -1 }, { -1, 1 }, { 0, 1 }, { 1, 1 } };           //跳过2，3，4方向，避免搜索原路返回
	int anticlock[8][2] = { { 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 1 }, { -1, 0 }, { -1, -1 }, { 0, -1 }, { 1, -1 } };
	int anticlock1[8][2] = { { -1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 1 }, { 1, 0 }, { 1, -1 }, { 0, -1 }, { -1, -1 } };
	//重排序后的blobmask，startx和starty在起点处，按x为主序排列（符合8邻域内的x的条件 差为1，-1，0）

	//链表跟踪值会超出寄存器容量吗？
	unsigned char *chaincode = new unsigned char[w];            //如果可以直接new分配存储空间的话，全零初始化显得没有必要，因为有自带的计数器
	unsigned char *clockcode = new unsigned char[w];
	unsigned char *anticlockcode = new unsigned char[w];
	unsigned char *anticlockcode1 = new unsigned char[w];
	int *tempmask = new int[2 * w];
	int *antichainmask = new int[2 * w];
	int *antichainmask1 = new int[2 * w];


	//这里chainposition因为是作为输出变量，不是函数内变量，所以不用new初始化，直接写入数据即可，而记录方向的clockcode等变量是为了后续的比较长度等等
	// 对应CPU版本中的vector模型，使用thrust库来对应，希望接口不会出问题,因为对应每一个blob，直接移到执行函数内部,因为thrust和fprintf一样是host函数，所以此处用不了

	//因为CPU中用到了vector向量，所以不急着实施，先看看输出图像效果怎样（链表分析函数是否顺利运行）
	int rx, ry, lx, ly, tx, ty, bx, by;  //分别储存四个坐标（八个数值）
	int startx, starty, endx, endy;
	rx = lx = tx = bx = blobmask[0];
	ry = ly = ty = by = blobmask[1];
	for (int i = 0; i < w; i++)
	{
		if ((blobmask[2 * i] > 0) && (blobmask[2 * i + 1] > 0)){
			if (blobmask[2 * i] > rx)
			{
				rx = blobmask[2 * i];
				ry = blobmask[2 * i + 1];
			}
			if (blobmask[2 * i] < lx)
			{
				lx = blobmask[2 * i];
				ly = blobmask[2 * i + 1];
			}
			if (blobmask[2 * i + 1] > ty)
			{
				tx = blobmask[2 * i];
				ty = blobmask[2 * i + 1];
			}
			if (blobmask[2 * i + 1] < by)
			{
				bx = blobmask[2 * i];
				by = blobmask[2 * i + 1];
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
		if (blobmask[2 * m] > 0){
			int mx = blobmask[2 * m];
			int my = blobmask[2 * m + 1];//作为被比较的点也得放进tempmask数组中
			tempmask[2 * c] = mx;
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
		antichainmask[2 * p] = x;
		antichainmask1[2 * p] = x;
		antichainmask[2 * p + 1] = y;
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
			int x = tempmask[2 * i];
			int y = tempmask[2 * i + 1];
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
				curx = startx + clock[k][1];
				cury = starty + clock[k][0];
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
							goto verifyh;           //break只能跳出最内层的循环，而break总是与if成对出现，来跳出循环
						}
					}

				}
			}
		verifyh:
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
			int x = antichainmask[2 * i];
			int y = antichainmask[2 * i + 1];
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
				curx = startx + anticlock[k][1];
				cury = starty + anticlock[k][0];
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
							goto antiverifyh;           //break只能跳出最内层的循环，而break总是与if成对出现，来跳出循环
						}
					}

				}
			}
		antiverifyh:
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
			int x = antichainmask1[2 * i];
			int y = antichainmask1[2 * i + 1];
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
				curx = startx + anticlock1[k][1];
				cury = starty + anticlock1[k][0];
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
							goto antverifyh;           //break只能跳出最内层的循环，而break总是与if成对出现，来跳出循环
						}
					}

				}
			}
		antverifyh:
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
	chainposition[0] = startx;
	chainposition[1] = starty;
	//不用将起点的坐标写入chainposition吗？
	if (count >= anticount)
	{
		if (count >= antcount)
		{
			retflag = 1;
			for (int k = 0; k < count; k++)
			{
				int jj = clockcode[k];  //at()有边界检查，而operator没有，有边界溢出可能
				chaincode[k] = jj;
				startx = startx + clock[jj][1];
				starty = starty + clock[jj][0];
				chainposition[2 * (k + 1) + 0] = startx;
				chainposition[2 * (k + 1) + 1] = starty;
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
				chaincode[k] = jj;
				startx = startx + anticlock1[jj][1];
				starty = starty + anticlock1[jj][0];
				//chainposition.push_back(start);
				maskimage[(starty)*WIDTH + (startx)] = 2;
				chainposition[2 * (k + 1) + 0] = startx;
				chainposition[2 * (k + 1) + 1] = starty;
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
				chaincode[k] = jj;
				startx = startx + anticlock[jj][1];
				starty = starty + anticlock[jj][0];
				//chainposition.push_back(start);
				maskimage[(starty)*WIDTH + (startx)] = 3;
				chainposition[2 * (k + 1) + 0] = startx;
				chainposition[2 * (k + 1) + 1] = starty;
			}
		}
		else
		{
			retflag = 2;
			for (int k = 0; k < antcount; k++)
			{
				int jj = anticlockcode1[k];
				chaincode[k] = jj;
				startx = startx + anticlock1[jj][1];
				starty = starty + anticlock1[jj][0];
				//chainposition.push_back(start);
				maskimage[(starty)*WIDTH + (startx)] = 2;
				chainposition[2 * (k + 1) + 0] = startx;
				chainposition[2 * (k + 1) + 1] = starty;
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
	delete chaincode;        //如果要观察chaincode的话，应当copy保存至一个数组中输出，以便后续传到host端
	return retflag;
}

__global__ void
d2_chaincode_global(int *id, unsigned char *maskimage, int *od, int w, int h){
	unsigned int y = blockIdx.x*blockDim.x + threadIdx.x;
	if (y<h){
		chaincodeh(&id[2 * y*w], maskimage, &od[2 * y*w], w, h);             //因为每一行实际长度为2*maxlinewidth，所以每一行的行首索引应该是2*y*w
	}
}

double cuda_converter::chaincodehkernel(int width, int height)
{
	// var for kernel computation timing
	double dKernelTime;

	//make sure input d_rgb was OK

	//maybe donnot need cudaThreadSynchronize() 

	d2_chaincode_global <<<  (height + BLOCKSIZE2 - 1) / BLOCKSIZE2, BLOCKSIZE2, 0 >>>(d2_ptr, d_maskimg, d2_outptr, width, height);

	return dKernelTime;

}

////////////////////////////////////////////////////////////////////////
void cuda_converter::MallocMemA(unsigned int len)  //len=width*height,ip:h_yuv  
{
	m_ret_len = len;


	cudaMalloc((void**)&d_blackver, sizeof(unsigned char)* m_ret_len); //output 黑色垂直线标记图
	cudaMalloc((void**)&d_blackhor, sizeof(unsigned char)* m_ret_len);
	cudaMalloc((void**)&d_whitever, sizeof(unsigned char)* m_ret_len); //output 黑色垂直线标记图
	cudaMalloc((void**)&d_whitehor, sizeof(unsigned char)* m_ret_len);

}

int cuda_converter::ReleaseMem()
{

	cudaFree(d_yuv);
	cudaFree(d_blackver);
	cudaFree(d_blackhor);
	cudaFree(d_whitever);
	cudaFree(d_whitehor);
	return 1;
}


__device__ void
d_markblack_ver(unsigned char *id, unsigned char *od, int w, int h, int msize)
{
	int Y0, s1, s2, sn, snedge, sum1, sum2, sntemp1, sntemp2;
	int T = 4;
	int masksize = msize;
	for (int j = 0; j<masksize / 2; j++)
	{
		od[j] = 0;
	}
	for (int j2 = w - masksize / 2; j2<w; j2++)
	{
		od[j2] = 0;
	}
	for (int i = masksize / 2; i<w - masksize / 2; i++)
	{
		Y0 = id[i];
		s1 = 0;
		s2 = 0;
		sum1 = 0;
		sum2 = 0;
		sntemp1 = 0;
		sntemp2 = 0;

		for (int k = -masksize / 2; k<0; k++)
		{
			s1 = s1 + id[i + masksize / 2] - Y0;
			s2 = s2 + id[i - masksize / 2] - Y0;
			sum1 = sum1 + id[i + masksize / 2];
			sum2 = sum2 + id[i - masksize / 2];

			if (abs(id[i + masksize / 2] - Y0) <= T)
			{
				sntemp1 = sntemp1 + 1;
			}
			if (abs(id[i - masksize / 2] - Y0) <= T)
			{
				sntemp1 = sntemp1 + 1;
			}
			if (abs(id[i + masksize / 2] - Y0) <= T)
			{
				sntemp2 = sntemp2 + 1;
			}
			if (abs(id[i - masksize / 2] - Y0) <= T)
			{
				sntemp2 = sntemp2 + 1;
			}
		}
		sn = sntemp1;
		snedge = sntemp2;

		if (s1>0 && s2>0 && sn<masksize / 2)         //线对象
		{
			od[i] = 1;
		}

		else
		{
			od[i] = 0;
		}
	}





}



__device__ void
d_markblack_hor(unsigned char *id, unsigned char *od, int w, int h, int msize)
{
	int Y0, s1, s2, sn, snedge, sum1, sum2, sntemp1, sntemp2;
	int T = 4;
	int masksize = msize;
	for (int j = 0; j<masksize / 2; j++)
	{
		od[j*w] = 0;
	}
	for (int s = h - masksize / 2; s<h; s++)
	{
		od[s*w] = 0;
	}
	for (int i = masksize / 2; i<h - masksize / 2; i++)
	{
		Y0 = id[i*w];
		s1 = 0;
		s2 = 0;
		sum1 = 0;
		sum2 = 0;
		sntemp1 = 0;
		sntemp2 = 0;

		for (int k = -masksize / 2; k<0; k++)
		{
			s1 = s1 + id[(i + masksize / 2)*w] - Y0;
			s2 = s2 + id[(i - masksize / 2)*w] - Y0;
			sum1 = sum1 + id[(i + masksize / 2)*w];
			sum2 = sum2 + id[(i - masksize / 2)*w];

			if (abs(id[(i + masksize / 2)*w] - Y0) <= T)
			{
				sntemp1 = sntemp1 + 1;
			}
			if (abs(id[(i - masksize / 2)*w] - Y0) <= T)
			{
				sntemp1 = sntemp1 + 1;
			}
			if (abs(id[(i + masksize / 2)*w] - Y0) <= T)
			{
				sntemp2 = sntemp2 + 1;
			}
			if (abs(id[(i - masksize / 2)*w] - Y0) <= T)
			{
				sntemp2 = sntemp2 + 1;
			}
		}
		sn = sntemp1;
		snedge = sntemp2;

		if (s1>0 && s2>0 && sn<masksize / 2)         //线对象
		{
			od[i*w] = 1;
		}

		else
		{
			od[i*w] = 0;
		}
	}





}

__device__ void
d_markwhite_ver(unsigned char *id, unsigned char *od, int w, int h, int msize)
{
	int Y0, s1, s2, sn, snedge, sum1, sum2, sntemp1, sntemp2;
	int T = 4;
	int masksize = msize;
	for (int j = 0; j<masksize / 2; j++)
	{
		od[j] = 0;
	}
	for (int j2 = w - masksize / 2; j2<w; j2++)
	{
		od[j2] = 0;
	}
	for (int i = masksize / 2; i<w - masksize / 2; i++)
	{
		Y0 = id[i];
		s1 = 0;
		s2 = 0;
		sum1 = 0;
		sum2 = 0;
		sntemp1 = 0;
		sntemp2 = 0;

		for (int k = -masksize / 2; k<0; k++)
		{
			s1 = s1 - id[i + masksize / 2] + Y0;
			s2 = s2 - id[i - masksize / 2] + Y0;
			sum1 = sum1 + id[i + masksize / 2];
			sum2 = sum2 + id[i - masksize / 2];

			if (abs(id[i + masksize / 2] - Y0) <= T)
			{
				sntemp1 = sntemp1 + 1;
			}
			if (abs(id[i - masksize / 2] - Y0) <= T)
			{
				sntemp1 = sntemp1 + 1;
			}
			if (abs(id[i + masksize / 2] - Y0) <= T)
			{
				sntemp2 = sntemp2 + 1;
			}
			if (abs(id[i - masksize / 2] - Y0) <= T)
			{
				sntemp2 = sntemp2 + 1;
			}
		}
		sn = sntemp1;
		snedge = sntemp2;

		if (s1>0 && s2>0 && sn<masksize / 2)         //线对象
		{
			od[i] = 1;
		}

		else
		{
			od[i] = 0;
		}
	}





}
__device__ void
d_markwhite_hor(unsigned char *id, unsigned char *od, int w, int h, int msize)
{
	int Y0, s1, s2, sn, snedge, sum1, sum2, sntemp1, sntemp2;
	int T = 4;
	int masksize = msize;
	for (int j = 0; j<masksize / 2; j++)
	{
		od[j*w] = 0;
	}
	for (int s = h - masksize / 2; s<h; s++)
	{
		od[s*w] = 0;
	}
	for (int i = masksize / 2; i<h - masksize / 2; i++)
	{
		Y0 = id[i*w];
		s1 = 0;
		s2 = 0;
		sum1 = 0;
		sum2 = 0;
		sntemp1 = 0;
		sntemp2 = 0;

		for (int k = -masksize / 2; k<0; k++)
		{
			s1 = s1 - id[(i + masksize / 2)*w] + Y0;
			s2 = s2 - id[(i - masksize / 2)*w] + Y0;
			sum1 = sum1 + id[(i + masksize / 2)*w];
			sum2 = sum2 + id[(i - masksize / 2)*w];

			if (abs(id[(i + masksize / 2)*w] - Y0) <= T)
			{
				sntemp1 = sntemp1 + 1;
			}
			if (abs(id[(i - masksize / 2)*w] - Y0) <= T)
			{
				sntemp1 = sntemp1 + 1;
			}
			if (abs(id[(i + masksize / 2)*w] - Y0) <= T)
			{
				sntemp2 = sntemp2 + 1;
			}
			if (abs(id[(i - masksize / 2)*w] - Y0) <= T)
			{
				sntemp2 = sntemp2 + 1;
			}
		}
		sn = sntemp1;
		snedge = sntemp2;

		if (s1>0 && s2>0 && sn<masksize / 2)         //线对象
		{
			od[i*w] = 1;
		}

		else
		{
			od[i*w] = 0;
		}
	}





}

__global__ void
d_markblack_ver_global(unsigned char *id, unsigned char *od, int w, int h, int msize){
	unsigned int y = blockIdx.x*blockDim.x + threadIdx.x;
	d_markblack_ver(&id[y * w], &od[y * w], w, h, msize);

}

__global__ void
d_markblack_hor_global(unsigned char *id, unsigned char *od, int w, int h, int msize){
	unsigned int x = blockIdx.x*blockDim.x + threadIdx.x;
	d_markblack_hor(&id[x], &od[x], w, h, msize);

}

__global__ void
d_markwhite_ver_global(unsigned char *id, unsigned char *od, int w, int h, int msize){
	unsigned int y = blockIdx.x*blockDim.x + threadIdx.x;
	d_markwhite_ver(&id[y * w], &od[y * w], w, h, msize);

}

__global__ void
d_markwhite_hor_global(unsigned char *id, unsigned char *od, int w, int h, int msize){
	unsigned int x = blockIdx.x*blockDim.x + threadIdx.x;
	d_markwhite_hor(&id[x], &od[x], w, h, msize);

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//__global__ void mark_blackver(unsigned char *ip, unsigned char *rd, int w, int h, int msize) //img:d_yuv(从host传输过来的Y) rd:(BlackHorEdge) 直接作为输出 
//
//{
//	int i, j, Y0, s1, s2, sn, snedge, sum1, sum2, sntemp1, sntemp2;
//	int T = 4;
//	int masksize = msize;
//
//	//	int masksize=13;                
//	//	int masksize2=9;
//
//	unsigned char *R = NULL;        //用于标记的像素点矩阵
//	unsigned char *Y = NULL;        //亮度矩阵
//
//
//	R = rd;
//	Y = ip;
//
//
//
//
//	uint col = blockIdx.x*blockDim.x + threadIdx.x;          //x
//	uint row = blockIdx.y*blockDim.y + threadIdx.y;          //y
//	uint tim = row*w + col;
//	unsigned char *buff;
//	buff = new unsigned char[w*1];
//	//输入
//	memcpy(buff, Y[tim], w);
//
//	for (int j = 0; j < w; j++)
//	{
//		if (buff[j] > 127)
//			buff[i] = 0;
//		else
//			buff[i] = 1;
//	}
//	memcpy(R[tim], buff, w);
//
//
//	//if ((col < w ) && (row < h ))
//	//{
//	//	if (Y[tim]>127)
//	//	{
//	//		R[tim] = 0;
//	//	}
//	//	else
//	//	{
//	//		R[tim] = 1;
//	//	}
//	//	s1 = 0;
//	//	s2 = 0;
//	//	sum1 = 0;
//	//	sum2 = 0;
//	//	sntemp1 = 0;
//	//	sntemp2 = 0;
//
//	//	for (int k = -masksize / 2; k < 0; k++)
//	//	{
//	//		s1 = s1 + Y[(row)*w + col + masksize / 2] - Y0;           //采用水平模板，同一行的不同列
//	//		s2 = s2 + Y[(row)*w + col - masksize / 2] - Y0;
//	//		sum1 = sum1 + Y[(row)*w + col + masksize / 2];
//	//		sum2 = sum2 + Y[(row)*w + col - masksize / 2];
//
//	//		if (abs(Y[(row)*w + col + masksize / 2] - Y0) <= T)
//	//		{
//	//			sntemp1 = sntemp1 + 1;
//	//		}
//	//		if (abs(Y[(row)*w + col - masksize / 2] - Y0) <= T)
//	//		{
//	//			sntemp1 = sntemp1 + 1;
//	//		}
//	//		if (abs(Y[(row)*w + col + masksize / 2] - Y0) <= T)
//	//		{
//	//			sntemp2 = sntemp2 + 1;
//	//		}
//	//		if (abs(Y[(row)*w + col - masksize / 2] - Y0) <= T)
//	//		{
//	//			sntemp2 = sntemp2 + 1;
//	//		}
//	//	}
//	//	sn = sntemp1;
//	//	snedge = sntemp2;
//
//	//	if (s1 > 0 && s2 > 0 && sn<masksize / 2)         //条带对象
//	//	{
//	//		R[row*w + col] = 1;
//	//	}
//	//	
//	//	else
//	//	{
//	//		R[row*w + col] = 0;
//	//	}
//
//
//
//	}
//
//}







/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double cuda_converter::blackverkernel(int width, int height)
{
	// var for kernel computation timing
	double dKernelTime;

	//make sure input d_rgb was OK

	//maybe donnot need cudaThreadSynchronize() 

	d_markblack_ver_global <<<  height / BLOCKSIZE1, BLOCKSIZE1, 0 >>>(d_yuv, d_blackver, width, height, 9);
	d_markblack_hor_global <<<  width / BLOCKSIZE2, BLOCKSIZE2, 0 >>>(d_yuv, d_blackhor, width, height, 9);

	return dKernelTime;

}

double cuda_converter::whiteverkernel(int width, int height)
{
	// var for kernel computation timing
	double dKernelTime;

	//make sure input d_rgb was OK

	//maybe donnot need cudaThreadSynchronize() 

	d_markwhite_ver_global <<<  height / BLOCKSIZE1, BLOCKSIZE1, 0 >>>(d_yuv, d_whitever, width, height, 9);
	d_markwhite_hor_global <<<  width / BLOCKSIZE2, BLOCKSIZE2, 0 >>>(d_yuv, d_whitehor, width, height, 9);

	return dKernelTime;

}































