#include "core.h"
#include "similarityMetric.h"
#include "ssim.h"
#include "msssim.h"

calcMSSSIM :: calcMSSSIM()
{
  K1 = 0.01;
  K2 = 0.03;
  gaussian_window = 11;
  gaussian_sigma = 1.5;
  level = 5;
  L = 255;
  ms_ssim_map = NULL;
  for (int i=0; i < 4; i++)
    ms_ssim_value.val[i] = -1; // Initialize with an out of bound value of mssim [0,1]

  // setting alpha, beta, gamma default values
  float alpha_t[5] = {0.0, 0.0, 0.0, 0.0, 0.1333};
  float beta_t[5] = {0.0448, 0.2856, 0.3001, 0.2363, 0.1333};
  float gamma_t[5] = {0.0448, 0.2856, 0.3001, 0.2363, 0.1333};
  alpha = new float[level];
  beta = new float[level];
  gamma = new float[level];
  memcpy(alpha, alpha_t, sizeof(alpha_t));
  memcpy(beta, beta_t, sizeof(beta_t));
  memcpy(gamma, gamma_t, sizeof(gamma_t));
}

calcMSSSIM :: ~calcMSSSIM()
{
  delete[] alpha;
  delete[] beta;
  delete[] gamma;
  int i;
  for (i=0; i < level; i++) {
    if (ms_ssim_map[i] != NULL)
      cvReleaseImage(&ms_ssim_map[i]);
  }
  if (ms_ssim_map !=NULL) 
    free(ms_ssim_map);
}

void calcMSSSIM :: releaseMSSSIM_map() {
  int i;
  for (i=0; i < level; i++) {
    if (ms_ssim_map[i] != NULL)
      cvReleaseImage(&ms_ssim_map[i]);
  }
  if (ms_ssim_map != NULL)
    free(ms_ssim_map);
}

// Prints all index maps of all the levels into different xml files
int calcMSSSIM :: print_map()
{
  if (ms_ssim_map == NULL)
  {
    cout<<"Error>> No Index_map_created.\n";
    return 0;
  }
  char file_name[50];
  // Printing the MS-SSIM_Map
  for (int i=0; i < level; i++)
  {
    sprintf(file_name, "img_MS-SSIM_map_level_%d.xml", i);
    cvSave(file_name, ms_ssim_map[i], NULL, "Testing MS-SSIM Index map");
  }
  return 1;
}

CvScalar calcMSSSIM :: compare(IplImage *source1, IplImage *source2, Colorspace space)
{
  // image dimensions
  int x = source1->width, y = source1->height;
  int nChan = source1->nChannels;
  int d = source1->depth;

  // creating a object of class calcSSIM 
  // setting the initial parameters values
  calcSSIM ssim;
  ssim.setK1(K1);
  ssim.setK2(K2);
  ssim.setGaussian_window(gaussian_window);
  ssim.setGaussian_sigma(gaussian_sigma);
  ssim.setL(L);
  //Creating an array of IplImages for ssim_map at various levels
  ms_ssim_map = (IplImage**)(malloc(sizeof(IplImage*)*level));

  #ifdef DEBUG
  cout<<"\nAlpha = "<<alpha[0]<<" "<<alpha[1]<<" "<<alpha[2]<<" "<<alpha[3]<<"\n";
  cout<<"\nBeta = "<<beta[0]<<" "<<beta[1]<<" "<<beta[2]<<" "<<beta[3]<<"\n";
  cout<<"\nGamma = "<<gamma[0]<<" "<<gamma[1]<<" "<<gamma[2]<<" "<<gamma[3]<<"\n";
  #endif

  for (int i=0; i<level; i++)
  {
    //Downsampling of the original images
    IplImage *downsampleSrc1, *downsampleSrc2;
    //Downsampling the images
    CvSize downs_size = cvSize((int)(x/pow(2, i)), (int)(y/pow(2, i)));
    downsampleSrc1 = cvCreateImage(downs_size, d, nChan);
    downsampleSrc2 = cvCreateImage(downs_size, d, nChan);
    cvResize(source1, downsampleSrc1, CV_INTER_NN);
    cvResize(source2, downsampleSrc2, CV_INTER_NN);

    #ifdef DEBUG
    cout<<"Values at level="<<i<<" \n";
    #endif 

    ssim.compare(downsampleSrc1, downsampleSrc2, space);

    CvScalar mssim_t = ssim.getMSSIM();
    CvScalar mcs_t = ssim.getMeanCSvalue();
    ms_ssim_map[i] = ssim.getSSIM_map();
    //releasing the CS_map since not required
    ssim.releaseCS_map(); 
    
    #ifdef DEBUG
    cout<<"Size of MAP at level = "<<i<<"size = "<<ms_ssim_map[i]->width<<" "<<ms_ssim_map[i]->height<<"\n";
    cout<<"Test values of ms_ssim = "<<mssim_t.val[0]<<" "<<mssim_t.val[1]<<" "<<mssim_t.val[2]<<"\n";
    #endif

    // calculating the withed average to find ms-ssim
    for (int j=0; j < 4; j++)
    {
      if (i == 0)
        ms_ssim_value.val[j] = pow((mcs_t.val[j]), (double)(beta[i]));
      else 
        if (i == level-1)
          ms_ssim_value.val[j] = (ms_ssim_value.val[j]) * pow((mssim_t.val[j]), (double)(beta[i]));
        else
          ms_ssim_value.val[j] = (ms_ssim_value.val[j]) * pow((mcs_t.val[j]), (double)(beta[i]));
    }
    //Release images
    cvReleaseImage(&downsampleSrc1);
    cvReleaseImage(&downsampleSrc2);

  }
  return ms_ssim_value;
}



