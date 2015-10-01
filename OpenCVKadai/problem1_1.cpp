#include <opencv2/opencv.hpp>
#include <stdio.h>

#define FLAG 1 // (0: direct access / 1: built-in function)
#define BLUR_LEVEL 10


char* preset_file = "/home/denjo/opencv.build/opencv-2.4.11/samples/cpp/fruits.jpg";

void convertColorToGray(cv::Mat &input, cv::Mat &processed);
void convertColorToBlur(cv::Mat &input, cv::Mat &blur);
void findEdge(cv::Mat &input, cv::Mat &edge);


int main(int argc, char *argv[])
{
  char *input_file;
  // 1. prepare Mat objects for input-image and output-image
  cv::Mat input, gray, blur, edge;

  if(argc == 2){
    input_file = argv[1];
  }
  else{
    input_file = preset_file;
  }

  // 2. read an image from the specified file
  input = cv::imread(input_file,1);
  if(input.empty()){
    fprintf(stderr, "cannot open %s\n", input_file);
    exit(0);
  }

  convertColorToGray(input, gray);
  convertColorToBlur(input, blur);
  findEdge(gray, edge);
 
  // 5. create windows
  cv::namedWindow("Original image", 1);
  cv::namedWindow("gray image", 1);
  cv::namedWindow("blur image", 1);
  cv::namedWindow("edge image", 1);

  // 6. show images
  cv::imshow("Original image", input);
  cv::imshow("gray image", gray);
  cv::imshow("blur image", blur);
  cv::imshow("edge image", edge);


  // 7. wait key input
  cv::waitKey(0);
  
  // 8. save the processed result
#if FLAG
  cv::imwrite("blur.jpg", blur);
  cv::imwrite("gray.jpg", gray);
  cv::imwrite("edge.jpg", edge);
#else
  cv::imwrite("blur-self.jpg", blur);
  cv::imwrite("gray-self.jpg", gray);
  cv::imwrite("edge-self.jpg", edge);
#endif
  
  return 0;
}
      

void convertColorToGray(cv::Mat &input, cv::Mat &processed)
{
#if FLAG // use built-in function

  //4. convert color to gray
  cv::Mat temp;
  std::vector<cv::Mat> planes;
  cv::cvtColor(input, temp, CV_BGR2YCrCb);
  cv::split(temp, planes);
  processed = planes[0];
 
#else

  // 3. create Mat for output-image
  cv::Size s = input.size();
  processed.create(s, CV_8UC1);

  for(int j=0;j<s.height;j++){
    uchar *ptr1, *ptr2;
    ptr1 = input.ptr<uchar>(j);
    ptr2 = processed.ptr<uchar>(j);

    //4. convert color to gray
    for(int i=0;i<s.width;i++){
      double y = 0.114 * ((double)ptr1[0]) + 0.587 * (double)ptr1[1] + 0.299 * (double)ptr1[2];
      
      if(y > 255) y = 255;
      if(y < 0) y = 0;

      *ptr2=(uchar)y;
      ptr1 += 3;
      ptr2++;      
    }
  }
#endif
}

void convertColorToBlur(cv::Mat &input, cv::Mat &blur){
#if FLAG // use built-in function

  //4. convert color to blur
  cv::GaussianBlur(input, blur, cv::Size(11,11), 10, 10);
 
#else

  // 3. create Mat for output-image
  cv::Size s = input.size();
  blur.create(s, CV_8UC3);

  for(int j=0;j<s.height;j++){
    uchar *ptr1, *ptr2;
    ptr1 = input.ptr<uchar>(j);
    ptr2 = blur.ptr<uchar>(j);
    for(int i=0;i<s.width;i++){
      int count = 0;
      double color[3] = {0,0,0};
      for(int a = -BLUR_LEVEL; a <= BLUR_LEVEL; a++){
	for(int b = -BLUR_LEVEL; b <= BLUR_LEVEL; b++){
	  if(j + a >= 0 && j + a <= s.height-1 && i + b >= 0 && i + b <= s.width-1){
	    count++;
	    for(int layer = 0; layer < 3; layer++){
	      color[layer] += (double)((input.ptr<uchar>(j+a)+3*(i+b))[layer]);
	    }
	  }
	}
      }
      for(int layer = 0; layer < 3; layer++){
	color[layer] /= (double)count;
	ptr2[layer] = (uchar)color[layer];
      }
      ptr2 += 3;
      ptr1 += 3;
    } 
  }
#endif
}

void findEdge(cv::Mat &input, cv::Mat &edge){//inputはgrayにされたもの
#if FLAG
  cv::Sobel(input, edge, -1, 1, 1, 5);
 
#else
  cv::Size s=input.size();
  edge.create(s,CV_8UC1);//channelは一つ
  int filterHS1[3][3] = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}
  };
  int filterHS2[3][3] = {
    {1,0,-1},
    {2,0,-2},
    {1,0,-1}
  };
  int filterVS1[3][3] = {
    {-1, -2, -1},
    {0, 0, 0},
    {1, 2, 1}
    //{0,0,0},
    //{0,0,0},
    //{0,0,0}
  };
  int filterVS2[3][3] = {
    {1,2,1},
    {0,0,0},
    {-1,-2,-1}
  };
  uchar *ptr1;
  uchar *ptr2;
  for(int j = 0; j < s.height; j++){
    for(int i = 0; i < s.width; i++){
      double tmpHS1 = 0;
      double tmpVS1 = 0;
      double tmpHS2 = 0;
      double tmpVS2 = 0;
      double tmp = 0;
      ptr2=edge.ptr<uchar>(j)+i;
      if(j == 0 || i == 0 || j == s.height-1 || i == s.width-1){
	ptr1=input.ptr<uchar>(j)+i;
	*ptr2 = *ptr1;//(uchar)(0.114*(double)ptr1[0]+0.587*(double)ptr1[1]+0.299*(double)ptr1[2]);
	continue;
      }
      for(int l = -1; l <= 1; l++){
	for(int k = -1; k <= 1; k++){
	  //double grayscale = (double)(input.ptr<uchar>(j+l)+i+k)[0]
	  double a = (double)(input.ptr<uchar>(j+l)+i+k)[0];
	  tmpHS1 += a * filterHS1[l+1][k+1];
	  tmpHS2 += a * filterHS2[l+1][k+1];
	  tmpVS1 += a * filterVS1[l+1][k+1];
	  tmpVS2 += a * filterVS2[l+1][k+1];
	}
      }
      tmp = sqrt(tmpHS1*tmpHS1 + tmpVS1*tmpVS1 + tmpHS2*tmpHS2 + tmpVS2*tmpVS2);
      //printf("tmpHS = %lf, tmpVS = %lf", tmpHS, tmpVS);
      *ptr2 = (uchar)tmp;
    }
  }
#endif
}


