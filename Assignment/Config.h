#pragma once

#include "Assignment/Light.h"

FB_PACKAGE1(assignment)

class Config
{
public:
	Config();

	bool readConfigFile(const DynamicString &fileName);

	const HeapString &getModelFileName() const;
	const HeapString &getOutputImageFileName() const;
	bool getUseKDTree() const;
	bool getBackfaceCullingEnabled() const;
	bool getFronfaceCullingEnabled() const;
	const PodVector<Light> &getLights() const;
	bool getShadowsEnabled() const;
	bool getAutomaticCameraEnabled() const;
	const math::VC3 &getRelativeCameraPosition() const;
	SizeType getImageWidth() const;
	SizeType getImageHeight() const;
	const StaticPodVector<math::VC3, 4> &getViewportCoordinates() const;
	const math::VC3 &getCameraPosition() const;
	const math::VC3 &getCameraDirection() const;
	const math::VC3 &getCameraUpVector() const;

private:
	bool validateConfigSettings();
	bool processLine(const HeapString &line);

	bool isNonInformative(const HeapString &trimmedLine) const;
	bool isSectionStart(const HeapString &trimmedLine) const;
	bool isParameter(const HeapString &trimmedLine) const;

	bool processSectionChange(const HeapString &trimmedLine);
	bool processParameter(const HeapString &trimmedLine);
	void getParameterName(const HeapString &trimmedLine, HeapString &nameOut);
	void getParameterValue(const HeapString &trimmedLine, HeapString &valueOut);

	bool isVariableSupported(const HeapString &variableName);
	bool readValue(const HeapString &name, const HeapString &value);
	bool readString(const HeapString &value, HeapString &parsedValueOut);
	bool readBool(const HeapString &value, bool &parsedValueOut);
	bool readInt(const HeapString &value, int &parsedValueOut);
	bool readUnsigned(const HeapString &value, SizeType &parsedValueOut);
	bool readVec3(const HeapString &value, math::VC3 &parsedValueOut);
	bool readVec3Group(const HeapString &value, PodVector<math::VC3> &parsedValuesOut);
	bool readLight(const HeapString &value, Light &parsedValueOut);

	enum Section 
	{
		SectionNone = 0,
		SectionGeneral, 
		SectionCamera, 
		SectionViewport, 
		SectionLights, 
	};

	struct ConfigurationData
	{
		HeapString modelFileName;
		HeapString outputFileName;
		bool backfaceCullingEnabled;
		bool frontfaceCullingEnabled;
		bool shadowsEnabled;
		bool useKDTree;
		SizeType hardwareThreadCount;
		int threadPriority;
		bool automaticCameraEnabled;
		math::VC3 cameraRelativePosition;
		math::VC3 cameraPosition;
		math::VC3 cameraDirection;
		math::VC3 cameraUpVector;
		SizeType imageWidth;
		SizeType imageHeight;
		StaticPodVector<math::VC3, 4> viewportCoordinates;
		PodVector<Light> lights;
	};

	DynamicString lastError;
	Section currentSection = SectionNone;
	ConfigurationData data;

};

FB_END_PACKAGE1()
