/**
 * author : masfu hisyam
 * Simple shape detector program.
 * It loads an image and tries to find simple shapes (rectangle, triangle, circle, etc) in it.
 * This program is a modified version of `squares.cpp` found in the OpenCV sample dir.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php_image_retrieval.h"

extern "C" {
#include "php_ini.h"
#include "ext/standard/info.h"
#include "string.h"
}

static zend_function_entry image_retrieval_functions[] = {
    PHP_FE(ir_shape_detect, NULL)
	PHP_FE(ir_histogram, NULL)
	PHP_FE(ir_facedetect, NULL)
    {NULL, NULL, NULL}
};
/* }}} */

/* {{{ facedetect_module_entry
 */
zend_module_entry image_retrieval_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    PHP_IMAGE_RETRIEVAL_EXTNAME,
    image_retrieval_functions,
    NULL,
    NULL,
    NULL,
    NULL,
    PHP_MINFO(image_retrieval),
#if ZEND_MODULE_API_NO >= 20010901
    PHP_IMAGE_RETRIEVAL_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};
/* }}} */


ZEND_GET_MODULE(image_retrieval)

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINFO_FUNCTION(image_retrieval)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "image_retrieval support", "enabled");
	php_info_print_table_row(2, "image_retrieval version", PHP_IMAGE_RETRIEVAL_VERSION);
	php_info_print_table_row(2, "OpenCV version", CV_VERSION);
	php_info_print_table_end();
}
/* }}} */

static double angle(cv::Point pt1, cv::Point pt2, cv::Point pt0)
{
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

void setLabel(cv::Mat& im, const std::string label, std::vector<cv::Point>& contour)
{
	int fontface = cv::FONT_HERSHEY_SIMPLEX;
	double scale = 0.4;
	int thickness = 1;
	int baseline = 0;

	cv::Size text = cv::getTextSize(label, fontface, scale, thickness, &baseline);
	cv::Rect r = cv::boundingRect(contour);

	cv::Point pt(r.x + ((r.width - text.width) / 2), r.y + ((r.height + text.height) / 2));
	cv::rectangle(im, pt + cv::Point(0, baseline), pt + cv::Point(text.width, -text.height), CV_RGB(255,255,255), CV_FILLED);
	cv::putText(im, label, pt, fontface, scale, CV_RGB(0,0,0), thickness, 8);
}

static void shape_detect(INTERNAL_FUNCTION_PARAMETERS)
{
	char *filename, *saved_name;
	int flen, slen = 0;
	
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &filename, &flen, &saved_name, &slen) == FAILURE) {
		RETURN_NULL();
	}
	cv::Mat src = cv::imread(filename);
	if (src.empty()){
		RETURN_STRING("File not found", 1);
	}	
	// Convert to grayscale
	cv::Mat gray;
	cv::cvtColor(src, gray, CV_BGR2GRAY);

	// Use Canny instead of threshold to catch squares with gradient shading
	cv::Mat bw;
	cv::Canny(gray, bw, 0, 50, 5);

	// Find contours
	std::vector<std::vector<cv::Point> > contours;
	cv::findContours(bw.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	std::vector<cv::Point> approx;
	cv::Mat dst = src.clone();
	
	array_init(return_value);
	
	for (int i = 0; i < contours.size(); i++)
	{
		// Approximate contour with accuracy proportional
		// to the contour perimeter
		cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*0.02, true);

		// Skip small or non-convex objects 
		if (std::fabs(cv::contourArea(contours[i])) < 100 || !cv::isContourConvex(approx))
			continue;

		if (approx.size() == 3)
		{
			add_next_index_string(return_value, "TRIANGLE", 1);
			setLabel(dst, "TRI", contours[i]); 
		}
		else if (approx.size() >= 4 && approx.size() <= 6)
		{
			// Number of vertices of polygonal curve
			int vtc = approx.size();

			// Get the cosines of all corners
			std::vector<double> cos;
			for (int j = 2; j < vtc+1; j++)
				cos.push_back(angle(approx[j%vtc], approx[j-2], approx[j-1]));

			// Sort ascending the cosine values
			std::sort(cos.begin(), cos.end());

			// Get the lowest and the highest cosine
			double mincos = cos.front();
			double maxcos = cos.back();

			// Use the degrees obtained above and the number of vertices
			// to determine the shape of the contour
			if (vtc == 4 && mincos >= -0.1 && maxcos <= 0.3)
			{
				add_next_index_string(return_value, "RECTANGLE", 1);
				setLabel(dst, "RECT", contours[i]);
			}
			else if (vtc == 5 && mincos >= -0.34 && maxcos <= -0.27)
			{
				add_next_index_string(return_value, "PENTAGONAL", 1);
				setLabel(dst, "PENTA", contours[i]);
			}
			else if (vtc == 6 && mincos >= -0.55 && maxcos <= -0.45)
			{
				add_next_index_string(return_value, "HEXAGONAL", 1);
				setLabel(dst, "HEXA", contours[i]);
			}
		}
		else
		{
			// Detect and label circles
			double area = cv::contourArea(contours[i]);
			cv::Rect r = cv::boundingRect(contours[i]);
			int radius = r.width / 2;

			if (std::abs(1 - ((double)r.width / r.height)) <= 0.2 &&
			    std::abs(1 - (area / (CV_PI * std::pow(radius, 2)))) <= 0.2){
				add_next_index_string(return_value, "CIRCLE", 1);
				setLabel(dst, "CIRCLE", contours[i]);
			}
		}
	}
	// saved image if destination name specified
	if(slen > 0){
		imwrite(saved_name, dst);
	}
}

static void histogram(INTERNAL_FUNCTION_PARAMETERS)
{
	char *filename,*saved_name;
	int flen, slen, histSize;
	zval *array;
	
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl|s", &filename, &flen, &histSize, &saved_name, &slen) == FAILURE) {
		RETURN_NULL();
	}
	
	cv::Mat src = cv::imread(filename);
	if (src.empty()){
		RETURN_STRING("File not found", 1);
	}	
	
	  std::vector<Mat> bgr_planes;
	  split(src, bgr_planes);

	  /// Set the ranges ( for B,G,R) )
	  float range[] = { 0, 256 } ;
	  const float* histRange = { range };

	  bool uniform = true; bool accumulate = false;

	  Mat b_hist, g_hist, r_hist;

	  /// Compute the histograms:
	  calcHist( &bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );
	  calcHist( &bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate );
	  calcHist( &bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate );

	  // Draw the histograms for B, G and R
	  int hist_w = 512; int hist_h = 400;
	  int bin_w = cvRound( (double) hist_w/histSize );

	  Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );

	  /// Normalize the result to [ 0, histImage.rows ]
	  normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
	  normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
	  normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );

	  array_init(return_value);

	  /// Draw for each channel
	  for( int i = 1; i < histSize; i++ )
	  {
	  
		MAKE_STD_ZVAL(array);
		array_init(array);

		add_assoc_long(array, "r", cvRound(r_hist.at<float>(i)));
		add_assoc_long(array, "g", cvRound(g_hist.at<float>(i)));
		add_assoc_long(array, "b", cvRound(b_hist.at<float>(i)));

		add_next_index_zval(return_value, array);
		
		if(slen > 0){	
		  line( histImage, Point( bin_w*(i-1), hist_h - cvRound(b_hist.at<float>(i-1)) ) ,
						   Point( bin_w*(i), hist_h - cvRound(b_hist.at<float>(i)) ),
						   Scalar( 255, 0, 0), 2, 8, 0  );
		  line( histImage, Point( bin_w*(i-1), hist_h - cvRound(g_hist.at<float>(i-1)) ) ,
						   Point( bin_w*(i), hist_h - cvRound(g_hist.at<float>(i)) ),
						   Scalar( 0, 255, 0), 2, 8, 0  );
		  line( histImage, Point( bin_w*(i-1), hist_h - cvRound(r_hist.at<float>(i-1)) ) ,
						   Point( bin_w*(i), hist_h - cvRound(r_hist.at<float>(i)) ),
						   Scalar( 0, 0, 255), 2, 8, 0  );
		}
	  }
	if(slen > 0){
		imwrite("histogram.jpg", histImage);
	}
}

static void facedetect(INTERNAL_FUNCTION_PARAMETERS)
{
	char *filename, *cascadefile, *saved_name;
	int flen, clen, slen = 0;
	zval *array;
	
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|s", &filename, &flen, &cascadefile, &clen, &saved_name, &slen) == FAILURE) {
		RETURN_NULL();
	}
	cv::Mat src = cv::imread(filename);
	cv::Mat dst = src.clone();
	if (src.empty()){
		RETURN_STRING("File not found", 1);
	}
	
    // Load Face cascade (.xml file)
    CascadeClassifier face_cascade;
    face_cascade.load( cascadefile);
 
    // Detect faces
    std::vector<Rect> faces;
    face_cascade.detectMultiScale( src, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30) );
 
    // Draw circles on the detected faces
	array_init(return_value);
	
    for( int i = 0; i < faces.size(); i++ )
    {
		MAKE_STD_ZVAL(array);
		array_init(array);
		add_assoc_long(array, "x", faces[i].x);
		add_assoc_long(array, "y", faces[i].y);
		add_assoc_long(array, "w", faces[i].width);
		add_assoc_long(array, "h", faces[i].height);

		add_next_index_zval(return_value, array);
        Point center( faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5 );
        ellipse( dst, center, Size( faces[i].width*0.5, faces[i].height*0.5), 0, 0, 360, Scalar( 255, 0, 255 ), 4, 8, 0 );
    }
    
	// saved image if destination name specified
	if(slen > 0){
		imwrite(saved_name, dst);
	}
}

/* {{{ proto array ir_shape_detect(string picture_path, string picture_path_saved)
   find object and determine it's name */
PHP_FUNCTION(ir_shape_detect)
{
	shape_detect(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

/* {{{ proto array ir_shape_detect(string picture_path, string picture_path_saved)
   find object and determine it's name */
PHP_FUNCTION(ir_histogram)
{
	histogram(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}

/* {{{ proto array ir_shape_detect(string picture_path, string picture_path_saved)
   find object and determine it's name */
PHP_FUNCTION(ir_facedetect)
{
	facedetect(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
