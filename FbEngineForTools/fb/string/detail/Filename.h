#pragma once

FB_DECLARE0(HeapString)
FB_DECLARE0(StringRef)

FB_PACKAGE2(string, detail)

/**
 * Returns file path with trailing slash
 * "a/b/c.d" -> "a/b/"
 * "a/b/" -> "a/b/"
 * "/c.d" -> "/"
 * "c.d" -> ""
 */
void appendPath(HeapString& result, const StringRef &str);

/**
 * Returns file extension without dot
 * "a/b/c.d" -> "d"
 * "a/b/c." -> ""
 * "a/b/c" -> ""
 * "a/b/c.d/" -> ""
 * "a/b/c./d" -> ""
 */
StringRef getExtension(const StringRef &str);

/**
 * Returns file and path but no extension (and no dot)
 * "a/b/c.d" -> "a/b/c"
 * "a/b/c." -> "a/b/c"
 * "a/b/c" -> "a/b/c"
 * "a/b/c.d/" -> "a/b/c.d/"
 * "a/b/c./d" -> "a/b/c./d"
 * "c.d" -> "c"
 */
void appendWithoutExtension(HeapString& result, const StringRef &str);

/**
 * Returns file without path
 * "a/b/c.d" -> "c.d"
 * "a/b/" -> ""
 * "/c.d" -> "c.d"
 * "c.d" -> "c.d"
 */
StringRef getFile(const StringRef &str);

/**
 * Returns file without path and without extension (and no dot)
 * "a/b/c.d" -> "c"
 * "a/b/" -> ""
 * "/c.d" -> "c"
 * "c.d" -> "c"
 */
void appendFileWithoutExtension(HeapString& result, const StringRef &str);

/**
 * Removes ".." and "." from path if possible
 * "a/../b/c" -> "b/c"
 * "a/b/../c/d" -> "a/c/d"
 * "a/.." -> ""
 * "../a/b" -> "../a/b"
 * "/../a/b" -> "/../a/b"
 * "./a/b" -> "a/b"
 */
void solvePath(HeapString &t);

FB_END_PACKAGE2()
