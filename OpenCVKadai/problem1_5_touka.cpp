#include <opencv2/opencv.hpp>
#include <stdio.h>

int size_of_mosaic = 0;
char *doraemon_file = "doraemon.jpg";
char *transparent_file = "face.png";

void transparentAdd(cv::Mat &img1, cv::Mat &img2, cv::Mat &blend);


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
  cv::Mat transparent = cv::imread(transparent_file, -1);
  cv::Mat transparent_resizezd;
  
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
    bool transparent_flag = false;
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
      if(transparent_flag) {
        std::vector<cv::Mat> mv, mv2;
        cv::split(mosaic, mv);
        for(int i = 0; i<4; i++) mv2.push_back(mv[0]);
        cv::Mat temp_mosaic;
        cv::merge(mv2, temp_mosaic);//4channelの大きさを持つmosaicをつくる
        cv::resize(transparent, transparent_resizezd, temp_mosaic.size());
          transparentAdd(transparent_resizezd, mosaic, mosaic);
      }else if(dora_flag){
          cv::resize(doraemon, doraemon_resized, mosaic.size());
          mosaic = cv::Scalar(0,0,0);// doraemon_resized;
          cv::add(doraemon_resized, mosaic, mosaic);
      }else{
      //モザイクの処理
        cv::Mat tmp;
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
    }else if(key == 't' || key == 'T'){
      transparent_flag = !transparent_flag;
    }

  }
 return 0;
}

void transparentAdd(cv::Mat &img1, cv::Mat &img2, cv::Mat &blend) {//img2がベース, img1はアルファチャンネル
  // BGRAチャンネルに分離
  std::vector<cv::Mat> mv;
  cv::split(img1, mv);
  
  /// 合成処理
  std::vector<cv::Mat> tmp_a;
  cv::Mat alpha, alpha32f;
  // 4番目のチャンネル=アルファ
  alpha = mv[3].clone();
  mv[3].convertTo(alpha32f, CV_32FC1);
  cv::normalize(alpha32f, alpha32f, 0., 1., cv::NORM_MINMAX);
  for(int i=0; i<3; i++) tmp_a.push_back(alpha32f);
  cv::Mat alpha32fc3, beta32fc3;
  cv::merge(tmp_a, alpha32fc3);
  cv::Mat tmp_ones = cv::Mat::ones(cv::Size(img2.rows, img2.cols*3), CV_32FC1);
  beta32fc3 = tmp_ones.reshape(3,img2.rows) - alpha32fc3;
  cv::Mat img1_rgb, img1_32f, img2_32f;
  mv.resize(3);
  cv::merge(mv, img1_rgb);
  img1_rgb.convertTo(img1_32f, CV_32FC3);
  img2.convertTo(img2_32f, CV_32FC3);
  // 二つの画像の重み付き和
  cv::Mat blend32f = img1_32f.mul(alpha32fc3) + img2_32f.mul(beta32fc3);
  blend32f.convertTo(blend, CV_8UC3);
}
