1. Compile the .cpp file using the following command on the shell prompt

>> g++ <filename> -I /usr/include/opencv -L /usr/lib  -lm -lcv -lhighgui -lcvaux


2. Run the executable with 2 input parameters. The filenames of two images to be compared.

>> ./a/out lena.jpg lena_blurred.jpg

3. Outputs the MSSIM and PSNR.
