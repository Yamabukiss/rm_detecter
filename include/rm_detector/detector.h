//
// Created by yamabuki on 2022/4/18.
//
#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>
#include <ros/ros.h>
#include <cv_bridge/cv_bridge.h>
#include <string>
#include <vector>
#include "std_msgs/Float32MultiArray.h"
#include "dynamic_reconfigure/server.h"
#include "rm_detector/dynamicConfig.h"
#include "sensor_msgs/CameraInfo.h"
#include "nodelet/nodelet.h"
#include <pluginlib/class_list_macros.h>
#include <fstream>
#include <sstream>
#include <numeric>
#include <chrono>
#include <dirent.h>
#include "NvInfer.h"
#include "cuda_runtime_api.h"
#include "logging.h"

struct Object
{
  cv::Rect_<float> rect;
  int label;
  float prob;
};

struct GridAndStride
{
  int grid0;
  int grid1;
  int stride;
};

namespace rm_detector
{
class Detector : public nodelet::Nodelet
{
public:
  Detector();

  virtual ~Detector();

  ros::NodeHandle nh_;

  void onInit() override;

  void receiveFromCam(const sensor_msgs::ImageConstPtr& image);

  void staticResize(cv::Mat& img);

  float* blobFromImage(cv::Mat& img);

  void generateGridsAndStride(const int target_w, const int target_h);

  void generateYoloxProposals(std::vector<GridAndStride> grid_strides, const float* feat_ptr, float prob_threshold,
                              std::vector<Object>& proposals);

  inline float intersectionArea(const Object& a, const Object& b);

  void qsortDescentInplace(std::vector<Object>& faceobjects, int left, int right);

  void qsortDescentInplace(std::vector<Object>& proposals);

  void nmsSortedBboxes(const std::vector<Object>& faceobjects, std::vector<int>& picked, float nms_threshold);

  void decodeOutputs(const float* prob, const int img_w, const int img_h);


  void drawObjects(const cv::Mat& bgr);

  void mainFuc(cv_bridge::CvImagePtr& image_ptr);

  void initalizeInfer();

  void dynamicCallback(rm_detector::dynamicConfig& config);

  void doInference(nvinfer1::IExecutionContext& context, float* input, float* output, const int output_size,
                   cv::Size input_shape);

  cv_bridge::CvImagePtr cv_image_;
  std::vector<GridAndStride> grid_strides_;
  float nms_thresh_;
  float bbox_conf_thresh_;
  std_msgs::Float32MultiArray roi_data_;
  std::vector<cv::Point2f> roi_point_vec_;
  cv::Point2f roi_data_point_r_;
  cv::Point2f roi_data_point_l_;
  cv::Mat_<float> discoeffs_;
  cv::Mat_<float> camera_matrix_;
  std::vector<float> discoeffs_vec_;
  std::vector<float> camera_matrix_vec_;
  std::vector<Object> objects_;
  std::string model_path_;
  int origin_img_w_;
  int origin_img_h_;
  float scale_;
  bool turn_on_image_;
  dynamic_reconfigure::Server<rm_detector::dynamicConfig> server_;
  dynamic_reconfigure::Server<rm_detector::dynamicConfig>::CallbackType callback_;
  std::string nodelet_name_;
  std::string camera_pub_name_;
  std::string roi_data1_name_;
  std::string roi_data2_name_;
  std::string roi_data3_name_;
  std::string roi_data4_name_;
  std::string roi_data5_name_;
  bool target_is_red_;
  bool target_is_blue_;
  cv::Mat roi_picture_;
  std::vector<cv::Mat> roi_picture_vec_;
  std::vector<cv::Mat> roi_picture_split_vec_;
  float ratio_of_pixels_;
  int counter_of_pixel_;
  int pixels_thresh_;
  std::vector<Object> filter_objects_;
  int binary_threshold_;
  float aspect_ratio_;
  char* trt_model_stream_{};
  nvinfer1::IRuntime* runtime_{};
  nvinfer1::ICudaEngine* engine_{};
  nvinfer1::IExecutionContext* context_{};
  float* prob_{};
  int output_size_;

private:
  ros::Publisher camera_pub_;
  ros::Publisher camera_pub2_;
  ros::Subscriber camera_sub_;
  std::vector<ros::Publisher> roi_data_pub_vec;
  ros::Publisher roi_data_pub1_;
  ros::Publisher roi_data_pub2_;
  ros::Publisher roi_data_pub3_;
  ros::Publisher roi_data_pub4_;
  ros::Publisher roi_data_pub5_;
};
}  // namespace rm_detector