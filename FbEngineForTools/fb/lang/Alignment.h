#pragma once

#include "fb/lang/Types.h"

FB_PACKAGE1(lang)

// Low-level alignments
static const SizeType GeneralAlignmentFast = 8;
static const SizeType GeneralAlignmentRecommended = 8;
static const SizeType GeneralAlignmentRequired = 8;

static const SizeType FloatAlignmentFast = 8;
static const SizeType FloatAlignmentRecommended = 8;
static const SizeType FloatAlignmentRequired = 8;

static const SizeType IntegerAlignmentFast = 8;
static const SizeType IntegerAlignmentRecommended = 8;
static const SizeType IntegerAlignmentRequired = 8;

static const SizeType ByteAlignmentFast = 1;
static const SizeType ByteAlignmentRecommended = 1;
static const SizeType ByteAlignmentRequired = 1;


// ToDo: Fix proper values
static const SizeType CacheLineAlignment = 64;

/// Default alignment if alignment-aware apis get 0 as alignment parameter
static const SizeType DefaultAlignment = GeneralAlignmentRecommended;

FB_END_PACKAGE1()
