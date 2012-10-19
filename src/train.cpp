#include "../headers/train.hpp"

using namespace ttrk;




void Train::LoadImages(const std::vector<std::string> &image_urls, const std::vector<std::string> &mask_urls, const LoadType type){

  //for each image
  for(size_t im=0;im<image_urls.size();im++){

    //load the image and the mask
    cv::Mat image = cv::imread(image_urls[im]);
    if(type == POSITIVE || type == BOTH)
      cv::Mat mask = cv::imread(mask_urls[im]);
    else //type == NEGATIVE
      cv::Mat mask = cv::Mat::zeros(image.size(),image.type());

    //create the ndimage and load its pixels to the training data mask
    try{

      nd_image_ = new NDImage(image);
      LoadPixels(nd_image_,mask,type);

    }catch(std::runtime_error &e){
      std::cerr << "Error, image " << image_urls[im] << " failed to load.\n" << e.what() << "\n";
      exit(1); //cannot continue as training matrix will be messed up
    }catch(std::bad_alloc &e){
      std::cerr << "Error allocating memory for NDImage.\n" << e.what() << "\n";
      exit(1);
    }

    delete nd_image_;
  
  }

}




void Train::LoadPixels(const NDImage *nd_image_, const cv::Mat &mask, const LoadType type){

  //check data
  if(nd_image_->bad())
    throw std::runtime_error("Error, image data was corrupt/null");
  if(mask.data == 0)
    throw std::runtime_error("Error, mask data was corrupt/null");

  //check size
  const int rows = nd_image_->rows();
  const int cols = nd_image_->cols();
  if(mask.rows != rows || mask.cols != cols)
    throw std::runtime_error("Error, image and mask have different dimensions");

  const unsigned char *mask_ptr = reinterpret_cast<const unsigned char *>(mask.data);  
  const int chans = mask.channels();

  //keep current index of training/responses matrix
  static int count=0;
  
  for(int r=0;r<rows;r++){
    for(int c=0;c<cols;c++){

      //copy the labels      
      const int index = (r * cols) + c;
      unsigned char mask_val = mask_ptr[index*chans];
      const cv::Mat &tmp = nd_image_->GetPixelData(r,c);
      
      //if positive image, just copy the positive pixels to the training data
      if(type == POSITIVE){
        if(mask_val > 127){
          training_labels_.at<size_t>(count,0) = 1;
          for(int i=0;i<training_data_.cols;i++)
            training_data_.at<float>(count,i) = tmp.at<float>(0,i);
          count++;
        }
      }
      
      if(type == NEGATIVE){
        for(int i=0;i<training_data_.cols;i++)
          training_data_.at<float>(count,i) = tmp.at<float>(0,i);
        training_labels_.at<size_t>(count,0) = 0;      
        count++;
      }

      if(type == BOTH){
        for(int i=0;i<training_data_.cols;i++)
          training_data_.at<float>(count,i) = tmp.at<float>(0,i);
        training_labels_.at<size_t>(count,0) = mask_val > 127;
        count++;
      }
  
      
    }
  }

#ifdef DEBUG
  assert(count <= training_labels_.rows && count <= training_data_.rows);
#endif

}

void Train::TrainClassifier(){

  // train
  const float priors[2] = {3.0,1.0};

  CvRTParams params(10, //max depth of trees
                    500, //minimum sample count at each leaf for a split
                    0.0, //minimum regression accuracy (ignored)
                    false, //use surrogates
                    10, //maximum number of categories to cluster - ignored in 2 class case
                    priors, //priors
                    true, //calculate the variable importance
                    0, //size of random subsets (0 = sqrt(N))
                    50, //max number of trees
                    0.01, //accuracy
                    CV_TERMCRIT_ITER | CV_TERMCRIT_EPS); //halting criteria


  CvMat *var_type = cvCreateMat(training_data_.cols+1,1,CV_8U);
  cvSet(var_type,cvScalarAll(CV_VAR_ORDERED)); //data is ordered (can be compared)
  cvSetReal1D(var_type,training_data_.cols,CV_VAR_CATEGORICAL); //labels are categorical
  cv::Mat var_type_(var_type,true);

#ifdef DEBUG
  std::cout << "Training...";
  std::cout.flush();
#endif

  classifier_.train(training_data_,
                    CV_ROW_SAMPLE, //samples are in row form
                    training_labels_,
                    cv::Mat(),//variable index, used to mask certain features from the training
                    cv::Mat(),//sample index, used to mask certain samples entirely
                    var_type,//variable type (regression or classifiaction)
                    cv::Mat(),//missing data mask
                    params);
               
   
#ifdef DEBUG
  std::cout << " Done" << std::endl;
#endif
  
  std::string classifier_save_path = root_dir_ + "/classifier/";

  boost::filesystem::create_directory(boost::filesystem::path(classifier_save_path));

  classifier_.save( (classifier_save_path + "forest.xml").c_str());
 
  cvReleaseMat(&var_type);

}



void Train::LoadTrainingData(){

  // set up directories
  const std::string positive_im_dir(root_dir_ + "/data/positive_data/training_images/");
  const std::string positive_mask_dir(root_dir_ + "/data/positive_data/masks/");
  const std::string negative_im_dir(root_dir_ + "/data/negative_data/");
  
  //vectors to store the urls
  std::vector<std::string> positive_ims;
  std::vector<std::string> negative_ims;
  std::vector<std::string> positive_masks;
  size_t num_pix=0;
  
  //get the file urls 
  GetImageURL(positive_im_dir,positive_ims);
  GetImageURL(negative_im_dir,negative_ims);
  GetImageURL(positive_mask_dir,positive_masks);

  //get the training sizes, true means just count the positive pixels false means count whole image
  GetTrainingSize(positive_masks,num_pix,true);
  GetTrainingSize(negative_ims,num_pix,false);
  
  //preallocate training data storage
  training_data_ = cv::Mat(cv::Size(NDImage::channels_,num_pix),CV_32FC1);
  training_labels_ = cv::Mat(cv::Size(1,num_pix),CV_32SC1);

  //load the data
  LoadImages(positive_ims, positive_masks, POSITIVE );
  LoadImages(negative_ims, std::vector<std::string>(), NEGATIVE );

}
