#include<opencv2/opencv.hpp>
#include "CameraWrapper.h"


//C++ Mat ->numpy
py::array_t<unsigned char> cv_mat_uint8_3c_to_numpy(cv::Mat& input) {
    py::array_t<unsigned char> dst = py::array_t<unsigned char>({ input.rows,input.cols,3}, input.data);
    return dst;
}

py::array_t<unsigned short> cv_mat_uint16_1c_to_numpy(cv::Mat& input) {
    py::array_t<unsigned short> dst = py::array_t<unsigned short>({ input.rows,input.cols}, 
    															(unsigned short*)input.data);
    return dst;
}


CameraWrapper::~CameraWrapper() {
	int nRet = pD2CSwapper->StreamStop();
	printf("stop stream :%d\n", nRet);
	nRet = pD2CSwapper->CameraUnInit();
	printf("UvcDeInit :%d\n", nRet);

	//
	if (pD2CSwapper)
	{
		delete pD2CSwapper;
		pD2CSwapper = NULL;
	}
}

CameraWrapper::CameraWrapper() {
	nearest = 400.;
	fars = 8000.;
	pD2CSwapper = new AstraD2C();
	int nRet = pD2CSwapper->CameraInit(SOFTWARE_D2C);
	if (nRet != CAMERA_STATUS_SUCCESS)
	{
		printf("camera init failed\n");
		exit(0);
	}
	memset(&cameraParams, 0, sizeof(OBCameraParams));

	nRet = pD2CSwapper->GetCameraParam(cameraParams);
	if (nRet != 0)
	{
		//»ñÈ¡Ïà»úÄÚÍâ²ÎÊ§°Ü
		printf("get camera param failed\n");
		exit(0);
	}

	nRet = pD2CSwapper->GetCameraResolution(nWidth, nHeight);
	if (nRet != 0)
	{
		printf("get camera resolution fail,please check pid\n");
		exit(0);
	}

	// ³õÊ¿»¯²ÎÊý
	this->initParams();
	printf("Initialized done!");
}

void CameraWrapper::initParams() {
	cv_rgb = cv::Mat(IMAGE_HEIGHT_480, IMAGE_WIDTH_640, CV_8UC3);
	cv_depth = cv::Mat(IMAGE_HEIGHT_480, IMAGE_WIDTH_640, CV_16UC1);//640x480
	cv_aligned_depth = cv::Mat(IMAGE_HEIGHT_480, IMAGE_WIDTH_640, CV_16UC1);
	CaliDepthHistogram = cv::Mat(IMAGE_HEIGHT_480, IMAGE_WIDTH_640, CV_16UC1);
}

// OBCameraParams CameraWrapper::getDeviceParams() {
// 	return cameraParams;
// }

int CameraWrapper::D2C(const uint16_t* depth_buffer, int depth_width, int depth_height,
	uint16_t* out_depth, int color_width, int color_height) const {
	memset(out_depth, 0, sizeof(uint16_t)*color_width*color_height);
	float f_x = cameraParams.l_intr_p[0]; //uÖáÉÏµÄ¹éÒ»»¯¿¹¿à
	float f_y = cameraParams.l_intr_p[1]; //vÖáÉÏµÄ¹éÒ»»¯¿¹¿à
	float c_x = cameraParams.l_intr_p[2]; //Ö÷µãx×ø±ê
	float c_y = cameraParams.l_intr_p[3]; //Ö÷µãy×ø±ê

	float fdx = f_x * ((float)(depth_width) / RESOULTION_X);
	float fdy = f_y * ((float)(depth_height) / RESOULTION_Y);
	float u0 = c_x * ((float)(depth_width) / RESOULTION_X);
	float v0 = c_y * ((float)(depth_height) / RESOULTION_Y);

	cv::Mat Rot(3, 3, CV_32F);
	float* ptr = Rot.ptr<float>();
	for(int i = 0; i < 9; i++) {
		*ptr++ = cameraParams.r2l_r[i];
	}

	// get pointClouds data
	cv::Mat pointClouds(depth_width * depth_height, 3, CV_32F);
	float* pc = pointClouds.ptr<float>();
	for (int r = 0; r < depth_height; r++) {
		for (int c = 0; c < depth_width; c++) {
			int bias = r * depth_width + c;

			float tx = (c - u0) / fdx;
			float ty = (r - v0) / fdy;

			uint16_t depth = depth_buffer[bias];
			*pc++ = depth * tx + cameraParams.r2l_t[0];
			*pc++ = depth * ty + cameraParams.r2l_t[1];
			*pc++ = depth + cameraParams.r2l_t[2];
		}
	}
	// transfer depth coordinates to color coordinates
	pointClouds = (Rot * pointClouds.t()).t();
	// gen outdepth
	f_x = cameraParams.r_intr_p[0]; //uÖáÉÏµÄ¹éÒ»»¯¿¹¿à
	f_y = cameraParams.r_intr_p[1]; //vÖáÉÏµÄ¹éÒ»»¯¿¹¿à
	c_x = cameraParams.r_intr_p[2]; //Ö÷µãx×ø±ê
	c_y = cameraParams.r_intr_p[3]; //Ö÷µãy×ø±ê

	fdx = f_x * ((float)(color_width) / RESOULTION_X);
	fdy = f_y * ((float)(color_height) / RESOULTION_Y);
	u0 = c_x * ((float)(color_width) / RESOULTION_X);
	v0 = c_y * ((float)(color_height) / RESOULTION_Y);

	for (int r = 0; r < color_height; r++) {
		for (int c = 0; c < color_width; c++) {
			int bias = r * color_width + c;
			float wx = pointClouds.at<float>(bias, 0);
			float wy = pointClouds.at<float>(bias, 1);
			float wz = pointClouds.at<float>(bias, 2);
			int v = (wy * fdy) / wz + v0;
			int u = (wx * fdx) / wz + u0;\
			if (v<0 || v > color_height - 1 || u<0 || u>color_width || wz < nearest || wz > fars) {
				continue;
			}
			int bb = v * color_width + u;
			unsigned int tmp = *(out_depth+bb);
			if (tmp > 0) {
				*(out_depth + bb) = min(tmp, (unsigned int)wz);
			}
			else {
				*(out_depth + bb) = wz;
			}
		}
	}
	return 0;
}

void CameraWrapper::getColorDepthData() {
	int nRet = pD2CSwapper->GetStreamData(cv_rgb, cv_depth);
	//int ret = d2c.D2C(cv_depth.ptr<uint16_t>(), cv_depth.cols, cv_depth.rows,
	//	cv_aligned_depth.ptr<uint16_t>(), cv_rgb.cols, cv_rgb.rows);
	int ret = this->D2C(cv_depth.ptr<uint16_t>(), cv_depth.cols, cv_depth.rows,
		cv_aligned_depth.ptr<uint16_t>(), cv_rgb.cols, cv_rgb.rows);

	if (ret)
	{
		fprintf(stderr, "D2C failed!\n");
		exit(0);
	}
	cv::flip(cv_aligned_depth, cv_aligned_depth, 1);

	mDraw.GetDepthHistogram(cv_aligned_depth, CaliDepthHistogram);
	Mat imgROI = cv_rgb(Rect(0, 0, CaliDepthHistogram.cols, CaliDepthHistogram.rows)).clone();
	cv::addWeighted(imgROI, 0.5, CaliDepthHistogram, 0.5, 0.0, CaliDepthHistogram);
}

py::array_t<unsigned char>  CameraWrapper::getColorImgNM() {
	return cv_mat_uint8_3c_to_numpy(cv_rgb);
}

py::array_t<unsigned short> CameraWrapper::getDepthImgNM() {
	return cv_mat_uint16_1c_to_numpy(cv_aligned_depth);
}

py::array_t<unsigned char> CameraWrapper::getHistImgNM() {
	return cv_mat_uint8_3c_to_numpy(CaliDepthHistogram);
}

void CameraWrapper::show() {
	vector<IplImage> imgsTwo(2);
	IplColor = cv_rgb;
	IplReg = CaliDepthHistogram;
	imgsTwo[0] = IplColor;
	imgsTwo[1] = IplReg;
	mDraw.ShowImagesSideBySide((char*)"demo", imgsTwo, "", 10, 450);
}

void CameraWrapper::setNearFar(double N, double F){
	nearest = N;
	fars = F;
	printf("set nearest %f and fars %f", N, F);
}


PYBIND11_MODULE(camerawrapper, m) {
    py::class_<CameraWrapper>(m, "CameraWrapper")
        .def(py::init<>())
        .def("setNearFar", &CameraWrapper::setNearFar)
        .def("getColorDepthData", &CameraWrapper::getColorDepthData)
        .def("getColorImgNM", &CameraWrapper::getColorImgNM)
        .def("getDepthImgNM", &CameraWrapper::getDepthImgNM)
        .def("getHistImgNM", &CameraWrapper::getHistImgNM)
        .def("show", &CameraWrapper::show);
}

// extern "C"{
// 	CameraWrapper obj;
// 	void setNearFar(int N, int F){
// 		obj.setNearFar(N, F);
// 	}
// 	OBCameraParams getDeviceParams(){
// 		return obj.getDeviceParams();
// 	}
// 	void initParams(){
// 		obj.initParams();
// 	}
// 	void getColorDepthData(){
// 		obj.getColorDepthData();
// 	}
// 	int D2C(const uint16_t* depth_buffer, int depth_width, int depth_height,
// 		uint16_t* out_depth, int color_width, int color_height){
// 		return obj.D2C(depth_buffer, depth_width, depth_height, out_depth, color_width, color_height);
// 	}
// 	cv::Mat getColorImg(){
// 		return obj.getColorImg();
// 	}
// 	cv::Mat getDepthImg(){
// 		return obj.getDepthImg();
// 	}
// 	cv::Mat getHistImg(){
// 		return obj.getHistImg();
// 	}

// 	void show(){
// 		obj.show();
// 	}
// }
