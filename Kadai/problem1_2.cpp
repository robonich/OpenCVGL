#include <opencv2/opencv.hpp>
#include <stdio.h>

cv::Mat inpaint_mask;
cv::Mat original_image, whiteLined_image, inpainted;

cv::Point prev_pt;

void on_mouse(int event, int x , int y , int flags, void *){
  if(whiteLined_image.empty()){
    return;
  }
  
  if(event == CV_EVENT_LBUTTONUP || !(flags & CV_EVENT_FLAG_LBUTTON)){
	  /* hatena */ // init the start point
  }
  else if(event == CV_EVENT_LBUTTONDOWN){
	  /* hatena */ // set the start point
  }
  else if(event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_LBUTTON)){
    cv::Point pt(x, y);
    if(prev_pt.x < 0){
      prev_pt = pt;
    }

    // draw a line from the start point to the current point
	/* hatena */
	/* hatena */

    // set the current point to the new start point
    prev_pt = pt;
   
    //cv::Mat img_hdr =whiteLined_image;
    cv::imshow("image",whiteLined_image);
  }
}

int main(int argc, char *argv[]){

  // 1. read image file
  char *filename = (argc >= 2) ? argv[1] : (char*)"fruits.jpg";
  original_image = cv::imread(filename);
  if(original_image.empty()){
	printf("ERROR: image not found!\n");
    return 0;
  }

  //print hot keys
  printf( "Hot keys: \n"
	  "\tESC - quit the program\n"
	  "\ti or ENTER - run inpainting algorithm\n"
	  "\t\t(before running it, paint something on the image)\n");
  
  // 2. prepare window
  cv::namedWindow("image",1);
  
  // 3. prepare Mat objects for processing-mask and processed-image
  /* hatena */
  /* hatena */
  /* hatena */
  
  /* hatena */
  /* hatena */
  
  // 4. show image to window for generating mask
  cv::imshow("image", whiteLined_image);
  
  // 5. set callback function for mouse operations
  /* hatena */
  
  bool loop_flag = true;
  while(loop_flag){
	
	// 6. wait for key input
    int c = cv::waitKey(0);
    
	// 7. process according to input
    switch(c){
      case 27://ESC
      case 'q':
        loop_flag = false;
        break;
  
      case 'r':
        inpaint_mask = cv::Scalar(0);
        original_image.copyTo(whiteLined_image);
        cv::imshow("image", whiteLined_image);
        break;
  
      case 'i':
      case 10://ENTER
        cv::namedWindow("inpainted image", 1);
		/* hatena */
        cv::imshow("inpainted image", inpainted);
        break;
    }
  }
  return 0;
}
