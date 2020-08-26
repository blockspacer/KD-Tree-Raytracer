#pragma once

FB_PACKAGE1(toolsengine)

class ConsoleOutputReceiver : public lang::ICharacterOutputReceiver
{
public:
	virtual void write(const StringRef &str);
};

FB_END_PACKAGE1()
