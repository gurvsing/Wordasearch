#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "tesseract/baseapi.h"


#include <iostream>
#include <fstream>
#include <utility>
#include <stack>
#include <set>
#include <math.h>
#include <string.h>
#include <locale.h>


#define BUG printf("BUG\n");


static const char* window_name = "haha";
static const int columns = 4;
static const int rows = 4;

int count_q=0;

//static const std::vector<std::pair<int, int> > adjacent = 
//{{-1, -1}, {-1, 0}, {-1, 1},
//  {0, -1},/*{0, 0},*/{0, 1},
//  {1, -1},  {1, 0},  {1, 1}};


bool sortByPosY(const std::pair<std::pair<int, int>, int>  &lhs, const std::pair<std::pair<int, int>, int>  &rhs) { return lhs.first.second <  rhs.first.second; }

bool sortByPosX(const std::pair<std::pair<int, int>, int>  &lhs, const std::pair<std::pair<int, int>, int>  &rhs) { return lhs.first.first <  rhs.first.first; }

std::vector<std::vector<cv::Point2f> > findSquares(cv::Mat& image)
{
  std::vector<std::vector<cv::Point2f> > squares;

  cv::Mat pyr, timg;
  // down-scale and upscale the image to filter out the noise
  cv::pyrDown(image, pyr, cv::Size(image.cols/2, image.rows/2));
  cv::pyrUp(pyr, timg, image.size());
  cv::cvtColor(timg,timg,CV_BGR2HSV);
  // use only one channel from an image
  int from_to[] = {0, 0};
  cv::Mat single_channel(image.size(), CV_8U);
  cv::mixChannels(&timg, 1, &single_channel, 1, from_to, 1);

  // detect edges using canny algorithm
  cv::Mat bw;
  cv::Canny(single_channel, bw, 100, 200, 3);
   
  // dilate the image in order to remove possible gaps on edges
  //cv::dilate(bw, bw, cv::Mat(), cv::Point2f(-1, -1));
  cv::imshow("1",bw);

  // extract contours from the image
  std::vector<std::vector<cv::Point> > contours;
  cv::findContours(bw, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

  std::vector<cv::Point> approx;
  for (int i=0;i<contours.size();i++)
  {
    // approximate contour with accuracy proportional
    // to the contour perimeter
    cv::approxPolyDP(cv::Mat(contours[i]), approx,
        cv::arcLength(cv::Mat(contours[i]), true) * 0.03, true);

    // squares we are looking for have four vertices
    // have area bigger than 0.005 part of total image area
    // and are convex
    if (approx.size() == 4 && std::fabs(cv::contourArea(contours[i])) > image.size().area() * 0.0001 &&
        cv::isContourConvex(approx))
    {
    	/*for(int k=0;k<approx.size();k++)
    	{
    		std::cout<<approx[k]<<" ";
    	}
    	std::cout<<std::endl;*/
      // also, bounding rectangle's dimensions must be square-like
      cv::Rect bounding_rect = cv::boundingRect(cv::Mat(approx));
      if ((float)bounding_rect.height < (float)bounding_rect.width * 1.2 &&
          (float)bounding_rect.height > (float)bounding_rect.width * 0.8)
      {
        std::vector<cv::Point2f> result;
        cv::Mat(approx).copyTo(result);
        squares.push_back(result);
      }
    }
  }
  //std::cout<<squares.size()<<std::endl;
  
  return squares;
}

// function was taken from http://opencv-code.com/tutorials/automatic-perspective-correction-for-quadrilateral-objects/
void sortCorners(std::vector<cv::Point2f>& corners, cv::Point centre)
{
  std::vector<cv::Point2f> top, bottom;

  for (int i=0;i<corners.size();i++)
    if (corners[i].y < centre.y) top.push_back(corners[i]);
    else bottom.push_back(corners[i]);

  cv::Point2f top_left = top[0].x > top[1].x ? top[1] : top[0];
  cv::Point2f top_right = top[0].x > top[1].x ? top[0] : top[1];
  cv::Point2f bottom_left = bottom[0].x > bottom[1].x ? bottom[1] : bottom[0];
  cv::Point2f bottom_right = bottom[0].x > bottom[1].x ? bottom[0] : bottom[1];

  corners.clear();
  corners.push_back(top_left);
  corners.push_back(top_right);
  corners.push_back(bottom_right);
  corners.push_back(bottom_left);
}

std::vector<std::vector<cv::Mat> > cutSquares(cv::Mat image, std::vector<std::vector<cv::Point2f> > squares)
{

  if (squares.size() != 16) return std::vector<std::vector<cv::Mat> >();
  //BUG;

  // find mass centres and sort square's corners
  std::vector<cv::Point2f> centres;
  std::vector<std::pair<std::pair<int, int>, int> > sorted;
  for (int i = 0; i < squares.size(); ++i)
  {
    cv::Point2f centre(0, 0);
    for (int j=0;j<squares[i].size();j++) centre += squares[i][j];

    centre *= 1.0 / squares[i].size();
    sortCorners(squares[i], centre);
    centres.push_back(centre);
    sorted.push_back(std::make_pair(std::make_pair(centre.x, centre.y), i));
    std::cout<<sorted[i].first.first<<" "<<sorted[i].first.second<<std::endl;
    
  }

  // sort squares
  std::stable_sort(sorted.begin(), sorted.end(),sortByPosY);

  for(int i=1;i<sorted.size();i++)
  {
  		if(abs(sorted[i].first.second-sorted[i-1].first.second)<=3)
  		{
  			sorted[i].first.second=sorted[i-1].first.second;
  		}	
  		 //std::cout<<sorted[i].first.first<<" "<<sorted[i].first.second<<" "<<sorted[i].second<<std::endl;
  		
  }


  std::stable_sort(sorted.begin(), sorted.end(),sortByPosX);


  for(int i=1;i<sorted.size();i++)
  {
  		if(abs(sorted[i].first.first-sorted[i-1].first.first)<=3)
  		{
  			sorted[i].first.first=sorted[i-1].first.first;
  		}	
  		 //std::cout<<sorted[i].first.first<<" "<<sorted[i].first.second<<" "<<sorted[i].second<<std::endl;
  		
  }

std::stable_sort(sorted.begin(), sorted.end(),sortByPosY);
std::stable_sort(sorted.begin(), sorted.end(),sortByPosX);

for(int i=0;i<sorted.size();i++)
  {
  			
  		 std::cout<<sorted[i].first.first<<" "<<sorted[i].first.second<<" "<<sorted[i].second<<std::endl;
  		
  }

  std::vector<std::vector<cv::Mat> > table(rows, std::vector<cv::Mat>(columns));
  for (int i = 0; i < rows; ++i)
    for (int j = 0; j < columns; ++j)
    {
      int index = sorted[i * columns + j].second;
      int square_size = std::min(image.size().height, image.size().width) / 4;
      cv::Mat result = cv::Mat(square_size, square_size, CV_8UC3);

      std::vector<cv::Point2f> result_points;
      result_points.push_back(cv::Point2f(0, 0));
      result_points.push_back(cv::Point2f(result.cols, 0));
      result_points.push_back(cv::Point2f(result.cols, result.rows));
      result_points.push_back(cv::Point2f(0, result.rows));

      cv::Mat transformation_matrix =
        cv::getPerspectiveTransform(squares[index], result_points);

      cv::warpPerspective(image, result, transformation_matrix, result.size());
      table[i][j] = result;
    }
  return table;
}

std::string extractLetters(cv::Mat square)
{
 cv::imshow("2",square);
  cv::Mat pyr, timg;
  // down-scale and upscale the image to filter out the noise
  cv::pyrDown(square, pyr, cv::Size(square.cols/2, square.rows/2));
  cv::pyrUp(pyr, timg, square.size());
  timg = timg(cv::Rect(timg.size().width * 0.05, timg.size().height * 0.05, 
        timg.size().width * 0.9, timg.size().height * 0.9));

  cv::imshow("2.1",timg);
  cv::Mat single_channel(timg.size(), CV_8U);
  cv::cvtColor(timg,single_channel,CV_RGB2GRAY);
  /*int from_to[] = {2, 0};
  
  cv::mixChannels(&timg, 1, &single_channel, 1, from_to, 1);*/
  cv::imshow("2.2",single_channel);
  //cv::threshold(single_channel, single_channel, 230, 255, cv::THRESH_BINARY_INV);
  cv::imshow("2.3",single_channel);
  cv::Mat bw;
  cv::Canny(single_channel, bw, 0, 50, 3);
  
  cv::imshow("2.5",bw);

  std::vector<std::vector<cv::Point> > contours;
  cv::findContours(bw, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
  std::vector<cv::Point> contour;
  for (int i=0;i<contours.size();i++)
  {
    cv::Rect bounding_rect = cv::boundingRect(cv::Mat(contours[i]));
    if (bounding_rect.height < square.size().height * 0.9 &&
        bounding_rect.height > square.size().height * 0.3)
      {for (int j=0;j<contours[i].size();j++)
        {contour.push_back(contours[i][j]);}}
  }
  if (contour.empty()) return "";

  cv::Rect bounds = cv::boundingRect(cv::Mat(contour));
  cv::Rect new_bounds;
  new_bounds.x = 0;
  new_bounds.width = timg.size().width;
  new_bounds.y = std::max(bounds.y - bounds.height * 0.1, 0.0);
  //new_bounds.y=0;

  new_bounds.height = std::min(bounds.height + bounds.height * 0.2,
      (double)timg.size().height - new_bounds.y);
  //new_bounds.height =timg.size().height;
  cv::Mat letters = timg(new_bounds).clone();
  cv::Mat letters_single_channel(letters.size(), CV_8U);
  letters_single_channel=letters.clone();
  //std::cout<<letters.channels()<<std::endl;
  //
  cv::imshow("2.6",letters_single_channel);
  //std::cout<<letters_single_channel.channels()<<std::endl;
  //int from_to[] = {0, 0};
  //cv::mixChannels(&letters, 1, &letters_single_channel, 1, from_to, 1);
  cv::threshold(letters_single_channel, letters_single_channel,180, 255, cv::THRESH_BINARY_INV);
  cv::imshow("3",letters_single_channel);
  //std::cout<<letters_single_channel.channels()<<std::endl;
  cv::cvtColor(letters_single_channel,letters_single_channel,CV_RGB2GRAY);
  cv::imshow("3.1",letters_single_channel);
  cv::rectangle(letters_single_channel, cv::Point(0,0),
      cv::Point(bounds.x, timg.size().height), cv::Scalar(255,255,255), CV_FILLED);
  cv::rectangle(letters_single_channel, cv::Point(bounds.x+bounds.width,0),
      cv::Point(timg.size().width, timg.size().height), cv::Scalar(255,255,255), CV_FILLED);

  ;cv::bitwise_not(letters_single_channel,letters_single_channel);
  //char ch[10];
      	
  char ch[10];
      	ch[0]=count_q++ +'a';
      	ch[1]='.';
      	ch[2]='j';
      	ch[3]='p';
      	ch[4]='g';
      	//cv::imwrite(ch,images_table[i][j]);
      	cv::imshow(ch,letters_single_channel);

  tesseract::TessBaseAPI tess;
  setlocale (LC_NUMERIC, "C");
  tess.Init(NULL, "eng");
  tess.SetVariable("tessedit_char_whitelist", "ABCDEFGHIiJKLMNOPQRSTUVWXYZ/-");
  tess.SetImage((uchar*)letters_single_channel.data,
      letters_single_channel.cols, letters_single_channel.rows, 1,
      letters_single_channel.cols);
  std::string str;
  char* out = tess.GetUTF8Text();
  //BUG;
  //std::cout<<out<<std::endl;
  for (int i = 0; out[i] != '\0'; ++i)
    if (isalpha(out[i]) || out[i] == '/')
      {
      	str.push_back(out[i]);
      	
      }

  if (str == "" && bounds.width < bounds.height * 0.3)
      str = "-";
  return str;
}



int main(int argc, char *argv[])
{
	cv::Mat frame;
	
	//loading the image in RGB format
	frame=cv::imread(argv[1],1);
	cv::resize(frame, frame, cv::Size(350, 550), 0, 0, CV_INTER_CUBIC);
	std::vector<std::vector<std::string> > table(rows,
      std::vector<std::string>(columns));

	std::vector<std::vector<cv::Point2f> > squares = findSquares(frame);
	BUG;
    std::vector<std::vector<cv::Mat> > images_table = cutSquares(frame, squares);

    if (images_table.empty())
    {
      cv::imshow(window_name, frame);
      //continue;
    }

    for (int i = 0; i < images_table.size(); ++i)
      {for (int j = 0; j < images_table[i].size(); ++j)
      {
      	char ch[10];
      	ch[0]=i*4+j+'a';
      	ch[1]='.';
      	ch[2]='j';
      	ch[3]='p';
      	ch[4]='g';
      	cv::imwrite(ch,images_table[i][j]);
      	// /cv::imshow(ch,images_table[i][j]);
        table[i][j] = extractLetters(images_table[i][j]);
        std::cout<<table[i][j];
        cv::putText(frame, table[i][j], cv::Point(50*(i+1), 50*(j+1)), cv::FONT_HERSHEY_SIMPLEX, 0.5, CV_RGB(255, 0, 0));
        //BUG;
      }
    cv::imshow(window_name, frame);
  }
  // Processing the table
  //getWords(table, trie);
  cv::waitKey(0);
	return 0;

}