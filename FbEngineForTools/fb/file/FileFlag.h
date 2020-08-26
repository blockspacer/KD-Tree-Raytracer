#pragma once

#include "fb/lang/BitmaskTypes.h"

FB_PACKAGE1(file)

FB_FLAG_DEF(FileFlag
	, ExcludeFiles
	, ExcludeDirs
	, NoRecurse
	/* TODO: ExcludeArchive is not supported. Requires IFilePackage support */
	//, ExcludeArchive
	, ExcludeNonArchive
	, ExcludeUser
	, ExcludeNormal
	, SuppressErrorMessages
	, CreatePathIfNecessary
);

FB_END_PACKAGE1()
