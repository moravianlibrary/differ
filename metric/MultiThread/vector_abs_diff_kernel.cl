__kernel void vector_abs_diff(__global float *A, __global float *B, __global float *C) {
    
    // Get the index of the current element
    int i = get_global_id(0);

    // Do the operation
    float temp =  A[i] - B[i];
    if (temp<0)
      C[i] = -temp;
    else
      C[i] = temp;

}
