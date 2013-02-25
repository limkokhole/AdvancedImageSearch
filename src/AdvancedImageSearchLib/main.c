/* AdvancedImageSearch Lib..
A fancy , OpenGL slideshow application !
URLs: http://ammar.gr
Written by Ammar Qammaz a.k.a. AmmarkoV 2013

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#include "AdvancedImageSearchLib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include "version.h"

#include "codecs/codecs.h"
#include "codecs/jpg.h"

#include "image_processing/histograms.h"

#include "tools/string_extension_scanner.h"
#include "tools/parameter_parser.h"

#define INITIAL_ALLOCATED_MEMORY_FOR_RESULTS 1000


const char * AISLib_Version()
{
  return FULLVERSION_STRING;
}


struct AISLib_SearchResults * createSearchResults(unsigned int initialNumberOfResults)
{
  struct AISLib_SearchResults * sr = (struct AISLib_SearchResults *)  malloc(initialNumberOfResults* sizeof(struct AISLib_SearchResults));
  if (sr==0) { fprintf(stderr,"Could not allocate new search results"); return 0;}

  sr->resultsMAX = initialNumberOfResults;
  return sr;
}

void destroySearchResults(struct AISLib_SearchResults * sr)
{
  if (sr==0) { return ; }
  free(sr);
  return;
}



struct AISLib_SearchResults * addMoreSearchResults(struct AISLib_SearchResults * initial_results,unsigned int addedNumberOfResults)
{
  struct AISLib_SearchResults * sr = (struct AISLib_SearchResults *)  realloc(initial_results , (initial_results->resultsMAX+addedNumberOfResults)* sizeof(struct AISLib_SearchResults));
  if (sr==0)
   {
       fprintf(stderr,"Could not allocate more results ");
       return initial_results;
   }

  sr->resultsMAX += addedNumberOfResults;
  return sr;
}



void AISLib_printHelp()
{
    printListOfParametersRecognized();
}

char * AISLib_loadDirAndCriteriaFromArgs(int argc, char *argv[], struct AISLib_SearchCriteria * criteria )
{
 return parseCommandLineParameters(argc,argv,criteria);
}





//This is a more complex check , could use it in the future
int scanFileForImage(char * filename)
{
    char command[1024]={0};
    strcpy(command,"file \"");
    strncat(command,filename,1000);
    strcat(command,"\"");

  FILE *fp;
    /* Open the command for reading. */
     fp = popen(command, "r");
     if (fp == 0 )
       {
         fprintf(stderr,"Failed to run command\n");
         return 0;
       }

 /* Read the output a line at a time - output it. */
  char output[2048]={0};
  unsigned int size_of_output = 2048;

  unsigned int i=0;
  while (fgets(output, size_of_output , fp) != 0)
    {
        ++i;
         fprintf(stderr,"\n\nline %u = %s \n",i,output);
        break;
    }


  /* close */
  pclose(fp);

    return 0;

}



inline int fileIsImage(char * filename)
{
  return scanStringForImageExtensionsSimple(filename);
}



int imageFitsCriteria(struct Image * img,struct AISLib_SearchCriteria * criteria)
{
   if (img==0) { fprintf(stderr,"imageFitsCriteria : Incorrect image \n");    return 0; }
   if (criteria==0) { fprintf(stderr,"imageFitsCriteria : Incorrect criteria \n"); return 0; }

   if (criteria->minDimensionsUsed)
    { //Ready to discard for violation of minimum dimensions
      if ( ( img->width < criteria->minWidth ) || ( img->height < criteria->minHeight ) ) { return 0; }
    }

   if (criteria->maxDimensionsUsed)
    { //Ready to discard for violation of maximum dimensions
      if ( ( img->width > criteria->maxWidth ) || ( img->height > criteria->maxHeight ) ) { return 0; }
    }

   if (criteria->colorRangeUsed)
    {//Ready to discard for violation of histogram
      struct Histogram * histogram = generateHistogram( (unsigned char * ) img->pixels , img->width , img->height , img->depth );
      if ( histogram!=0 )
      {
         if (! histogramIsCloseToColor(histogram,  criteria->colorRangeSpecificR  ,
                                                   criteria->colorRangeSpecificG  ,
                                                   criteria->colorRangeSpecificB,
                                                   criteria->colorRange,
                                                   img->width * img->height * img->depth ,
                                                   30.0 ) )
                                                   {
                                                    free(histogram);
                                                    histogram=0;
                                                    return 0;
                                                   }
        free(histogram);
        histogram=0;
       } else
       {
          //No Histogram no color but range enforced , doesnt fit the criteria
          return 0;
       }
    }



    return 1;
}



struct AISLib_SearchResults * AISLib_Search(char * directory,struct AISLib_SearchCriteria * criteria)
{
  DIR *dpdf=0;
  struct dirent *epdf=0;

  char fullPath[4096]={0};

  if (directory == 0) { dpdf = opendir("./"); } else
                      { dpdf = opendir(directory); }

  if (dpdf != 0)
   {
     struct AISLib_SearchResults * sr = createSearchResults(INITIAL_ALLOCATED_MEMORY_FOR_RESULTS);
     if (sr==0) { return 0; }

     epdf = readdir(dpdf);
     while ( epdf !=  0 )
      {
         //printf("RAW %s\n",epdf->d_name);
         unsigned int image_type  = fileIsImage(epdf->d_name);
         if (image_type!=0)
          {
              strcpy(fullPath,directory);
              strcat(fullPath,"/");
              strcat(fullPath,epdf->d_name);

              struct Image * img = readImage(fullPath , image_type , searchCriteriaRequireOnlyImageHeaderLoaded(criteria) );
              if (  img!=0 )
              {
                 if (imageFitsCriteria(img,criteria))
                 {
                   printf("%s ",epdf->d_name);
                 }

                //fprintf(stderr,"Survived read , I have a %ux%u image ",pic.width,pic.height);
                if (img->pixels!=0) { free(img->pixels); img->pixels=0; }
                if (img!=0) { free(img); img=0; }
              }
          }

        //Next Value
        epdf = readdir(dpdf);
      }

      closedir(dpdf);
      return sr;
    }

  return 0;
}
