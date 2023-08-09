# opt-flow-evaluation (OFE)
       (8/8/23)
An open source project to evaluate the following three sparse optical flow algorithms
(1) cv::calcOpticalFlowPyrLK()
(2) cv::cuda::SparsePyrLKOpticalFlow()
(3) vpiSubmitOpticalFlowPyrLK()
In order to easily describe the following sections, let's define an env var:
***OFE_ROOT=~/opt-flow-evaluation***
## dataset
	There are three datasets at $(OFE_ROOT)/dataset folder: car, cars, and street. Each of them has two subfolders:fpt and img, fpt stands for feature points and img for images. For example:
	$(OFE_ROOT)/dataset/car/img has 100 png files named by their frame number, and 
	$(OFE_ROOT)/dataset/car/fpt includes the corresponding features points of each images precomputed by cv::goodFeaturesToTrack().
## Dependency
     (1) opencv
        CV_INC=/usr/local/include/opencv4
        CV_LIB=/usr/local/lib
	(2) boost
        BOOST_INC=/usr/include
        BOOST_LIB=/usr/lib		
	(3) cuda
        CUDA_INC=/usr/local/cuda-11.4/targets/aarch64-linux/include
        CUDA_LIB=/usr/local/cuda-11.4/targets/aarch64-linux/lib
	(4) jetson
	    JETSON_INFER_INC=/usr/local/include/jetson-inference
        JETSON_UTIL_INC=/usr/local/include/jetson-utils
        JETSON_LIB=/usr/local/lib
## How to build 
	   (1) edit $(OFE_ROOT)/src/makeFiles/Makefile_app_header.mak make sure the env vars match your platform
	   (2) go into $(OFE_ROOT)/src/makeFiles, then run
	      $make -f Makefile_util.mak
	   	  $make -f Makefile_test.mak
	   The building results will be at 	 $(OFE_ROOT)/build
## How to run
       (1) cfg files: there there three predefined configuration files at
           $(OFE_ROOT)/src/test/cfg
           you can edit them before runing it
       (2) to run it
           a) goto $(OFE_ROOT)/build/bin
           b) $./test.out ../../src/test/cfg/cfg-opt-flow-car.xml
## More info
   goto nvidia developer forum discussion page:  <https://forums.developer.nvidia.com/t/accuracy-of-nv-vpi-sparse-optical-flow-on-orin/262329>

   

       

	   	  
