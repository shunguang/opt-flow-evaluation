-include Makefile.inc

PROJ_NAME=test

include Makefile_app_header.mak


#the target binary name
TARGETFILE=$(ODIR_BIN)/test.out

#include flags
CFLAGS  := -Wall -c $(DEBUG) -DqDNGDebug=1 -D__xlC__=1 -DNO_FCGI_DEFINES=1 -DqDNGUseStdInt=0 -DUNIX_ENV=1 -D__LITTLE_ENDIAN__=1 -DqMacOS=0 -DqWinOS=0 -std=gnu++11 \
        -I$(SDIR_PROJ) -I$(SDIR_ROOT) -I$(CUDA_INC) $(I_GST_INC) -I$(JETSON_UTIL_INC) -I$(CV_INC) -I$(BOOST_INC) -I$(PLTF_INC)

#link flags and lib searching paths
LFLAGS  := -Wall $(DEBUG) -L$(CV_LIB) -L$(ODIR_LIB) -L$(GST_LIB) -L$(BOOST_LIB) -L$(CUDA_LIB) -L/usr/lib/aarch64-linux-gnu/tegra -L/usr/lib/aarch64-linux-gnu -L$(PLTF_LIB)

#-L/usr/lib/aarch64-linux-gnu/tegra

# link libs
LIBS    := -lUtil \
        -lboost_timer -lboost_filesystem -lboost_system -lboost_date_time -lboost_regex \
        -lboost_chrono -lpthread -lboost_thread \
        -lnvvpi \
        -lopencv_cudaarithm -lopencv_cudaoptflow -lopencv_cudaimgproc \
        -lopencv_highgui -lopencv_video -lopencv_videoio  -lopencv_imgcodecs -lopencv_imgproc -lopencv_core \
        -lgthread-2.0 -lgstvideo-1.0 -lgstbase-1.0 -lgstreamer-1.0 -lgobject-2.0 -lglib-2.0 -lgstapp-1.0 -lglib-2.0 -lpng -lz -lv4l2 \
        -lnppc_static -lnppif_static -lnppig_static -lnppial_static -lnppicc_static -lnppisu_static -lnppidei -lculibos -lcublas_static -lcudart_static \
        -lv4l2 -lEGL -lGLESv2 -lX11 -lnvbuf_utils -lnvjpeg \
        -lavcodec -lavformat -lavutil -lswresample -lswscale -llzma -ldl -lm -lpthread -lrt

#        -ljetson-utils -lncurses\
#        -lopencv_ml -lopencv_shape -lopencv_video -lopencv_calib3d -lopencv_features2d -lopencv_flann \
#        -lopencv_stitching -lopencv_superres -lopencv_videostab \
#       -lavcodec -lavformat -lavutil -lswresample -lswscale -llzma -ldl -lm -lpthread -lrt
#       -lopencv_cudaarithm -lopencv_cudaimgproc -lopencv_cudafeatures2d -lopencv_cudawarping \

OBJS = \
        $(ODIR_OBJ)/test_of_cv_cpu_timing.o \
        $(ODIR_OBJ)/test_of_cv_gpu_timing.o \
        $(ODIR_OBJ)/test_of_nv_vpi_timing.o \
        $(ODIR_OBJ)/test_of_cv_cpu_1frm.o \
        $(ODIR_OBJ)/test_of_cv_gpu_1frm.o \
        $(ODIR_OBJ)/test_of_nv_vpi_1frm.o \
        $(ODIR_OBJ)/test_of_cv_cpu_multi_frm.o \
        $(ODIR_OBJ)/test_of_cv_gpu_multi_frm.o \
        $(ODIR_OBJ)/test_of_nv_vpi_multi_frm.o \
        $(ODIR_OBJ)/test_utils.o \
        $(ODIR_OBJ)/test_cal_feature_pts_and_save.o \
        $(ODIR_OBJ)/main.o



#       $(ODIR_OBJ)/test_mux_enc.o \

default:  directories $(TARGETFILE)

directories:
	mkdir -p $(ODIR_OBJ)
	mkdir -p $(ODIR_LIB)
	mkdir -p $(ODIR_BIN)

#the output binary file name is <$(TARGETFILE)>
$(TARGETFILE)   :       $(OBJS)
	$(CXX) $(LFLAGS) $(OBJS) $(LIBS) $(LIBS) -o $(TARGETFILE)


$(ODIR_OBJ)/main.o      :       $(SDIR_PROJ)/main.cpp
	$(CXX) -o $(ODIR_OBJ)/main.o $(CFLAGS) $(SDIR_PROJ)/main.cpp

$(ODIR_OBJ)/test_of_cv_cpu_timing.o     :       $(SDIR_PROJ)/test_of_cv_cpu_timing.cpp
	$(CXX) -o $(ODIR_OBJ)/test_of_cv_cpu_timing.o $(CFLAGS) $(SDIR_PROJ)/test_of_cv_cpu_timing.cpp
$(ODIR_OBJ)/test_of_cv_gpu_timing.o     :       $(SDIR_PROJ)/test_of_cv_gpu_timing.cpp
	$(CXX) -o $(ODIR_OBJ)/test_of_cv_gpu_timing.o $(CFLAGS) $(SDIR_PROJ)/test_of_cv_gpu_timing.cpp
$(ODIR_OBJ)/test_of_nv_vpi_timing.o       :       $(SDIR_PROJ)/test_of_nv_vpi_timing.cpp
	$(CXX) -o $(ODIR_OBJ)/test_of_nv_vpi_timing.o $(CFLAGS) $(SDIR_PROJ)/test_of_nv_vpi_timing.cpp

$(ODIR_OBJ)/test_of_cv_cpu_1frm.o     :       $(SDIR_PROJ)/test_of_cv_cpu_1frm.cpp
	$(CXX) -o $(ODIR_OBJ)/test_of_cv_cpu_1frm.o $(CFLAGS) $(SDIR_PROJ)/test_of_cv_cpu_1frm.cpp
$(ODIR_OBJ)/test_of_cv_gpu_1frm.o     :       $(SDIR_PROJ)/test_of_cv_gpu_1frm.cpp
	$(CXX) -o $(ODIR_OBJ)/test_of_cv_gpu_1frm.o $(CFLAGS) $(SDIR_PROJ)/test_of_cv_gpu_1frm.cpp
$(ODIR_OBJ)/test_of_nv_vpi_1frm.o       :       $(SDIR_PROJ)/test_of_nv_vpi_1frm.cpp
	$(CXX) -o $(ODIR_OBJ)/test_of_nv_vpi_1frm.o $(CFLAGS) $(SDIR_PROJ)/test_of_nv_vpi_1frm.cpp

$(ODIR_OBJ)/test_of_cv_cpu_multi_frm.o       :       $(SDIR_PROJ)/test_of_cv_cpu_multi_frm.cpp
	$(CXX) -o $(ODIR_OBJ)/test_of_cv_cpu_multi_frm.o $(CFLAGS) $(SDIR_PROJ)/test_of_cv_cpu_multi_frm.cpp
$(ODIR_OBJ)/test_of_cv_gpu_multi_frm.o       :       $(SDIR_PROJ)/test_of_cv_gpu_multi_frm.cpp
	$(CXX) -o $(ODIR_OBJ)/test_of_cv_gpu_multi_frm.o $(CFLAGS) $(SDIR_PROJ)/test_of_cv_gpu_multi_frm.cpp
$(ODIR_OBJ)/test_of_nv_vpi_multi_frm.o       :       $(SDIR_PROJ)/test_of_nv_vpi_multi_frm.cpp
	$(CXX) -o $(ODIR_OBJ)/test_of_nv_vpi_multi_frm.o $(CFLAGS) $(SDIR_PROJ)/test_of_nv_vpi_multi_frm.cpp


$(ODIR_OBJ)/test_utils.o :       $(SDIR_PROJ)/test_utils.cpp
	$(CXX) -o $(ODIR_OBJ)/test_utils.o $(CFLAGS) $(SDIR_PROJ)/test_utils.cpp

$(ODIR_OBJ)/test_cal_feature_pts_and_save.o :       $(SDIR_PROJ)/test_cal_feature_pts_and_save.cpp
	$(CXX) -o $(ODIR_OBJ)/test_cal_feature_pts_and_save.o $(CFLAGS) $(SDIR_PROJ)/test_cal_feature_pts_and_save.cpp

clean:
	\rm $(ODIR_OBJ)/*.o $(TARGETFILE)

rm_test:
	\rm $(TARGETFILE)
