/**
* author : masfu hisyam
**/

#ifndef PHP_IMAGE_RETRIEVAL_H
#define PHP_IMAGE_RETRIEVAL_H

#define PHP_IMAGE_RETRIEVAL_VERSION "1.1.0"
#define PHP_IMAGE_RETRIEVAL_EXTNAME "image_retrieval"

extern "C" {
#include "php.h"
#include "zend_exceptions.h"

#ifdef ZTS
#include "TSRM.h"
#endif

}
extern zend_module_entry image_retrieval_module_entry;
#define phpext_opencv_ptr &opencv_module_entry

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/objdetect/objdetect.hpp"
#include <cmath>
#include <iostream>


using namespace cv;
using namespace std;

PHP_MINFO_FUNCTION(image_retrieval);

PHP_FUNCTION(ir_shape_detect);
PHP_FUNCTION(ir_histogram);
PHP_FUNCTION(ir_facedetect);
#ifdef ZTS
#define OPENCV_G(v) TSRMG(opencv_globals_id, zend_opencv_globals *, v)
#else
#define OPENCV_G(v) (opencv_globals.v)
#endif
#endif

