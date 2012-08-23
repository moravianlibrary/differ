__kernel void ms_ssim_B (
    __global float *mu1, 
    __global float *mu2, 
    __constant float *filter,  
    __global float *img1_sq, 
    __global float *img2_sq,
    __global float *img1_img2,
    __global float *ssim_index, 
    __global float *cs_index,
    const int img_width,
    const int img_height,
    const int nChan,
    const int size_filter, 
    const float K1, 
    const float K2 ) 
{
    
    // Get the index of the current element
    int w = get_global_id(0);
    int h = get_global_id(1);
    float sum_img1_sq[3] = {0.0, 0.0, 0.0};
    float sum_img2_sq[3] = {0.0, 0.0, 0.0};
    float sum_img1_img2[3] = {0.0, 0.0, 0.0};
    float mu1_sq[3] = {0.0, 0.0, 0.0};
    float mu2_sq[3] = {0.0, 0.0, 0.0};
    float mu1_mu2[3] = {0.0, 0.0, 0.0};
    float sigma1[3] = {0.0, 0.0, 0.0};
    float sigma2[3] = {0.0, 0.0, 0.0};
    float sigma12[3] = {0.0, 0.0, 0.0};
    float num;
    float denom;
    float num_cs;
    float denom_cs;
    float C1 = K1;
    float C2 = K2;

    int rowOffset = h*img_width*nChan;
    int curLoc = rowOffset + nChan*w;
    int c; 
    for( c=0; c<nChan; c++) {
        mu1_sq[c] = mu1[curLoc+c]*mu1[curLoc+c];
        mu2_sq[c] = mu2[curLoc+c]*mu2[curLoc+c];
        mu1_mu2[c] = mu1[curLoc+c]*mu2[curLoc+c];
    }
    
    if ( h>=size_filter/2 && h<img_height-size_filter/2 && w>=size_filter/2 && w<img_width-size_filter/2 )
    {
        int k=0;
        for(int  i=-size_filter/2 ; i<=size_filter/2; i++)
        {
          int curRow = curLoc + i*(img_width*nChan);
          for(int  j=-size_filter/2 ; j<=size_filter/2; j++,k++)
          {
              int offset = j*nChan;
              for( c=0; c<nChan; c++)
              {
                sum_img1_sq[c]   = sum_img1_sq[c]   + (img1_sq[curRow + offset +c] * filter[k]);
                sum_img2_sq[c]   = sum_img2_sq[c]   + (img2_sq[curRow + offset +c] * filter[k]);
                sum_img1_img2[c] = sum_img1_img2[c] + (img1_img2[curRow + offset +c] * filter[k]);
              }
          }
        }

        for( c=0; c<nChan; c++) {
          sigma1[c] = sum_img1_sq[c] - mu1_sq[c];
          sigma2[c] = sum_img2_sq[c] - mu2_sq[c] ;
          sigma12[c] = sum_img1_img2[c] - mu1_mu2[c];
        }
    }
    else 
    {
       for( c=0; c<nChan; c++) {
          sigma1[c] = img1_sq[curLoc +c] - mu1_sq[c];
          sigma2[c] = img2_sq[curLoc +c] - mu2_sq[c] ;
          sigma12[c] = img1_img2[curLoc +c] - mu1_mu2[c];
        }
    }

    for( c=0; c<nChan; c++) 
    {
      num = ( 2*mu1_mu2[c] + C1 ) * ( 2*sigma12[c] + C2 );
      denom = ( mu1_sq[c] + mu2_sq[c] + C1 ) * (sigma1[c] + sigma2[c] + C2 );
      num_cs = ( 2*sigma12[c] + C2 );
      denom_cs = (sigma1[c] + sigma2[c] + C2 );
      ssim_index[curLoc + c] = (1.0*num)/denom;
      cs_index[curLoc + c] = (1.0*num_cs)/denom_cs;
    }

}
