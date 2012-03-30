#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <iterator>
#include <getopt.h>
// header files for kakadu
#ifdef USE_KAKADU
#include <jpx.h>
#include <jp2.h>
#include <kdu_stripe_decompressor.h>
#endif
// header files for libtiff
#include <tiffio.h>
// header files for mhash
#include <mhash.h>
// header files for libjpeg
#include <jpeglib.h>
// header files for djvu
// #include "config.h"
//#include "GContainer.h"
//#include "ByteStream.h"
//#include "ByteStream.cpp"

// #include <>
// #include <libdjvu/ddjvuapi.h>
// #include <libdjvu/miniexp.h>

#include "xmlwriter.h"

using namespace std;

struct rgb_t {
   unsigned long long r; // was uint32 
   unsigned long long g;
   unsigned long long b;
   public:
      rgb_t():r(0),g(0),b(0) {};
};

class image_reader_t {
   public:
      virtual int get_height() = 0 ;
      virtual int get_width() = 0;
      virtual string get_format() = 0;
      virtual uint32* read_line() = 0;
      int get_size() { return (this->get_height() * this->get_width()); } 
};

/***********************
 *     TIFF reader     *
 ***********************/

class tiff_reader_t : public image_reader_t {
   public:
      tiff_reader_t(string filename);
      int get_height() { return height; }
      int get_width() { return width; }
      string get_format() { return "TIFF"; }
      uint32* read_line();
   protected:
      int height;
      int width;
      int line;
      uint32* raster;
};

tiff_reader_t::tiff_reader_t(string filename):line(0) {
   TIFF* tif = TIFFOpen(filename.c_str(), "r");
   if (tif != NULL) {
     TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &this->width);
     TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &this->height);
     int size = this->width * this->height;
#if DEBUG
     std::cout << "size: " << this->width << "x" << this->height << endl;
#endif
     this->raster = (uint32*) _TIFFmalloc(size * sizeof (uint32));
     if (raster != NULL) {
        if (!TIFFReadRGBAImageOriented(tif, this->width, this->height, this->raster, ORIENTATION_TOPLEFT, 0)) {
           _TIFFfree(raster);
           raster = NULL;
        }
     }
   }
   TIFFClose(tif);
}

uint32* tiff_reader_t::read_line() {
   if (this->line >= this->height) {
      return NULL;
   }
   uint32* ref = raster;
   ref += (this->line * this->width);
   line++;
   return ref;
}

/***********************
*      MD5 Hash        *
************************/
class md5_hash_t {
   protected:
      MHASH td;
   public:
      md5_hash_t();
      void add(unsigned char* buffer, int len);
      string finish();
};

md5_hash_t::md5_hash_t() {
   td = mhash_init(MHASH_MD5);
}

void md5_hash_t::add(unsigned char* buffer, int len) {
   mhash(td, buffer, len);
}

string md5_hash_t::finish() {
   unsigned char* hash = (unsigned char*) mhash_end(td);
   char* result = new char[64];
   for (int i = 0; i < mhash_get_block_size(MHASH_MD5); i++) {
      sprintf(&result[i*2], "%.2x", hash[i]);
   }
   result[(mhash_get_block_size(MHASH_MD5)*2) + 1] = 0;
   return string(result);
}

/***********************
 *     DJVU reader     *
 ***********************/

class djvu_reader_t : public image_reader_t {
   public:
      djvu_reader_t(string filename);
      int get_height() { return reader->get_height(); }
      int get_width() { return reader->get_width(); }
      string get_format() { return "DJVU"; }
      uint32* read_line() { return reader->read_line(); }
   protected:
      tiff_reader_t* reader;
};

djvu_reader_t::djvu_reader_t(string input):reader(0) {
   char* output = tmpnam(NULL);
   int child = fork();
   if (child < 0) {
      std::cout << "Failed to fork GUI process...Exiting" << std::endl;
      exit(-1);
   } else if (child == 0) {
     const char* argv[] = { const_cast<char*>(""), const_cast<char*>("-format=tiff"), const_cast<char*>("-page"), const_cast<char*>("1"), input.c_str(), output, NULL };
     execvp ("/usr/local/bin/ddjvu", const_cast<char**>(argv));
   } else {
     int status;
     waitpid(child, &status, 0);
     reader = new tiff_reader_t(output);
   }
}



#ifdef USE_KAKADU
/***********************
 *   JPEG2000 reader   *
 ***********************/

class jp2_reader_t : public image_reader_t {
   public:
      jp2_reader_t(string filename);
      int get_height() { return height; }
      int get_width() { return width; }
      string get_format() { return "JPEG2000"; }
      uint32* read_line();
   protected:
      int height;
      int width;
      int line;
      // kdu_int16 **stripe_bufs;
      kdu_byte **stripe_bufs;
      kdu_byte* line_buffer;
};

jp2_reader_t::jp2_reader_t(string filename):line(0) {
   jp2_family_src src;
   jp2_source jp2_in;
   src.open(filename.c_str());
   if (!jp2_in.open(&src)) {
      std::cout << "error opening JPEG2000 file" << endl;
   }
   jp2_in.read_header();
   kdu_codestream codestream;
   codestream.create(&jp2_in);
   int num_components = codestream.get_num_components(true);
#if DEBUG
   std::cout << "number of components: " << num_components << endl;
#endif
   kdu_dims *comp_dims = new kdu_dims[num_components];
   for (int n=0; n < num_components; n++) {
      codestream.get_dims(n, comp_dims[n], true);
      this->width = comp_dims[n].access_size()->get_x();
      this->height = comp_dims[n].access_size()->get_y();
#if DEBUG
      std::cout << "size: " << this->width << "x" << this->height << endl;
#endif
   }
   int *stripe_heights = new int[num_components];
   int *max_stripe_heights = new int[num_components];
   int preferred_min_stripe_height, absolute_max_stripe_height;
   // threading
   int num_threads = 4;
   kdu_thread_env env, *env_ref = NULL;
   if(num_threads > 0 ){
      env.create();
      for (int nt=0; nt < num_threads; nt++){
         // Unable to create all the threads requested
         if( !env.add_thread() ) num_threads = nt;
      }
    env_ref = &env;
  }
   //
   //kdu_int16 **stripe_bufs = new kdu_int16 *[num_components];
   // this->stripe_bufs = new kdu_int16 *[num_components];
   this->stripe_bufs = new kdu_byte *[num_components];
   kdu_stripe_decompressor decompressor;
   // decompressor.start(codestream, false, false, env_ref, NULL, env_dbuf_height);
   decompressor.start(codestream, false, false, env_ref, NULL, NULL);
   /*decompressor.get_recommended_stripe_heights(preferred_min_stripe_height,
                                               absolute_max_stripe_height,
                                               stripe_heights,
                                               max_stripe_heights);*/
   for (int n = 0; n < num_components; n++) {
      stripe_heights[n] = comp_dims[n].size.y;
      // stripe_bufs[n] = new kdu_int16[comp_dims[n].size.x*comp_dims[n].size.y];
      stripe_bufs[n] = new kdu_byte[comp_dims[n].size.x*comp_dims[n].size.y];
   }
   // decompressor.pull_stripe(stripe_bufs,stripe_heights,NULL,NULL, precisions);
   decompressor.pull_stripe(stripe_bufs, stripe_heights, NULL, NULL, NULL);
#if DEBUG
   std::cout << "done reading JP2 file" << endl;
#endif
   line_buffer = new kdu_byte[this->width * 4];
}

uint32* jp2_reader_t::read_line() {
   if (this->line >= this->height) {
      return NULL;
   }
   for (int i = 0; i!= width; i++) {
      int offset = i + (line * this->width);
      int index = i * 4;
      line_buffer[index + 0] = stripe_bufs[0][offset];
      line_buffer[index + 1] = stripe_bufs[1][offset];
      line_buffer[index + 2] = stripe_bufs[2][offset];
      line_buffer[index + 3] = 0;
   }
   line++;
   return (uint32*) line_buffer;
}
#endif

/**********************
*    JPEG reader      *
***********************/
class jpeg_reader_t : public image_reader_t {
   public:
      jpeg_reader_t(string filename);
      int get_height() { return height; }
      int get_width() { return width; }
      string get_format() { return "JPEG"; }
      uint32* read_line();
   protected:
      int height;
      int width;
      int line;
      //unsigned char *raw_image;
      JSAMPROW row_pointer[1];
      struct jpeg_decompress_struct cinfo;
      struct jpeg_error_mgr jerr;
      unsigned long location;
      unsigned char* line_buffer;
};

jpeg_reader_t::jpeg_reader_t(string filename):line(0),location(0) {
   FILE *infile = fopen(filename.c_str(), "rb");
   if (!infile) {
      printf("Error opening jpeg file %s\n!", filename.c_str());
   }
   /* here we set up the standard libjpeg error handler */
   this->cinfo.err = jpeg_std_error(&jerr);
   /* setup decompression process and source, then read JPEG header */
   jpeg_create_decompress(&this->cinfo);
   /* this makes the library read from infile */
   jpeg_stdio_src(&cinfo, infile);
   /* reading the image header which contains image information */
   jpeg_read_header(&cinfo, TRUE);
   /* Start decompression jpeg here */
   jpeg_start_decompress(&cinfo);
   /* allocate memory to hold the uncompressed image */
   //this->raw_image = (unsigned char*) malloc(cinfo.output_width * cinfo.output_height * cinfo.num_components);
   /* now actually read the jpeg into the raw buffer */
   this->line_buffer = new unsigned char[cinfo.output_width * 4];
   this->row_pointer[0] = (unsigned char *) malloc(cinfo.output_width*cinfo.num_components);
   this->width = cinfo.image_width;
   this->height = cinfo.image_height;
   /* wrap up decompression, destroy objects, free pointers and close open files */
   //jpeg_finish_decompress(&cinfo);
   //jpeg_destroy_decompress(&cinfo);
   //free(row_pointer[0]);
   //fclose(infile);
}

uint32* jpeg_reader_t::read_line() {
   /* read one scan line at a time */
   //cout << this->width << ":" << this->height << endl;
   if (cinfo.output_scanline < cinfo.image_height ) {
      jpeg_read_scanlines(&cinfo, row_pointer, 1);
      int n = 0;
      for (int i=0; i<cinfo.image_width*cinfo.num_components; i++) {
         this->line_buffer[n] = (unsigned char) row_pointer[0][i];
         n++;
         if ((n % 4) == 3) {
            n++;
         }
      }
      return (uint32*) line_buffer;
   } else {
      return NULL;
   }
}

/**********************
 *   NULL reader      *
 **********************/
class null_reader_t : public image_reader_t {
   public:
      null_reader_t(string filename);
      int get_height() { return height; }
      int get_width() { return width; }
      string get_format() { return "NULL"; }
      uint32* read_line();
   protected:
      static const int height = 4096;
      static const int width = 4096;
      uint32* line_buffer;
};

null_reader_t::null_reader_t(string filename) {
   line_buffer = new uint32[width];
   for (int i = 0; i!= width; i++) {
      line_buffer[i] = (uint32) 0;
   }
}

uint32* null_reader_t::read_line() {
   return line_buffer;
}

/**********************
*     DJVU reader     *
***********************/
#if 0
class djvu_reader_t : public image_reader_t {
   public:
      djvu_reader_t(string filename);
      int get_height() { return height; }
      int get_width() { return width; }
      uint32* read_line(){}
   protected:
      static const int height = 4096;
      static const int width = 4096;
      uint32* line_buffer;
};

djvu_reader_t::djvu_reader_t(string filename) {
   //using namespace DJVU;
   //ByteStream* input = new ByteStream::Stdio(filename);
   // ByteStream* input = new StdioByteStream(filename);
   StdioByteStream bs(filename, "wb");
}
#endif

/**********************
*   PPM writer        *
***********************/
class ppm_writer_t {
   public:
      ppm_writer_t(ostream *os, int wid, int hei, int depth);
      void write_line(uint32* line);
   protected:
      void write_header();
   private:
      ostream* output;
      int height;
      int width;
      int bit_depth;
};

ppm_writer_t::ppm_writer_t(ostream *os, int wid, int hei, int depth):output(os),height(hei),width(wid),bit_depth(depth) {
   this->write_header();
}

void ppm_writer_t::write_header() {
   *output << "P3" << endl;
   *output << width << " " << height << endl;
   *output << bit_depth << endl;
}

void ppm_writer_t::write_line(uint32* line) {
   string sep("");
   for (uint32 i = 0; i!= this->width; i++) {
      uint8* pix = (uint8*) &line[i];
      *output << sep << (int) pix[0] << " " << (int) pix[1] << " " << (int) pix[2];
      sep = " "; 
   }
   *output << endl;
}

/**********************
*                     *
***********************/

int sqr(int a) {
   return a*a;
}

#if 0
int main(int argc, char* argv[]) {
   string file1(argv[1]);
   image_reader_t* img = new jpeg_reader_t(file1);
   ofstream os(argv[2]);
   ppm_writer_t image_writer(&os, img->get_width(), img->get_height(), 256);
   uint32* line;
   while ((line = img->read_line()) != NULL) {
      image_writer.write_line(line);
   }
}
#endif

string JP2_SUFFIX = ".jp2";
string TIFF_SUFFIX = ".tif";
string JPEG_SUFFIX = ".jpg";
string JPX_SUFFIX = ".jpx";
string JPF_SUFFIX = ".jpf";
string DJVU_SUFFIX = ".djvu";

image_reader_t *open_image(string filename) {
   string to_lower(filename);
   std::transform(to_lower.begin(), to_lower.end(), to_lower.begin(), ::tolower);
   if (std::equal(TIFF_SUFFIX.rbegin(), TIFF_SUFFIX.rend(), to_lower.rbegin())) {
      return new tiff_reader_t(filename);
#ifdef USE_KAKADU
   } else if (std::equal(JP2_SUFFIX.rbegin(), JP2_SUFFIX.rend(), to_lower.rbegin())) {
      return new jp2_reader_t(filename);
   } else if (std::equal(JPX_SUFFIX.rbegin(), JPX_SUFFIX.rend(), to_lower.rbegin())) {
      return new jp2_reader_t(filename);
   } else if (std::equal(JPF_SUFFIX.rbegin(), JPF_SUFFIX.rend(), to_lower.rbegin())) {
      return new jp2_reader_t(filename);
#endif
   } else if (std::equal(JPEG_SUFFIX.rbegin(), JPEG_SUFFIX.rend(), to_lower.rbegin())) {
      return new jpeg_reader_t(filename);
   } else if (std::equal(DJVU_SUFFIX.rbegin(), DJVU_SUFFIX.rend(), to_lower.rbegin())) {
      return new djvu_reader_t(filename);
   } else if (filename == "null") {
      return new null_reader_t("null");
   } else {
      return NULL;
   }
}

string int2str(int i) {
  std::stringstream ss;
  ss << i;
  return ss.str();
}

string double2str(double d) {
  std::stringstream ss;
  ss << d;
  return ss.str();
}

struct arguments_t {
   ostream* image1_out;
   string image1_out_str;
   ostream* image2_out;
   string image2_out_str;
   arguments_t():image1_out(0),image2_out(0){}
};

double getcputime() {
   struct timeval tim;
   struct rusage ru;
   getrusage(RUSAGE_SELF, &ru);
   tim=ru.ru_utime;
   double t=(double)tim.tv_sec + (double)tim.tv_usec / 1000000.0;
   tim=ru.ru_stime;
   t+=(double)tim.tv_sec + (double)tim.tv_usec / 1000000.0; 
   return t;
}

#if 1
int main(int argc, char* argv[]) {
   int c;
   int digit_optind = 0;
   arguments_t arguments;
   while (1) {
      int this_option_optind = optind ? optind : 1;
      int option_index = 0;
      static struct option long_options[] = {
         {"image1", 0, 0, 0},
         {"image2", 0, 0, 0},
         {0, 0, 0, 0}
      };
      c = getopt_long (argc, argv, "1:2:", long_options, &option_index);
      switch (c) {
         case '1':
            arguments.image1_out_str = string(optarg);
            arguments.image1_out = new ofstream(optarg);
            break;
         case '2':
            arguments.image2_out_str = string(optarg);
            arguments.image2_out = new ofstream(optarg);
            break;
      }
      if (c == -1) break;
   }
   //if (optind < argc) {
   //}
   //string file1(argv[1]);
   //string file2(argv[2]);
   string file1(argv[optind]);
   string file2("null");
   if (argv[optind+1] != NULL) {
      file2 = argv[optind+1];
   }
   md5_hash_t* hash1 = new md5_hash_t();
   md5_hash_t* hash2 = new md5_hash_t() ;
   image_reader_t* img1 = open_image(file1);
   image_reader_t* img2 = open_image(file2);
   uint32* line1;
   uint32* line2;
   rgb_t diff, rel;
   // difference
   //ofstream os(argv[3]);
   ofstream os(argv[optind+2]);
   ppm_writer_t image_writer(&os, img1->get_width(), img1->get_height(), 256);
   ppm_writer_t* image_writer1 = 0;
   ppm_writer_t* image_writer2 = 0;
   if (arguments.image1_out != 0) {
      image_writer1 = new ppm_writer_t(arguments.image1_out, img1->get_width(), img1->get_height(), 256);
   }
   if (arguments.image2_out != 0) {
      image_writer2 = new ppm_writer_t(arguments.image2_out, img2->get_width(), img2->get_height(), 256);
   }
   uint32* diff_line = new uint32[img1->get_width()];
   rgb_t* histogram = new rgb_t[256];
   rgb_t* histogram1 = new rgb_t[256];
   rgb_t* histogram2 = new rgb_t[256];
   xml_writer_t* writer = new xml_writer_t(&cout);
   writer->xsl_instruction("test.xsl");
   writer->push("compare");
   if (file2 != "null" && img1->get_width() != img2->get_width() && img1->get_height() != img2->get_height()) {
      writer->push("error", "image dimensions are not equal!")->pop();
      writer->pop();
      return 10;
   }
   while ((line1 = img1->read_line()) != NULL) {
      line2 = img2->read_line();
      hash1->add((unsigned char*) line1, img1->get_width() * 4);
      hash2->add((unsigned char*) line2, img2->get_width() * 4);
      for (uint32 i = 0; i!= img1->get_width(); i++) {
         //uint8* pix1 = (uint8*) &line1[i];
         //uint8* pix2 = (uint8*) &line2[i];
         unsigned char* pix1 = (unsigned char*) &line1[i];
         unsigned char* pix2 = (unsigned char*) &line2[i];
         rel.r+= (pix1[0] != pix2[0]);
         rel.g+= (pix1[1] != pix2[1]);
         rel.b+= (pix1[2] != pix2[2]);
         diff.r+= sqr(pix1[0] - pix2[0]);
         diff.g+= sqr(pix1[1] - pix2[1]);
         diff.b+= sqr(pix1[2] - pix2[2]);
         uint8* diff_pixel = (uint8*) &(diff_line[i]);
         for (int n = 0; n!= 3; n++) {
            // diff_line[n] = abs(pix1[n] - pix2[n]);
            diff_pixel[n] = abs(pix1[n] - pix2[n]);
         }
         histogram[abs(pix1[0] - pix2[0])].r++;
         histogram[abs(pix1[1] - pix2[1])].g++;
         histogram[abs(pix1[2] - pix2[2])].b++;
         if (histogram1 != NULL) {
            histogram1[pix1[0]].r++;
            histogram1[pix1[1]].g++;
            histogram1[pix1[2]].b++;
         }
         if (histogram2 != NULL) {
            histogram2[pix2[0]].r++;
            histogram2[pix2[1]].g++;
            histogram2[pix2[2]].b++;
         }
      }
      image_writer.write_line(diff_line);
      if (image_writer1 != 0) {
         image_writer1->write_line(line1);
      }
      if (image_writer2 != 0) {
         image_writer2->write_line(line2);
      }
   }
   int size = img1->get_size();
   double r = (1.0/size) * diff.r;
   double g = (1.0/size) * diff.g;
   double b = (1.0/size) * diff.b;
   double psnr_r = 10*log10((255*255)/r);
   double psnr_g = 10*log10((255*255)/g);
   double psnr_b = 10*log10((255*255)/b);
   // average psnr
   vector<double> psnr_vec;
   if (psnr_r != INFINITY) { psnr_vec.push_back(psnr_r); }
   if (psnr_g != INFINITY) { psnr_vec.push_back(psnr_g); }
   if (psnr_b != INFINITY) { psnr_vec.push_back(psnr_b); }
   double avg = std::accumulate(psnr_vec.begin(), psnr_vec.end(), 0.0 ) / ((psnr_vec.size()) * 1.0);
   //double avg = (psnr_r + psnr_g + psnr_b) / 3.0;
   writer->push("image");
   writer->push("width", int2str(img1->get_width()))->pop();
   writer->push("height", int2str(img1->get_height()))->pop();
   writer->push("format", img1->get_format())->pop();
   writer->pop();
   string psnrs[][2] = { {"avg", double2str(avg) }, { "r", double2str(psnr_r) },
          { "g", double2str(psnr_g) }, { "b", double2str(psnr_b) } };
   writer->push("psnr", psnrs, 3)->pop();
   writer->push("histogram");
   for (int i = 0; i !=256; i++) {
      string levels[][2] = { { "level", int2str(i)} , { "r", int2str(histogram[i].r) },
          { "g", int2str(histogram[i].g) }, { "b", int2str(histogram[i].b) } };
      writer->push("rgb", levels, 3)->pop();
   }
   writer->pop();
   //
   if (histogram1 != NULL) {
      writer->push("histogram1");
      for (int i = 0; i !=256; i++) {
         string levels[][2] = { { "level", int2str(i)} , { "r", int2str(histogram1[i].r) },
             { "g", int2str(histogram1[i].g) }, { "b", int2str(histogram1[i].b) } };
         writer->push("rgb", levels, 3)->pop();
      }
      writer->pop();
   }
   if (histogram2 != NULL) {
      writer->push("histogram2");
      for (int i = 0; i !=256; i++) {
         string levels[][2] = { { "level", int2str(i)} , { "r", int2str(histogram2[i].r) },
             { "g", int2str(histogram2[i].g) }, { "b", int2str(histogram2[i].b) } };
         writer->push("rgb", levels, 3)->pop();
      }
      writer->pop();
   }
   //
   writer->push("image1");
   writer->push("checksum", hash1->finish())->pop();
   if (image_writer1 != 0) {
      writer->push("preview", arguments.image1_out_str)->pop();
      delete image_writer1;
   }
   writer->push("width", int2str(img1->get_width()))->pop();
   writer->push("height", int2str(img1->get_height()))->pop();
   writer->push("format", img1->get_format())->pop();
   writer->pop();
   writer->push("image2");
   writer->push("checksum", hash2->finish())->pop();
   if (image_writer2 != 0) {
      writer->push("preview", arguments.image2_out_str)->pop();
      delete image_writer2;
   }
   writer->push("width", int2str(img2->get_width()))->pop();
   writer->push("height", int2str(img2->get_height()))->pop();
   writer->push("format", img2->get_format())->pop();
   writer->pop();
   if (arguments.image1_out != 0) {
      delete arguments.image1_out;
   }
   if (arguments.image2_out != 0) {
      delete arguments.image2_out;
   }
   writer->push("cputime", double2str(getcputime()))->pop();
   writer->pop();
   // hash1.finish();
   /*
   printf("%d %d %d\n", rel.r, rel.g, rel.b);
   printf("%d %d %d\n", diff.r, diff.g, diff.b);
   printf("%f %f %f\n", r, g, b);
   printf("%f %f %f\n", psnr_r, psnr_g, psnr_b);
   printf("%f\n", avg);
   cout << "abs r g b" << endl;
   for (int i = 0; i !=256; i++) {
       cout << i << " " << ((histogram[i].r*100.0)/size) << " "
            << ((histogram[i].g*100.0)/size) << " "
            << ((histogram[i].b*100.0)/size) << endl;
   }
   */
   return 0;
}
#endif
