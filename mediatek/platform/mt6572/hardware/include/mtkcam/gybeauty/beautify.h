#ifndef _GY_BEAUTIFY_H_
#define _GY_BEAUTIFY_H_


#define YUV_422 0
#define YUV_420 1

#ifdef __cplusplus
extern "C"
{
#endif
/*
*/
int GYBeautyInit(int mode, int width, int height, int flag);

int GYBeautyUnInit();

int GYBeautySetSmooth(int ssRate);

int GYBeautySetWhitening(int whRate);

int GYBeautySkinFace(unsigned char *y);

#ifdef __cplusplus
}
#endif
#endif
