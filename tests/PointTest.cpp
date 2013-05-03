
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
// Unit tests for src/core/SkPoint.cpp and its header

#include "SkPoint.h"
#include "SkRect.h"
#include "Test.h"

static void test_casts(skiatest::Reporter* reporter) {
    SkPoint p = { 0, 0 };
    SkRect  r = { 0, 0, 0, 0 };

    const SkScalar* pPtr = SkTCast<const SkScalar*>(&p);
    const SkScalar* rPtr = SkTCast<const SkScalar*>(&r);

    REPORTER_ASSERT(reporter, p.asScalars() == pPtr);
    REPORTER_ASSERT(reporter, r.asScalars() == rPtr);
}

// Tests SkPoint::Normalize() for this (x,y)
static void test_Normalize(skiatest::Reporter* reporter,
                           SkScalar x, SkScalar y) {
    SkPoint point;
    point.set(x, y);
    SkScalar oldLength = point.length();
    SkScalar returned = SkPoint::Normalize(&point);
    SkScalar newLength = point.length();
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(returned, oldLength));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(newLength, SK_Scalar1));
}

// Tests that SkPoint::length() and SkPoint::Length() both return
// approximately expectedLength for this (x,y).
static void test_length(skiatest::Reporter* reporter, SkScalar x, SkScalar y,
                        SkScalar expectedLength) {
    SkPoint point;
    point.set(x, y);
    SkScalar s1 = point.length();
    SkScalar s2 = SkPoint::Length(x, y);
    //The following should be exactly the same, but need not be.
    //See http://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(s1, s2));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(s1, expectedLength));
    
    test_Normalize(reporter, x, y);
}

// test that we handle very large values correctly. i.e. that we can
// successfully normalize something whose mag overflows a float.
static void test_overflow(skiatest::Reporter* reporter) {
    SkPoint pt = { SkFloatToScalar(3.4e38f), SkFloatToScalar(3.4e38f) };
    
    SkScalar length = pt.length();
    REPORTER_ASSERT(reporter, !SkScalarIsFinite(length));

    // this should succeed, even though we can't represent length
    REPORTER_ASSERT(reporter, pt.setLength(SK_Scalar1));

    // now that pt is normalized, we check its length
    length = pt.length();
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(length, SK_Scalar1));
}

// test that we handle very small values correctly. i.e. that we can
// report failure if we try to normalize them.
static void test_underflow(skiatest::Reporter* reporter) {
    SkPoint pt = { SkFloatToScalar(1.0e-37f), SkFloatToScalar(1.0e-37f) };
    SkPoint copy = pt;

    REPORTER_ASSERT(reporter, 0 == SkPoint::Normalize(&pt));
    REPORTER_ASSERT(reporter, pt == copy);  // pt is unchanged

    REPORTER_ASSERT(reporter, !pt.setLength(SK_Scalar1));
    REPORTER_ASSERT(reporter, pt == copy);  // pt is unchanged
}

static void PointTest(skiatest::Reporter* reporter) {
    test_casts(reporter);

    static const struct {
        SkScalar fX;
        SkScalar fY;
        SkScalar fLength;
    } gRec[] = {
        { SkIntToScalar(3), SkIntToScalar(4), SkIntToScalar(5) },
        { SkFloatToScalar(0.6f), SkFloatToScalar(0.8f), SK_Scalar1 },
    };
    
    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); ++i) {
        test_length(reporter, gRec[i].fX, gRec[i].fY, gRec[i].fLength);
    }

    test_underflow(reporter);
    test_overflow(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Point", PointTestClass, PointTest)
