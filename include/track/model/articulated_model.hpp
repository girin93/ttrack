#ifndef __ARTICULATED_MODEL_HPP__
#define __ARTICULATED_MODEL_HPP__
#include "model.hpp"
#include <cinder/Json.h>

namespace ttrk {


  struct ArticulatedNode  {

    typedef boost::shared_ptr<ArticulatedNode> Ptr;

    ci::Matrix44d GetTransform();
    void LoadData(ci::JsonTree &tree, ArticulatedNode::Ptr parent, const std::string &root_dir) ;
    void Rotate(const double angle) { articulation_.rotate(Axis(),angle); }
    ci::Vec3d Axis() { return axis_of_rotation_; }
    
    
    ci::Vec3d center_of_mass_;
    ci::Vec3d axis_of_rotation_;

    //bool direction_; (use negative angles)
    bool movable_;

    ci::Matrix44d	transform_; //const
    ci::Matrix44d articulation_;
    double min_angle_;
    double max_angle_;

    ArticulatedNode::Ptr parent_;
    std::vector<ArticulatedNode::Ptr> children_;

    ci::TriMesh model_;   

  };


  class ArticulatedTree {

  public:

    ArticulatedTree() : root_node_(new ArticulatedNode()) {}
    ArticulatedNode::Ptr RootNode() { return root_node_; }

  protected:

    ArticulatedNode::Ptr root_node_;
    
  };



   /**
  * @class MISTool
  * @brief A simple implementation of a MIS tool that is built out of primitives
  * Subclass this to create a simple MIS Tool 
  */
  class ArticulatedTool : public Model {

  public:

    /**
    * Construct a cylinder using its minimal parameters.
    * @param[in] model_parameter_file A datafile which contains the parameters of each articulated component in the model.
    */
    ArticulatedTool(const std::string &model_parameter_file);

  protected:

    void LoadFromJsonFile(const std::string &json_file);
    void ParseJsonTree(ci::JsonTree &jt, ArticulatedNode::Ptr node, ArticulatedNode::Ptr parent, const std::string &root_dir);

    boost::shared_ptr<ArticulatedTree> articulated_model_;


  };



  class IntuitiveSurgicalLND : public ArticulatedTool {

  public:

    IntuitiveSurgicalLND(const std::string &model_parameter_file) : ArticulatedTool(model_parameter_file) {
      body_ =  articulated_model_->RootNode()->children_[0];
      clasper_left_ = body_->children_[0];
      clasper_right_ = body_->children_[1];
    }


    void RotateHead(const double angle);
    void RotateClaspers(const double angle_1, const double angle_2);

  protected:

    ArticulatedNode::Ptr clasper_left_;
    ArticulatedNode::Ptr clasper_right_;
    ArticulatedNode::Ptr body_;

  };






}


#endif
