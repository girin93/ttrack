#include "../../../headers/track/stt/monocular_tool_tracker.hpp"
#include "../../../headers/track/pwp3d/mono_pwp3d.hpp"

using namespace ttrk;

MonocularToolTracker::MonocularToolTracker(const int radius, const int height, const std::string &calibration_filename):SurgicalToolTracker(radius,height),camera_(calibration_filename){
  localizer_.reset(new MonoPWP3D);
}

bool MonocularToolTracker::Init(){

  std::vector<std::vector<cv::Vec2i> >connected_regions;
  if(!FindConnectedRegions((frame_->Mat()),connected_regions)) return false;

  for(auto connected_region = connected_regions.cbegin(); connected_region != connected_regions.end(); connected_region++){

    KalmanTracker new_tracker;
    new_tracker.model_.reset( new MISTool(radius_,height_) );

    tracked_models_.push_back( new_tracker ); 

    Init2DPoseFromMOITensor(*connected_region);

  }

  return true;

}


void MonocularToolTracker::Init2DPoseFromMOITensor(const std::vector<cv::Vec2i> &connected_region){

  const cv::Vec2i center_of_mass = FindCenterOfMass(connected_region);

}

const cv::Vec2i MonocularToolTracker::FindCenterOfMass(const std::vector<cv::Vec2i> &connected_region) const {

  cv::Vec2i ret(0,0);
  return ret;

}





