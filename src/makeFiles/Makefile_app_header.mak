-include Makefile.inc

#-----------------------------------------------------------------------
# Todo: generic for different users
#
# env variables need to defined in /home/maven/.bashrc 
#
# export APP_ROOT=/home/maven/gst-ffmpeg-evaluation
# export CUDA_INC=/usr/local/cuda-10.0/targets/aarch64-linux/include
# export CUDA_LIB=/usr/local/cuda-10.0/targets/aarch64-linux/lib
# ...
#----------------------------------------------------------------------

#build intermediat output paths

APP_ROOT=/home/maven/wus1/opt-flow-evaluation
SDIR_ROOT=$(APP_ROOT)/src
SDIR_PROJ=$(APP_ROOT)/src/$(PROJ_NAME)

ODIR_ROOT=$(APP_ROOT)/build
ODIR_OBJ=$(ODIR_ROOT)/$(PROJ_NAME)
ODIR_LIB=$(ODIR_ROOT)/libs
ODIR_BIN=$(ODIR_ROOT)/bin

#include and lib paths of the platform
PLTF_INC=/usr/include
PLTF_LIB=/usr/lib
BOOST_INC=/usr/include
BOOST_LIB=/usr/lib

CV_INC=/usr/local/include/opencv4
CV_LIB=/usr/local/lib

CUDA_INC=/usr/local/cuda-11.4/targets/aarch64-linux/include
CUDA_LIB=/usr/local/cuda-11.4/targets/aarch64-linux/lib

JETSON_INFER_INC=/usr/local/include/jetson-inference
JETSON_UTIL_INC=/usr/local/include/jetson-utils
JETSON_LIB=/usr/local/lib

I_GST_INC=-I/usr/include/gstreamer-1.0 -I/usr/include/glib-2.0 -I/usr/lib/aarch64-linux-gnu/glib-2.0/include
GST_LIB=/usr/lib/aarch64-linux-gnu/gstreamer-1.0

CC = /usr/bin/gcc
CXX = /usr/bin/g++

#DEBUG = -g
DEBUG = -DNDEBUG -g
#DEBUG = -DDEBUG -g

#include flags
CFLAGS = -Wall -static -c $(DEBUG) -DqDNGDebug=1 -D__xlC__=1 -DNO_FCGI_DEFINES=1 -DqDNGUseStdInt=0 -DUNIX_ENV=1 -D__LITTLE_ENDIAN__=1 -DqMacOS=0 -DqWinOS=0 -std=gnu++11 \
	-I$(SDIR_PROJ) -I$(SDIR_ROOT) -I$(CUDA_INC) $(I_GST_INC) -I$(JETSON_UTIL_INC) -I$(CV_INC) -I$(BOOST_INC) -I$(PLTF_INC)

TARGETFILE=$(ODIR_LIB)/$(PROJ_NAME).a

$(info $$SDIR_PROJ is [${SDIR_PROJ}])
$(info $$ODIR_ROOT is [${ODIR_ROOT}])
$(info $$ODIR_OBJ is [${ODIR_OBJ}])
$(info $$ODIR_LIB is [${ODIR_LIB}])
