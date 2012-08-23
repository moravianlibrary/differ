__kernel void ssim_A (
    const __global float *src1, 
    const __global float *src2,
    __constant float *filter,
    __global float *img1_sq, 
    __global float *img2_sq,
    __global float *img1_img2, 
    __global float *mu1, 
    __global float *mu2, 
    const int img_width, 
    const int img_height, 
    const int nChan, 
    const int size_filter ) 
{
    
    // Get the index of the current element
    int w = get_global_id(0);
    int h = get_global_id(1);
    float sum_mu1[3] = {0.0, 0.0, 0.0};
    float sum_mu2[3] = {0.0, 0.0, 0.0};
    
    int rowOffset = h*img_width*nChan;
    int curLoc = rowOffset + nChan*w;
    int c; 
    if ( h>=size_filter/2 && h<img_height-size_filter/2 && w>=size_filter/2 && w<img_width-size_filter/2 )
    {
        int k=0;
        for(int  i=-size_filter/2 ; i<=size_filter/2; i++)
        {
          int curRow = curLoc + i*(img_width*nChan);
          for(int  j=-size_filter/2 ; j<=size_filter/2; j++,k++)
          {
              int offset = j*nChan;
              for( c=0; c<nChan; c++) {
                sum_mu1[c] = sum_mu1[c] + (src1[curRow + offset + c] * filter[k]);
                sum_mu2[c] = sum_mu2[c] + (src2[curRow + offset + c] * filter[k]);
              }
          }
        }
        for( c=0; c<nChan; c++) {
          mu1[curLoc + c] = sum_mu1[c];
          mu2[curLoc + c] = sum_mu2[c];
        }
    }
    else {
        for( c=0; c<nChan; c++) {
          mu1[curLoc + c] = src1[curLoc + c];
          mu2[curLoc + c] = src2[curLoc + c];
        }
    }

    //creating the image squares for source images
        
    for( c=0; c<nChan; c++) {
      img1_sq[curLoc + c] = src1[curLoc + c]*src1[curLoc + c];
      img2_sq[curLoc + c] = src2[curLoc + c]*src2[curLoc + c];
      img1_img2[curLoc + c] = src1[curLoc + c]*src2[curLoc + c];
    }
}
