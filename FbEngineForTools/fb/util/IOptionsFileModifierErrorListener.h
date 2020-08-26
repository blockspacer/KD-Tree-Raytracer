#pragma once

FB_PACKAGE1(util)

/**
 * The OptionsFileModifier type errors may be monitored by providing an object implementing this interface. "OptionsFileModifier logger"
 * Getting or setting the option values may cause typing errors to occur, when the parsed value or the type defined in the list or in file
 * do not seem to match. 
 *
 * The options modifier will still generally attempt to parse the value. And in some occasions the result may be sensible regardless
 * of the error, but sometimes it might not.
 *
 * If not listener is provided, the modifier will just silently ignore any of these errors and attempt to perform the parsing.
 */
class IOptionsFileModifierErrorListener
{
public:
	virtual ~IOptionsFileModifierErrorListener() { };

	/**
	 * Called whenever some warning occurs (mostly when some type does not fully match, but a "human-readable" assumption is made and the value
	 * has been converted in a way that probably makes sense). These can usually be ignored.
	 */
	virtual void optionTypeWarning(const char *msg) = 0;

	/**
	 * When a type did not match or could not be properly parsed. The user should usually be notified of this.
	 */
	virtual void optionTypeError(const char *msg) = 0;

	/**
	 * When a line parsing fails completely, containing a syntax error or something.
	 */
	virtual void lineParseError(const char *msg, SizeType lineNumber) = 0;
};


FB_END_PACKAGE1()
