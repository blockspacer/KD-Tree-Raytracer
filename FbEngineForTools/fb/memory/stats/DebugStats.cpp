#include "Precompiled.h"
#include "DebugStats.h"

#include "DebugStatsVariable.h"
#include "DebugStatsScope.h"
#include "DebugStatsVariableIterator.h"
#include "DebugStatsScopeIterator.h"
#include "fb/container/LinearMap.h"
#include "fb/container/PodVector.h"
#include "fb/container/Vector.h"
#include "fb/lang/FBPrintf.h"
#include "fb/lang/NumericLimits.h"
#include "fb/lang/FBSingleThreadAssert.h"
#include "fb/string/StaticString.h"
#include "fb/string/util/CreateTemporaryHeapString.h"

FB_PACKAGE2(memory, stats)

class DebugStatsImpl
{
private:
	~DebugStatsImpl()
	{
		scopes.clear();
		variables.clear();
		for (const DebugStatsVariableImplementingType *arrayVar : arrayVariables)
			delete[] arrayVar;

		arrayVariables.clear();
	}

	/* In Trine 4, there less than 80 scopes and 400 vars */
	static const SizeType maxNumScopes = 128;
	static const SizeType maxNumVariables = 1 * 1024;
	StaticVector<DebugStatsScope, maxNumScopes> scopes;
	StaticPodVector<DebugStatsVariableImplementingType, maxNumVariables> variableStore;
	/* There's currently only one array variable... */
	CachePodVector<DebugStatsVariableImplementingType*, 2> arrayVariables;

	friend class DebugStats;
	friend class DebugStatsScopeIterator;
	friend class BreakpointListenerImpl;

	static DebugStats *instance;
	static bool stopped;

	typedef StaticString StringType;
	typedef LinearMap<StringType, DebugStatsVariableImplementingType *> Variables;
	Variables variables;
};

DebugStats *DebugStatsImpl::instance = nullptr;
bool DebugStatsImpl::stopped = false;



DebugStats *DebugStats::getInstance()
{
	if (!DebugStatsImpl::stopped)
	{
		fb_single_thread_assert();
	}
	// FIXME: still, a new instance gets created here! (this happens at static uninit)

	// you are supposed to create the instance with createInstance or register an existing one with setInstanceForLibrary
	// NOTICE: there are specific macros for these!
	if (DebugStatsImpl::instance == nullptr)
	{
		///fb_assert(0 && "You are supposed to create or register the DebugStats instance before use.");

		// but, in order to prevent a crash, still creating a new instance... you just may end up with multiple instance
		// because of this!
		createInstance();
	}
	
	return DebugStatsImpl::instance;
}


void DebugStats::createInstance()
{
	if (!DebugStatsImpl::stopped)
	{
		fb_single_thread_assert();
	}

	// you are supposed to create only once instance. If you have called this from within a library, then you 
	// have made a mistake - you should have used the setInstanceForLibrary instead (to share the main apps instance)
	fb_assert(DebugStatsImpl::instance == nullptr && "Attempt to create multiple instances of DebugStats");

	DebugStatsImpl::instance = new DebugStats();
}


void DebugStats::stopped()
{
	DebugStatsImpl::stopped = true;
}


void DebugStats::cleanInstance()
{
	fb_single_thread_assert();

	fb_assert(DebugStatsImpl::instance != nullptr && "Attempt to clean DebugStats instance, when it does not exist");

	delete DebugStatsImpl::instance;
	
	// NOTICE: any pointer references in libraries to the instance are now invalid, unless specifically cleared

	DebugStatsImpl::instance = nullptr;
}


void DebugStats::setInstanceForLibrary(DebugStats *instance)
{
	fb_assert(0 && "DebugStats::setInstanceForLibrary - DEPRECATED");

	fb_single_thread_assert();
	
	if (instance == DebugStatsImpl::instance)
	{
		// it is ok to set the same instance for a library that already exists there, the rationale for this is 
		// that if you happen to use some static library in such a way, that it gets directly linked into the main
		// app (so that it shares the static instance variable), then you may get this scenario where you are actually
		// trying to set the debugstats for the library, while it really is not a seperate library.
		// (this is allowed an such attempt is just silently ignored)
	}
	else
	{
		// if this triggers, then you are probably setting the instance for the library, while another created instance
		// already exists.
		fb_assert(DebugStatsImpl::instance == nullptr || instance == nullptr);

		DebugStatsImpl::instance = instance;
	}
}


DebugStats::DebugStats()
	: impl(new DebugStatsImpl())
{
	impl->scopes.reserve(128);
}


DebugStats::~DebugStats()
{
	delete impl;
	impl = nullptr;
}


void DebugStats::defineScope(const StaticString &scopeName)
{
	fb_single_thread_assert();

	// scope already exists?
	// TODO: optimize, O(n) lookup here.
	for (SizeType i = 0; i < impl->scopes.getSize(); i++)
	{
		if (impl->scopes[i].getName() == scopeName)
			return;
	}

	fb_assertf(scopeName.doesStartWith("fb::"), "'%s' scope is missing namespaces. Please fix. This serialization can't handle surprises.", scopeName.getPointer());
	impl->scopes.pushBack(DebugStatsScope(scopeName));
	/* Use this to check number of scopes */
	//FB_PRINTF("%s\n", FB_MSG("Defined debug stat scope number ", impl->scopes.getSize()).getPointer());
}


DebugStatsVariable *DebugStats::defineVariable(const StaticString &name, DebugStatsVariableImplementingType &variable, const StaticString &scopeName, bool *breakpointFlag)
{
	fb_single_thread_assert();

	DebugStatsScope *scope = findScopeByName(scopeName);
	if (scope != nullptr)
	{
		DebugStatsVariable var(name, variable, DebugStatsVariable::VariableTypeUndefinedInt, breakpointFlag);
		return &scope->addVariable(var);
	}
	else
	{
		fb_assert(0 && "DebugStats::defineVariable - Debug stats scope with given name not found.");
	}
	return nullptr;
}


DebugStatsVariable *DebugStats::defineArrayVariable(const StaticString &name, SizeType arrayElements, DebugStatsVariableImplementingType *variable, const StaticString &scopeName)
{
	fb_single_thread_assert();

	DebugStatsScope *scope = findScopeByName(scopeName);
	if (scope != nullptr)
	{
		DebugStatsVariable var(name, *variable, DebugStatsVariable::VariableTypeIntArray, nullptr, arrayElements);
		return &scope->addVariable(var);
	}
	else
	{
		fb_assert(0 && "DebugStats::defineVariable - Debug stats scope with given name not found.");
	}
	return nullptr;
}


DebugStatsVariable *DebugStats::defineAllocationVariable(const StaticString &name, DebugStatsVariableImplementingType &variable, const StaticString &scopeName, bool *breakpointFlag)
{
	fb_single_thread_assert();

	DebugStatsScope *scope = findScopeByName(scopeName);
	if (scope != nullptr)
	{
		DebugStatsVariable var(name, variable, DebugStatsVariable::VariableTypeAllocationCount, breakpointFlag);
		return &scope->addVariable(var);
	}
	else
	{
		fb_assert(0 && "DebugStats::defineAllocationVariable - Debug stats scope with given name not found.");
	}
	return nullptr;
}


void DebugStats::checkDebugScopeForEmptiness(const DynamicString &scopeName)
{
#if FB_ASSERT_ENABLED == FB_TRUE
	fb_single_thread_assert();

	// the scope name can either match completely or it can starts with the given scope name followed by two colons
	// (in the latter case, it is assumed to mean that the upper scope and all of its children are to be checked)
	TempString startingScopeName = scopeName;
	startingScopeName += "::";

	bool found = false;
	for (const DebugStatsScope &scope : impl->scopes)
	{
		if (scope.getName() == scopeName || scope.getName().doesStartWith(startingScopeName))
		{

			DebugStatsVariableIterator varIter = scope.getVariables();
			while (const DebugStatsVariable *var = varIter.next())
			{
				if (var->getVariableType() == DebugStatsVariable::VariableTypeAllocationCount)
				{
					if (var->getImplementingVariable() != DebugStatsVariableImplementingType(0))
					{
						const DynamicString &varName = var->getName();
						FB_UNUSED_NAMED_VAR(DebugStatsVariableImplementingType, allocationCount) = var->getImplementingVariable();

						fb_assert_logf("DebugStats::checkDebugScopeForEmptiness - allocation %s::%s = %i\n", scope.getName().getPointer(), varName.getPointer(), allocationCount);
						found = true;
					}
				}
			}
		}
	}

	if (found)
	{
		fb_assert(0 && "DebugStats - Something is leaking memory. Check log/assert.log");
	}
#endif
}


DebugStatsScopeIterator DebugStats::getDebugScopeIterator()
{
	fb_single_thread_assert();

	return fb::memory::stats::DebugStatsScopeIterator(this);
}


DebugStatsVariableIterator DebugStats::getDebugStatsVariableIterator(const DynamicString &scopeName)
{
	fb_single_thread_assert();

	DebugStatsScope *scope = findScopeByName(scopeName);
	if (scope != nullptr)
	{
		// ok.
	}
	else
	{
		fb_assert(0 && "DebugStats::getDebugStatsVariableIterator - Debug stats scope with given name not found.");
		// umm. should return a dummy iterator?
	}
	return scope->getVariables();
}


DebugStatsScope *DebugStats::findScopeByName(const DynamicString &scopeName)
{
	fb_single_thread_assert();

	for (DebugStatsScope &scope : impl->scopes)
	{
		if (scope.getName() == scopeName)
			return &scope;
	}
	return nullptr;
}

DebugStatsVariable *DebugStats::findDebugStatsVariableByName(const DynamicString &scopeName, const DynamicString &variableName)
{
	fb_single_thread_assert();

	DebugStatsScope *scope = findScopeByName(scopeName);
	if (scope != nullptr)
	{
		DebugStatsVariableIterator varIter = scope->getVariables();
		const DebugStatsVariable *var;
		while ((var = varIter.next()) != nullptr)
		{
			if (var->getName() == variableName)
				return const_cast<DebugStatsVariable *>(var);
		}
	}
	return nullptr;
}


// (this is inside this file due to a dependency on DebugStats private implementation.)

const DebugStatsScope *DebugStatsScopeIterator::next() 
{ 
	fb_expensive_assert(nextScopeIndex <= debugStats->impl->scopes.getSize());
	fb_single_thread_assert();

	if (nextScopeIndex < debugStats->impl->scopes.getSize())
		return &debugStats->impl->scopes[nextScopeIndex++];
	else
		return nullptr;
}


void DebugStats::increaseVariable(DebugStatsVariableImplementingType &variable, DebugStatsVariable *varInDebugStatsScope) 
{ 
	// Use FB_DSTAT_INC macro instead of using this directly.
	fb_expensive_assert(varInDebugStatsScope != nullptr);
	fb_expensive_assert(*(varInDebugStatsScope->breakpointFlag));

	++variable; 
}


void DebugStats::decreaseVariable(DebugStatsVariableImplementingType &variable, DebugStatsVariable *varInDebugStatsScope) 
{ 
	// Use FB_DSTAT_DEC macro instead of using this directly.
	fb_expensive_assert(varInDebugStatsScope != nullptr);
	fb_expensive_assert(*(varInDebugStatsScope->breakpointFlag));

	--variable; 
}


void DebugStats::setVariable(DebugStatsVariableImplementingType &variable, int64_t value, DebugStatsVariable *varInDebugStatsScope)
{ 
	// Use FB_DSTAT_SET macro instead of using this directly.
	fb_expensive_assert(varInDebugStatsScope != nullptr);
	fb_expensive_assert(*(varInDebugStatsScope->breakpointFlag));

	variable = DebugStatsVariableImplementingType(value); 
}

void DebugStats::setVariable(DebugStatsVariableImplementingType &variable, uint64_t value, DebugStatsVariable *varInDebugStatsScope)
{
	fb_expensive_assert(value > uint64_t(lang::NumericLimits<int64_t>::getHighest()) && "Value out of range");
	setVariable(variable, int64_t(value), varInDebugStatsScope);
}

int64_t DebugStats::getVariable(DebugStatsVariableImplementingType &variable, DebugStatsVariable *varInDebugStatsScope) 
{ 
	// Use FB_DSTAT_GET macro instead of using this directly.
	fb_expensive_assert(varInDebugStatsScope != nullptr);
	fb_expensive_assert(*(varInDebugStatsScope->breakpointFlag));

	// nop
	return variable; 
}


void DebugStats::update()
{
}

DebugStatsVariableImplementingType &DebugStats::allocateVariable(const StaticString &name)
{
	return *allocateVariableArray(name, 1);
}

DebugStatsVariableImplementingType *DebugStats::allocateVariableArray(const StaticString &name, SizeType arraySize)
{
	DebugStatsImpl::Variables::ConstIterator it = impl->variables.find(name);
	if (it != impl->variables.getEnd())
		return it.getValue();

	/* Reserve normal variables from variableStore to avoid unnecessary allocation spam. Only reserve arrays dynamically */
	DebugStatsVariableImplementingType *var = nullptr;
	if (arraySize == 1)
	{
		fb_assert(impl->variableStore.getSize() < impl->variableStore.getCapacity() && "Out of debug stats space. Increase vector size");
		impl->variableStore.pushBack(DebugStatsVariableImplementingType(0));
		var = &impl->variableStore.getBack();
		/* Use this to check number of vars */
		//FB_PRINTF("%s\n", FB_MSG("Allocated debug stat variable number ", impl->variableStore.getSize(), " from storage").getPointer());
	}
	else
	{
		var = new DebugStatsVariableImplementingType[arraySize];
		impl->arrayVariables.pushBack(var);
		for (SizeType i = 0; i < arraySize; i++)
			var[i] = 0;
	}

	impl->variables[name] = var;
	return var;
}

bool DebugStats::setBadnessLimits(const DynamicString &scopeName, const DynamicString &variableName, DebugStatsVariableImplementingType lowLimit, DebugStatsVariableImplementingType highLimit)
{
	DebugStatsVariable *var = findDebugStatsVariableByName(scopeName, variableName);
	if (var)
	{
		var->setBadnessLimits(lowLimit, highLimit);
		return true;
	}
	fb_assertf(false, "Could not find requested variable %s::%s", scopeName.getPointer(), variableName.getPointer());
	return false;
}


FB_END_PACKAGE2()
