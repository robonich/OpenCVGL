#include <opencv2/opencv.hpp>
#include <stdio.h>

int size_of_mosaic = 0;
char *doraemon_file = "rakuten_card.jpeg";
int min_size = 0;
int max_size = 30;

#define MAX_SIZE 100
#define MIN_SIZE 30

int main(int argc, char *argv[])
{
  // 1. load classifier
	std::string cascadelefteyeName = "/usr/local/share/OpenCV/haarcascades/haarcascade_mcs_lefteye.xml";
	std::string cascaderighteyeName = "/usr/local/share/OpenCV/haarcascades/haarcascade_mcs_righteye.xml";
	std::string cascadefaceName = "/usr/local/share/OpenCV/haarcascades/haarcascade_frontalface_alt.xml";

	cv::CascadeClassifier cascaderighteye, cascadelefteye, cascadeface;
	if(!cascadelefteye.load(cascadelefteyeName)){
		printf("ERROR: cascadelefteyeFile not found\n");
		return -1;
	}

	if(!cascaderighteye.load(cascaderighteyeName)){
		printf("ERROR: cascaderighteyeFile not found\n");
		return -1;
	}

	if(!cascadeface.load(cascadefaceName)){
		printf("ERROR: cascadefaceFile not found\n");
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
	cv::namedWindow("lefteyes", 1);
	cv::namedWindow("righteyes", 1);

	cv::createTrackbar("size", "result", &size_of_mosaic, 30, 0);
	cv::createTrackbar("minsize", "result", &min_size, MIN_SIZE, 0);
	cv::createTrackbar("maxsize", "result", &max_size, MAX_SIZE, 0);

	double scale = 4.0;
	cv::Mat gray, smallImg(cv::saturate_cast<int>(frame.rows/scale), cv::saturate_cast<int>(frame.cols/scale), CV_8UC1);

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
		std::vector<cv::Rect> lefteyes;
		std::vector<cv::Rect> righteyes;
		std::vector<cv::Rect> faces;
    ///multi-scale face searching
    // image, size, scale, num, flag, smallest rect
		cascadelefteye.detectMultiScale(smallImg, lefteyes,
			1.1,
				2,//この引数を大きくすると検出が早くなる精度も悪くなる
				CV_HAAR_SCALE_IMAGE,
				cv::Size(2.0*min_size/3.0, min_size),
				cv::Size(2.0*max_size/3.0, max_size));

		cascaderighteye.detectMultiScale(smallImg, righteyes,
			1.1,
				2,//この引数を大きくすると検出が早くなる精度も悪くなる
				CV_HAAR_SCALE_IMAGE,
				cv::Size(2.0*min_size/3.0, min_size),
				cv::Size(2.0*max_size/3.0, max_size));

		cascadeface.detectMultiScale(smallImg, faces, 1.1, 2, CV_HAAR_SCALE_IMAGE);

    // 7. mosaic(pixelate) face-region
    //std::vector<cv::Rect>::const_iterator r = faces.begin();
		int i;

		for(i=0;i<(faces.size()==0?0:1);++i){
			cv::Point facecenter;
			int faceradius;
			facecenter.x = cv::saturate_cast<int>((faces[i].x + faces[i].width*0.5)*scale);
			facecenter.y = cv::saturate_cast<int>((faces[i].y + faces[i].height*0.5)*scale);
			faceradius = cv::saturate_cast<int>((faces[i].width + faces[i].height)*0.25*scale);

			//cv::Rect face_rect(facecenter.x-faceradius,facecenter.y-faceradius, faceradius*2, faceradius*2);

			for(i=0;i<(lefteyes.size()/*==0?0:1*/);++i){
				cv::Point center;
				int radius;
				double radiusx, radiusy;
				center.x = cv::saturate_cast<int>((lefteyes[i].x + lefteyes[i].width*0.5)*scale);
				center.y = cv::saturate_cast<int>((lefteyes[i].y + lefteyes[i].height*0.5)*scale);
				radius = cv::saturate_cast<int>((lefteyes[i].width + lefteyes[i].height)*0.25*scale);
				if(center.x < facecenter.x) break;
          		if(center.y > facecenter.y) break;

      //画像の縦横比で正規化
				if(doraemon.size().width > doraemon.size().height) {
					radiusx = (double)radius;
					radiusy = (double)radius * doraemon.size().height / doraemon.size().width;
				} else {
					radiusy = (double)radius;
					radiusx = (double)radius * doraemon.size().width / doraemon.size().height;
				}

      //mosaic

				if(size_of_mosaic < 1) size_of_mosaic = 1;
				cv::Rect roi_rect(center.x-radiusx,center.y-radiusy, radiusx*2, radiusy*2);
      			cv::Mat lefteye = frame(roi_rect);//顔の部分を切り出している
      			cv::imshow("lefteyes", lefteye);
      			if(dora_flag){

      				cv::resize(doraemon, doraemon_resized, lefteye.size());
          			lefteye = cv::Scalar(0,0,0);// doraemon_resized;
          			cv::add(doraemon_resized, lefteye, lefteye);
          		}else{
          			cv::Mat tmp;
      //モザイクの処理
          			cv::resize(lefteye,tmp,cv::Size(radiusx / size_of_mosaic, radiusy/ size_of_mosaic),0,0);
          			cv::resize(tmp,lefteye, cv::Size(radiusx*2, radiusy*2),0,0,CV_INTER_NN);
          		}
      //cv::imshow("mosaic", mosaic);
          	}

          	for(i=0;i<(righteyes.size()/*==0?0:1*/);++i){
          		cv::Point center;
          		int radius;
          		double radiusx, radiusy;
          		center.x = cv::saturate_cast<int>((righteyes[i].x + righteyes[i].width*0.5)*scale);
          		center.y = cv::saturate_cast<int>((righteyes[i].y + righteyes[i].height*0.5)*scale);
          		radius = cv::saturate_cast<int>((righteyes[i].width + righteyes[i].height)*0.25*scale);
          		if(center.x > facecenter.x) break;
          		if(center.y > facecenter.y) break;

      //画像の縦横比で正規化
          		if(doraemon.size().width > doraemon.size().height) {
          			radiusx = (double)radius;
          			radiusy = (double)radius * doraemon.size().height / doraemon.size().width;
          		} else {
          			radiusy = (double)radius;
          			radiusx = (double)radius * doraemon.size().width / doraemon.size().height;
          		}

      //mosaic

          		if(size_of_mosaic < 1) size_of_mosaic = 1;
          		cv::Rect roi_rect(center.x-radiusx,center.y-radiusy, radiusx*2, radiusy*2);
      			cv::Mat righteye = frame(roi_rect);//顔の部分を切り出している
      			cv::imshow("righteyes", righteye);
      			if(dora_flag){

      				cv::resize(doraemon, doraemon_resized, righteye.size());
          			righteye = cv::Scalar(0,0,0);// doraemon_resized;
          			cv::add(doraemon_resized, righteye, righteye);
          		}else{
          			cv::Mat tmp;
      //モザイクの処理
          			cv::resize(righteye,tmp,cv::Size(radiusx / size_of_mosaic, radiusy/ size_of_mosaic),0,0);
          			cv::resize(tmp,righteye, cv::Size(radiusx*2, radiusy*2),0,0,CV_INTER_NN);
          		}
          	}
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

