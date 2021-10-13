
#include<pybind11/pybind11.h>
#include<pybind11/numpy.h>
#include<pybind11/stl.h>
#include "AstraD2C.h"
#include "Draw.h"
#define RESOULTION_X 640.0  //标犊时的分辨率
#define RESOULTION_Y 480.0  //标犊时的分辨率

namespace py = pybind11;

class CameraWrapper {
public:
	CameraWrapper();
	~CameraWrapper();
	// OBCameraParams getDeviceParams();
	void setNearFar(double N, double F);
	void initParams();
	void getColorDepthData();
	int D2C(const uint16_t* depth_buffer, int depth_width, int depth_height,
		uint16_t* out_depth, int color_width, int color_height) const;
	py::array_t<unsigned char> getColorImgNM();
	py::array_t<unsigned short> getDepthImgNM();
	py::array_t<unsigned char> getHistImgNM();

	void show();

private:
	d2cSwapper* pD2CSwapper;
	OBCameraParams cameraParams;
	Draw mDraw;

	int nWidth;
	int nHeight;


	cv::Mat cv_rgb, cv_depth, cv_aligned_depth;
	cv::Mat CaliDepthHistogram;

	IplImage IplColor, IplDepth, IplReg;

	double nearest, fars;
};

