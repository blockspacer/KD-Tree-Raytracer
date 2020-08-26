#pragma once

FB_PACKAGE2(sys, versioncontrol)

class SVN
{
public:
	/// Potentially expensive construction. Caching the instance recommended 
	/// if running multiple commands on row.
	SVN();
	~SVN();

	enum Command
	{
		Update,
		Commit,
		Add,
		Delete,
		Lock,
		Unlock
	};

	/**
	 * Checks timestamp to see if file could have been modified
	 */
	bool isFilePossiblyChanged(const StringRef &filename);
	bool isInVersionControl(const StringRef &filename);
	bool runCommand(Command command, const HeapString *files, int fileAmount, const HeapString &logMessage);

private:
	// Not implemented
	SVN(const SVN &);
	void operator =(const SVN &);

	struct Data;
	Data *data;
};

FB_END_PACKAGE2()
