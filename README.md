# Image Processor Project

This project demonstrates the use of multi-threading in C to process images by applying color transformations (grayscale, red, green, blue), rotating the image, and upscaling the image resolution to 1080p. The program compares the performance of single-threaded and multi-threaded implementations, highlighting the benefits of multi-threading in image processing.

## Features
- Convert an image to Grayscale, Red, Green, or Blue.
- Rotate the image by 90 degrees (optional).
- Upscale the image to 1080p (optional).
- Compare single-threaded vs. multi-threaded processing with execution time and thread utilization.

## Requirements
- GCC
- Pthread library
- stb_image.h and stb_image_write.h

## Installation
Clone the repository:
```bash
git clone https://github.com/aryadityad/image_processor_project.git
cd image_processor_project
