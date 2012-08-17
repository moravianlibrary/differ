__kernel void vector_mse (
    __global float *A, 
    __global float *B, 
    __global float *C ) 
{
    
    // Get the index of the current element
    int i = get_global_id(0);

    // Do the operation
    float diff, sq;
    diff =  A[i] - B[i];
    sq = diff * diff;
    C[i] = sq;

}
