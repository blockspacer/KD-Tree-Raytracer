#include "Precompiled.h"

#include "Assignment/Config.h"
#include "Assignment/Image.h"
#include "Assignment/Model.h"
#include "Assignment/Raytracer.h"
#include "fb/lang/time/ScopedTimer.h"
#include "fb/ToolsMain.h"

FB_PACKAGE1(assignment)

int assignmentMain(const int argc, const char **argv)
{
	StaticString appId("FBAssignment");
	/* If you want more threads you can just set this to 0 */
	uint32_t processorOverrideCount = 4;
	toolsengine::ToolsMain::initialize(appId, argc, argv, processorOverrideCount);

	StaticString helloMessage(FB_MSG("-------------------------------------\n\n", "Simple KD Tree Raytracer\n", 
		"\nRaytracer (c) Jukka Larja, 2009\n",
		"FbEngineForTools (c) Frozenbyte Oy, 2020\n", 
		"Raytracer modified to use FBEngineForTools and to comply with Frozenbyte coding conventions by Eetu Ropelinen\n",
		"Compiled ", __DATE__, " ", __TIME__, "\n"));
	FB_LOG_INFO(helloMessage);

	Config config;
	if (!config.readConfigFile(DynamicString("raytracer.ini")))
	{
		FB_LOG_ERROR("Reading the config file failed");
		return 0;
	}

	Model model;
	model.loadModel(config.getModelFileName());
	if (config.getUseKDTree())
	{
		FB_LOG_INFO("Creating KDTree...");
		ScopedTimer timer;
		model.initializeKDTree();
		FB_LOG_INFO(FB_MSG("KDTree created in ", timer.getTime().getSecondsAsFloat(), " seconds"));
	}

	Raytracer raytracer(model);
	raytracer.setCulling(config.getBackfaceCullingEnabled(), config.getFronfaceCullingEnabled());
	raytracer.addLights(config.getLights());
	raytracer.setShadows(config.getShadowsEnabled());
	raytracer.setResolution(config.getImageWidth(), config.getImageHeight());
	raytracer.setCameraUpVector(config.getCameraUpVector());

	if (config.getAutomaticCameraEnabled()) 
	{
		raytracer.setRelativeCameraPosition(config.getRelativeCameraPosition());
	}
	else 
	{
		raytracer.setViewportCorners(config.getViewportCoordinates());
		raytracer.setCameraPosition(config.getCameraPosition());
		raytracer.setCameraDirection(config.getCameraDirection());
	}

	Image image(config.getImageWidth(), config.getImageHeight());

	{
		FB_LOG_INFO("Raytracing...");
		ScopedTimer timer;
		raytracer.raytrace(image);
		FB_LOG_INFO(FB_MSG("Image traced in ", timer.getTime().getSecondsAsFloat(), " seconds"));
	}

	FB_LOG_INFO(FB_MSG("Writing image to file: ", config.getOutputImageFileName()));
	image.writeToFile(config.getOutputImageFileName());
	
	return 0;
}

FB_END_PACKAGE1()

int main(const int argc, const char **argv)
{
	return fb::assignment::assignmentMain(argc, argv);
}