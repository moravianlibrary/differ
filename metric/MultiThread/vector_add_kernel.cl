__kernel void vector_add(__global float *A, __global float *B, __global float *C) {
    
    // Get the index of the current element
    int i = get_global_id(0);

    // Do the operation
    C[i] = A[i] + B[i];
}
