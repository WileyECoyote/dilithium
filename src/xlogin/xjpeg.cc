/** xjpeg.cc
 *
 * @par A jpeg decoding utilitity for X Clients
 * 
 * (c) 2008 Jonathan Andrews
 *
 * Date: "14 Sep 2008\0";
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110
 * -1301 USA
 *
 */
/*!   @file    xjpeg.cc
 *    @brief   C++ Source file JPEG Decoder for X clients
 *    @author  Jonathan Andrews
 *    @date    2008.09.14
 *
 *    @note    Modified from of xlogin-rootjpeg.c, by Jonathan Andrews.
 *             Modifications made by Wiley Hill for user as module.
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include "jpeglib.h"         /* The order matters here  */
#include <jerror.h>
#include <signal.h>
#include <setjmp.h>
#include <errno.h>
#include <string.h>

/* X11 Stuff */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

/*! \brief Declare anonymous function to store either 16 or 32 bit data */
void (*store_data) (int w, int x, int y, int r, int g, int b);

void convert_for_16 ();
void convert_for_32 ();

unsigned short int *buffer_16bpp;
long *buffer_32bpp;

void convert_for_16 (int w, int x, int y, int r, int g, int b)
{
 buffer_16bpp[y * w + x] = ((r >> 3) << 11) + ((g >> 2) << 5) + (b >> 3);
}

void convert_for_32 (int w, int x, int y, int r, int g, int b)
{
 buffer_32bpp[y * w + x] = ((r << 16) + (g << 8) + b) & 0xFFFFFFFF;
}

struct jpg_error_mgr
{
 struct jpeg_error_mgr pub;
 sigjmp_buf setjmp_buffer;
};
typedef struct jpg_error_mgr * jpg_error_ptr;

/*! \brief jpeg decode error handler
 *  \par Function Description
 *  This function write an error message and then returns control
 *  to the address specified in the jpg_error_mgr structure.
 *
 */
void decodejpeg_error_exit (j_common_ptr cinfo)
{
 jpg_error_ptr jerr = (jpg_error_ptr) cinfo->err;

 fprintf (stderr, "decode_jpeg: error - ");
 fflush(stderr);
 (*cinfo->err->output_message) (cinfo);
 fflush(stderr);

 /* Return to address marked by setjmp */
 longjmp(jerr->setjmp_buffer,1);
}

/*! \brief jpeg_decode
 *  \par Function Description
 *  This function set up and error handler for the decode and opens
 *  the jpeg file and loads and decompresses the pixel data.
 *
 * \retval XImage pointer to an image decoded buffer. If an error
 *         occured then the image is a dummy image.
 */
XImage* jpeg_decode (const char filename[], Display *display, int xdepth,
                     int scalenum, int scaledenom, int *didxcreate, int *sucess)
{
   FILE *infile;
   struct jpeg_decompress_struct cinfo;
   struct jpg_error_mgr jerr;
   JSAMPARRAY buffer;
   int row_stride;
   int g, i, a;
   int bpix;
   int width, height;
   XImage *xim;         /* Uninitialised pointer to an Ximage strcuture  */

   *sucess=false;       /* Set sucess, passed by pointer, so reference with  */
   *didxcreate=false;

   if ((infile = fopen (filename, "rb")) == NULL)
   {
      fprintf (stderr,"\ndecode_jpeg was unable to open the file: [%s]\n", filename);
      fflush (stderr);
      return NULL;
   }

   cinfo.err = jpeg_std_error (&jerr.pub);
   jerr.pub.error_exit = decodejpeg_error_exit;

   if (sigsetjmp(jerr.setjmp_buffer,1))
   {
      /* Whoops there was a jpeg error */
      fclose (infile);
      jpeg_destroy_decompress (&cinfo);

      *sucess=false;
      fprintf(stderr, "decode_jpeg: error handler exits here, didxcreate=%d\n", *didxcreate);
   return(xim);         /* Return or dummy image  */
   }

   /* It is possible to generate an error setting up for decompress if the
    * file is zero bytes, or has been removed just before we arrive here */
   jpeg_create_decompress (&cinfo);
   jpeg_stdio_src (&cinfo, infile);
   jpeg_read_header (&cinfo, false);
   cinfo.scale_num=scalenum;
   cinfo.scale_denom=scaledenom;
   cinfo.do_fancy_upsampling = false;
   cinfo.do_block_smoothing = false;
   jpeg_start_decompress (&cinfo);

   /* When the jpeg routines get a garbage header they leave width and
    * height containing any old shit, often a very large number */
   if ((cinfo.output_width>1280)|(cinfo.output_height>1280))
   {
      /* Set dummy values -
       * the code is comitted to allocating buffers and create XImages */
      cinfo.output_width=320;
      cinfo.output_height==256;
   }
   width = cinfo.output_width;
   height = cinfo.output_height;

   if (cinfo.output_components>4)
   {
      cinfo.output_components=4;
   }

   if (xdepth == 16)
   {
      /* The store_data function points to the function handler convert_for_16.
       * This is a very fast way to ensure that one of the 3 buffer conversion
       * routines is called without stack pushes for function arguments  */
      store_data = &convert_for_16;
      buffer_16bpp = (unsigned short int *) malloc (width * height * 2);

      /* XCreateImage allocates the memory for the xim strcuture */
      xim=XCreateImage (display, CopyFromParent, xdepth, ZPixmap, 0,
                        (char *) buffer_16bpp, width, height, 16, width * 2);
      *didxcreate=true;
   }
   else
   {
      if (xdepth == 24)
      {
         store_data = &convert_for_32;

         /* Alocate memory for 32 bit image image */
         buffer_32bpp = (long*) malloc (width * height * 4);
         xim=XCreateImage (display, CopyFromParent, xdepth, ZPixmap, 0,
                           (char *) buffer_32bpp, width, height, 32, width * 4);
         *didxcreate=true;
      }
      else
      {
         if (xdepth == 32)
         {
            store_data = &convert_for_32;
            buffer_32bpp = (long*) malloc (width * height * 4);
            xim=XCreateImage (display, CopyFromParent, xdepth, ZPixmap, 0,
                              (char *) buffer_32bpp, width, height, 32, width * 4);
            *didxcreate=true;
         }
         else
         {
            fprintf (stderr, "decode_jpeg: Unsupported depth.%d\n",xdepth);
            fclose(infile);
            return NULL;
         }
      }
   }

   g = 0;

   row_stride = cinfo.output_width * cinfo.output_components;

   buffer = (*cinfo.mem->alloc_sarray)  ((j_common_ptr) & cinfo, JPOOL_IMAGE, row_stride, 1);

   bpix = cinfo.output_components;

   while (cinfo.output_scanline < cinfo.output_height)
   {
      jpeg_read_scanlines (&cinfo, buffer, 1);
      a = 0;
      for (i = 0; i < bpix * cinfo.output_width; i += bpix)
      {
         (*store_data) (width, a, g, buffer[0][i], buffer[0][i + 1], buffer[0][i + 2]);
         a++;
      }
      g++;
   }
   jpeg_finish_decompress (&cinfo);
   jpeg_destroy_decompress (&cinfo);
   fclose (infile);

   *sucess=true;
   return(xim);   /* Return pointer to structure now alloced by XCreateImage  */
}


