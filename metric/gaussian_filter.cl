__kernel void gaussian_filter (
    const __global float *input, 
    __constant float *filter,
    __global float *out,
    const int img_height, 
    const int img_width, 
    const int nChan,
    const int size_filter )
{
    
    // Get the index of the current element
    int h = get_global_id(0);
    int w = get_global_id(1);
    float sumR = 0.0;
    float sumG = 0.0;
    float sumB = 0.0;
    int rowOffset = h*img_width*nChan;
    int curLoc = rowOffset + nChan*w;
    
    if ( h>=size_filter/2 && h<img_height-size_filter/2 && w>=size_filter/2 && w<img_width-size_filter/2 )
    {
        int k=0;
        for(int  i=-size_filter/2 ; i<=size_filter/2; i++)
        {
          int curRow = curLoc + i*(img_width*nChan);
          for(int  j=-size_filter/2 ; j<=size_filter/2; j++,k++)
          {
              int offset = j*nChan;
              sumB += input[curRow + offset    ] * filter[k];
              sumG += input[curRow + offset + 1] * filter[k];
              sumR += input[curRow + offset + 2] * filter[k];
          }
        }
        out[curLoc    ] = sumB;
        out[curLoc + 1] = sumG;
        out[curLoc + 2] = sumR;
    }
    else {
      out[curLoc    ] =  input[curLoc    ];
      out[curLoc + 1] =  input[curLoc + 1];
      out[curLoc + 2] =  input[curLoc + 2];
    }
}
