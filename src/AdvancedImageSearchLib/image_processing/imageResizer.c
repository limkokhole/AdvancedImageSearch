#include "imageResizer.h"


#include <stdio.h>
#include <stdlib.h>

unsigned char * resizeImageInternal3bytes(unsigned char * rgb, unsigned int originalWidth ,unsigned int originalHeight , unsigned int resizeWidth,unsigned int resizeHeight)
{
  if ( ( resizeHeight == 0 ) || (resizeWidth == 0 ) ) { fprintf(stderr,"Will not resize to 0  \n"); return rgb; }
  unsigned char * output = (unsigned char *) malloc(resizeWidth*resizeHeight*3);
  if ( output != 0 ) { fprintf(stderr,"Could not allocate image for resizing \n"); return rgb; }

  unsigned int xBlockCount = originalWidth  / resizeWidth;
  unsigned int yBlockCount = originalHeight / resizeHeight;

  unsigned int xBlock=0;
  unsigned int yBlock=0;

  unsigned char * rgbOut = output;
  unsigned char * rgbIn = rgb;

  for (yBlock = 0 ;  yBlock<yBlockCount; yBlock++)
  {
    for (xBlock = 0 ;  xBlock<xBlockCount; xBlock++)
    {

      //Sample a group of pixels that will become one pixel!
      unsigned int value = 0;
      unsigned char * tmpRGB = rgbIn;
      unsigned char * tmpRGBLimit = rgbIn + xBlockCount*3;
      unsigned int repeatTimes = yBlockCount;
      unsigned int i=0;
      for (i=0; i<repeatTimes; i++)
      {
        value+= *tmpRGB;
        tmpRGB+=originalWidth*3;
        tmpRGBLimit+=originalWidth*3;
      }
      //Finished

      value/=xBlockCount*yBlockCount;
      if (value>255) { value = 255; }

      *rgbOut = (unsigned char) value;
      rgbOut+=3;

      rgbIn+= xBlockCount*3;
    }

    rgbIn+= originalWidth*3*xBlockCount;
  }

  return output;
}




struct Image * resizeImage(struct Image * img,unsigned int resizeWidth , unsigned int resizeHeight )
{
 struct Image * smallerImg  = (struct Image *) malloc(sizeof (struct Image));
 if (smallerImg==0) { fprintf(stderr,"Could not allocate space for smaller image \n"); return img; }

 smallerImg->width = resizeWidth;
 smallerImg->height = resizeHeight;
 smallerImg->depth = img->depth;

 smallerImg->pixels = resizeImageInternal3bytes(img->pixels, img->width, img->height, smallerImg->width , smallerImg->height);

 return smallerImg;
}


