// holoplay.h
#ifndef HOLOPLAY_INCLUDE
#define HOLOPLAY_INCLUDE

#if _WIN32
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API 
#endif

#include "glfw/glfw3.h"
#include <string>
#include <vector>
#include <math.h>
#include "hidapi.h"

// classes
struct LKGDisplay {
	GLFWmonitor* monitor;
	std::string windowsName = "";
	std::string lkgName = "";
	int xpos = 0;
	int ypos = 0;
	int calIndex = 0;
	int unityIndex = 0;
};
enum Model {
	standard = 0,
	large = 1
};
struct ModelProperties {
	int screenWidth;
	int screenHeight;
	float dpi;
	float viewCone;
	float aspect;
};
const ModelProperties modelProperties[] = {
	{2560, 1600, 338.0f, 35.0f, 2560.f / 1600.f}, // standard
	{3840, 2160, 283.0f, 35.0f, 3840.f / 2160.f}, // large
};
struct Calibration {
	Model model;
	std::string lkgName;
	std::string serial;
	float pitch;
	float slope;
	float center;
	ModelProperties prop() { return modelProperties[model]; }
	void process() {
		pitch *= (float)prop().screenWidth / prop().dpi;
		pitch *= cos(atan(1.0f / slope));
		slope = (float)prop().screenHeight / (prop().screenWidth * slope);
	}
};

// exported vars
extern "C" EXPORT_API bool initialized;
extern "C" EXPORT_API bool haveCheckedLKG;
extern "C" EXPORT_API bool verbose;
extern "C" EXPORT_API hid_device *hidapi_handle;
extern "C" EXPORT_API std::vector<std::string> calibrationStrs;
extern "C" EXPORT_API std::vector<Calibration> calibrations;
extern "C" EXPORT_API int calibrationCount;
extern "C" EXPORT_API std::vector<LKGDisplay> lkgDisplays;

// functions
extern "C" EXPORT_API void SetVerbose(bool);
// calibration
extern "C" EXPORT_API int LoadCalibrations();
extern "C" EXPORT_API int ReadCalibrations(char*);
extern "C" EXPORT_API int CalibrationCount();
extern "C" EXPORT_API int GetModel(int);
extern "C" EXPORT_API int GetScreenWidth(int);
extern "C" EXPORT_API int GetScreenHeight(int);
extern "C" EXPORT_API float GetDPI(int);
extern "C" EXPORT_API float GetViewCone(int);
extern "C" EXPORT_API float GetAspect(int);
extern "C" EXPORT_API float GetPitch(int);
extern "C" EXPORT_API float GetSlope(int);
extern "C" EXPORT_API float GetCenter(int);
extern "C" EXPORT_API int GetSerial(int cal, char* output);
extern "C" EXPORT_API int GetLKGName(int cal, char* output);
// monitor stuff
extern "C" EXPORT_API int PopulateLKGDisplays();
extern "C" EXPORT_API void PopulateLKGDisplayFromEdid();
extern "C" EXPORT_API int GetLKGcount();
extern "C" EXPORT_API int GetLKGcalIndex(int);
extern "C" EXPORT_API int GetLKGunityIndex(int);
extern "C" EXPORT_API int GetLKGxpos(int);
extern "C" EXPORT_API int GetLKGypos(int);
extern "C" EXPORT_API int GetLKGdisplayName(int i, char* output);

#endif
