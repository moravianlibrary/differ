                                            
    <h2>Help</h2>
    <div id="summary">
<!-- ************************************************************* -->
		
The main purpose of this tool is to compare two images and calculate differences between them. It also lists various parameters of these images, even in case when only one image file is selected.<br /> <br />

<b>How to use DIFFER in a few steps:</b>
<ol>
	<li /> On the File manager tab, upload images you want to analyse and compare. Images for the comparation must have same pixel dimensions.
  <li /> Select 1 image to analyse or 2 images to compare them and press Execute button. 
	<li /> You'll be redirected on the Compare tab. Review the results.
</ol>
<br />	
		
<!-- ///////////////////////////////////////////////////////////// -->		
    </div>

    <h2>Glossary</h2>
    <div id="summary">
<!-- ************************************************************* -->

<!-- <br /><a name="file_val" /><h4>Actual file values</h4> -->
<br /><a name="bps" /><h4>Bits per sample</h4>Bits per sample is a variation of the compression ratio. The metrics indicates the average number of bits to represent a single sample of the data (e.g., bits per pixel for image coding).
Compression ratio is defined as the ratio of the number of bits to represent the original data to the number of bits to represent the original data.
<br /><a name="bypass" /><h4>Bypass</h4>(Tier-1 Coding in JPEG2000 - Fractional Bit-Plane Coding)<br />
Coding passes<br />
There are three coding passes - significance propagation pass (SPP), magnitude refinement pass (MRP), and cleanup pass (CUP). Three different coding passes are applied to each bit-plane of a code-block except the first bit-plane (the most significant bit-plane), which is applied only with the cleanup pass. After each coding pass completes a run of scan pattern in the current bit-plane, the next coding pass restarts the scan pattern from the beginning. The first bit-plane is only decoded by the CUP. The remaining bit-planes are coded in the order of SPP, MRP and CUP. 
<br />
Selective binary arithmetic coding-bypass mode: Instead of applying the binary arithmetic coding (MQ-coder) on symbols (the contexts and decision bits) generated during all three coding passes, the bypass mode allows bypassing MQ-coder for the SPP and MRP after the four most significant bit-planes are coded. In other words, only those symbols generated in the CUP will be coded during the SPP and MRP, if the bypass mode is selected. 
<br /><a name="cblk" /><h4>Code Block (Cblk)</h4>During entropy coding, each wavelet subband is further divided into a number of code-blocks. At this stage all elements in all the subbands are represented in sign and magnitude representation of integers instead of two’s complement. Dimension of the code-blocks is always a power of 2 with the minimum height and width being 4 and maximum height and width being 1024. Further restriction in dimension of a code-block is that if height of a code-block is 2x and width of the code-block 2y then x + y is limited to be less than or equal to 12. Typical choice of code-block size is 64 x 64 or 32 x 32. It has been found experimentally that the compression performance degrades when the code-block size is chosen below 16 x 16. It should be noted that the profile-0 of JPEG2000 Part 1 amendments further restricts the code-block size to be either 32 x 32 or 64 x 64. 
During the coding phase, each code-block is decomposed into a number of bit-planes. If the precision of the subband is P bits, then each code-block in the subband is decomposed into P number of bit-planes. Bit-plane coding (BPC) is applied on each bit-plane of the code-blocks to generate intermediate data in the form of a context and binary decision value. The intermediate data is input to the binary arithmetic coding (BAC) step to generate the final compressed bitsream.
<br /><a name="color_space" /><h4>Color space</h4>Color space is constituted of all possible combinations of the primary colors - color components. 

A color model is an abstract mathematical model describing the way colors can be represented as tuples of numbers, typically as three or four values or color components (e.g. RGB and CMYK are color models). 
<br /><a name="color_space_hdr" /><h4>Color space - Header of A / B file</h4>Color space declared in the header of the first file/color space declared in the header of the second file.
<br /><a name="color_space_body" /><h4>Color space - Body of A / B file</h4>Color space detected in the body of the first file/color space detected in the body of the second file.
<br /><a name="color_trans" /><h4>Color transformation, Subsampling</h4>Chroma subsampling is the practice of encoding images by implementing less resolution for chroma information than for luma information, taking advantage of the human visual system's lower acuity for color differences than for luminance.
It is used in many video encoding schemes - both analog and digital - and also in JPEG encoding.
Because of storage and transmission limitations, there is always a desire to reduce (or compress) the signal. Since the human visual system is much more sensitive to variations in brightness than color, a video system can be optimized by devoting more bandwidth to the luma component (usually denoted Y'), than to the color difference components Cb and Cr. In compressed images, for example, the 4:2:2 Y'CbCr scheme requires two-thirds the bandwidth of (4:4:4) R'G'B'. This reduction results in almost no visual difference as perceived by the viewer.
<br /><a name="comp_profile" /><h4>Compared profile</h4>Values declared in the profile. The actual file values should comply with these values.
<br /><a name="profile" /><h4>Compliance with NDK profile</h4>Profile for compliance check. JP2 files can be tested whether they conform to NDK profile or to other uploaded profiles. 
<br /><a name="compression" /><h4>Compression (Creversible)</h4>Data compression is the technique to reduce the redundancies in data representation in order to decrease data storage requirements and hence communication costs. It is a method that takes an input data and generates a shorter representation of that data with a fewer number of bits. The reverse process is called decompression, which takes the compressed data and generates or reconstructs the data.  
Lossless compression  - the reconstructed data are identical to the original data (exact replica of the original data).  The process of compression is fully reversible.
Lossy compression - the reconstructed data are not identical to the original data.
<br /><a name="csv" /><h4>CSV data</h4>A comma-separated values (CSV) file stores tabular data (numbers and text) in plain-text form. As a result, such a file is easily human-readable (e.g., in a text editor).
CSV is a simple file format that is widely supported by consumer, business, and scientific applications. Among its most common uses is to move tabular data between programs that naturally operate on a more efficient or complete proprietary format. For example: a CSV file might be used to transfer information from a database program to a spreadsheet.
<br /><a name="cdecomp" /><h4>Decomposition (Cdecomp)</h4>
<br /><a name="clevels" /><h4>Decomposition levels (Clevels)</h4>In the compression phase the JPEG2000 standard adopts the discrete wavelet transform (DWT) instead of the discrete cosine transform (DCT) used in JPEG standard.
The DWT essentially analyzes tile (image) component to decompose it into a number of subbands at different levels of resolution. The two-dimensional DWT is performed by applying the one-dimensional DWT row-wise and then column-wise in each component.
Applying the one-dimensional transform in each row, we produce two subbands in each row. When the low-frequency subbands of all the rows (L) are put together, it looks like a thin version of the input signal. Similarly we put together the high-frequency subbands of all the rows to produce the H subband, which contains mainly the high-frequency information around discontinuities (edges in an image) in the input signal. Then applying a one-dimensional DWT column-wise on these L and H subbands, we produce four subbands LL, LH, HL, and HH.<br /> <br />
Multiresolution decomposition approach in the two-dimensional signal:<br />
After the first level of decoposition, it generates four subbands LL1, HL1, LH1, and HH1 (L - low-frequency subband, H - high-frequency subband). Considering the input signal is an image, the LL1 subband can be considered as a 2:1 subsampled  (both horizontally and vertically) version of the original image. The other three subbands HL1, LH1, and HH1 contain higher frequency detail information. These spatially oriented (horizontal, vertical and diagonal) subbands mostly contain information of local discontinuities in the image and the bulk of the energy in each of these three subbands is concentrated in the vicinity areas corresponding to edge activities in the original image. Since LL1 is the coarser approximation of the input, it has similar spatial and statistical characteristics to the original image. As a result, it can be further decomposed into four subbands - LL2, HL2, LH2, and HH2. Accordingly the image is decomposed into 10 subbands LL3, HL3, LH3, HH3, HL2, LH2, HH2, HL1, LH1, and HH1 after three levels of pyramidal multiresolution subband decomposition. The same computation can continue to further decompose LL3 into higher levels. 
The maximum number of levels of decomposition allowed in Part 1 of the JPEG2000 standard is 32.
<br /><a name="qderived" /><h4>Derived quantization (Qderived)</h4>Pattern = "B" - Quantization steps derived from LL band parameters? If true, all quantization step sizes will be related to the LL subband's step sizes through appropriate powers of 2 and only the LL band step size will be written in code-stream markers. Otherwise, a separate step size will be recorded for every subband. You cannot use this option with reversible compression. Default is not derived.
<br /><a name="diff_pic" /><h4>Differential picture</h4>Differential picture is a result of subtraction of one uploaded picture from another.

The pixel subtraction operator takes two images as input and produces as output a third image whose pixel values are simply those of the first image minus the corresponding pixel values from the second image.
If the pixel values in the input images are actually vectors rather than scalar values (e.g. for color images) then the individual components (e.g. red, blue and green components) are simply subtracted separately to produce the output value.
<br /><a name="droid" /><h4>Droid</h4>DROID (Digital Record Object Identification) is a software tool developed by The National Archives to perform automated batch identification of file formats. It is one of a planned series of tools utilising PRONOM to provide specific digital preservation services.
<br /><a name="eph" /><h4>EPH (End Of Packet Header) (Cuse_eph)</h4>JPEG2000 Part 1 standard:
End Of Packet Header is an In Bitstream Marker Segment. 
The bitstream marker segments are used for error resilience.
<br /><a name="equal_hash" /><h4>Equal HASH</h4>Comparison of hashes which were generated from uploaded files. If hashes are equal, the output value is "Yes”. If hashes are different, the output value is "No”.
<br /><a name="exiftool" /><h4>Exiftool</h4>ExifTool is a free software  program for reading, writing, and manipulating image, audio, and video metadata . ExifTool is commonly incorporated into different types of digital  workflows  and supports many types of metadata including Exif , IPTC , XMP , JFIF , GeoTIFF , ICC Profile , Photoshop IRB, FlashPix , AFCP and ID3 , as well as the manufacturer-specific metadata formats of many digital cameras .
<br /><a name="hash" /><h4>File HASH</h4>Fingerprint (computing)
In computer science, a fingerprinting algorithm is a procedure that maps an arbitrarily large data item (such as a computer file) to a much shorter bit string, its fingerprint, that uniquely identifies the original data for all practical purposes just as human fingerprints uniquely identify people for practical purposes. This fingerprint may be used for data deduplication purposes.
Fingerprint functions may be seen as high-performance hash functions used to uniquely identify substantial blocks of data where cryptographic hash functions may be unnecessary. 
<br /><a name="characterization" /><h4>Characterization</h4>Format characterization is the process of determining the format-specific significant properties of an object of a given format, e.g.: "I have an object of format F; what are its salient properties?"
<br /><a name="icc" /><h4>ICC profile</h4>ICC profile is a set of data that characterizes a color input or output device, or a color space, according to standards promulgated by the International Color Consortium (ICC). Profiles describe the color attributes of a particular device or viewing requirement by defining a mapping between the device source or target color space and a profile connection space (PCS). This PCS is either CIELAB (L*a*b*) or CIEXYZ. Mappings may be specified using tables, to which interpolation is applied, or through a series of parameters for transformations.
Every device that captures or displays color can be profiled. <br />
The ICC defines the format precisely but does not define algorithms or processing details. This means there is room for variation between different applications and systems that work with ICC profiles.
<br /><a name="ident" /><h4>Identification</h4>Format identification is the process of determining the format to which a digital object conforms; in other words, it answers the question: "I have a digital object; what format is it?"
<br /><a name="jhove" /><h4>Jhove</h4>JHOVE (pronounced "jove"), the JSTOR/Harvard Object Validation Environment. JSTOR  and the Harvard University Library  are collaborating on a project to develop an extensible framework for format validation. JHOVE provides functions to perform format-specific identification, validation, and characterization of digital objects.
<br /><a name="jp2checker" /><h4>JP2Checker</h4>
<br /><a name="kduex" /><h4>KDU_expand</h4>
<br /><a name="lin_hist" /><h4>Linear histogram</h4>Color histogram is a representation of the distribution of colors in an image. For digital images, a color histogram represents the number of pixels that have colors in each of a fixed list of color ranges, that span the image's color space, the set of all possible colors.
If the set of possible color values is sufficiently small, each of those colors may be placed on a range by itself; then the histogram is merely the count of pixels that have each possible color. Most often, the space is divided into an appropriate number of ranges, often arranged as a regular grid, each containing many similar color values. The color histogram may also be represented and displayed as a smooth function defined over the color space that approximates the pixel counts.
Like other kinds of histograms, the color histogram is a statistic that can be viewed as an approximation of an underlying continuous distribution of colors values.
<br /><a name="log_hist" /><h4>Logarithmic histogram</h4> Linear color histogram transformed logarithmically. 

Presentation of data on a logarithmic scale can be helpful when the data cover a large range of values. The use of the logarithms of the values rather than the actual values reduces a wide range to a more manageable size. 
<br /><a name="no_components" /><h4>Number of components</h4>Number of components of the color space - for example, RGB color space has three components - red, green and blue, CMYK color space has four components - cyan, magenta, yellow and black.
<br /><a name="qguard" /><h4>Number of guard bits (Qguard)</h4>Pattern = "I" - Number of guard bits to prevent overflow in the magnitude bit-plane representation. Typical values are 1 or 2. Default is 1. 
<br /><a name="cuse_precints" /><h4>Precinct Size (Cuse_precints)</h4>(JPEG2000 Standard)
Precint is a partition in each resolution (formed in DWT domain). Power of 2 in size (lines up with code-block size boundary). It does not cause block (tiles) artifacts.
<br /><a name="corder" /><h4>Progression order (Corder)</h4>(JPEG2000 Standard)
The arithmetic encoding of the bit-planes is referred to as tier-1 coding, whereas the packetization of the compressed data and encoding of the packet header information is known as tier-2 coding. In order to change the sequence in which the packets appear in the code-stream, it is necessary to decode the packet header information, but it is not necessary to perform arithmetic decoding. This allows the code-stream to be recognized with minimal computational complexity.<br />
The order in which packets appear in the code-stream is called the progression order and is controlled by specific markers. Regardless of the ordering, it is necessary that the coding passes for each code-block appear in the code-stream in causal order from the most significant bit. For a given tile, four parameters are needed to uniquely identify a packet. These are component, resolution, layer, and position (precint).The packets for a particular component, resolution, and layer are generated by scanning the precints in a raster order. All the packets for a tile can be ordered by using nested "for loops” where each "for loop” varies one parameter from the above list. By changing the nesting order of the "for loops”, a number of different progression orders can be generated. - The JPEG 2000 Suite  
<br /> <br />
JPEG2000 standard allows five progression orders:<br />
Layer-resolution-component-position progressive<br />
Resolution-layer-component-position progressive<br />
Resolution-position-component-layer progressive<br />
Position-component-resolution-layer progressive<br />
Component-position-resolution-layer progressive<br />
The standard has the layer-resolution-component-position progressive order as the default order. 

<br /><a name="psnr" /><h4>PSNR (Peak Signal-to-Noise Ratio)</h4>The phrase peak signal-to-noise ratio, often abbreviated PSNR, is an engineering term for the ratio between the maximum possible power of a signal and the power of corrupting noise that affects the fidelity of its representation. Because many signals have a very wide dynamic range, PSNR is usually expressed in terms of the logarithmic decibel scale.<br />
The PSNR is most commonly used as a measure of quality of reconstruction of lossy compression codecs (e.g., for image compression). The signal in this case is the original data, and the noise is the error introduced by compression. When comparing compression codecs it is used as an approximation to human perception of reconstruction quality, therefore in some cases one reconstruction may appear to be closer to the original than another, even though it has a lower PSNR (a higher PSNR would normally indicate that the reconstruction is of higher quality). One has to be extremely careful with the range of validity of this metric; it is only conclusively valid when it is used to compare results from the same codec (or codec type) and same content.
Typical values for the PSNR in lossy image are between 30 and 50 dB, where higher is better. When the two images are identical, the PSNR is undefined.
<br /><a name="psnr_rgb" /><h4>PSNR (R/G/B) </h4>PSNR computed for the red/green/blue channel of the RGB color space.
<br /><a name="clayers" /><h4>Quality layers (Clayers)</h4>The coded data (bitstream) of each code-block is distributed across one or more layers in the code-stream. Each layer consists of some number consecutive bit-plane coding passes from each code-block in the tile, including all subbands of all components for that tile.
<br /><a name="resolution" /><h4>Resolution</h4>Image resolution in pixels per inch.
<br /><a name="sop" /><h4>SOP (Start Of Packet Header) (Cuse_sop)</h4>JPEG2000 Part 1 standard:
Start Of Packet is an In Bitstream Marker Segment. 
The bitstream marker segments are used for error resilience.
<br /><a name="mssim" /><h4>Structure Similarity Index - MSSIM</h4>The structural similarity (SSIM) index is a method for measuring the similarity between two images. The SSIM index is a full reference metric, in other words, the measuring of image quality based on an initial uncompressed or distortion-free image as reference. SSIM is designed to improve on traditional methods like peak signal-to-noise ratio (PSNR) and mean squared error (MSE), which have proved to be inconsistent with human eye perception.
<br /><a name="tiling" /><h4>Tiling</h4>The whole compression system (of the JPEG2000 Part 1) is simply divided into three phases. We call them (1) image preprocessing, (2) compression, and (3) compressed bitstream formation. Tiling is the first preprocessing operation. In this step, the input source image is (optionally) partitioned into a number of rectangular nonoverlapping blocks if the image is very large. Each of these blocks is called a tile. All the tiles have exactly the same dimension except the tiles at the image boundary if the dimension of the image is not an integer multiple of the dimension of the tiles. For an image with multiple components each tile also consists of these components. For a grayscale image, the tile has a single component. Since the tiles are compressed independently, visible artifacts may be created at the tile boundaries when it is heavily quantized for very-low-bit-rate compression as typical in any block transform coding. Obviously, no tiling offers the best visual quality. On the other hand, if the tile size is too large, it requires larger memory buffers for implementation either by software or hardware. The tile size 256 x 256 or 512 x 512 is found to be a typical choice for VLSI implementation based on the cost, area and power consideration
<br /><a name="ckernels" /><h4>Transformation (Ckernels)</h4>For lossy compression, the default wavelet filter used in the JPEG2000 standard is the Daubechies biorthogonal spline filter. By we indicate that the analysis filter is formed by 9-tap low-pass FIR (Finite Impulse Response) filter and 7-tap high-pass FIR filter. Both filters are symmetric. <br />
For lossless compression, the default wavelet filter used in the JPEG2000 standard is the Le Gall  spline filter. Although this is the default filter for lossess transformation, it can be applied in lossy compression as well. However, experimentally it has been observed that the filter produces better visual quality and compression efficiency in lossy mode than the filter.
<br /><a name="jp2_profiles" /><h4>Uploaded JP2 profiles</h4>Profiles in XML format for compliance check. JP2 files can be tested whether they conform to uploaded profiles.
<br /><a name="validate" /><h4>Validation</h4>Format validation is the process of determining the level of compliance of a digital object to the specification for its purported format, e.g.: "I have an object purportedly of format F; is it?"<br />
Format validation conformance is determined at three levels: well-formedness, validity, and consistency.
A digital object is well-formed if it meets the purely syntactic requirements for its format. 
An object is valid if it is well-formed and it meets the higher-level semantic requirements for format validity. 
An object is consistent if it is valid and its internally extracted representation information is consistent with externally supplied representation information.
<br /> <br />
<h4>References</h4>
<ol>
		<li>Wikipedia - <a href="http://en.wikipedia.org" target="_blank">en.wikipedia.org</a></li> 
<li>Acharya, T., Tsai, P.: JPEG2000 standard for image compression: concepts,
algorithms and VLSI architecture (John Wiley & Sons, Inc., Hoboken, New Jersey,
2005)</li>
<li>Schelkens, P., Skodras, A., Ebrahimi, T.: The JPEG 2000 Suite (John Wiley & Sons,
Ltd., Chichester, UK, 2009)</li>
</ol>		
		
<!-- ///////////////////////////////////////////////////////////// -->		
    </div>
