#include "yuv.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>


//#include "upsample.h"

#define CLIP3(x, min, max) ((x)<(min)?(min):((x)>(max)?(max):(x)))

CIYuv::CIYuv()
{
  Y = U = V = NULL;
  comp[0] = comp[1] = comp[2] = NULL;
  pBuffer = NULL;
  FuncFilter = NULL;
}


CIYuv::CIYuv(int h, int w, int chroma_format)
{
  height = h; 
  width = w;
  sampling = chroma_format;

  Y = U = V = NULL;
  comp[0] = comp[1] = comp[2] = NULL;
  pBuffer = NULL;
  FuncFilter = NULL;

  switch(sampling)
  {
  case 400: // padding chroma components
    heightUV = height/2;
    widthUV = width/2;
    break;
  case 420:
    heightUV = height/2;
    widthUV = width/2;
    break;
  case 422:
    heightUV = height;
    widthUV = width/2;
    break;
  case 444:
    heightUV = height;
    widthUV = width;
    break;
  default:
    fprintf(stderr, "Unknown chroma sampling\n");
    height = width = sampling = 0;
    Y = U = V = NULL;
    comp[0] = comp[1] = comp[2] = NULL;
    return;
  }

  picsizeY = height*width;
  picsizeUV = heightUV*widthUV;
  size_in_byte = picsizeY + picsizeUV*2;

  if(!allocate_mem())
  {
    height = width = sampling = 0;
    Y = U = V = NULL;
    comp[0] = comp[1] = comp[2] = NULL;
    pBuffer = NULL;
  }
}

CIYuv::~CIYuv()
{
  release_mem();
}

bool CIYuv::Resize(int h, int w, int chroma_format)
{
  height = h; 
  width = w;
  sampling = chroma_format;

  switch(sampling)
  {
  case 400: // padding chroma components
    heightUV = height/2;
    widthUV  = width/2;
    break;
  case 420:
    heightUV = height/2;
    widthUV  = width/2;
    break;
  case 422:
    heightUV = height;
    widthUV  = width/2;
    break;
  case 444:
    heightUV = height;
    widthUV  = width;
    break;
  default:
    fprintf(stderr, "Unknown chroma sampling\n");
    Y = U = V = NULL;
    return false;
  }

  picsizeY = height*width;
  picsizeUV = heightUV*widthUV;
  size_in_byte = picsizeY + picsizeUV*2;

  return allocate_mem();  //<!转化为用二维数组形式进行访问。
}

/*
 * \brief
 *    Allocate mem for the YUV image.
 *    Physical memory is allocated as a 1D array, pointed by pBuffer
 *    However, in order to facilitate to access Y, U and V components in a 2D way,
 *    the following pointers can be refered:
 *    comp[0] (redundant of Y?)
 *    comp[1] (redundant of U?)
 *    comp[2] (redundant of V?)
 * \return
 *    true: succeed
 *    false: fail
 */

bool CIYuv::allocate_mem()
{
  int h, pos;
  BYTE *buf1D;
  BYTE **buf2D;

  release_mem();

  if( (buf1D=(unsigned char *)malloc(size_in_byte*sizeof(BYTE))) == NULL ) return false;

  if( (buf2D=(BYTE **)malloc((height+heightUV*2)*sizeof(BYTE *))) == NULL )
  {
    free(buf1D);
    return false;
  }
  memset(&buf1D[0], 0, picsizeY);
  memset(&buf1D[picsizeY], 128, picsizeUV*2);

  Y = buf2D;
  U = &(buf2D[height]);
  V = &(buf2D[height+heightUV]);

  for(h=pos=0; h<height; h++, pos+=width)//<!对YUV重新赋值，转化为用二维数组进行访问。
    Y[h] = &(buf1D[pos]);
  for(h=0; h<heightUV; h++, pos+=widthUV)
    U[h] = &(buf1D[pos]);
  for(h=0; h<heightUV; h++, pos+=widthUV)
    V[h] = &(buf1D[pos]);

  comp[0] = Y;
  comp[1] = U;
  comp[2] = V;
  
  pBuffer = Y[0]; // buf1D;

  return true;
}

void CIYuv::release_mem()
{
  if(Y!=NULL)
  {
    if(Y[0]!=NULL) free(Y[0]);
    free(Y);
    Y = U = V = NULL;
    comp[0] = comp[1] = comp[2] = NULL;
    pBuffer = NULL;
  }
}

/*
 * \brief
 *    Read one frame from file to buffer
 * \return
 *    true: succeed
 *    false: fail
 *
 */
bool CIYuv::ReadOneFrame(FILE *fp, int frameno)
{
  if(Y==NULL || fp==NULL || pBuffer==NULL) return false;

  if (frameno != -1)
  {
    long offs = long(size_in_byte)*long(frameno);
    fseek(fp, offs, SEEK_SET);
  }

  if(fread(Y[0], size_in_byte, 1, fp)!=1)
  {
    fprintf(stderr, "EOF\n");
    return false;
  }

  return true;
}

bool CIYuv::WriteOneFrame(FILE *fp)
{
  if(Y==NULL || fp==NULL || pBuffer==NULL) return false;

  if(fwrite(Y[0], size_in_byte, 1, fp)!=1)
  {
    fprintf(stderr, "EOF\n");
    return false;
  }

  return true;
}

bool CIYuv::WriteOneFrame_inYUV(FILE *fp)
{
  if(Y==NULL || fp==NULL || pBuffer==NULL) return false;

  int x, y;
  unsigned char *src;
  src = pBuffer;

  switch(sampling)
  {
  case 444:
    for (y=0;y<height;y++) {
      for (x=0;x<width;x++) {
        fputc(src[y*width*3 + x*3 + 0], fp);
      }
    }
    for (y=0;y<height;y++) {
      for (x=0;x<width;x++) {
        fputc(src[y*width*3 + x*3 + 1], fp);
      }
    }
    for (y=0;y<height;y++) {
      for (x=0;x<width;x++) {
        fputc(src[y*width*3 + x*3 + 2], fp);
      }
    }
    break;
  case 420:
	  for (y = 0; y < height; y++) {
		  for (x = 0; x < width; x++) {
			  fputc(src[y*width * 3 + x * 3 + 0], fp);
		  }
	  }
	  for (y = 0; y < height / 2; y++) {
		  for (x = 0; x < width / 2; x++) {
			  fputc(src[(y * 2)*(width * 3) + (x * 2) * 3 + 1], fp);
		  }
	  }
	  for (y = 0; y < height / 2; y++) {
		  for (x = 0; x < width / 2; x++) {
			  fputc(src[(y * 2)*(width * 3) + (x * 2) * 3 + 2], fp);
		  }
	  }
	  break;
  case 400:
    for (y=0;y<height;y++) {
      for (x=0;x<width;x++) {
        fputc(src[y*width + x + 0], fp);
      }
    }
    break;
  }


  return true;
}


void CIYuv::getData_inBGR(cv::Mat *imgBGR)
{
	int h, w, cH, cW;
	int ir, ig, ib;
	unsigned char *bufBGR;

	switch (sampling)
	{
	case 400:
		/*for(h=0; h<height; h++)
		{
		bufBGR = (unsigned char *)&(imgBGR->imageData[h*imgBGR->widthStep]);
		for(w=0; w<width; w++)
		{
		*bufBGR++ = Y[h][w];
		*bufBGR++ = Y[h][w];
		*bufBGR++ = Y[h][w];
		}
		}*/
		break;
	case 420:
		for (h = cH = 0; h<height; h++)
		{
			cH = h >> 1;
			bufBGR = (unsigned char *)&(imgBGR->data[3 * h*imgBGR->cols]);
			for (w = cW = 0; w<width; w++, cW++)
			{
				ib = (int)(Y[h][w] + 1.772*(U[cH][cW] - 127) + 0.5);
				*bufBGR++ = CLIP3(ib, 0, 255);
				ig = (int)(Y[h][w] - 0.344*(U[cH][cW] - 127) - 0.714*(V[cH][cW] - 127) + 0.5);
				*bufBGR++ = CLIP3(ig, 0, 255);
				ir = (int)(Y[h][w] + 1.402*(V[cH][cW] - 127) + 0.5);
				*bufBGR++ = CLIP3(ir, 0, 255);
				w++;
				ib = (int)(Y[h][w] + 1.772*(U[cH][cW] - 127) + 0.5);
				*bufBGR++ = CLIP3(ib, 0, 255);
				ig = (int)(Y[h][w] - 0.344*(U[cH][cW] - 127) - 0.714*(V[cH][cW] - 127) + 0.5);
				*bufBGR++ = CLIP3(ig, 0, 255);
				ir = (int)(Y[h][w] + 1.402*(V[cH][cW] - 127) + 0.5);
				*bufBGR++ = CLIP3(ir, 0, 255);
			}
		}
		break;
	case 422:
		/* for(h=0; h<height; h++)
		{
		bufBGR = (unsigned char *)&(imgBGR->imageData[h*imgBGR->widthStep]);
		for(w=cW=0; w<width; w++, cW++)
		{
		ib = (int) (Y[h][w] + 1.772*(U[h][cW]-127)                        + 0.5);
		*bufBGR++ = CLIP3(ib, 0, 255);
		ig = (int) (Y[h][w] - 0.344*(U[h][cW]-127) - 0.714*(V[h][cW]-127) + 0.5);
		*bufBGR++ = CLIP3(ig, 0, 255);
		ir = (int) (Y[h][w]                        + 1.402*(V[h][cW]-127) + 0.5);
		*bufBGR++ = CLIP3(ir, 0, 255);
		w++;
		ib = (int) (Y[h][w] + 1.772*(U[h][cW]-127)                        + 0.5);
		*bufBGR++ = CLIP3(ib, 0, 255);
		ig = (int) (Y[h][w] - 0.344*(U[h][cW]-127) - 0.714*(V[h][cW]-127) + 0.5);
		*bufBGR++ = CLIP3(ig, 0, 255);
		ir = (int) (Y[h][w]                        + 1.402*(V[h][cW]-127) + 0.5);
		*bufBGR++ = CLIP3(ir, 0, 255);
		}
		}*/
		break;
	case 444:
		/* for(h=0; h<height; h++)
		{
		bufBGR = (unsigned char *)&(imgBGR->imageData[h*imgBGR->widthStep]);
		for(w=0; w<width; w++)
		{
		ib = (int) (Y[h][w] + 1.772*(U[h][w]-127)                       + 0.5);
		*bufBGR++ = CLIP3(ib, 0, 255);
		ig = (int) (Y[h][w] - 0.344*(U[h][w]-127) - 0.714*(V[h][w]-127) + 0.5);
		*bufBGR++ = CLIP3(ig, 0, 255);
		ir = (int) (Y[h][w]                       + 1.402*(V[h][w]-127) + 0.5);
		*bufBGR++ = CLIP3(ir, 0, 255);
		}
		}*/
		break;
	default:
		/*cvZero(imgBGR);*/
		break;
	}
}

void CIYuv::getData_inRGB(cv::Mat *imgBGR)
{
	int h, w, cH, cW;
	int ir, ig, ib;
	unsigned char *bufBGR;

	switch (sampling)
	{
	case 400:
		/*for (h = 0; h < height; h++)
		{
			bufBGR = (unsigned char *)&(imgBGR->imageData[h*imgBGR->widthStep]);
			for (w = 0; w < width; w++)
			{
				*bufBGR++ = Y[h][w];
				*bufBGR++ = Y[h][w];
				*bufBGR++ = Y[h][w];
			}
		}*/
		break;
	case 420:
		for (h = cH = 0; h < height; h++)
		{
			cH = h >> 1;
			bufBGR = (unsigned char *)&(imgBGR->data[3 * h*imgBGR->cols]);
			for (w = cW = 0; w < width; w++, cW++)
			{
				ir = (int)(Y[h][w] + 1.402*(V[cH][cW] - 127) + 0.5);
				*bufBGR++ = CLIP3(ir, 0, 255);
				ig = (int)(Y[h][w] - 0.344*(U[cH][cW] - 127) - 0.714*(V[cH][cW] - 127) + 0.5);
				*bufBGR++ = CLIP3(ig, 0, 255);
				ib = (int)(Y[h][w] + 1.772*(U[cH][cW] - 127) + 0.5);
				*bufBGR++ = CLIP3(ib, 0, 255);
				w++;
				ir = (int)(Y[h][w] + 1.402*(V[cH][cW] - 127) + 0.5);
				*bufBGR++ = CLIP3(ir, 0, 255);
				ig = (int)(Y[h][w] - 0.344*(U[cH][cW] - 127) - 0.714*(V[cH][cW] - 127) + 0.5);
				*bufBGR++ = CLIP3(ig, 0, 255);
				ib = (int)(Y[h][w] + 1.772*(U[cH][cW] - 127) + 0.5);
				*bufBGR++ = CLIP3(ib, 0, 255);
			}
		}
		break;
	//case 422:
	//	for (h = 0; h < height; h++)
	//	{
	//		bufBGR = (unsigned char *)&(imgBGR->imageData[h*imgBGR->widthStep]);
	//		for (w = cW = 0; w < width; w++, cW++)
	//		{
	//			ib = (int)(Y[h][w] + 1.772*(U[h][cW] - 127) + 0.5);
	//			*bufBGR++ = CLIP3(ib, 0, 255);
	//			ig = (int)(Y[h][w] - 0.344*(U[h][cW] - 127) - 0.714*(V[h][cW] - 127) + 0.5);
	//			*bufBGR++ = CLIP3(ig, 0, 255);
	//			ir = (int)(Y[h][w] + 1.402*(V[h][cW] - 127) + 0.5);
	//			*bufBGR++ = CLIP3(ir, 0, 255);
	//			w++;
	//			ib = (int)(Y[h][w] + 1.772*(U[h][cW] - 127) + 0.5);
	//			*bufBGR++ = CLIP3(ib, 0, 255);
	//			ig = (int)(Y[h][w] - 0.344*(U[h][cW] - 127) - 0.714*(V[h][cW] - 127) + 0.5);
	//			*bufBGR++ = CLIP3(ig, 0, 255);
	//			ir = (int)(Y[h][w] + 1.402*(V[h][cW] - 127) + 0.5);
	//			*bufBGR++ = CLIP3(ir, 0, 255);
	//		}
	//	}
	//	break;
	//case 444:
	//	for (h = 0; h < height; h++)
	//	{
	//		bufBGR = (unsigned char *)&(imgBGR->imageData[h*imgBGR->widthStep]);
	//		for (w = 0; w < width; w++)
	//		{
	//			ib = (int)(Y[h][w] + 1.772*(U[h][w] - 127) + 0.5);
	//			*bufBGR++ = CLIP3(ib, 0, 255);
	//			ig = (int)(Y[h][w] - 0.344*(U[h][w] - 127) - 0.714*(V[h][w] - 127) + 0.5);
	//			*bufBGR++ = CLIP3(ig, 0, 255);
	//			ir = (int)(Y[h][w] + 1.402*(V[h][w] - 127) + 0.5);
	//			*bufBGR++ = CLIP3(ir, 0, 255);
	//		}
	//	}
	//	break;
	default:
		cvZero(imgBGR);
		break;
	}
}

void CIYuv::setDataFromImgBGR(cv::Mat *imgBGR)
{
	int h, w;
	int fr, fg, fb;
	int iy, iu, iv;
	unsigned char *bufBGR;

	switch (sampling)
	{
	case 400:
		/* for(h=0; h<height; h++)
		{
		bufBGR = (unsigned char *)&(imgBGR->imageData[h*imgBGR->widthStep]);
		for(w=0; w<width; w++)
		{
		fb = *bufBGR++;  //B
		fg = *bufBGR++;  //G
		fr = *bufBGR++;  //R
		iy = (int)(0.299 * fr + 0.587 * fg + 0.114 * fb + 0.5);
		Y[h][w] = CLIP3(iy, 0, 255);
		}
		}*/
		break;
	case 420:
		for (h = 0; h<height; h++)
		{
			bufBGR = (unsigned char *)&(imgBGR->data[3 * h*imgBGR->cols]);
			for (w = 0; w<width; w++)
			{
				fb = *bufBGR++;  //B
				fg = *bufBGR++;  //G
				fr = *bufBGR++;  //R
				iy = (int)(0.299 * fr + 0.587 * fg + 0.114 * fb + 0.5);
				Y[h][w] = CLIP3(iy, 0, 255);
				if (h % 2 == 0 && w % 2 == 0)
				{
					iu = (int)(-0.169 * fr - 0.331 * fg + 0.500 * fb + 127.5);
					iv = (int)(0.500 * fr - 0.419 * fg - 0.081 * fb + 127.5);
					U[h / 2][w / 2] = CLIP3(iu, 0, 255);
					V[h / 2][w / 2] = CLIP3(iv, 0, 255);
				}
			}
		}
		break;
	case 422:
		/*for(h=0; h<height; h++)
		{
		bufBGR = (unsigned char *)&(imgBGR->imageData[h*imgBGR->widthStep]);
		for(w=0; w<width; w++)
		{
		fb = *bufBGR++;  //B
		fg = *bufBGR++;  //G
		fr = *bufBGR++;  //R
		iy = (int)(0.299 * fr + 0.587 * fg + 0.114 * fb + 0.5);
		Y[h][w] = CLIP3(iy, 0, 255);
		if(w%2==0)
		{
		iu = (int)(-0.169 * fr - 0.331 * fg + 0.500 * fb + 127.5);
		iv = (int)( 0.500 * fr - 0.419 * fg - 0.081 * fb + 127.5);
		U[h][w/2] = CLIP3(iu, 0, 255);
		V[h][w/2] = CLIP3(iv, 0, 255);
		}
		}
		}*/
		break;
	case 444:
		/* for(h=0; h<height; h++)
		{
		bufBGR = (unsigned char *)&(imgBGR->imageData[h*imgBGR->widthStep]);
		for(w=0; w<width; w++)
		{
		fb = *bufBGR++;  //B
		fg = *bufBGR++;  //G
		fr = *bufBGR++;  //R
		iy = (int)(0.299 * fr + 0.587 * fg + 0.114 * fb + 0.5);
		iu = (int)(-0.169 * fr - 0.331 * fg + 0.500 * fb + 127.5);
		iv = (int)( 0.500 * fr - 0.419 * fg - 0.081 * fb + 127.5);
		Y[h][w] = CLIP3(iy, 0, 255);
		U[h][w] = CLIP3(iu, 0, 255);
		V[h][w] = CLIP3(iv, 0, 255);
		}
		}*/
		break;
	}
}

bool CIYuv::setData444_inIBGR(CIYuv *yuvSrc)
{
  int h, w, cH, cW;
  int ir, ig, ib;

  if(sampling!=444 || Y==NULL || height!=yuvSrc->getHeight() || width!=yuvSrc->getWidth()) return false;

  switch(yuvSrc->getSampling())
  {
  case 400:
    memcpy(comp[0][0], yuvSrc->Y[0], picsizeY);
    memcpy(comp[1][0], yuvSrc->Y[0], picsizeY);
    memcpy(comp[2][0], yuvSrc->Y[0], picsizeY);
  case 420:
    for(h=cH=0; h<height; h++)
    {
      cH = h>>1;
      for(w=cW=0; w<width; w++, cW++)
      {
        ib = (int) (yuvSrc->Y[h][w] + 1.772*(yuvSrc->U[cH][cW]-127)                                + 0.5);
        ig = (int) (yuvSrc->Y[h][w] - 0.344*(yuvSrc->U[cH][cW]-127) - 0.714*(yuvSrc->V[cH][cW]-127) + 0.5);
        ir = (int) (yuvSrc->Y[h][w]                                + 1.402*(yuvSrc->V[cH][cW]-127) + 0.5);
        comp[0][h][w] = CLIP3(ib, 0, 255);
        comp[1][h][w] = CLIP3(ig, 0, 255);
        comp[2][h][w] = CLIP3(ir, 0, 255);
        w++;
        ib = (int) (yuvSrc->Y[h][w] + 1.772*(yuvSrc->U[cH][cW]-127)                                + 0.5);
        ig = (int) (yuvSrc->Y[h][w] - 0.344*(yuvSrc->U[cH][cW]-127) - 0.714*(yuvSrc->V[cH][cW]-127) + 0.5);
        ir = (int) (yuvSrc->Y[h][w]                                + 1.402*(yuvSrc->V[cH][cW]-127) + 0.5);
        comp[0][h][w] = CLIP3(ib, 0, 255);
        comp[1][h][w] = CLIP3(ig, 0, 255);
        comp[2][h][w] = CLIP3(ir, 0, 255);
      }
    }
    break;
  case 422:
    for(h=0; h<height; h++)
    {
      for(w=cW=0; w<width; w++, cW++)
      {
        ib = (int) (yuvSrc->Y[h][w] + 1.772*(yuvSrc->U[h][cW]-127)                               + 0.5);
        ig = (int) (yuvSrc->Y[h][w] - 0.344*(yuvSrc->U[h][cW]-127) - 0.714*(yuvSrc->V[h][cW]-127) + 0.5);
        ir = (int) (yuvSrc->Y[h][w]                               + 1.402*(yuvSrc->V[h][cW]-127) + 0.5);
        comp[0][h][w] = CLIP3(ib, 0, 255);
        comp[1][h][w] = CLIP3(ig, 0, 255);
        comp[2][h][w] = CLIP3(ir, 0, 255);
        w++;
        ib = (int) (yuvSrc->Y[h][w] + 1.772*(yuvSrc->U[h][cW]-127)                               + 0.5);
        ig = (int) (yuvSrc->Y[h][w] - 0.344*(yuvSrc->U[h][cW]-127) - 0.714*(yuvSrc->V[h][cW]-127) + 0.5);
        ir = (int) (yuvSrc->Y[h][w]                               + 1.402*(yuvSrc->V[h][cW]-127) + 0.5);
        comp[0][h][w] = CLIP3(ib, 0, 255);
        comp[1][h][w] = CLIP3(ig, 0, 255);
        comp[2][h][w] = CLIP3(ir, 0, 255);
      }
    }
    break;
  case 444:
    for(h=0; h<height; h++)
    {
      for(w=0; w<width; w++)
      {
        ib = (int) (yuvSrc->Y[h][w] + 1.772*(yuvSrc->U[h][w]-127)                              + 0.5);
        ig = (int) (yuvSrc->Y[h][w] - 0.344*(yuvSrc->U[h][w]-127) - 0.714*(yuvSrc->V[h][w]-127) + 0.5);
        ir = (int) (yuvSrc->Y[h][w]                              + 1.402*(yuvSrc->V[h][w]-127) + 0.5);
        comp[0][h][w] = CLIP3(ib, 0, 255);
        comp[1][h][w] = CLIP3(ig, 0, 255);
        comp[2][h][w] = CLIP3(ir, 0, 255);
      }
    }
    break;
  default:
    memset(comp[0][0], 0, size_in_byte);
    break;
  }

  return true;
}

bool CIYuv::setData444_inIYUV(CIYuv *yuvSrc)
{
  int h, dstH, dstW, srcH, srcW;

  if(sampling!=444 || Y==NULL || height!=yuvSrc->getHeight() || width!=yuvSrc->getWidth()) return false;

  memcpy(Y[0], yuvSrc->Y[0], picsizeY);

  switch(yuvSrc->getSampling())
  {
  case 400:
    memset(U[0], 128, picsizeUV*2);
    break;
  case 420:
    for(srcH=dstH=0; dstH<height; dstH+=2, srcH++)
    {
      for(srcW=dstW=0; dstW<width; dstW+=2, srcW++)
      {
        U[dstH][dstW] = U[dstH+1][dstW] = U[dstH][dstW+1] = U[dstH+1][dstW+1] = yuvSrc->U[srcH][srcW];
        V[dstH][dstW] = V[dstH+1][dstW] = V[dstH][dstW+1] = V[dstH+1][dstW+1] = yuvSrc->V[srcH][srcW];
      }
    }
    break;
  case 422:
    for(h=0; h<height; h++)
    {
      for(srcW=dstW=0; dstW<width; dstW+=2, srcW++)
      {
        U[h][dstW] = U[h][dstW+1] = yuvSrc->U[h][srcW];
        V[h][dstW] = V[h][dstW+1] = yuvSrc->V[h][srcW];
      }
    }
    break;
  case 444:
    memcpy(U[0], yuvSrc->U[0], picsizeUV*2);
    break;
  }

  return true;
}

void CIYuv::setDataFromImgYUV(IplImage *imgYUV)
{
  int h, w;
  int iu, iv;
  unsigned char *bufYUV1, *bufYUV2;

  switch(sampling)
  {
  case 400:
    for(h=0; h<height; h++)
    {
      bufYUV1 = (unsigned char *)&(imgYUV->imageData[h*imgYUV->widthStep]);
      for(w=0; w<width; w++)
      {
        Y[h][w] = *bufYUV1;  // Y
        bufYUV1 += 3;
      }
    }
    break;
  case 420:
    for(h=0; h<height; h+=2)
    {
      bufYUV1 = (unsigned char *)&(imgYUV->imageData[ h   *imgYUV->widthStep]);
      bufYUV2 = (unsigned char *)&(imgYUV->imageData[(h+1)*imgYUV->widthStep]);
      for(w=0; w<width; w+=2)
      {
        Y[h  ][w  ] = *bufYUV1++;
        iu = *bufYUV1++;
        iv = *bufYUV1++;

        Y[h+1][w  ] = *bufYUV2++;
        iu += *bufYUV2++;
        iv += *bufYUV2++;

        Y[h  ][w+1] = *bufYUV1++;
        iu += *bufYUV1++;
        iv += *bufYUV1++;

        Y[h+1][w+1] = *bufYUV2++;
        iu += *bufYUV2++;
        iv += *bufYUV2++;

        U[h/2][w/2] = CLIP3((iu + 2)/4, 0, 255);
        V[h/2][w/2] = CLIP3((iv + 2)/4, 0, 255);
      }
    }
    break;
  case 422:
    for(h=0; h<height; h++)
    {
      bufYUV1 = (unsigned char *)&(imgYUV->imageData[h*imgYUV->widthStep]);
      for(w=0; w<width; w+=2)
      {
        Y[h][w  ] = *bufYUV1++;
        iu = *bufYUV1++;
        iv = *bufYUV1++;

        Y[h][w+1] = *bufYUV1++;
        iu += *bufYUV1++;
        iv += *bufYUV1++;

        U[h][w/2] = CLIP3((iu + 1)/2, 0, 255);
        V[h][w/2] = CLIP3((iv + 1)/2, 0, 255);
      }
    }
    break;
  case 444:
    for(h=0; h<height; h++)
    {
      bufYUV1 = (unsigned char *)&(imgYUV->imageData[h*imgYUV->widthStep]);
      for(w=0; w<width; w++)
      {
        Y[h][w] = *bufYUV1++;
        U[h][w] = *bufYUV1++;
        V[h][w] = *bufYUV1++;
      }
    }
    break;
  }
}

bool CIYuv::setUpsampleFilter(unsigned int filter, unsigned int precision)
{/*
  switch(precision)
  {
  case 1: // INTEGER PEL
    FuncFilter = DummyFilter_2D;
    break;
  case 2: // HALF PEL
    switch(filter)
    {
    case 0: // BI-LINEAR
      FuncFilter = HorizontalLinearFilter_2D_half;
      break;
    case 1: // BI-CUBIC
      FuncFilter = HorizontalCubicFilter_2D_half;
      break;
    case 2: // AVC
      FuncFilter = HorizontalAVCFilter_2D_half;
      break;
    default:
      return false;
      break;
    }
    break;
  case 4: // QUARTER PEL
    switch(filter)
    {
    case 0: // BI-LINEAR
      FuncFilter = HorizontalLinearFilter_2D_qpel;
      break;
    case 1: // BI-CUBIC
      FuncFilter = HorizontalCubicFilter_2D_qpel;
      break;
    case 2: // AVC
      FuncFilter = HorizontalAVCFilter_2D_qpel;
      break;
    default:
      return false;
      break;
    }
    break;
  default:
    return false;
    break;
  }
  */
  return true;
}

bool CIYuv::upsampling(CIYuv *src, int padding_size)
{
  // need to check buffer size

  FuncFilter(src->Y, Y, src->getWidth(), src->getHeight(), padding_size);
  FuncFilter(src->U, U, src->getWidthUV(), src->getHeightUV(), padding_size);
  FuncFilter(src->V, V, src->getWidthUV(), src->getHeightUV(), padding_size);

  return true;
}
