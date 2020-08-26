#pragma once

#include <assert.h>

#include "fb/lang/Const.h"
#include "fb/math/Vec2.h"
#include "fb/math/Color3.h"


// NOTICE!!!
// By default, assuming that this is included by the engine/game, thus allowing all sorts of includes.
// If that is not the case though, you should define FB_OPTIONS_LEAN_AND_MEAN to strip all the extra dependencies off...


#ifndef FB_FALSE 
	#error "Missing FB_FALSE define."
#endif
#ifndef FB_TRUE
	#error "Missing FB_TRUE define."
#endif
#if (FB_TRUE == FB_FALSE)
	#error "FB_TRUE define is the same as FB_FALSE."
#endif


// should we support temporary heap string?
#ifndef FB_UTIL_OPTIONSFILEMODIFIER_SUPPORT_TEMPORARYHEAPSTRING
	#ifdef FB_OPTIONS_LEAN_AND_MEAN
		// going standalone, with minimal engine includes.
		#define FB_UTIL_OPTIONSFILEMODIFIER_SUPPORT_TEMPORARYHEAPSTRING FB_FALSE
	#else
		// assuming engine integration by default
		#define FB_UTIL_OPTIONSFILEMODIFIER_SUPPORT_TEMPORARYHEAPSTRING FB_TRUE
	#endif
#endif

// which assert to use?
#ifdef FB_OPTIONS_LEAN_AND_MEAN
	#define FB_OFM_ASSERT assert
#else
	#include "fb/lang/FBAssert.h"
	#define FB_OFM_ASSERT fb_assert
#endif

#if (FB_UTIL_OPTIONSFILEMODIFIER_SUPPORT_TEMPORARYHEAPSTRING == FB_TRUE)
#include "fb/string/HeapString.h"
#endif

#include "IOptionsFileModifierErrorListener.h"

FB_PACKAGE1(util)

/**
 * The option "manager" for an external (launcher/configurator) program. 
 * Can be used by the game engine as well, to read in the options file as text, rather than reading them in and running as Lua script.
 */
class OptionsFileModifier
{
public:
	class Impl;

	static const int maxOptions = 128; // max number of options supported. (actual number may be lower)
	static const int stringsPerOptionEntry = 4;
	typedef const char *OptionStringType;
	typedef OptionStringType OptionsListType[maxOptions][stringsPerOptionEntry];

	/**
	 * Options are versioned. The version number should be increased if a change requires options.txt to be invalidated.
	 * options.txt should have version number (otherwise it is considered to be 1). When the file is read, the version is checked,
	 * and in case of mismatch, the file is backed up and the default options are loaded and written to options.txt
	 */
	FB_CONST_POD(int, OptionsVersion, 1);

	/**
	 * @param knownOptionsList, a multidimensional array of C-strings, the known options list must be provided, 
	 * but the modifier does not need to be aware of all possible options.
	 * The list must contain 4 strings per option, option type, the module name, property name and default value.
	 * The default value may be provided as NULL to indicate no default value support. (Note, it is best to give all values as NULLs in that case)
	 * The list must be terminated with 4 NULLs (This is to be included in the given amountOfKnownOptions).
	 * The supported types can be "int", "bool", "float" or "string" or "" (empty) for an unspecified type.
	 *
	 * For example, the following is an array that should be passed with the size 3
	 * { { "int", "MyModule", "MyProperty", "123" }, { "int", "AnotherModule", "AnotherProperty", "456" }, { NULL, NULL, NULL, NULL } }
	 */
	OptionsFileModifier(const OptionsListType *knownOptionsList, SizeType amountOfKnownOptions, IOptionsFileModifierErrorListener *errorListener = NULL);
	~OptionsFileModifier();

	/**
	 * Adds an options file version conversion info entry. This can be used to support backward compatibility for old options files
	 * which have relocated/renamed properties or modules. ,
	 * These conversion entries should be added right after creating the options file modifier (before calling load).
	 * The old and new module and property names must always be provided but the oldValue and newValue parameters are optional.
	 *
	 * @param oldValue, when a non-null string is specified, this value works as a condition and the conversion is only applied if
	 *        the loaded value matches this one. (Notice, that string values are provided inside additional double quotes.)
	 * @param newValue, when a non-null string is specified, this value will replace the value that was read in.
	 *
	 * Usage examples:
	 * - To correctly convert a case where an options file entry contains a module which has been renamed. 
	 * addVersionConversionRule("myOldModule", "Prop1", NULL, "theNewModule", "Prop1", NULL);
	 * 
	 * - To correctly convert a case where an options file entry contains a property which has been renamed (but still using same values)
	 * addVersionConversionRule("someModule", "OldPropName", NULL, "someModule", "NewPropName", NULL);
	 *
	 * - To convert a property type by specifying some conversion values (for example, a boolean prop to some int prop value)
	 * addVersionConversionRule("someModule", "SomeBoolProp", "false", "someModule", "TheNewIntProp", "0");
	 * addVersionConversionRule("someModule", "SomeBoolProp", "true", "someModule", "TheNewIntProp", "1");
	 *
	 * - To reset some known incorrect specific property value (lets say value 2 is not acceptable anymore best replacement is 0,
	 *   still leaving any other values intact)
	 * addVersionConversionRule("myModule", "MyProp", "2", "myModule", "MyProp", "0");
	 *
	 * - To always reset a property value to some specific value, regardless of its old value (in this case, a string value)
	 * addVersionConversionRule("myModule", "MyName", NULL, "myModule", "MyName", "\"DefaultUserName\"");
	 *
	 * All of the module name, property name and optionally the value may be converted simultaneously by a single entry.
	 * Also, in case of old value specifications, multiple entries may be supplied, one entry for each old value to be converted.
	 */
	void addVersionConversionRule(const char *oldModuleName, const char *oldPropertyName, const char *oldValue, const char *newModuleName, const char *newPropertyName, const char *newValue);

	/**
	 * Loads the options from the given file.
	 *
	 * @param allowEmptyFile bool, when true completely empty files are considered to be ok, otherwise a non-options file.
	 * @param allowNonExistingFile bool, when true completely empty files are considered to be ok.
	 *
	 * @return true on success, false if the file open completely failed or the file did not seem like a proper options file at all.
	 * Any single line parse errors will not cause this to return false, use containsUnrecognizedOptions() and containsParseFailingLines() to query 
	 * about loading failures in detail.
	 */
	bool load(const DynamicString &optionsFilePath, bool allowEmptyFile = true, bool allowNonExistingFile = true);
	void loadFromStream(const void *data, SizeType dataSize);

	/**
	 * Saves the options back to the file it was loaded from.
	 *
	 * @return true on success, false if the file could not be saved to.
	 */
	bool save();

	/**
	 * Saves the options to the given file.
	 * (If the file already exists, silently re-writes it.) 
	 *
	 * @return true on success
	 */
	bool saveAs(const StringRef &optionsFilePath);

	/**
	 * Writes the save file to given memory buffer, at most given maxSize size.
	 * @return the string length written to the buffer (not counting the null terminator, which will be written as well).
	 *         or -1, if the given maxSize was not adequate large to hold the entire save data (plus a null terminator char).
	 */
	SizeType getSaveToMemoryBuffer(char *buffer, SizeType maxSize);

	/**
	 * Returns true if the option was parsed from the options file.
	 * (You can use this to differentiate the values parsed from the options file and the values returned due to defaults)
	 */
	bool wasOptionParsedFromFile(const char *moduleName, const char *propertyName) const;

	/**
	 * Returns the unparsed/unprocessed option value. 
	 * The option must be in the list of known option values.
	 * Notice, that string values, for example, will be contained in double quotes in this case.
	 * If the option did not exist in the options file, but it has a known default value, it is returned istead.
	 * (To differentiate the default and parsed values, use wasOptionParsedFromFile)
	 *
	 * NOTICE: The returned string is only guaranteed to be valid until any other function call, make a copy of the string ASAP.
	 * NOTICE: In case of (asserting) failure, may return NULL.
	 *
	 * @see getOptionValueAsTemporaryHeapString
	 * @see getOptionValueAllowingUnknown
	 */
	const char *getOptionValue(const char *moduleName, const char *propertyName) const;

	/**
	 * Like getOptionValue, but allows getting values of option that were parsed in but were not listed in the known options list.
	 * @return the value for the parsed option or default if the option was not parsed in, or NULL if unknown option and not parsed in either.
	 */
	const char *getOptionValueAllowingUnknown(const char *moduleName, const char *propertyName) const;

	/**
	 * Returns the given options value, parsed as an integer.
	 */
	int getIntOptionValue(const char *moduleName, const char *propertyName) const;

	/**
	 * Returns the given options value, parsed as an unsigned integer.
	 */
	uint32_t getUnsignedIntOptionValue(const char *moduleName, const char *propertyName) const;

	/**
	 * Returns the given option value, parsed as a boolean value.
	 */
	bool getBoolOptionValue(const char *moduleName, const char *propertyName) const;

	/**
	 * Returns the given option value, parsed as a float value.
	 */
	float getFloatOptionValue(const char *moduleName, const char *propertyName) const;
	
	/**
	 * Returns the given options value, parsed as an Vector2.
	 */
	fb::math::VC2 getVc2OptionValue(const char *moduleName, const char *propertyName) const;
	
	/**
	 * Returns the given options value, parsed as an math::COL.
	 */
	fb::math::COL getColOptionValue(const char *moduleName, const char *propertyName) const;

	/**
	 * Returns the given option value, parsed as a string.
	 * Note, even though strings are contained within double-quotes, but this method will return the string without the quotes.
	 *
	 * NOTICE: The returned string is only guaranteed to be valid until any other function call, make a copy of the string ASAP.
	 * NOTICE: In case of (asserting) failure, may return NULL.
	 *
	 * @see getStringOptionValueAsTemporaryHeapString
	 */
	const char *getStringOptionValue(const char *moduleName, const char *propertyName) const;

	/**
	 * Sets the given option to a specific value, regardless of the option type.
	 * Notice, that string values, for example, will need to be contained in double quotes in this case.
	 */
	void setOptionValue(const char *moduleName, const char *propertyName, const char *value);

	void setIntOptionValue(const char *moduleName, const char *propertyName, int value);
	void setBoolOptionValue(const char *moduleName, const char *propertyName, bool value);
	void setFloatOptionValue(const char *moduleName, const char *propertyName, float value);
	void setStringOptionValue(const char *moduleName, const char *propertyName, const char *value);

#if (FB_UTIL_OPTIONSFILEMODIFIER_SUPPORT_TEMPORARYHEAPSTRING == FB_TRUE)
	/**
	 * A safer version of the const char * returning method, but supporting this drags a whole lot of code to be included with it.
	 * (Also, incidentally, this is more ineffective)
	 *
	 * Returns the unparsed/unprocessed option value.
	 * Notice, that string values, for example, will be contained in double quotes in this case.
	 */
	inline HeapString getOptionValueAsTemporaryHeapString(const char *moduleName, const char *propertyName) const
	{
		const char *s = getOptionValue(moduleName, propertyName);
		if (s != NULL) 
		{
			return HeapString(s);
		} else {
			return HeapString::empty;
		}
	}

	inline HeapString getStringOptionValueAsTemporaryHeapString(const char *moduleName, const char *propertyName) const
	{
		const char *s = getStringOptionValue(moduleName, propertyName);
		if (s != NULL) 
		{
			return HeapString(s);
		} else {
			return HeapString::empty;
		}
	}

#endif


	/**
	 * Just simply copies the contents of the given default file to the contents of the options file.
	 *
	 * @param clearOnCopyFailure bool, if the clearOnCopyFailure is true, then if the default options file is not found or reading it fails, 
	 * a clear will be performed instead.
	 * @return true on success (including on clear, if the clear parameter was set to true and file was not found), false on failure
	 */
	bool resetAllToDefaultsFromFile(const char *defaultOptionsFilePath, bool clearOnCopyFailure);

	/**
	 * Resets all options to defaults from the known options list.
	 * Notice, that this reset will not remove any malformed options, etc. Only the recognized values are reset.
	 *
	 * @return true on success. Can only fail and return false, if the knownOptions list has some NULL values for defaults (indicating 
	 * that no hard-coded defaults are provided.
	 */
	bool resetRecognizedToDefaults();

	/**
	 * Clears the entire options file (assumes that the engine will use defaults instead, and then later on re-write the options text)
	 * Will modify the file immediately (without an explicit save call)!
	 */
	void clearAllAndSave();

	/**
	 * Re-writes the entire options file contents in the memory (based on successfully parsed values). 
	 * YOU MUST CALL save or saveAs to actually write the changes to disk though!
	 * The rewrite attempts to preserve the option values it manages to parse in, but any malformed lines or comment lines, etc. will be lost.
	 *
	 * @param recognizedOptionsOnly bool, if the recognizedOptionsOnly is true, then only those options that this modifier supports, will be written 
	 * (use of this feature is NOT advised, since if this file is not up to date, some options may be lost, even when they are valid.)
	 */
	//void rewriteFile(bool recognizedOptionsOnly);
	
	/**
	 * @return true if the options file contains some options that are not listed in the known options list given at construction time.
	 * This is not necessarily an error, it just means that this modifier does not know about some options that the actual game/engine might 
	 * know about. It means however, that rewriteFile(true) would lose such options.
	 */
	bool doesContainUnrecognizedOptions() const;

	/**
	 * Queries if the options had some lines that were not successfully parsed in.
	 * This should be considered as a syntax error in the file. (Or the engine/game supporting some syntax unsupported by this modifier)
	 *
	 * @return true if the options files was considered to contain unrecognized options or otherwise possibly malformed lines.
	 * If you get true from this, rewriteFile() will always lose something from the file.
	 */
	bool doesContainParseFailingLines() const;

	/**
	 * Hacked up parsed option iteration, you should normally have no use for these.
	 * Returns true if next parsed option is available, and the next module/property name to the given out pointers.
	 */
	bool getFirstParsedOption(const char **nextModuleNameOut, const char **nextPropertyNameOut) const;
	bool getNextParsedOption(const char *moduleName, const char *propertyName, const char **nextModuleNameOut, const char **nextPropertyNameOut) const;

	SizeType getNumParsedOptions();
	bool getParsedOptionNameByIndex(SizeType index, const char **moduleNameOut, const char **propertyNameOut) const;

	/**
	 * Hacked up known option list iteration, you should normally have no use for these.
	 * Returns true if next known option is available, and the next module/property name to the given out pointers.
	 */
	bool getFirstKnownOption(const char **nextModuleNameOut, const char **nextPropertyNameOut) const;
	bool getNextKnownOption(const char *moduleName, const char *propertyName, const char **nextModuleNameOut, const char **nextPropertyNameOut) const;

private:
	FB_CONST_POD(SizeType, InvalidIndex, 0xFFFFFFFF);

	Impl *impl;
};


FB_END_PACKAGE1()
