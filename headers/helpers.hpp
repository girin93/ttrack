#ifndef _HELPERS_HPP_
#define _HELPERS_HPP_
#include <string>
#include <vector>
#include "headers.hpp"
#include <unordered_map>

namespace ttrk{

  /**
  * Computes a smoothed heaviside function.
  * @param[in] x The value on the x-axis.
  * @param[in] a The smoothing value.
  * @ The smoothed heaviside value. Will be between 0-1.
  */

  inline double Heaviside(double x, double a=0.4){
    return 1.0/(1.0 + exp(-a*x));
  }


  inline int round(double r){

    return (r > 0.0) ? (int)floor(r + 0.5) : (int)ceil(r - 0.5);

  }



  inline void SAFE_EXIT(){
  #ifdef __linux__
    exit(1);
  #elif defined (_WIN32) || (_WIN64)
    system("pause");
    exit(1);
  #else
    exit(1);
  #endif
  
  }

  inline bool IS_IMAGE(const std::string &extension){
    return (extension == ".png" || extension == ".jpg");
  }

  cv::Mat &ConvertMatSingleToTriple(cv::Mat &im);

  struct hash_Vec{
    size_t operator()(const cv::Vec3b &b) const{
      return std::hash<int>()((int)b[0]) ^ std::hash<int>()((int)b[1]) ^ std::hash<int>()((int)b[2]);
    }
  };


  /** 
  * Wrap allows an iteration of a list/array index with wrapping so when it reaches
  * the max/min value you don't go out of bounds, just back to the start.
  * @param index The index you want to wrap.
  * @param lowerbound The min index.
  * @param upperbound  The max index
  * @return The wrapped index
  */
  inline int Wrap(int x, const int lowerbound, const int upperbound){
    int range_size = upperbound - lowerbound + 1;

    if (x < lowerbound)
      x += range_size * ((lowerbound - x) / range_size + 1);

    return lowerbound + (x - lowerbound) % range_size;
  }

}


#endif
