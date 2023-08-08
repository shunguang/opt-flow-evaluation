#this Makefile outputs image as a static lib for <PLTF> machine

PROJ_NAME=libUtil
include Makefile_app_header.mak

OBJS = \
	$(ODIR_OBJ)/AppLog.o \
	$(ODIR_OBJ)/AppUtil.o \
	$(ODIR_OBJ)/CapImg.o \
	$(ODIR_OBJ)/Cfg.o \
	$(ODIR_OBJ)/OptFlowVis.o \
	$(ODIR_OBJ)/vpiUtils.o
	
default:  directories $(TARGETFILE)

directories:    
	mkdir -p $(ODIR_ROOT)
	mkdir -p $(ODIR_OBJ)
	mkdir -p $(ODIR_LIB)


#the output lib file name is <$(TARGETFILE)>
$(TARGETFILE) : $(OBJS)
	ar rcs $(TARGETFILE) $(OBJS)

$(ODIR_OBJ)/AppLog.o: $(SDIR_PROJ)/AppLog.cpp $(SDIR_PROJ)/AppLog.h
	$(CXX) -o $(ODIR_OBJ)/AppLog.o $(CFLAGS) $(SDIR_PROJ)/AppLog.cpp

$(ODIR_OBJ)/CapImg.o: $(SDIR_PROJ)/CapImg.cpp $(SDIR_PROJ)/CapImg.h
	$(CXX) -o $(ODIR_OBJ)/CapImg.o $(CFLAGS) $(SDIR_PROJ)/CapImg.cpp

$(ODIR_OBJ)/Cfg.o: $(SDIR_PROJ)/Cfg.cpp $(SDIR_PROJ)/Cfg.h
	$(CXX) -o $(ODIR_OBJ)/Cfg.o $(CFLAGS) $(SDIR_PROJ)/Cfg.cpp

$(ODIR_OBJ)/OptFlowVis.o: $(SDIR_PROJ)/OptFlowVis.cpp $(SDIR_PROJ)/OptFlowVis.h
	$(CXX) -o $(ODIR_OBJ)/OptFlowVis.o $(CFLAGS) $(SDIR_PROJ)/OptFlowVis.cpp

$(ODIR_OBJ)/vpiUtils.o: $(SDIR_PROJ)/vpiUtils.cpp $(SDIR_PROJ)/vpiUtils.h
	$(CXX) -o $(ODIR_OBJ)/vpiUtils.o $(CFLAGS) $(SDIR_PROJ)/vpiUtils.cpp

$(ODIR_OBJ)/AppUtil.o: $(SDIR_PROJ)/AppUtil.cpp $(SDIR_PROJ)/AppUtil.h
	$(CXX) -o $(ODIR_OBJ)/AppUtil.o $(CFLAGS) $(SDIR_PROJ)/AppUtil.cpp

clean:
	\rm -r $(ODIR_OBJ)/*.o $(TARGETFILE)

