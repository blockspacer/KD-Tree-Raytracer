#pragma once

#include "fb/string/HeapString.h"

FB_PACKAGE1(file)

/* Calling for already existing directory logs an error depending on boolean errorIfAlreadyExists.
 * Returns true, if successful, false if there's an (unexpected) error */
bool createDirectory(const StringRef &dir, bool errorIfAlreadyExists);
/* Calling for already existing directory is fine. Does NOT create intermediate directories. 
 * Returns true, if successful, false if there's an (unexpected) error */
bool createDirectoryIfMissing(const StringRef &dir);
/* Calling for already existing directory logs an error. 
 * Returns true, if successful, false if there's an (unexpected) error */
bool createDirectory(const StringRef &dir);

/* Similar set for creating paths. Note that if only directories are specified (i.e. a/b/c/), path must end in slash. 
 * This allows creating path for a file, e.g. a/b/c/file.txt */
bool createPath(const StringRef &path, bool errorIfAlreadyExists);
bool createPathIfMissing(const StringRef &path);
bool createPath(const StringRef &path);

FB_END_PACKAGE1()
