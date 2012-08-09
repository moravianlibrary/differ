__kernel void ms_ssim_A(const __global float *src1, const __global float *src2, __constant float *filter, __global float *img1_sq, __global float *img2_sq, __global float *img1_img2, __global float *mu1, __global float *mu2, const int img_width, const int img_height, const int nChan, const int size_filter) {
    
    // Get the index of the current element
    int w = get_global_id(0);
    int h = get_global_id(1);
    float sum_mu1[3] = {0.0, 0.0, 0.0};
    float sum_mu2[3] = {0.0, 0.0, 0.0};
    
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
              sum_mu1[0] += src1[curRow + offset    ] * filter[k];
              sum_mu1[1] += src1[curRow + offset + 1] * filter[k];
              sum_mu1[2] += src1[curRow + offset + 2] * filter[k];
              sum_mu2[0] += src2[curRow + offset    ] * filter[k];
              sum_mu2[1] += src2[curRow + offset + 1] * filter[k];
              sum_mu2[2] += src2[curRow + offset + 2] * filter[k];
          }
        }
        mu1[curLoc    ] = sum_mu1[0];
        mu1[curLoc + 1] = sum_mu1[1];
        mu1[curLoc + 2] = sum_mu1[2];
        mu2[curLoc    ] = sum_mu2[0];
        mu2[curLoc + 1] = sum_mu2[1];
        mu2[curLoc + 2] = sum_mu2[2];
    }
    else {
        mu1[curLoc    ] = src1[curLoc    ];
        mu1[curLoc + 1] = src1[curLoc + 1];
        mu1[curLoc + 2] = src1[curLoc + 2];
        mu2[curLoc    ] = src2[curLoc    ];
        mu2[curLoc + 1] = src2[curLoc + 1];
        mu2[curLoc + 2] = src2[curLoc + 2];
    }

    //creating the image squares for source images
    img1_sq[curLoc    ] = src1[curLoc    ]*src1[curLoc    ];
    img1_sq[curLoc + 1] = src1[curLoc + 1]*src1[curLoc + 1];
    img1_sq[curLoc + 2] = src1[curLoc + 2]*src1[curLoc + 2];
    
    img2_sq[curLoc    ] = src2[curLoc    ]*src2[curLoc    ];
    img2_sq[curLoc + 1] = src2[curLoc + 1]*src2[curLoc + 1];
    img2_sq[curLoc + 2] = src2[curLoc + 2]*src2[curLoc + 2];
    
    img1_img2[curLoc    ] = src1[curLoc    ]*src2[curLoc    ];
    img1_img2[curLoc + 1] = src1[curLoc + 1]*src2[curLoc + 1];
    img1_img2[curLoc + 2] = src1[curLoc + 2]*src2[curLoc + 2];
}
