__kernel void vector_pow(__global float *A, __global float *C, const int n) {
    
    // Get the index of the current element
    int i = get_global_id(0);
    int j;
    float temp = 1;
    // Do the operation
    //for(j=0; j<n; j++)
      //  temp = temp * A[i];
    temp = pow(A[i],n);

    C[i] = temp;
}
