#include <opencv2/opencv.hpp>
#include <stdio.h>

int size_of_mosaic = 0;
char *doraemon_file = "doraemon.jpg";

int main(int argc, char *argv[])
{
  // 1. load classifier
  std::string cascadeName = "/usr/local/share/OpenCV/haarcascades/haarcascade_frontalface_alt.xml"; //Haar-like
  cv::CascadeClassifier cascade;
  if(!cascade.load(cascadeName)){
    printf("ERROR: cascadeFile not found\n");
    return -1;
  }

  cv::Mat doraemon = cv::imread(doraemon_file);
  cv::Mat doraemon_resized;
  
  // 2. initialize VideoCapture
  cv::Mat frame;
  cv::VideoCapture cap;
  cap.open(0);
  cap >> frame;
  
  // 3. prepare window and trackbar
  cv::namedWindow("result", 1);
  //cv::namedWindow("mosaic", 1);
  cv::createTrackbar("size", "result", &size_of_mosaic, 30, 0);

  double scale = 4.0;
  cv::Mat gray, smallImg(cv::saturate_cast<int>(frame.rows/scale),
               cv::saturate_cast<int>(frame.cols/scale), CV_8UC1);

    bool dora_flag = false;
  for(;;){
    
    // 4. capture frame
    cap >> frame;
    //convert to gray scale
    cv::cvtColor( frame, gray, CV_BGR2GRAY );
    
    // 5. scale-down the image
	  cv::resize(gray, smallImg, smallImg.size(), 0, 0, cv::INTER_LINEAR);
	  cv::equalizeHist(smallImg, smallImg);
    
    // 6. detect face using Haar-classifier
    std::vector<cv::Rect> faces;
    ///multi-scale face searching
    // image, size, scale, num, flag, smallest rect
	  cascade.detectMultiScale(smallImg, faces,
      1.1,
      4,//この引数を大きくすると検出が早くなる
      CV_HAAR_SCALE_IMAGE,
      cv::Size(30,30));
    
    // 7. mosaic(pixelate) face-region
    //std::vector<cv::Rect>::const_iterator r = faces.begin();
    int i;
    for(i=0;i<faces.size();++i){
      cv::Point center;
      int radius;
      center.x = cv::saturate_cast<int>((faces[i].x + faces[i].width*0.5)*scale);
      center.y = cv::saturate_cast<int>((faces[i].y + faces[i].height*0.5)*scale);
      radius = cv::saturate_cast<int>((faces[i].width + faces[i].height)*0.25*scale);
      //mosaic
      if(size_of_mosaic < 1) size_of_mosaic = 1;
      cv::Rect roi_rect(center.x-radius,center.y-radius,radius*2,radius*2);
      cv::Mat mosaic = frame(roi_rect);//顔の部分を切り出している
      if(dora_flag){
          cv::resize(doraemon, doraemon_resized, mosaic.size());
          mosaic = cv::Scalar(0,0,0);// doraemon_resized;
          cv::add(doraemon_resized, mosaic, mosaic);
      }else{
          cv::Mat tmp;
      //モザイクの処理
          cv::resize(mosaic,tmp,cv::Size(radius / size_of_mosaic, radius / size_of_mosaic),0,0);
          cv::resize(tmp,mosaic, cv::Size(radius*2, radius*2),0,0,CV_INTER_NN);
      }
      //cv::imshow("mosaic", mosaic);
    }
    
    // 8. show mosaiced image to window
    cv::imshow("result", frame );

    int key = cv::waitKey(10);
    if(key == 'q' || key == 'Q')
        break;
    else if(key == 'd' || key == 'D'){
	dora_flag = !dora_flag;
    }


  }
 return 0;
}

