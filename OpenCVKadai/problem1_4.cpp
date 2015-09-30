#include <opencv2/opencv.hpp>
#include <stdio.h>

char *background = "/home/denjo/opencv.build/opencv-2.4.11/samples/cpp/fruits.jpg";


int main(int argc, char *argv[])
{
  int INIT_TIME = 50;
  int width, height;
  double B_PARAM = 1.0 / 50.0;
  double T_PARAM = 1.0 / 200.0;
  double Zeta = 10.0;

  cv::VideoCapture cap;
  cv::Mat frame;
  cv::Mat avg_img, sgm_img;
  cv::Mat lower_img, upper_img, tmp_img;
  cv::Mat dst_img, msk_img, bitwised_msk_img;
  cv::Mat background_img = cv::imread(background);
  cv::Mat background_img2;
  
  // 1. initialize VideoCapture 
  if(argc >= 2){
    cap.open(argv[1]);
  }else{
    cap.open(0);
  }
  if(!cap.isOpened()){
    printf("Cannot open the video.\n");
    exit(0);
  }

  // 2. prepare window for showing images
  cv::namedWindow("Input", 1);
  cv::namedWindow("FG", 1);
  cv::namedWindow("mask", 1);
  cv::namedWindow("background", 1);

  // 3. calculate initial value of background
  cap >> frame;
  if(frame.empty()) return 0;
  
  cv::Size s = frame.size();
  
  avg_img.create(s, CV_32FC3);
  sgm_img.create(s, CV_32FC3);
  lower_img.create(s, CV_32FC3);
  upper_img.create(s, CV_32FC3);
  tmp_img.create(s, CV_32FC3);
  
  dst_img.create(s, CV_8UC3);
  msk_img.create(s, CV_8UC1);
  bitwised_msk_img.create(s, CV_8UC1);

  background_img2.create(s,CV_8UC3);

  cv::resize(background_img, background_img, s);
  
  printf("Background statistics initialization start\n");
  
  avg_img = cv::Scalar(0,0,0);
  
  for( int i = 0; i < INIT_TIME; i++){
    cap >> frame;
    if(frame.empty()) break;

    cv::Mat tmp;
    frame.convertTo(tmp, avg_img.type());
    cv::accumulate(tmp, avg_img);
  }
 
  avg_img.convertTo(avg_img, -1,1.0 / INIT_TIME);
  sgm_img = cv::Scalar(0,0,0);

  cv::Mat preFrame;
  preFrame.create(s, CV_32FC3);

  for( int i = 0; i < INIT_TIME; i++){
    cap >> frame;

    if(frame.empty()) {
    	return 0;
	}	
    frame.convertTo(tmp_img, avg_img.type());
    cv::subtract(tmp_img, avg_img, tmp_img);
    cv::pow(tmp_img, 2.0, tmp_img);
    tmp_img.convertTo(tmp_img, -1, 2.0);
    cv::sqrt(tmp_img, tmp_img);
    cv::accumulate(tmp_img, sgm_img);	
  }

  sgm_img.convertTo(sgm_img, -1, 1.0 / INIT_TIME);

  printf("Background statistics initialization finish\n");


  bool loop_flag = true;
  while(loop_flag){
 	frame.copyTo(preFrame);
    cap >> frame;
    if(frame.empty()){
    	imshow("Input", preFrame);
    	break;
    }
    frame.convertTo(tmp_img, tmp_img.type());

    // 4. check whether pixels are background or not
    cv::subtract(avg_img, sgm_img, lower_img);
    cv::subtract(lower_img, Zeta, lower_img);
    cv::add(avg_img, sgm_img, upper_img);
    cv::add(upper_img, Zeta, upper_img);
    cv::inRange(tmp_img, lower_img, upper_img, msk_img);

    // 5. recalculate 
    cv::subtract(tmp_img, avg_img, tmp_img);
    cv::pow(tmp_img, 2.0, tmp_img);
    tmp_img.convertTo(tmp_img, -1, 2.0);
    cv::sqrt(tmp_img, tmp_img);

    // 6. renew avg_img and sgm_img
    cv::accumulateWeighted(frame, avg_img, B_PARAM, msk_img);
    cv::accumulateWeighted(tmp_img, sgm_img, B_PARAM, msk_img);
    
    cv::bitwise_not(msk_img, msk_img);
    cv::accumulateWeighted(tmp_img, sgm_img, T_PARAM, msk_img);

    dst_img = cv::Scalar(0);
    background_img2 = cv::Scalar(0);
    frame.copyTo(dst_img, msk_img);
    cv::bitwise_not(msk_img, bitwised_msk_img);
    background_img.copyTo(background_img2, bitwised_msk_img);

    cv::add(background_img2, dst_img, dst_img);
    
    cv::imshow("Input", frame);
    cv::imshow("FG", dst_img);
    cv::imshow("mask", msk_img);
    cv::imshow("background", background_img2);
    
    char key =cv::waitKey(20);
    if(key == 27){
      loop_flag = false;
    }        
  }
  while(1){
  	char key2 = cv::waitKey();
  	if(key2 == 27) break;
  }
  return 0;
}
