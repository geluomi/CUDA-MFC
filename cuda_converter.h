
#ifndef __CUDA_CONVERTER_H__
#define __CUDA_CONVERTER_H__



#include <cuda_runtime.h>


class cuda_converter
{
public:
	cuda_converter(void);
	~cuda_converter(void);

	int InitCUDA();
	void MallocMemA(unsigned int len);
	void MallocMemP(unsigned int len);
	void MallocMemM(unsigned int len);
	void MallocMemH(unsigned int len);
	
	int TranslateOutput(int width, int height, unsigned char *out_data, unsigned char *ou_data);
	int TranslateOutputB(int width, int height, unsigned char *out_data, unsigned char *ou_data);
	int TranslateOutputP(int len, int *out_data);
	int TranslateOutputH(int len, int *out_data);
	int TranslateOutputM(int len, unsigned char *out_data);
	int TranslateInput(int width, int height, unsigned char *pImage);
	int TranslateInputB(int width, int height, int *pImage);
	int TranslateInputH(int width, int height, int *pImage);
	int ReleaseMem();
	int ReleaseMemP();
	int ReleaseMemM();
	int ReleaseMemH();

	double blackverkernel(int width, int height);
	double whiteverkernel(int width, int height);
	double chaincodekernel(int width, int height);
	double chaincodehkernel(int width, int height);
private:
	unsigned char	*d_blackver;
	unsigned char	*d_blackhor;
	unsigned char	*d_whitever;
	unsigned char	*d_whitehor;
	unsigned char   *d_yuv;
	int              m_ret_len;
	int              n_ret_len;
	unsigned char   *d_maskimg;
	int   *d_ptr;
	int   *d2_ptr;
	int   *d_outptr;                     
	int   *d2_outptr;
};


#endif // __CUDA_CLASS_H__
