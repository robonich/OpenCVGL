#include <opencv2/opencv.hpp>
#include <stdio.h>

cv::Mat inpaint_mask, bitwise_mask, bitwise_mask_backup;
cv::Mat original_image, whiteLined_image, inpainted, whiteLined_image_backup, bitwised;

cv::Point prev_pt_L, prev_pt_R;

void on_mouse(int event, int x , int y , int flags, void *){
  if(whiteLined_image.empty()){
    return;
  }

  if(event == CV_EVENT_RBUTTONUP || !(flags & CV_EVENT_FLAG_RBUTTON)){
    prev_pt_R = cv::Point(-1, -1);
    prev_pt_R = cv::Point(x, y);
    whiteLined_image.copyTo(whiteLined_image_backup);
    bitwise_mask.copyTo(bitwise_mask_backup);
  }
  else if(event == CV_EVENT_RBUTTONDOWN){
    prev_pt_R = cv::Point(x, y);
  }
  else if(event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_RBUTTON)){
    cv::Point ptR(x, y);
    if(prev_pt_R.x < 0){
      prev_pt_R = ptR;
    }
    whiteLined_image_backup.copyTo(whiteLined_image);
    bitwise_mask_backup.copyTo(bitwise_mask);
    cv::rectangle(bitwise_mask, prev_pt_R, ptR, cv::Scalar(255), -1);
    cv::rectangle(whiteLined_image, prev_pt_R, ptR, cv::Scalar::all(255));

    cv::imshow("image", whiteLined_image);
  }
  
  if(event == CV_EVENT_LBUTTONUP || !(flags & CV_EVENT_FLAG_LBUTTON)){
    prev_pt_L = cv::Point(-1, -1); //init the start point
  }
  else if(event == CV_EVENT_LBUTTONDOWN){
    prev_pt_L = cv::Point(x, y);// set the start point
  }
  else if(event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_LBUTTON)){
    cv::Point pt(x, y);
    if(prev_pt_L.x < 0){
      prev_pt_L = pt;
    }
    // draw a line from the start point to the current point
    cv::line(inpaint_mask, prev_pt_L, pt, cv::Scalar(255), 5, 8, 0);
    cv::line(whiteLined_image, prev_pt_L, pt, cv::Scalar::all(255), 5, 8, 0);
    cv::line(whiteLined_image_backup, prev_pt_L, pt, cv::Scalar::all(255), 5, 8, 0);//線だけのバックアップを取る
    // set the current point to the new start point
    prev_pt_L = pt;
   
    //cv::Mat img_hdr =whiteLined_image;
    cv::imshow("image",whiteLined_image);
  }
}

int main(int argc, char *argv[]){

  // 1. read image file
  char *filename = (argc >= 2) ? argv[1] : (char*)"/home/denjo/opencv.build/opencv-2.4.11/samples/cpp/fruits.jpg";
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
  whiteLined_image = original_image.clone();
  whiteLined_image_backup = whiteLined_image.clone();
  inpainted = original_image.clone();
  bitwised = original_image.clone();
  inpaint_mask.create(original_image.size(), CV_8UC1);
  bitwise_mask.create(original_image.size(), CV_8UC1);
  bitwise_mask_backup = bitwise_mask.clone();
  
  inpaint_mask = cv::Scalar(0);
  bitwise_mask = cv::Scalar(0);
  bitwise_mask_backup = cv::Scalar(0);
  inpainted = cv::Scalar(0);
  /* hatena */
  
  // 4. show image to window for generating mask
  cv::imshow("image", whiteLined_image);
  
  // 5. set callback function for mouse operations
  cv::setMouseCallback("image", on_mouse, 0);
  
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
        cv::namedWindow("inpainted image", 1);
	cv::inpaint(original_image, inpaint_mask, inpainted, 3.0, cv::INPAINT_TELEA);
        cv::imshow("inpainted image", inpainted);
        break;
	
    case 'b':
      cv::namedWindow("bitwise_not image", 1);
      cv::bitwise_not(original_image, bitwised, bitwise_mask);
      cv::imshow("bitwised image", bitwised);
      break;

    case 10://ENTER
      cv::namedWindow("inpainted and bitwised not image", 1);
      cv::inpaint(original_image, inpaint_mask, inpainted, 3.0, cv::INPAINT_TELEA);
      original_image.copyTo(bitwised);
      cv::bitwise_not(inpainted, bitwised, bitwise_mask);
      cv::imshow("inpainted and bitwised not image", bitwised);
      cv::imwrite("inpaintedBitwisedNot.jpg", bitwised);
    }
  }
  return 0;
}
