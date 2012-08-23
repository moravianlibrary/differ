__kernel void img_qi_B ( 
    __global float *mu1, 
    __global float *mu2,
    __global float *img1_sq, 
    __global float *img2_sq,
    __global float *img1_img2,
    __global float *iqi_index, 
    const int img_width, 
    const int img_height,
    const int nChan, 
    const int size_filter ) 
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
    int total = size_filter * size_filter;

    int rowOffset = h*img_width*nChan;
    int curLoc = rowOffset + nChan*w;
    int c; 
    for( c=0; c<nChan; c++) {
        mu1_sq[c] = mu1[curLoc+c]*mu1[curLoc+c];
        mu2_sq[c] = mu2[curLoc+c]*mu2[curLoc+c];
        mu1_mu2[c] = mu1[curLoc+c]*mu2[curLoc+c];
    }
    
    if ( h>=size_filter/2 && h<=img_height-size_filter/2 && w>=size_filter/2 && w<=img_width-size_filter/2 )
    {
        int k=0;
        for(int  i=-size_filter/2 ; i<size_filter/2; i++)
        {
          int curRow = curLoc + i*(img_width*nChan);
          for(int  j=-size_filter/2 ; j<size_filter/2; j++,k++)
          {
              int offset = j*nChan;
              for( c=0; c<nChan; c++)
              {
                sum_img1_sq[c]   = sum_img1_sq[c]   + img1_sq[curRow + offset +c];
                sum_img2_sq[c]   = sum_img2_sq[c]   + img2_sq[curRow + offset +c];
                sum_img1_img2[c] = sum_img1_img2[c] + img1_img2[curRow + offset +c];
              }
          }
        }
        for( c=0; c<nChan; c++) {
          sigma1[c] = (sum_img1_sq[c])/total - mu1_sq[c];
          sigma2[c] = (sum_img2_sq[c])/total - mu2_sq[c] ;
          sigma12[c] = (sum_img1_img2[c])/total - mu1_mu2[c];
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
      num = 4 * (mu1_mu2[c]) * (sigma12[c]);
      denom = ( mu1_sq[c] + mu2_sq[c] ) * (sigma1[c] + sigma2[c] );
      if(denom==0)
       iqi_index[curLoc + c] = (1.0); 
      else
        iqi_index[curLoc + c] = (1.0*num)/(denom);
    }

}
