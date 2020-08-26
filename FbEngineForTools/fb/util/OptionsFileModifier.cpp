#include "Precompiled.h"
#include "OptionsFileModifier.h"

#include "fb/container/Vector.h"
#include "fb/container/PodVector.h"
#include "fb/lang/platform/LineFeed.h"
#include "fb/stream/InputStream.h"
#include "fb/string/HeapString.h"
#include "fb/file/CreateDirectory.h"
#include "fb/file/FileManager.h"
#include "fb/file/InputFile.h"
#include "fb/file/OutputFile.h"

#include <cstring>
#include <stdlib.h> // For atoi and atof

FB_PACKAGE1(util)

// type, module name, property name, default value
static const int typeNameIndex = 0;
static const int moduleNameIndex = 1;
static const int propertyNameIndex = 2;
static const int defaultValueIndex = 3;
static const int entriesPerOption = OptionsFileModifier::stringsPerOptionEntry; // 4

static const int maxParsedStringLength = 2048;
static char parsedStringRetBuf[maxParsedStringLength + 1];

class OptionsFileModifier::Impl
{
private:
	const OptionsListType *knownOptionsList;
	SizeType amountOfKnownOptions;
	IOptionsFileModifierErrorListener *errorListener;
	bool someOptionsDontHaveDefaultValues;
	struct OptionEntry
	{
		OptionEntry()
			: moduleName(NULL)
			, propertyName(NULL)
			, value(NULL)
		{ }
		OptionEntry(const char *moduleName, const char *propertyName, const char *value)
			: moduleName(moduleName)
			, propertyName(propertyName)
			, value(value)
		{ }

		// note, these strings may have been allocated from the heap, or they may reside inside the dataBufferSplitted.
		// (all unmodified strings are normally inside dataBufferSplitted for optimality)
		// this has to be taken into consideration on delete!

		const char *moduleName;   // name of the option module
		const char *propertyName; // name of the option property
		const char *value;        // the option value as a string

		// RESERVED:
		// - when the moduleName is null and propertyName is null, the value may be a comment line
	};
	PodVector<OptionEntry> parsedOptions;
	int parsedVersion;

	bool loaded;
	HeapString loadedFilePath;
	char *dataBufferSplitted; // the entire options file read in, split to pieces with null string terminator characters
	SizeType dataBufferSplittedSize; // not the string length, but the allocation length (contains null term as well)
	bool containsParseErrors;
	bool containsUnrecognizedOptions;
	bool loadHasBeenCalled;

	struct VersionConversionRule
	{
		HeapString oldModuleName;
		HeapString newModuleName;
		HeapString oldPropertyName;
		HeapString newPropertyName;
		HeapString oldValue;
		HeapString newValue;
		bool nullOldValue;
		bool nullNewValue;
	};
	Vector<VersionConversionRule> versionConversionRules;


	Impl() 
		: knownOptionsList(NULL)
		, amountOfKnownOptions(0)
		, errorListener(NULL)
		, someOptionsDontHaveDefaultValues(false)
		, parsedVersion(1)
		, loaded(false)
		, loadedFilePath("")
		, dataBufferSplitted(NULL)
		, dataBufferSplittedSize(0)
		, containsParseErrors(false)
		, containsUnrecognizedOptions(false)
		, loadHasBeenCalled(false)
	{
	}

	// note, won't remove the entry from the list, just frees the memory!
	void freeOptionStrings(SizeType index)
	{
		FB_OFM_ASSERT(index < parsedOptions.getSize());

		if (parsedOptions[index].moduleName >= dataBufferSplitted
			&& parsedOptions[index].moduleName < dataBufferSplitted + dataBufferSplittedSize)
		{
			// inside the split buffer, ignore free
		} else {
			delete [] parsedOptions[index].moduleName;
		}
		parsedOptions[index].moduleName = NULL;

		if (parsedOptions[index].propertyName >= dataBufferSplitted
			&& parsedOptions[index].propertyName < dataBufferSplitted + dataBufferSplittedSize)
		{
			// inside the split buffer, ignore free
		} else {
			delete [] parsedOptions[index].propertyName;
		}
		parsedOptions[index].propertyName = NULL;

		if (parsedOptions[index].value >= dataBufferSplitted
			&& parsedOptions[index].value < dataBufferSplitted + dataBufferSplittedSize)
		{
			// inside the split buffer, ignore free
		} else {
			delete [] parsedOptions[index].value;
		}
		parsedOptions[index].value = NULL;

	}


	void unallocateAllOptions()
	{
		for (SizeType i = 0; i < parsedOptions.getSize(); i++)
		{
			freeOptionStrings(i);
		}
		parsedOptions.clear();
	}


	void unloadData()
	{
		unallocateAllOptions();

		if (dataBufferSplitted != NULL)
		{
			delete [] dataBufferSplitted;
			dataBufferSplitted = NULL;
			dataBufferSplittedSize = 0;
		}
		loaded = false;
		loadedFilePath = "";
	}


	void addParsedOptionWithVersionConversion(const char *moduleName, const char *propertyName, const char *valueStr)
	{
		int conversionRuleMatchCount = 0;
		for (SizeType i = 0; i < versionConversionRules.getSize(); i++)
		{
			// Note, the struct contains HeapString variables, the parameters only are plain C-strings.
			bool ruleMatches = false;
			if (versionConversionRules[i].oldModuleName == moduleName 
				&& versionConversionRules[i].oldPropertyName == propertyName)
			{
				// old module and property names matched, what about the optional value...?

				if (versionConversionRules[i].nullOldValue)
				{
					// no optional old value in use, thus this is a match.
					ruleMatches = true;
				} else {
					// optional old value specified, it must match too...
					if (versionConversionRules[i].oldValue == valueStr)
					{
						// old value matches
						ruleMatches = true;
					}
				}
			}
			if (ruleMatches)
			{
				// module name does not match the new one? if so, change.
				if (versionConversionRules[i].newModuleName != moduleName)
				{
					if (moduleName >= dataBufferSplitted && moduleName < dataBufferSplitted + dataBufferSplittedSize)
					{
						// inside the split buffer, ignore free
					}
					else
					{
						delete [] moduleName;
					}
					moduleName = NULL;

					char *newStr = new char[versionConversionRules[i].newModuleName.getLength() + 1];
					strcpy(newStr, versionConversionRules[i].newModuleName.getPointer());
					moduleName = newStr;
				}

				// property name does not match the new one? if so, change.
				if (versionConversionRules[i].newPropertyName != propertyName)
				{
					if (propertyName >= dataBufferSplitted
						&& propertyName < dataBufferSplitted + dataBufferSplittedSize)
					{
						// inside the split buffer, ignore free
					} else {
						delete [] propertyName;
					}
					propertyName = NULL;

					char *newStr = new char[versionConversionRules[i].newPropertyName.getLength() + 1];
					strcpy(newStr, versionConversionRules[i].newPropertyName.getPointer());
					propertyName = newStr;
				}

				bool changeValue = false;
				if (!versionConversionRules[i].nullNewValue)
				{
					changeValue = true;
				}
				if (changeValue)
				{
					if (valueStr >= dataBufferSplitted
						&& valueStr < dataBufferSplitted + dataBufferSplittedSize)
					{
						// inside the split buffer, ignore free
					} else {
						delete [] valueStr;
					}
					valueStr = NULL;

					char *newStr = new char[versionConversionRules[i].newValue.getLength() + 1];
					strcpy(newStr, versionConversionRules[i].newValue.getPointer());
					valueStr = newStr;
				}

				conversionRuleMatchCount++;
			}
		}

		// Either no conversion rule at all, or only a single rule only should apply. 
		// The apply behavior of multiple rules matching is not specified. (first only?, chained?, order of apply?, etc.)
		// Even if this triggers, it should not be a fatal error, but the actual result logic has not really been specified,
		// so please don't make any conversion rules that assume multiple conversion rule matches are ok.
		// (This getting triggered may also indicate that you copy&pasted some rule and forgot to change the actual strings.)
		FB_OFM_ASSERT(conversionRuleMatchCount <= 1);

		parsedOptions.pushBack(OptionEntry(moduleName, propertyName, valueStr));
	}


	// the nearly single pass supah-efficient (and supah-error-prone) options file line parsing :P
	void parseLine(char *lineStr, SizeType lineStrLen, SizeType lineNumber)
	{
		char *startingWhitespaceTrimmedLine = NULL;
		for (SizeType j = 0; j < lineStrLen; j++)
		{
			if (lineStr[j] == ' ' || lineStr[j] == '\t')
			{
				// continue skipping
			}
			else
			{
				// start processing the actual contents please...
				startingWhitespaceTrimmedLine = &lineStr[j];
				break;
			}
		}
		if (startingWhitespaceTrimmedLine != NULL)
		{
			if (startingWhitespaceTrimmedLine[0] == '-' && startingWhitespaceTrimmedLine[1] == '-')
			{
				// consider this a comment line, its ok, but insert it to the list of options to make sure they won't disappear on save
				parsedOptions.pushBack(OptionEntry(NULL, NULL, startingWhitespaceTrimmedLine));
			}
			else
			{
				const char *setOptionPrefixStr = "setOption(";
				SizeType setOptionPrefixStrLen = 10;
				FB_OFM_ASSERT(strlen(setOptionPrefixStr) == setOptionPrefixStrLen);

				const char *setVersionPrefixStr = "setVersion(";
				SizeType setVersionPrefixStrLen = 11;
				FB_OFM_ASSERT(strlen(setVersionPrefixStr) == setVersionPrefixStrLen);

				if (strncmp(startingWhitespaceTrimmedLine, setOptionPrefixStr, setOptionPrefixStrLen) == 0)
				{
					// fine, starts with setOption(
					char *modNameStrStart = &startingWhitespaceTrimmedLine[setOptionPrefixStrLen];
					char *propNameStrStart = NULL;
					char *valueStrStart = NULL;
					SizeType modNameStartStrLen = SizeType(strlen(modNameStrStart));

					// first trim trailing whitespaces and the ending )
					for (SizeType j = modNameStartStrLen - 1; j < modNameStartStrLen; j--)
					{
						if (modNameStrStart[j] == ' ' || modNameStrStart[j] == '\t')
						{
							modNameStrStart[j] = '\0';
						}
						else
						{
							// ok, done with whitespaces.
							if (modNameStrStart[j] == ')')
							{
								// and get rid of the ending parenthesis, then done here.
								modNameStrStart[j] = '\0';
							}
							else
							{
								// missing the ending parenthesis
								parseLineErrorMessage("Missing the closing parenthesis for a setOption(...) line.", lineNumber);
							}
							break;
						}
					}

					modNameStartStrLen = SizeType(strlen(modNameStrStart));

					for (SizeType j = 0; j < modNameStartStrLen; j++)
					{
						if (modNameStrStart[j] == ',')
						{
							modNameStrStart[j] = '\0';
							if (propNameStrStart == NULL)
							{
								// get the property name string
								j++;
								propNameStrStart = &modNameStrStart[j];
								// skip whitespaces from property name
								while (modNameStrStart[j] == ' ' || modNameStrStart[j] == '\t')
								{
									j++;
									propNameStrStart = &modNameStrStart[j];
								}
							}
							else
							{
								// get the value string
								j++;
								valueStrStart = &modNameStrStart[j];
								// skip whitespaces from property name
								while (modNameStrStart[j] == ' ' || modNameStrStart[j] == '\t')
								{
									j++;
									valueStrStart = &modNameStrStart[j];
								}
								break; // Stop looking for a value when one is found (ignore rest of the commas as they broke VC2 parsing)
							}
						}
					}
					// now, we should have: modNameStrStart pointing to the module name, whitespace untrimmed
					// now, we should have: propNameStrStart pointing to the quoted "property name", left whitespace trimmed
					// now, we should have: valueStrStart pointing to the value, left whitespace trimmed 
					if (propNameStrStart != NULL && valueStrStart != NULL) 
					{
						// TODO: right trim the module name, property name for trailing whitespaces

						int propNameLen = int(strlen(propNameStrStart));
						if (propNameLen > 2 && propNameStrStart[0] == '"' && propNameStrStart[propNameLen - 1] == '"')
						{
							// ok, drop out the quotes
							propNameStrStart[propNameLen - 1] = '\0';
							propNameStrStart = &propNameStrStart[1];

							// alright, we have them all now..
							addParsedOptionWithVersionConversion(modNameStrStart, propNameStrStart, valueStrStart);
						} else {
							// error
							parseLineErrorMessage("The property name must be given within double-quotes.", lineNumber);
						}
					} else {
						parseLineErrorMessage("Expected 3 comma separated parameters: module name, property name and value.", lineNumber);
					}
				} else if (strncmp(startingWhitespaceTrimmedLine, setVersionPrefixStr, setVersionPrefixStrLen) == 0) {
					// fine, starts with setVersion(
					int ret = sscanf(startingWhitespaceTrimmedLine, "setVersion( %d )", &parsedVersion);
					if (ret != 1) {
						parseLineErrorMessage("Couldn't parse version number.", lineNumber);
					}
				} else {
					if (errorListener != NULL)
					{
						parseLineErrorMessage("Expected a setOption(...) or setVersion(...) line.", lineNumber);
					}
				}
			}
		} else {
			// all whitespaces, ignore the line.
		}
	}


	void parseLoaded()
	{
		FB_OFM_ASSERT(dataBufferSplitted != NULL);
		FB_OFM_ASSERT(dataBufferSplittedSize > 0);
		FB_OFM_ASSERT(loaded != false);

		containsParseErrors = false;
		containsUnrecognizedOptions = false;

		size_t s = strlen(dataBufferSplitted);
		if (s > dataBufferSplittedSize)
		{
			FB_OFM_ASSERT(0 && "String terminator null missing?");
			s = dataBufferSplittedSize;
		}


		// split on a line-by-line basis.
		// and every line gets split to module name, property name and value strings
		SizeType lineNumber = 1;
		SizeType lastLineStart = 0;
		for (SizeType i = 0; i < s + 1; i++) // NOTE: processes the last null char too to prevent requirement of linefeed at the last line
		{
			if (dataBufferSplitted[i] == '\r' || dataBufferSplitted[i] == '\n' || dataBufferSplitted[i] == '\0')
			{
				if (dataBufferSplitted[i] == '\n') lineNumber++;

				dataBufferSplitted[i] = '\0';

				// (ignore any totally empty lines)
				if (i > lastLineStart)
				{
					// this is our line string... process it next...
					char *lineStr = &dataBufferSplitted[lastLineStart];

					SizeType lineStrLen = i - lastLineStart;
					FB_OFM_ASSERT(lineStrLen == strlen(lineStr));

					parseLine(lineStr, lineStrLen, lineNumber);
				}
				lastLineStart = i + 1;
			}
		}

		// should have processed all, even the null term char now.
		FB_OFM_ASSERT(lastLineStart == s + 1); 

		// now we can check if all of the parsed options were known ones
		// and do additional type checks to see if they really are of expected type
		for (SizeType i = 0; i < parsedOptions.getSize(); i++)
		{
			const char *mName = parsedOptions[i].moduleName;
			const char *pName = parsedOptions[i].propertyName;
			if (mName != NULL && pName != NULL)
			{
				const char *v = parsedOptions[i].value;

				SizeType knownIndex = findKnownIndexForOption(mName, pName);
				if (knownIndex != getInvalidIndex())
				{
					// test parsing in the value with the known type, just to see that it matches...
					const char *t = getTypeForKnownIndex(knownIndex);
					bool valueParseError = false;
					bool valueParseWarning = false;
					const char *parseAttemptType = "";
					if (strcmp(t, "int") == 0)
					{
						parseAttemptType = "integer";
						int dummy = parseValueAsInt(v, valueParseError, valueParseWarning); (void)dummy;
					}
					else if (strcmp(t, "float") == 0)
					{
						parseAttemptType = "floating point";
						float dummy = parseValueAsFloat(v, valueParseError, valueParseWarning); (void)dummy;
					}
					else if (strcmp(t, "string") == 0)
					{
						parseAttemptType = "string";
						const char *dummy = parseValueAsString(v, valueParseError, valueParseWarning); (void)dummy;
					}
					else if (strcmp(t, "boolean") == 0)
					{
						parseAttemptType = "boolean";
						bool dummy = parseValueAsBool(v, valueParseError, valueParseWarning); (void)dummy;
					} else {
						FB_OFM_ASSERT(0 && "Unsupported option type.");
					}
					if (valueParseError)
					{
						valueParseErrorMessage(mName, pName, v, parseAttemptType);
					}
					if (valueParseWarning)
					{
						valueParseWarningMessage(mName, pName, v, parseAttemptType);
					}
				}
				else
				{
					containsUnrecognizedOptions = true;
				}
			}
		}
	}


	void quickSanityCheck()
	{
		FB_OFM_ASSERT(knownOptionsList != NULL);
		FB_OFM_ASSERT(amountOfKnownOptions >= 1); // at least nulls required

		// sanity check, list must be terminated by NULLs
		// (included in the amount of known options, as this way the check is safer)
		// (the other way around, we might be checking outside the array boundary, and accidentally get zero initialized memory, thus everything
		// seeming to be okay, but in reality accessing outside the buffer)
		for (int i = 0; i < entriesPerOption; i++)
		{
			// Did you increase the amountOfKnownOptions by correct amount when adding new options? (or decrease if options removed)
			FB_OFM_ASSERT((*knownOptionsList)[amountOfKnownOptions - 1][i] == NULL);
		}
	}


	void fullSanityCheck()
	{
		quickSanityCheck();

		// check that the char array has no nulls or empty strings where they should not be
		for (SizeType i = 0; i < amountOfKnownOptions - 1; i++)
		{
			FB_OFM_ASSERT((*knownOptionsList)[i][typeNameIndex] != NULL); // type cannot be null, it may be empty though
			FB_OFM_ASSERT((*knownOptionsList)[i][moduleNameIndex] != NULL && (*knownOptionsList)[i][moduleNameIndex][0] != '\0'); // module name cannot be null, nor can it be empty
			FB_OFM_ASSERT((*knownOptionsList)[i][propertyNameIndex] != NULL && (*knownOptionsList)[i][propertyNameIndex][0] != '\0'); // property name cannot be null, nor can it be empty
			//FB_OFM_ASSERT((*knownOptionsList)[i][defaultValueIndex] != NULL); // default value can actually be null
		}

		// quickly check that int types seem to have int default values
		// quickly check that bool types seem to have bool default values
		// quickly check that float types seem to have float default values
		// quickly check that string types seem to have string default values
		for (SizeType i = 0; i < amountOfKnownOptions - 1; i++)
		{
			const char *t = (*knownOptionsList)[i][typeNameIndex];
			FB_OFM_ASSERT(t != NULL); // type cannot be null, it may be empty though
			const char *defValue = (*knownOptionsList)[i][defaultValueIndex];

			if (defValue != NULL)
			{
				if (strcmp(t, "int") == 0)
				{
					FB_OFM_ASSERT(atoi(defValue) != 0 || (defValue[0] == '0' && defValue[1] == '\0'));
				}
				else if (strcmp(t, "float") == 0)
				{
					FB_OFM_ASSERT(atof(defValue) != 0 || (defValue[0] == '0')); // not a very full check here.
				}
				else if (strcmp(t, "string") == 0)
				{
					FB_OFM_ASSERT(defValue[0] == '\"'); // not a very full check here.
				}
				else if (strcmp(t, "boolean") == 0)
				{
					FB_OFM_ASSERT(strcmp(defValue, "false") == 0 || strcmp(defValue, "true") == 0);
				} else {
					FB_OFM_ASSERT(0 && "Unsupported option type.");
				}
			}
		}

	}

	// Returns the index for the property in the list of known types list
	// returns -1 if not in known options list
	SizeType findKnownIndexForOption(const char *moduleName, const char *propertyName)
	{
		// (Notice that the last index is the null terminator, thus leaving it out)
		for (SizeType i = 0; i < amountOfKnownOptions - 1; i++)
		{
			const char *mn = (*knownOptionsList)[i][moduleNameIndex];
			FB_OFM_ASSERT(mn != NULL);

			if (strcmp(moduleName, mn) == 0)
			{
				const char *pn = (*knownOptionsList)[i][propertyNameIndex];
				FB_OFM_ASSERT(pn != NULL);
				if (strcmp(propertyName, pn) == 0)
				{
					return i;
				}
			}
		}
		return getInvalidIndex();
	}


	// Returns the index for the property in the list of parsed stuff 
	// returns -1 if not parsed in
	SizeType findParsedOptionIndex(const char *moduleName, const char *propertyName)
	{
		for (SizeType i = 0; i < parsedOptions.getSize(); i++)
		{
			if (parsedOptions[i].moduleName != NULL && parsedOptions[i].propertyName != NULL)
			{
				if (strcmp(parsedOptions[i].moduleName, moduleName) == 0
					&& strcmp(parsedOptions[i].propertyName, propertyName) == 0)
				{
					// found it.
					return i;
				}
			}
			else
			{
				// maybe it was a comment line?
				// assume both module and prop name were null in such case
				FB_OFM_ASSERT(parsedOptions[i].moduleName == NULL && parsedOptions[i].propertyName == NULL);
				// and value contains the comment line
				FB_OFM_ASSERT(parsedOptions[i].value != NULL);
			}
		}
		return getInvalidIndex();
	}


	const char *getTypeForKnownIndex(SizeType knownOptionListIndex)
	{
		// (Notice that the last index is the null terminator, thus leaving it out)
		FB_OFM_ASSERT(knownOptionListIndex < amountOfKnownOptions - 1);

		return (*knownOptionsList)[knownOptionListIndex][typeNameIndex];;
	}

	const char *getDefaultValueForKnownIndex(SizeType knownOptionListIndex)
	{
		// (Notice that the last index is the null terminator, thus leaving it out)
		FB_OFM_ASSERT(knownOptionListIndex < amountOfKnownOptions - 1);

		return (*knownOptionsList)[knownOptionListIndex][defaultValueIndex];
	}

	const char *getParsedOptionValue(SizeType parsedOptionIndex)
	{
		FB_OFM_ASSERT(parsedOptionIndex < parsedOptions.getSize());

		return parsedOptions[parsedOptionIndex].value;
	}

	bool parseValueAsBool(const char *rawValue, bool &errorOut, bool &warningOut)
	{
		// TODO: proper parsing!
		if (strcmp(rawValue, "true") == 0)
		{
			return true;
		}
		else if (strcmp(rawValue, "false") == 0)
		{
			return false;
		}

		// loosely checking cases...
		if (strcmp(rawValue, "True") == 0)
		{
			warningOut = true;
			return true;
		}
		else if (strcmp(rawValue, "False") == 0)
		{
			warningOut = true;
			return false;
		}

		if (strcmp(rawValue, "TRUE") == 0)
		{
			warningOut = true;
			return true;
		}
		else if (strcmp(rawValue, "FALSE") == 0)
		{
			warningOut = true;
			return false;
		}

		// loosely, accept 1 as true, 0 as false, and empty as false.
		if (strcmp(rawValue, "1") == 0)
		{
			warningOut = true;
			return true;
		}
		else if (strcmp(rawValue, "0") == 0 || strcmp(rawValue, "") == 0)
		{
			warningOut = true;
			return false;
		}

		errorOut = true;
		return false;
	}

	int parseValueAsInt(const char *rawValue, bool &errorOut, bool &warningOut)
	{
		// TODO: proper parsing
		bool hasCrap = false;
		int s = int(strlen(rawValue));
		for (int i = 0; i < s; i++)
		{
			if ((rawValue[i] >= '0' && rawValue[i] <= '9')
				|| (rawValue[i] == '-' && i == 0))
			{
				// ok
			} else {
				// non acceptable char found
				hasCrap = true;
			}
		}

		// loosely accept empty as 0
		if (rawValue[0] == '\0')
		{
			warningOut = true;
			return 0;
		}

		int ret = atoi(rawValue);
		if (ret == 0)
		{
			// zero you say? then if there was some other non-number crap too, that is probably an error. 
			// (note, whitespaces around an otherwise valid zero will cause this too)
			errorOut = true;
		} else {
			// other than zero, probably just some postfix crap or whitespaces, loosely accept it and just warn about the situation
			warningOut = hasCrap;
		}
		return ret;
	}

	uint32_t parseValueAsUnsignedInt(const char *rawValue, bool &errorOut, bool &warningOut)
	{
		uint32_t value = 0;
		if (sscanf(rawValue, "%u", &value) != 1)
			errorOut = true;

		return value;
	}

	float parseValueAsFloat(const char *rawValue, bool &errorOut, bool &warningOut)
	{
		// TODO: proper parsing
		bool hasCrap = false;
		int s = int(strlen(rawValue));
		for (int i = 0; i < s; i++)
		{
			if ((rawValue[i] >= '0' && rawValue[i] <= '9')
				|| (rawValue[i] == '-' && i == 0) 
				|| rawValue[i] == '.')
			{
				// ok
			} else {
				// non acceptable char found
				hasCrap = true;
			}
		}

		// loosely accept empty as 0
		if (rawValue[0] == '\0')
		{
			warningOut = true;
			return 0;
		}

		float ret = float(atof(rawValue));
		if (ret == 0)
		{
			// zero you say? then if there was some other non-number crap too, that is probably an error. 
			// (note, whitespaces around an otherwise valid zero will cause this too)
			errorOut = true;
		} else {
			// other than zero, probably just some postfix crap or whitespaces, loosely accept it and just warn about the situation
			warningOut = hasCrap;
		}
		return ret;
	}

	math::VC2 parseValueAsVc2(const char *rawValue, bool &errorOut, bool &warningOut)
	{
		math::VC2 result = math::VC2::zero;

		if (rawValue[0] != 'V' || rawValue[1] != 'C' || rawValue[2] != '2' || rawValue[3] != '(')
		{
			// Vector2 value must start with "VC2("
			errorOut = true;
			return result;
		}
		else
		{
			char buffer[256];
			SizeType i = 4;
			bool hasCrap = false;

			// Parse X
			{
				// Skip stuff
				while (!((rawValue[i] >= '0' && rawValue[i] <= '9') || (rawValue[i] == '-' && i == 0) || rawValue[i] == '.'))
				{
					if (rawValue[i] == '\0')
					{
						// Unexpected end of string
						errorOut = true;
						return result;
					}
					else if (!(rawValue[i] == ' ' || rawValue[i] == '\t' || rawValue[i] == '\r' || rawValue[i] == '\n'))
					{
						hasCrap = true;
					}
					++i;
				}
				SizeType firstStart = i;

				// strlen
				bool decimalDotEncountered = false;
				while ((rawValue[i] >= '0' && rawValue[i] <= '9') || (rawValue[i] == '-' && i == 0) || rawValue[i] == '.')
				{
					if (rawValue[i] == '.')
					{
						if (decimalDotEncountered)
						{
							errorOut = true; // Found multiple decimal dots
							return result;
						}
						decimalDotEncountered = true;
					}
					++i;
				}
				SizeType firstLength = i - firstStart;

				if (firstLength == 0 || firstLength >= 255)
				{
					// zero or invalid length string found
					errorOut = true;
					return result;
				}

				for (SizeType j = 0; j < firstLength; ++j)
				{
					buffer[j] = rawValue[firstStart + j];
				}
				buffer[firstLength] = '\0';

				result.x = float(atof(buffer));
			}

			// Parse y
			{
				// Skip stuff
				bool foundComma = false;
				while (!((rawValue[i] >= '0' && rawValue[i] <= '9') || (rawValue[i] == '-' && i == 0) || rawValue[i] == '.'))
				{
					if (rawValue[i] == '\0')
					{
						// Unexpected end of string
						errorOut = true;
						return result;
					}
					else if (rawValue[i] == ')')
					{
						// Closing bracket found unexpectedly before second value
						errorOut = true;
						return result;
					}
					else if (rawValue[i] == ',')
					{
						foundComma = true;
					}
					else if (!(rawValue[i] == ' ' || rawValue[i] == '\t' || rawValue[i] == '\r' || rawValue[i] == '\n'))
					{
						hasCrap = true;
					}
					++i;
				}
				if (!foundComma)
				{
					// Didn't find comma character in VC2 block
					warningOut = true;
				}

				// strlen
				bool decimalDotEncountered = false;
				SizeType secondStart = i;
				while ((rawValue[i] >= '0' && rawValue[i] <= '9') || (rawValue[i] == '-' && i == 0) || rawValue[i] == '.')
				{
					if (rawValue[i] == '.')
					{
						if (decimalDotEncountered)
						{
							errorOut = true; // Found multiple decimal dots
							return result;
						}
					}
					++i;
				}
				SizeType secondLength = i - secondStart;

				if (secondLength == 0 && secondLength >= 255)
				{
					// zero or invalid length string found
					errorOut = true;
					return result;
				}

				for (SizeType j = 0; j < secondLength; ++j)
				{
					buffer[j] = rawValue[secondStart + j];
				}
				buffer[secondLength] = '\0';
				result.y = float(atof(buffer));
			}

			if (hasCrap)
			{
				// Probably has some prefix or postfix crap around numbers
				warningOut = true;
			}
			else
			{
				bool closingBracketFound = false;
				while (rawValue[i] != '\0')
				{
					if (rawValue[i] == ')')
					{
						closingBracketFound  = true;
					}
					else if (!(rawValue[i] == ' ' || rawValue[i] == '\t' || rawValue[i] == '\r' || rawValue[i] == '\n'))
					{
						// Has some crap between nubmers and end
						warningOut = true;
					}
					++i;
				}

				if (!closingBracketFound)
				{
					warningOut = true;
				}
			}

		}

		return result;
	}

	math::COL parseValueAsCOL(const char *rawValue, bool &errorOut, bool &warningOut)
	{
		math::COL result = math::COL::white;

		if (rawValue[0] != 'C' || rawValue[1] != 'O' || rawValue[2] != 'L' || rawValue[3] != '(')
		{
			// Vector2 value must start with "VC2("
			errorOut = true;
			return result;
		}
		else
		{
			char buffer[256];
			SizeType i = 4;
			bool hasCrap = false;

			// Parse R
			{
				// Skip stuff
				while (!((rawValue[i] >= '0' && rawValue[i] <= '9') || (rawValue[i] == '-' && i == 0) || rawValue[i] == '.'))
				{
					if (rawValue[i] == '\0')
					{
						// Unexpected end of string
						errorOut = true;
						return result;
					}
					else if (!(rawValue[i] == ' ' || rawValue[i] == '\t' || rawValue[i] == '\r' || rawValue[i] == '\n'))
					{
						hasCrap = true;
					}
					++i;
				}
				SizeType firstStart = i;

				// strlen
				bool decimalDotEncountered = false;
				while ((rawValue[i] >= '0' && rawValue[i] <= '9') || (rawValue[i] == '-' && i == 0) || rawValue[i] == '.')
				{
					if (rawValue[i] == '.')
					{
						if (decimalDotEncountered)
						{
							errorOut = true; // Found multiple decimal dots
							return result;
						}
						decimalDotEncountered = true;
					}
					++i;
				}
				SizeType firstLength = i - firstStart;

				if (firstLength == 0 || firstLength >= 255)
				{
					// zero or invalid length string found
					errorOut = true;
					return result;
				}

				for (SizeType j = 0; j < firstLength; ++j)
				{
					buffer[j] = rawValue[firstStart + j];
				}
				buffer[firstLength] = '\0';

				result.r = float(atof(buffer));
			}

			// Parse G
			{
				// Skip stuff
				while (!((rawValue[i] >= '0' && rawValue[i] <= '9') || (rawValue[i] == '-' && i == 0) || rawValue[i] == '.'))
				{
					if (rawValue[i] == '\0')
					{
						// Unexpected end of string
						errorOut = true;
						return result;
					}
					else if (!(rawValue[i] == ' ' || rawValue[i] == '\t' || rawValue[i] == '\r' || rawValue[i] == '\n'))
					{
						hasCrap = true;
					}
					++i;
				}
				SizeType firstStart = i;

				// strlen
				bool decimalDotEncountered = false;
				while ((rawValue[i] >= '0' && rawValue[i] <= '9') || (rawValue[i] == '-' && i == 0) || rawValue[i] == '.')
				{
					if (rawValue[i] == '.')
					{
						if (decimalDotEncountered)
						{
							errorOut = true; // Found multiple decimal dots
							return result;
						}
						decimalDotEncountered = true;
					}
					++i;
				}
				SizeType firstLength = i - firstStart;

				if (firstLength == 0 || firstLength >= 255)
				{
					// zero or invalid length string found
					errorOut = true;
					return result;
				}

				for (SizeType j = 0; j < firstLength; ++j)
				{
					buffer[j] = rawValue[firstStart + j];
				}
				buffer[firstLength] = '\0';

				result.g = float(atof(buffer));
			}

			// Parse B
			{
				// Skip stuff
				bool foundComma = false;
				while (!((rawValue[i] >= '0' && rawValue[i] <= '9') || (rawValue[i] == '-' && i == 0) || rawValue[i] == '.'))
				{
					if (rawValue[i] == '\0')
					{
						// Unexpected end of string
						errorOut = true;
						return result;
					}
					else if (rawValue[i] == ')')
					{
						// Closing bracket found unexpectedly before second value
						errorOut = true;
						return result;
					}
					else if (rawValue[i] == ',')
					{
						foundComma = true;
					}
					else if (!(rawValue[i] == ' ' || rawValue[i] == '\t' || rawValue[i] == '\r' || rawValue[i] == '\n'))
					{
						hasCrap = true;
					}
					++i;
				}
				if (!foundComma)
				{
					// Didn't find comma character in VC2 block
					warningOut = true;
				}

				// strlen
				bool decimalDotEncountered = false;
				SizeType secondStart = i;
				while ((rawValue[i] >= '0' && rawValue[i] <= '9') || (rawValue[i] == '-' && i == 0) || rawValue[i] == '.')
				{
					if (rawValue[i] == '.')
					{
						if (decimalDotEncountered)
						{
							errorOut = true; // Found multiple decimal dots
							return result;
						}
					}
					++i;
				}
				SizeType secondLength = i - secondStart;

				if (secondLength == 0 && secondLength >= 255)
				{
					// zero or invalid length string found
					errorOut = true;
					return result;
				}

				for (SizeType j = 0; j < secondLength; ++j)
				{
					buffer[j] = rawValue[secondStart + j];
				}
				buffer[secondLength] = '\0';
				result.b = float(atof(buffer));
			}

			if (hasCrap)
			{
				// Probably has some prefix or postfix crap around numbers
				warningOut = true;
			}
			else
			{
				bool closingBracketFound = false;
				while (rawValue[i] != '\0')
				{
					if (rawValue[i] == ')')
					{
						closingBracketFound = true;
					}
					else if (!(rawValue[i] == ' ' || rawValue[i] == '\t' || rawValue[i] == '\r' || rawValue[i] == '\n'))
					{
						// Has some crap between nubmers and end
						warningOut = true;
					}
					++i;
				}

				if (!closingBracketFound)
				{
					warningOut = true;
				}
			}

		}

		return result;
	}

	const char *parseValueAsString(const char *rawValue, bool &errorOut, bool &warningOut)
	{
		parsedStringRetBuf[0] = '\0';

		size_t s = strlen(rawValue);

		// we have a limited buffer size for string parsing.
		if (s >= maxParsedStringLength)
		{
			FB_OFM_ASSERT(0 && "Raw value string length exceeds string parsing buffer size.");
			errorOut = true;
			return parsedStringRetBuf;
		}

		// whitespace trimming
		SizeType afterLeadingWhitespacePos = 0;
		bool trimmedStart = false;
		size_t trailingWhitespacePos = s;
		for (SizeType i = 0; i < s; i++)
		{
			if (rawValue[s] == ' ' || rawValue[s] == '\t')
			{
				if (!trimmedStart)
				{
					// possible end of leading whitespaces next maybe?
					afterLeadingWhitespacePos = i + 1;
				}
			}
			else
			{
				if (!trimmedStart)
				{
					trimmedStart = true;
				}
				else
				{
					// start of trailing whitespaces next maybe? (may and probably will get overridden later on)
					trailingWhitespacePos = i + 1;
				}
			}
		}

		size_t trimmedLen = trailingWhitespacePos - afterLeadingWhitespacePos;
		FB_OFM_ASSERT(trailingWhitespacePos >= afterLeadingWhitespacePos);
		if (trailingWhitespacePos > afterLeadingWhitespacePos)
		{
			// inside double quotes?
			const char *startOfTrimmedInRaw = &rawValue[afterLeadingWhitespacePos];
			if (trimmedLen >= 2 && startOfTrimmedInRaw[0] == '"' && startOfTrimmedInRaw[trimmedLen - 1] == '"')
			{
				strncpy(parsedStringRetBuf, &startOfTrimmedInRaw[1], trimmedLen - 2);
				parsedStringRetBuf[trimmedLen - 2] = '\0';
			} else {
				// surrounding double quotes missing. error.
				parsedStringRetBuf[0] = '\0';
				errorOut = true;
			}
		}
		else
		{
			// totally empty string, accept loosely with a warning?
			parsedStringRetBuf[0] = '\0';
			warningOut = true;
		}

		// convert double slashes to slashes (this obviously doesn't support escaped characters)
		for (int i = 0; parsedStringRetBuf[i] != 0; i++)
		{
			if (parsedStringRetBuf[i] == '\\' && parsedStringRetBuf[i+1] == '\\')
			{
				memmove(parsedStringRetBuf+i, parsedStringRetBuf+i+1, strlen(parsedStringRetBuf+i+1)+1);
				i++;
			}
		}

		return parsedStringRetBuf;
	}


	void parseLineErrorMessage(const char *msg, SizeType lineNumber)
	{
		containsParseErrors = true;
		if (errorListener != NULL)
		{
			errorListener->lineParseError(msg, lineNumber);
		}
	}


	void valueParseErrorMessage(const char *moduleName, const char *propertyName, const char *rawValue, const char *typeName)
	{
		if (errorListener != NULL)
		{
			TempString msg;
			msg += "Could not parse the value for module \"";
			msg += moduleName;
			msg += "\" property \"";
			msg += propertyName;
			msg += "\" as ";
			msg += typeName;
			msg += " type. (The string to parse was: \"";
			msg += rawValue;
			msg += "\".)";
			errorListener->optionTypeError(msg.getPointer());
		}
	}


	void valueParseWarningMessage(const char *moduleName, const char *propertyName, const char *rawValue, const char *typeName)
	{
		if (errorListener != NULL)
		{
			TempString msg;
			msg += "Parsing for module \"";
			msg += moduleName;
			msg += "\" property \"";
			msg += propertyName;
			msg += "\" value was loosely interpreted as ";
			msg += typeName;
			msg += " type, though the string did not appear to be of that type.";
			errorListener->optionTypeWarning(msg.getPointer());
		}
	}

	void addVersionConversionRule(const char *oldModuleName, const char *oldPropertyName, const char *oldValue, const char *newModuleName, const char *newPropertyName, const char *newValue)
	{
		FB_OFM_ASSERT(oldModuleName != NULL);
		FB_OFM_ASSERT(oldPropertyName != NULL);
		FB_OFM_ASSERT(newModuleName != NULL);
		FB_OFM_ASSERT(newPropertyName != NULL);

		// should never call this after load. 
		// (Even though the current version should survive the scenario, it makes little sense and one might in theory just use 
		// the string's internal c_str pointers for values, for optimality, in which case calling this after load could cause 
		// use of invalid pointers. Such optimization would require a more elaborate memory freeing logic as well though.)
		FB_OFM_ASSERT(!loadHasBeenCalled);

		VersionConversionRule r;
		r.oldModuleName = oldModuleName;
		r.oldPropertyName = oldPropertyName;
		r.newModuleName = newModuleName;
		r.newPropertyName = newPropertyName;
		if (oldValue != NULL)
		{
			r.nullOldValue = false;
			r.oldValue = oldValue;
		} else {
			r.nullOldValue = true;
			r.oldValue = "";
		}
		if (newValue != NULL)
		{
			r.nullNewValue = false;
			r.newValue = newValue;
		} else {
			r.nullNewValue = true;
			r.newValue = "";
		}

		versionConversionRules.pushBack(r);
	}

	friend class OptionsFileModifier;
};



OptionsFileModifier::OptionsFileModifier(const OptionsListType *knownOptionsList, SizeType amountOfKnownOptions, IOptionsFileModifierErrorListener *errorListener)
{
	impl = new Impl();

	impl->knownOptionsList = knownOptionsList;
	impl->amountOfKnownOptions = amountOfKnownOptions;
	impl->errorListener = errorListener;

	// Sanity check, cause a whole bunch of asserts if the provided const char array data is bogus.
	impl->fullSanityCheck();
}

OptionsFileModifier::~OptionsFileModifier()
{
	FB_OFM_ASSERT(impl != NULL);
	delete impl;
}

void OptionsFileModifier::addVersionConversionRule(const char *oldModuleName, const char *oldPropertyName, const char *oldValue, const char *newModuleName, const char *newPropertyName, const char *newValue)
{
	impl->addVersionConversionRule(oldModuleName, oldPropertyName, oldValue, newModuleName, newPropertyName, newValue);
}

bool OptionsFileModifier::load(const DynamicString &optionsFilePath, bool allowEmptyFile, bool allowNonExistingFile)
{
	impl->loadHasBeenCalled = true;
	impl->unloadData();

	file::FileManager &fileManager = fb::getFileManager();
	file::File optFile = fileManager.openFile(optionsFilePath);
	if (optFile)
	{
		if (optFile.getSize() == 0)
		{
			if (allowEmptyFile)
			{
				// success by loading an empty file
				FB_OFM_ASSERT(impl->dataBufferSplitted == NULL);
				impl->dataBufferSplitted = new char[1];
				impl->dataBufferSplitted[0] = '\0';
				impl->dataBufferSplittedSize = 1;
				impl->loaded = true;
				impl->loadedFilePath = optionsFilePath;
				return true;
			}
			else
			{
				return false;
			}
		}
		FB_OFM_ASSERT(optFile.getSize() > 0);
		FB_OFM_ASSERT(impl->dataBufferSplitted == NULL);

		stream::InputStream<fb::lang::BigEndian> strm(optFile.getData(), (SizeType)optFile.getSize());
		loadFromStream(strm.getData() + strm.getBytesRead(), strm.getBytesLeft());

		// check file version
		// if differs, save original options to "options_vXXX.txt", then load defaults and overwrite the original options file
		if (impl->parsedVersion != getOptionsVersion())
		{
			// trim filename from full path
			TempString originalPath(optionsFilePath);
			TempString backupPath;
			backupPath.appendPathFromString(originalPath);
			/* Trim / from the end */
			backupPath.trimRight(1);

			// convert version to string (should use snprintf for safety)
			char versionString[32];
			sprintf(versionString, "%d", impl->parsedVersion);

			// assemble new path
			backupPath << "/options_v" << versionString << ".txt";

			// back up original options
			impl->loadedFilePath = backupPath;
			save();

			// load defaults and overwrite the original options file
			impl->loadedFilePath = originalPath;
			impl->parsedVersion = getOptionsVersion();
			clearAllAndSave();
		}

		return true;
	}
	else
	{
		if (allowNonExistingFile)
		{
			FB_OFM_ASSERT(impl->dataBufferSplitted == NULL);
			impl->dataBufferSplitted = new char[1];
			impl->dataBufferSplitted[0] = '\0';
			impl->dataBufferSplittedSize = 1;
			impl->loaded = true;
			impl->loadedFilePath = HeapString(optionsFilePath);
			return true;
		} else {
			return false;
		}
	}
}

void OptionsFileModifier::loadFromStream(const void *data, SizeType dataSize)
{
	impl->loadHasBeenCalled = true;
	impl->unloadData();

	FB_OFM_ASSERT(impl->dataBufferSplitted == NULL);
	impl->dataBufferSplitted = new char[dataSize + 1];
	memcpy(impl->dataBufferSplitted, data, dataSize);
	impl->dataBufferSplitted[dataSize] = '\0';
	impl->loaded = true;
	impl->loadedFilePath.clear();
	impl->dataBufferSplittedSize = dataSize + 1;
	
	impl->parseLoaded();
}

bool OptionsFileModifier::save()
{
	FB_OFM_ASSERT(impl->loaded);
	FB_OFM_ASSERT(!impl->loadedFilePath.isEmpty());

	if (impl->loadedFilePath.isEmpty())
	{
		// attempt to save when nothing loaded?
		return false;
	}

	// create the config folder, unless it already exists.	
	// for whatever reason, this is not available for the consoles... thus, doing this for PC only...
	file::createPathIfMissing(impl->loadedFilePath);
	return saveAs(impl->loadedFilePath);
}


bool OptionsFileModifier::saveAs(const StringRef &optionsFilePath)
{
	FB_OFM_ASSERT(impl->loaded);

	// write all of the parsed in options (and comments) back to the file
	static const int maxSaveSize = 1024*1024; // 1 megabyte
	char *tmp = new char[maxSaveSize];
	size_t saveSize = getSaveToMemoryBuffer(tmp, maxSaveSize);

	if (saveSize == 0)
	{
		delete [] tmp;
		return false;
	}

	file::OutputFile optFile;
	if (optFile.open(optionsFilePath))
	{
		/* No error handling in writeData() */
		optFile.writeData(tmp, saveSize);
		if (impl->loadedFilePath != optionsFilePath)
		{
			// NOTE: save calls this function, and gives a pointer parameter inside this string we're about to set, so after re-setting it, 
			// we cannot use the function param anymore!
			impl->loadedFilePath = optionsFilePath;
		}

		delete [] tmp;
		return true;
	}
	else
	{
		delete [] tmp;
		return false;
	}
}


bool OptionsFileModifier::wasOptionParsedFromFile(const char *moduleName, const char *propertyName) const
{
	SizeType parsedIndex = impl->findParsedOptionIndex(moduleName, propertyName);
	return parsedIndex != getInvalidIndex();
}


const char *OptionsFileModifier::getOptionValueAllowingUnknown(const char *moduleName, const char *propertyName) const
{
	FB_OFM_ASSERT(moduleName != NULL);
	FB_OFM_ASSERT(propertyName != NULL);
	FB_OFM_ASSERT(impl->loaded);

	// was the value parsed in from the file?
	SizeType parsedIndex = impl->findParsedOptionIndex(moduleName, propertyName);
	if (parsedIndex != getInvalidIndex())
	{
		return impl->getParsedOptionValue(parsedIndex);
	}

	// if it was not parsed in, return known default... if one exists.
	SizeType knownOptIndex = impl->findKnownIndexForOption(moduleName, propertyName);
	if (knownOptIndex != getInvalidIndex())
	{
		return impl->getDefaultValueForKnownIndex(knownOptIndex);
	}

	// not in list of known, nor in parsed options, return null.
	return NULL;
}


const char *OptionsFileModifier::getOptionValue(const char *moduleName, const char *propertyName) const
{
	FB_OFM_ASSERT(moduleName != NULL);
	FB_OFM_ASSERT(propertyName != NULL);
	FB_OFM_ASSERT(impl->loaded);

	SizeType knownOptIndex = impl->findKnownIndexForOption(moduleName, propertyName);
	if (knownOptIndex == getInvalidIndex())
	{
		// you may not try to get values for options that are not known.
		// (use getOptionValueAllowingUnknown if you really must be able to do so)
		FB_OFM_ASSERT(0 && "Attempt to get an option value for an unknown option.");
		return NULL;
	}

	// was the value parsed in from the file?
	SizeType parsedIndex = impl->findParsedOptionIndex(moduleName, propertyName);
	if (parsedIndex != -1)
	{
		return impl->getParsedOptionValue(parsedIndex);
	}

	// if it was not parsed in, return known default...
	return impl->getDefaultValueForKnownIndex(knownOptIndex);
}


int OptionsFileModifier::getIntOptionValue(const char *moduleName, const char *propertyName) const
{
	const char *v = getOptionValueAllowingUnknown(moduleName, propertyName);
	if (v != NULL)
	{
		bool parseError = false;
		bool parseWarning = false;
		int ret = impl->parseValueAsInt(v, parseError, parseWarning);
		if (parseError) { impl->valueParseErrorMessage(moduleName, propertyName, v, "integer"); }
		if (parseWarning) { impl->valueParseWarningMessage(moduleName, propertyName, v, "integer"); }
		return ret;
	}
	FB_OFM_ASSERT(0 && "Cannot parse a null option value.");
	return 0;
}


uint32_t OptionsFileModifier::getUnsignedIntOptionValue(const char *moduleName, const char *propertyName) const
{
	const char *v = getOptionValueAllowingUnknown(moduleName, propertyName);
	if (v != NULL)
	{
		bool parseError = false;
		bool parseWarning = false;
		uint32_t ret = impl->parseValueAsUnsignedInt(v, parseError, parseWarning);
		if (parseError) { impl->valueParseErrorMessage(moduleName, propertyName, v, "unsigned integer"); }
		if (parseWarning) { impl->valueParseWarningMessage(moduleName, propertyName, v, "unsigned integer"); }
		return ret;
	}
	FB_OFM_ASSERT(0 && "Cannot parse a null option value.");
	return 0;
}


bool OptionsFileModifier::getBoolOptionValue(const char *moduleName, const char *propertyName) const
{
	const char *v = getOptionValueAllowingUnknown(moduleName, propertyName);
	if (v != NULL)
	{
		bool parseError = false;
		bool parseWarning = false;
		bool ret = impl->parseValueAsBool(v, parseError, parseWarning);
		if (parseError) { impl->valueParseErrorMessage(moduleName, propertyName, v, "boolean"); }
		if (parseWarning) { impl->valueParseWarningMessage(moduleName, propertyName, v, "boolean"); }
		return ret;
	}
	FB_OFM_ASSERT(0 && "Cannot parse a null option value.");
	return false;
}


float OptionsFileModifier::getFloatOptionValue(const char *moduleName, const char *propertyName) const
{
	const char *v = getOptionValueAllowingUnknown(moduleName, propertyName);
	if (v != NULL)
	{
		bool parseError = false;
		bool parseWarning = false;
		float ret = impl->parseValueAsFloat(v, parseError, parseWarning);
		if (parseError) { impl->valueParseErrorMessage(moduleName, propertyName, v, "floating point"); }
		if (parseWarning) { impl->valueParseWarningMessage(moduleName, propertyName, v, "floating point"); }
		return ret;
	}
	FB_OFM_ASSERT(0 && "Cannot parse a null option value.");
	return 0.0f;
}

fb::math::VC2 OptionsFileModifier::getVc2OptionValue(const char *moduleName, const char *propertyName) const
{
	const char *v = getOptionValueAllowingUnknown(moduleName, propertyName);
	if (v != NULL)
	{
		bool parseError = false;
		bool parseWarning = false;
		math::VC2 ret = impl->parseValueAsVc2(v, parseError, parseWarning);
		if (parseError) { impl->valueParseErrorMessage(moduleName, propertyName, v, "Vector2"); }
		if (parseWarning) { impl->valueParseWarningMessage(moduleName, propertyName, v, "Vector2"); }
		return ret;
	}
	FB_OFM_ASSERT(0 && "Cannot parse a null option value.");
	return math::VC2::zero;
}

fb::math::COL OptionsFileModifier::getColOptionValue(const char *moduleName, const char *propertyName) const
{
	const char *v = getOptionValueAllowingUnknown(moduleName, propertyName);
	if (v != NULL)
	{
		bool parseError = false;
		bool parseWarning = false;
		math::COL ret = impl->parseValueAsCOL(v, parseError, parseWarning);
		if (parseError) { impl->valueParseErrorMessage(moduleName, propertyName, v, "Vector2"); }
		if (parseWarning) { impl->valueParseWarningMessage(moduleName, propertyName, v, "Vector2"); }
		return ret;
	}
	FB_OFM_ASSERT(0 && "Cannot parse a null option value.");
	return math::COL::white;
}

const char *OptionsFileModifier::getStringOptionValue(const char *moduleName, const char *propertyName) const
{
	const char *v = getOptionValueAllowingUnknown(moduleName, propertyName);
	if (v != NULL)
	{
		bool parseError = false;
		bool parseWarning = false;
		const char *ret = impl->parseValueAsString(v, parseError, parseWarning);
		if (parseError) { impl->valueParseErrorMessage(moduleName, propertyName, v, "string"); }
		if (parseWarning) { impl->valueParseWarningMessage(moduleName, propertyName, v, "string"); }
		return ret;
	}
	FB_OFM_ASSERT(0 && "Cannot parse a null option value.");
	return NULL;
}


void OptionsFileModifier::setOptionValue(const char *moduleName, const char *propertyName, const char *value)
{
	FB_OFM_ASSERT(moduleName != NULL);
	FB_OFM_ASSERT(propertyName != NULL);
	FB_OFM_ASSERT(value != NULL);
	FB_OFM_ASSERT(impl->loaded);
	FB_OFM_ASSERT(impl->dataBufferSplitted != NULL);

	SizeType index = impl->findParsedOptionIndex(moduleName, propertyName);
	if (index != getInvalidIndex())
	{
		// previously parsed in, modify the value only...
		if (impl->parsedOptions[index].value >= impl->dataBufferSplitted
			&& impl->parsedOptions[index].value < impl->dataBufferSplitted + impl->dataBufferSplittedSize)
		{
			// inside the split buffer, ignore free
		}
		else
		{
			delete [] impl->parsedOptions[index].value;
		}
		impl->parsedOptions[index].value = NULL;


		size_t slen = strlen(value);
		char *newValueBuf = new char[slen + 1];
		strcpy(newValueBuf, value);
		impl->parsedOptions[index].value = newValueBuf;
	}
	else
	{
		// was not previously parsed in, add it to the list of parsed values.
		// these modified entries will be allocated from the heap without pooling, 
		// but only the launcher generally modifies them, so it should not really matter.
		char *mCopy = new char[strlen(moduleName) + 1]; 
		strcpy(mCopy, moduleName);
		char *pCopy = new char[strlen(propertyName) + 1];
		strcpy(pCopy, propertyName);
		char *vCopy = new char[strlen(value) + 1];
		strcpy(vCopy, value);
		impl->parsedOptions.pushBack(fb::util::OptionsFileModifier::Impl::OptionEntry(mCopy, pCopy, vCopy));
	}
}


void OptionsFileModifier::setIntOptionValue(const char *moduleName, const char *propertyName, int value)
{
	char buf[40];
	sprintf(buf, "%d", value);
	setOptionValue(moduleName, propertyName, buf);
}


void OptionsFileModifier::setBoolOptionValue(const char *moduleName, const char *propertyName, bool value)
{
	if (value)
	{
		setOptionValue(moduleName, propertyName, "true");
	} else {
		setOptionValue(moduleName, propertyName, "false");
	}
}


void OptionsFileModifier::setFloatOptionValue(const char *moduleName, const char *propertyName, float value)
{
	char buf[64];
	sprintf(buf, "%f", value);
	setOptionValue(moduleName, propertyName, buf);
}


void OptionsFileModifier::setStringOptionValue(const char *moduleName, const char *propertyName, const char *value)
{
	// encapsulate in quotes.
	// TODO: should ensure there are no quotes within the string!
	char *buf = new char[strlen(value) + 2 + 1];
	sprintf(buf, "\"%s\"", value);
	setOptionValue(moduleName, propertyName, buf);
	delete [] buf;
}


bool OptionsFileModifier::resetAllToDefaultsFromFile(const char *defaultOptionsFilePath, bool clearOnCopyFailure)
{
	// TODO: ...
	return true;
}


bool OptionsFileModifier::resetRecognizedToDefaults()
{
	// TODO: ...
	return true;
}


SizeType OptionsFileModifier::getSaveToMemoryBuffer(char *buffer, SizeType maxSize)
{
	FB_OFM_ASSERT(maxSize > 0);

	buffer[0] = '\0';

	SizeType printedChars = 0;

	// print file version first
	int len = sprintf(&buffer[printedChars], "setVersion(%d)" FB_PLATFORM_LF, impl->parsedVersion);
	if (len < 0)
	{
		// sprintf failure?
		return 0;
	}
	printedChars += len;

	for (SizeType i = 0; i < impl->parsedOptions.getSize(); i++)
	{
		// FIXME: should use snprintf instead for actual safeness!!! (or calculate the correct line print lengths in advance)
		// For now, just assuming no single line exceeds 5000 bytes.
		// THIS CAN BUFFER OVERFLOW IF AN OPTION LINE LENGTH EXCEEDS THAT AMOUNT!!!

		if (printedChars + 5000 > maxSize)
		{
			// out of buffer size.
			return 0;
		}		

		int p = 0;
		if (impl->parsedOptions[i].moduleName == NULL && impl->parsedOptions[i].propertyName == NULL)
		{
			// comment line apparently.
			if (impl->parsedOptions[i].value != NULL)
			{
				p = sprintf(&buffer[printedChars], "%s" FB_PLATFORM_LF, impl->parsedOptions[i].value);
			}
		}
		else
		{
			// normal option setting line.
			p = sprintf(&buffer[printedChars], "setOption(%s, \"%s\", %s)" FB_PLATFORM_LF, impl->parsedOptions[i].moduleName, impl->parsedOptions[i].propertyName, impl->parsedOptions[i].value);
		}
		if (p < 0)
		{
			// sprintf failure?
			return 0;
		}
		printedChars += p;
	}

	FB_OFM_ASSERT(strlen(buffer) == printedChars);
	return printedChars;
}


void OptionsFileModifier::clearAllAndSave()
{
	FB_OFM_ASSERT(impl->loaded);

	// this is actually "clear parsed options" (yet, won't free the splitted loaded buffer at this point)
	impl->unallocateAllOptions();

	save();
}


/*
void OptionsFileModifier::rewriteFile(bool recognizedOptionsOnly)
{
	// TODO: ...
	// (this has little use over save, which currently also does the same, except having no recognizedOptionsOnly option)
	// (in the future, if the save didn't actually re-write the file every time, this might be sensible)
}
*/
	

bool OptionsFileModifier::doesContainUnrecognizedOptions() const
{
	return impl->containsUnrecognizedOptions;
}


bool OptionsFileModifier::doesContainParseFailingLines() const
{
	return impl->containsParseErrors;
}


SizeType OptionsFileModifier::getNumParsedOptions()
{
	return impl->parsedOptions.getSize();
}

bool OptionsFileModifier::getParsedOptionNameByIndex(SizeType index, const char **moduleNameOut, const char **propertyNameOut) const
{
	FB_OFM_ASSERT(index < impl->parsedOptions.getSize());
	if (index < impl->parsedOptions.getSize())
	{
		if (impl->parsedOptions[index].moduleName != NULL)
		{
			*moduleNameOut = impl->parsedOptions[index].moduleName;
			*propertyNameOut = impl->parsedOptions[index].propertyName;
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

FB_END_PACKAGE1()

