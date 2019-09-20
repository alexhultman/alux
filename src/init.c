#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/reboot.h>
#include <linux/reboot.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>

/* For mouse input */
#include <linux/input.h>
#include <linux/input-event-codes.h>

#include <string.h>

int width = 1920;
int height = 1080;

int mouse_x = 0;
int mouse_y = 0;

char *framebuffer;

char *wallpaper;

#include <png.h>


// return the image
void load() {


int width, height;
png_byte color_type;
png_byte bit_depth;
png_bytep *row_pointers = NULL;



  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  //if(!png) abort();

  png_infop info = png_create_info_struct(png);
  //if(!info) abort();

  //if(setjmp(png_jmpbuf(png))) abort();


  char *filename = "/wallpaper.png";
  FILE *fp = fopen(filename, "rb");
  png_init_io(png, fp);

  png_read_info(png, info);

  width      = png_get_image_width(png, info);
  height     = png_get_image_height(png, info);
  color_type = png_get_color_type(png, info);
  bit_depth  = png_get_bit_depth(png, info);

  printf("bit_depth = %d\ncolor: %d\nwidth %d\nheight %d\n", bit_depth, color_type, width, height);

  // Read any color_type into 8bit depth, RGBA format.
  // See http://www.libpng.org/pub/png/libpng-manual.txt

  if(bit_depth == 16)
    png_set_strip_16(png);

  if(color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(png);

  // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
  if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand_gray_1_2_4_to_8(png);

  if(png_get_valid(png, info, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png);

  // These color_type don't have an alpha channel then fill it with 0xff.
  if(color_type == PNG_COLOR_TYPE_RGB ||
     color_type == PNG_COLOR_TYPE_GRAY ||
     color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

  if(color_type == PNG_COLOR_TYPE_GRAY ||
     color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png);

  png_read_update_info(png, info);


  row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
  for(int y = 0; y < height; y++) {
    row_pointers[y] = (png_byte*) &wallpaper[width * y * 4];//malloc(png_get_rowbytes(png,info));
  }


  png_read_image(png, row_pointers);

  fclose(fp);

  png_destroy_read_struct(&png, &info, NULL);


  // invert rbga to agbr

  for (int i = 0; i < width * height; i++) {
	  char r = wallpaper[4 * i];
	  char g = wallpaper[4 * i + 1];
	  char b = wallpaper[4 * i + 2];
	  char a = wallpaper[4 * i + 3];

	  wallpaper[4 * i] = b;
	  wallpaper[4 * i + 1] = g;
	  wallpaper[4 * i + 2] = r;
	  wallpaper[4 * i + 3] = 255;
  }

}

// cat /sys/class/graphics/fb0/virtual_size

/* Render full frame with mouse cursor */
void render(int fbfd) {

	/* 4 bytes per pixel */

	char blue[4] = {255, 0, 0, 255};

	char black[4] = {0, 255, 255, 255};

	lseek(fbfd, 0, SEEK_SET);


	memcpy(framebuffer, wallpaper, width * height * 4);


	for (int x = 0; x < width; x++) {
		memcpy(&framebuffer[(mouse_y * width + x) * 4], blue, 4);
	}

	for (int y = 0; y < height; y++) {
		memcpy(&framebuffer[(y * width + mouse_x) * 4], blue, 4);
	}


	// 32 bits per pixel, uppifrån vänster rad för rad ner

	//memcpy(&framebuffer[0], blue, 4);

	/*for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {


			/* Draw mouse cursor as a shader */



			/*if (mouse_x == x || mouse_y == y) {
				memcpy(&framebuffer[(y * width + x) * 4], blue, 4);
			} else {
				memcpy(&framebuffer[(y * width + x) * 4], black, 4);
			}*/
			/*
		}
	}*/

	write(fbfd, framebuffer, width * height * 4);
}

int main() {

	/* Allocate framebuffer */
	framebuffer = malloc(width * height * 4);

	wallpaper = malloc(width *height * 4);

	load();

	//return 0;

	/* Open framebuffer, mouse, terminal */
	int fbfd = open("/dev/fb0", O_RDWR);
	int msfd = open("/dev/input/event5", O_RDWR | O_NONBLOCK);
	int ttyfd = open("/dev/tty0", O_RDWR);



	/* Render once */
	render(fbfd);

	/* Wait for mouse input */
	while (1) {

		/* Select on fds */
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(msfd, &rfds);

		struct timeval tv;
    	tv.tv_sec = 5;
        tv.tv_usec = 0;

		//dprintf(ttyfd, "Polling for read\n");
		//select(1, &rfds, NULL, NULL, &tv);

		//dprintf(ttyfd, "Done Polling for read\n");

		while (1) {
			struct input_event mouse_event;
			int last_read = read(msfd, &mouse_event, sizeof(struct input_event));

			if (last_read == -1) {
				break;
			}

			if (EV_REL == mouse_event.type) {
				if (REL_X == mouse_event.code) {
					mouse_x += mouse_event.value;
				} else if (REL_Y == mouse_event.code) {
					mouse_y += mouse_event.value;
				}

				/* Cap mouse x */
				if (mouse_x < 0) {
					mouse_x = 0;
				} else if (mouse_x >= width) {
					mouse_x = width - 1;
				}

				/* Cap mouse y */
				if (mouse_y < 0) {
					mouse_y = 0;
				} else if (mouse_y >= height) {
					mouse_y = height - 1;
				}
			}
		}

		/* Only render once we emptied all events */
		render(fbfd);
	}
}
