#include "Precompiled.h"
#include "Config.h"

#include "fb/file/LineReader.h"
#include "fb/string/util/SplitString.h"

FB_PACKAGE1(assignment)

Config::Config()
{
}

bool Config::readConfigFile(const DynamicString &fileName)
{
	file::LineReader lineReader;
	if (!lineReader.open(fileName))
	{
		FB_LOG_ERROR(FB_MSG("Failed to open config file ", fileName.getPointer()));
		return false;
	}

	SizeType lineNum = 1;
	bool errors = false;
	while (!lineReader.isEOF())
	{
		HeapString line;
		if (!lineReader.readLine(line))
			break;

		if (!processLine(line)) 
		{
			errors = true;
			FB_LOG_ERROR(FB_MSG("Error on line ", lineNum, ": ", lastError));
		}

		++lineNum;
	}

	lineReader.close();
	FB_LOG_INFO("Config file parsed");
	
	if (!validateConfigSettings())
	{
		errors = true;
		FB_LOG_ERROR(FB_MSG("Error in config value: ", lastError));
	}

	return !errors;
}

const HeapString &Config::getModelFileName() const
{
	return data.modelFileName;
}

const HeapString &Config::getOutputImageFileName() const
{
	return data.outputFileName;
}

bool Config::getUseKDTree() const
{
	return data.useKDTree;
}

bool Config::getBackfaceCullingEnabled() const
{
	return data.backfaceCullingEnabled;
}

bool Config::getFronfaceCullingEnabled() const
{
	return data.frontfaceCullingEnabled;
}

const PodVector<Light> &Config::getLights() const
{
	return data.lights;
}

bool Config::getShadowsEnabled() const
{
	return data.shadowsEnabled;
}

bool Config::getAutomaticCameraEnabled() const
{
	return data.automaticCameraEnabled;
}

const math::VC3 &Config::getRelativeCameraPosition() const
{
	return data.cameraRelativePosition;
}

SizeType Config::getImageWidth() const
{
	return data.imageWidth;
}

SizeType Config::getImageHeight() const
{
	return data.imageHeight;
}

const StaticPodVector<math::VC3, 4> &Config::getViewportCoordinates() const
{
	return data.viewportCoordinates;
}

const math::VC3 &Config::getCameraPosition() const
{
	return data.cameraPosition;
}

const math::VC3 &Config::getCameraDirection() const
{
	return data.cameraDirection;
}

const math::VC3 &Config::getCameraUpVector() const
{
	return data.cameraUpVector;
}

bool Config::validateConfigSettings()
{
	if (data.modelFileName.isEmpty())
	{
		lastError = "Empty model file name";
		return false;
	}

	if (data.outputFileName.isEmpty())
	{
		lastError = "Empty output file name";
		return false;
	}
	
	if (data.threadPriority < -2 || data.threadPriority > 3) 
	{
		lastError = "Invalid thread priority. Please use value [-2, 3]";
		return false;
	}

	if (data.imageHeight == 0 || data.imageWidth == 0)
	{
		lastError = "Invalid output image size. Please use values larger than 0";
		return false;
	}

	if (data.viewportCoordinates.getSize() != 4) 
	{
		lastError = "Invalid amount of viewport coordinates";
		return false;
	}

	return true;
}

bool Config::processLine(const HeapString &line)
{
	HeapString trimmedLine = line;
	trimmedLine.trimWhiteSpace();
	if (isNonInformative(trimmedLine))
		return true;
	else if (isSectionStart(trimmedLine))
		return processSectionChange(trimmedLine);
	else if (isParameter(trimmedLine))
		return processParameter(trimmedLine);

	lastError = FB_MSG("Could not identify line: ", line);
	return false;
}

bool Config::isNonInformative(const HeapString &trimmedLine) const
{
	return trimmedLine.isEmpty() || trimmedLine.doesStartWith("#");
}

bool Config::isSectionStart(const HeapString &trimmedLine) const
{
	return !trimmedLine.isEmpty() && trimmedLine.doesStartWith("[") && trimmedLine.doesEndWith("]");
}

bool Config::isParameter(const HeapString &trimmedLine) const
{
	return !isNonInformative(trimmedLine) && trimmedLine.doesContain("=");
}

bool Config::processSectionChange(const HeapString &trimmedLine)
{
	HeapString sectionName = trimmedLine;
	sectionName.trimCharsLeft('[');
	sectionName.trimCharsRight(']');
	
	if (sectionName == "general")
	{
		currentSection = SectionGeneral;
		return true;
	}
	else if (sectionName == "camera")
	{
		currentSection = SectionCamera;
		return true;
	}
	else if (sectionName == "viewport")
	{
		currentSection = SectionViewport;
		return true;
	}
	else if (sectionName == "lights")
	{
		currentSection = SectionLights;
		return true;
	}
	else 
	{
		currentSection = SectionNone;
		this->lastError = FB_MSG("Unknown section name: ", sectionName);
		return false;
	}
}

bool Config::processParameter(const HeapString &trimmedLine)
{
	HeapString name;
	HeapString value;
	getParameterName(trimmedLine, name);
	getParameterValue(trimmedLine, value);

	if (!isVariableSupported(name)) 
	{
		lastError = FB_MSG("Unknown parameter ", name, " in section ", currentSection);
		return false;
	}

	if (!readValue(name, value))
	{
		lastError = FB_MSG("Failed to read value for line: ", trimmedLine);
		return false;
	}

	return true;
}

void Config::getParameterName(const HeapString &trimmedLine, HeapString &nameOut)
{
	nameOut = trimmedLine;
	nameOut.truncateToSize(nameOut.find("=") - 1);
	nameOut.trimWhiteSpace();
}

void Config::getParameterValue(const HeapString &trimmedLine, HeapString &valueOut)
{
	valueOut = trimmedLine;
	valueOut.trimLeft(valueOut.find("=") + 1);
	valueOut.trimWhiteSpace();
}

/* General */
static const StaticString modelFileNameVar("model file name");
static const StaticString outputFileNameVar("output file name");
static const StaticString backfaceCullingVar("backface culling");
static const StaticString frontfaceCullingVar("frontface culling");
static const StaticString shadowsVar("shadows");
static const StaticString useKDTreeVar("useKDTree");
static const StaticString hardwareThreadsVar("hardware threads");
static const StaticString threadPriorityVar("thread priority");
/* Camera */
static const StaticString automaticCameraVar("automatic camera");
static const StaticString cameraRelativePositionVar("camera relative position");
static const StaticString cameraPositionVar("camera position");
static const StaticString cameraDirectionVar("camera direction");
static const StaticString cameraUpVecVar("camera up vector");
/* Viewport */
static const StaticString imageWidthVar("image width");
static const StaticString imageHeightVar("image height");
static const StaticString viewportCoordinatesVar("viewport coordinates");
/* Lights */
static const StaticString lightVar("light");

bool Config::isVariableSupported(const HeapString &variableName)
{
	switch (currentSection)
	{
	case SectionNone:
	{
		return false;
	}
	break;

	case SectionGeneral:
	{
		return variableName == modelFileNameVar || variableName == outputFileNameVar || variableName == backfaceCullingVar || variableName == frontfaceCullingVar || variableName == shadowsVar ||
			variableName == useKDTreeVar || variableName == hardwareThreadsVar || variableName == threadPriorityVar; 
	}
	break;

	case SectionCamera:
	{
		return variableName == automaticCameraVar || variableName == cameraRelativePositionVar || variableName == cameraPositionVar || variableName == cameraDirectionVar || variableName == cameraUpVecVar;
	}
	break;

	case SectionViewport:
	{
		return variableName == imageWidthVar || variableName == imageHeightVar || variableName == viewportCoordinatesVar;
	}
	break;

	case SectionLights:
	{
		return variableName == lightVar;
	}
	break;

	default:
	{
		return false;
	}
	break;

	}
}

bool Config::readValue(const HeapString &name, const HeapString &value)
{
	if (name == modelFileNameVar)
		return readString(value, data.modelFileName);
	else if (name == outputFileNameVar)
		return readString(value, data.outputFileName);
	else if (name == backfaceCullingVar)
		return readBool(value, data.backfaceCullingEnabled);
	else if (name == frontfaceCullingVar)
		return readBool(value, data.frontfaceCullingEnabled);
	else if (name == shadowsVar)
		return readBool(value, data.shadowsEnabled);
	else if (name == useKDTreeVar)
		return readBool(value, data.useKDTree);
	else if (name == hardwareThreadsVar)
		return readUnsigned(value, data.hardwareThreadCount);
	else if (name == threadPriorityVar)
		return readInt(value, data.threadPriority);
	else if (name == automaticCameraVar)
		return readBool(value, data.automaticCameraEnabled);
	else if (name == cameraRelativePositionVar)
		return readVec3(value, data.cameraRelativePosition);
	else if (name == cameraPositionVar)
		return readVec3(value, data.cameraPosition);
	else if (name == cameraDirectionVar)
		return readVec3(value, data.cameraDirection);
	else if (name == cameraUpVecVar)
		return readVec3(value, data.cameraUpVector);
	else if (name == imageWidthVar)
		return readUnsigned(value, data.imageWidth);
	else if (name == imageHeightVar)
		return readUnsigned(value, data.imageHeight);
	else if (name == viewportCoordinatesVar)
		return readVec3Group(value, data.viewportCoordinates);
	else if (name == lightVar)
		return readLight(value, data.lights.pushBack());

	lastError = FB_MSG("Unknown value: ", name, ", with value: ", value);
	return false;
}

bool Config::readString(const HeapString &value, HeapString &parsedValueOut)
{
	parsedValueOut = value;
	return true;
}

bool Config::readBool(const HeapString &value, bool &parsedValueOut)
{
	HeapString s = value;
	s.trimWhiteSpace();
	return s.parse(parsedValueOut);
}

bool Config::readInt(const HeapString &value, int &parsedValueOut)
{
	HeapString s = value;
	s.trimWhiteSpace();
	return s.parse(parsedValueOut);
}

bool Config::readUnsigned(const HeapString &value, SizeType &parsedValueOut)
{
	HeapString s = value;
	s.trimWhiteSpace();
	return s.parse(parsedValueOut);
}

bool Config::readVec3(const HeapString &value, math::VC3 &parsedValueOut)
{
	HeapString s = value;
	s.trimWhiteSpace();
	string::SplitString splitString(s, " ");
	const string::SplitString::Pieces &pieces = splitString.getPieces();
	if (pieces.getSize() != 3)
	{
		lastError = "Invalid count of values for a vector 3";
		return false;
	}

	for (SizeType i = 0; i < pieces.getSize(); ++i)
	{
		if (!pieces[i].parse(parsedValueOut[i]))
		{
			lastError = FB_MSG("Failed to parse value ", pieces[i], " at index ", i, " for a vector 3");
			return false;
		}
	}

	return true;
}

bool Config::readVec3Group(const HeapString &value, PodVector<math::VC3> &parsedValuesOut)
{
	HeapString s = value;
	s.trimWhiteSpace();
	string::SplitString splitString(s, " ");
	const string::SplitString::Pieces &pieces = splitString.getPieces();
	if (pieces.getSize() % 3 != 0)
	{
		lastError = "Invalid count of values for a vector 3 group";
		return false;
	}

	for (SizeType vecIndex = 0; vecIndex < pieces.getSize(); vecIndex += 3)
	{
		math::VC3 &vec = parsedValuesOut.pushBack();
		for (SizeType axisIndex = 0; axisIndex < 3; ++axisIndex)
		{
			SizeType pieceIndex = vecIndex + axisIndex;
			if (!pieces[pieceIndex].parse(vec[axisIndex]))
			{
				lastError = FB_MSG("Failed to parse value ", pieces[pieceIndex], " at index ", pieceIndex, " for a vector 3 group");
				return false;
			}
		}
	}

	return true;
}

static const StaticString lightTypeAmbient("ambient");
static const StaticString lightTypeDiffuse("diffuse");
static const StaticString lightTypePoint("point");

bool Config::readLight(const HeapString &value, Light &parsedValueOut)
{
	Light light;
	HeapString s = value;
	s.trimWhiteSpace();
	if (s.doesStartWith(lightTypeAmbient))
	{
		light.setType(LightTypeAmbient);
		s.trimLeft(lightTypeAmbient.getLength());
	}
	else if (s.doesStartWith(lightTypeDiffuse))
	{
		light.setType(LightTypeDiffuse);
		s.trimLeft(lightTypeDiffuse.getLength());
	}
	else if (s.doesStartWith(lightTypePoint))
	{
		light.setType(LightTypePoint);
		s.trimLeft(lightTypePoint.getLength());
	}
	else
	{
		lastError = FB_MSG("Unknown light type in ", value);
		return false;
	}

	PodVector<math::VC3> vecs;
	if (!readVec3Group(s, vecs))
	{
		lastError = FB_MSG("Could not read light vec properties in ", value);
		return false;
	}

	if (vecs.getSize() != 2)
	{
		lastError = FB_MSG("Invalid number of vec3s (", vecs.getSize(), ") in light property ", value);
		return false;
	}

	light.setPosition(vecs[0]);
	light.setColor(vecs[1]);
	parsedValueOut = light;
	return true;
}

FB_END_PACKAGE1()