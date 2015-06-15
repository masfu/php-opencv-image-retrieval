# php-opencv-image-retrieval
php native opencv wrapper for image retrieval using histogram, shape detection and face recognition



install opencv on your machine and install php-devel and make sure to also install g++ and make
compile and install
<pre>
phpize
./configure
make && sudo make install
</pre>
add image_retrieval.so in php.ini

this function to determine basic shape in a picture
<pre>
ir_shape_detect(string picture_path, string picture_path_saved);
</pre>

to calculate histogram you can use this function just set picture path and second is number of bins
<pre>
ir_histogram(string picture_path, 255, string picture_path_saved);
</pre>

and detect and count faces is a picture, first is your picture path, second cascade xml classsifier, and third is optional whether you want to save or not. and return value is array containing position of faces
<pre>
ir_facedetect(string picture_path, string cascade_file, string picture_path_saved);
</pre>





