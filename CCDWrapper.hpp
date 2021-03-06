/*
 * Copyright (C) 2018, Matt Zucker
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Original source: https://github.com/mzucker/ccdwrapper
 */

#ifndef _CCDWRAPPER_H_
#define _CCDWRAPPER_H_

#include <vector>
#include <string>
#include "ccd_eigen.hpp"

namespace ccdw {

void support(const void* obj,
             const ccd_vec3_t* dir,
             ccd_vec3_t* vec);

void center(const void* obj,
            ccd_vec3_t* center);

class Convex;
class Point;
class Line;
class Box;
class Cylinder;
class DilatedConvex;
class TransformedConvex;

// useful for stuff like rendering so we can decouple
// hierarchy traversal from this file
// https://en.wikipedia.org/wiki/Visitor_pattern
class ConvexConstVisitor {
public:

    virtual void visit(const Convex* c);
    virtual void visit(const Point* c);
    virtual void visit(const Line* c);
    virtual void visit(const Box* c);
    virtual void visit(const Cylinder* c);
    virtual void visit(const DilatedConvex* c);
    virtual void visit(const TransformedConvex* c);

    virtual ~ConvexConstVisitor();

};

//////////////////////////////////////////////////////////////////////

class Convex {
public:

    virtual ~Convex();

    std::string description() const;

    virtual void describe(std::ostream& ostr) const =0;

    virtual ccd_real_t maxDist() const =0;

    virtual void support(const vec3& dir, vec3& s) const=0;

    virtual void center(vec3& c) const;

    virtual bool isDilated() const;

    virtual bool contains(const vec3& p, vec3* pc) const =0;

    virtual void accept(ConvexConstVisitor* v) const;

};

static inline std::ostream& operator<<(std::ostream& ostr,
                                       const Convex& c) {
    c.describe(ostr);
    return ostr;
}

//////////////////////////////////////////////////////////////////////

class Box: public Convex {
public:

    vec3 extents;

    Box();
    Box(const vec3& extents);

    virtual ~Box();

    virtual void describe(std::ostream& ostr) const;
    virtual ccd_real_t maxDist() const;
    virtual void support(const vec3& dir, vec3& s) const;
    virtual bool contains(const vec3& p, vec3* pc) const;

    virtual void accept(ConvexConstVisitor* v) const;

};

//////////////////////////////////////////////////////////////////////

class Point: public Convex {
public:

    Point();

    virtual ~Point();

    virtual void describe(std::ostream& ostr) const;
    virtual ccd_real_t maxDist() const;
    virtual void support(const vec3& dir, vec3& s) const;
    virtual bool contains(const vec3& p, vec3* pc) const;

    virtual void accept(ConvexConstVisitor* v) const;

};

//////////////////////////////////////////////////////////////////////

class Line: public Convex {
public:

    ccd_real_t length;

    Line();
    Line(ccd_real_t length);

    virtual ~Line();

    virtual void describe(std::ostream& ostr) const;
    virtual ccd_real_t maxDist() const;
    virtual void support(const vec3& dir, vec3& s) const;
    virtual bool contains(const vec3& p, vec3* pc) const;

    virtual void accept(ConvexConstVisitor* v) const;

};

//////////////////////////////////////////////////////////////////////

class Cylinder: public Convex {
public:

    ccd_real_t length;
    ccd_real_t radius;

    Cylinder();
    Cylinder(ccd_real_t length, ccd_real_t radius);

    virtual ~Cylinder();

    virtual void describe(std::ostream& ostr) const;
    virtual ccd_real_t maxDist() const;
    virtual void support(const vec3& dir, vec3& s) const;
    virtual bool contains(const vec3& p, vec3* pc) const;

    virtual void accept(ConvexConstVisitor* v) const;

};

//////////////////////////////////////////////////////////////////////

class DilatedConvex: public Convex {
public:

    const Convex* child;
    ccd_real_t dilation;

    DilatedConvex();
    DilatedConvex(const Convex* child, ccd_real_t dilation);

    virtual ~DilatedConvex();

    virtual void describe(std::ostream& ostr) const;
    virtual ccd_real_t maxDist() const;
    virtual void support(const vec3& dir, vec3& s) const;
    virtual bool isDilated() const;
    virtual bool contains(const vec3& p, vec3* pc) const;

    virtual void accept(ConvexConstVisitor* v) const;

};

//////////////////////////////////////////////////////////////////////

class TransformedConvex: public Convex {
public:

    const Convex* child;
    Transform3 xform;

    TransformedConvex();
    TransformedConvex(const Convex* child, const Transform3& xform);

    virtual ~TransformedConvex();

    virtual void describe(std::ostream& ostr) const;
    virtual ccd_real_t maxDist() const;
    virtual void support(const vec3& dir, vec3& s) const;
    virtual void center(vec3& c) const;
    virtual bool isDilated() const;
    virtual bool contains(const vec3& p, vec3* pc) const;

    virtual void accept(ConvexConstVisitor* v) const;

};


//////////////////////////////////////////////////////////////////////

DilatedConvex* sphere(ccd_real_t radius);
DilatedConvex* capsule(ccd_real_t length, ccd_real_t radius);

TransformedConvex* capsule(const vec3& p0,
                           const vec3& p1,
                           ccd_real_t radius);

TransformedConvex* cylinder(const vec3& p0,
                            const vec3& p1,
                            ccd_real_t radius);

TransformedConvex* transform(const Convex* c, const Transform3& xform);

Convex* dilate(const Convex* c, ccd_real_t radius);

//////////////////////////////////////////////////////////////////////

enum AlgorithmType {
    ALGORITHM_GJK=0,
    ALGORITHM_MPR,
    NUM_ALGORITHMS
};

enum QueryType {
    QUERY_INTERSECT=0,
    QUERY_SEPARATION,
    QUERY_PENETRATION,
    NUM_QUERY_TYPES,
};

enum ReportFlags {
    INTERSECT       = 0x01,
    HAVE_SEPARATION = 0x02,
    HAVE_POSITION   = 0x04
};

class Report {
public:

    const Convex* c1;
    const Convex* c2;

    int flags;
    ccd_real_t distance;
    vec3 direction;
    vec3 pos1;
    vec3 pos2;
    AlgorithmType algorithm;

    Report(const Convex* c1=0, const Convex* c2=0);

};

class Checker {
public:

    ccd_t ccd;
    AlgorithmType algorithm;

    Checker();

    bool intersect(const Convex* c1, const Convex* c2,
                   ccd_real_t dmin=0) const;

    bool separate(Report& report,
                  const Convex* c1, const Convex* c2,
                  ccd_real_t dmin=0) const;

    bool penetration(Report& report,
                     const Convex* c1, const Convex* c2,
                     ccd_real_t dmin=0) const;

    bool query(QueryType qtype, Report* report,
               const Convex* c1, const Convex* c2,
               ccd_real_t dmin=0) const;

};

//////////////////////////////////////////////////////////////////////

void cubePoints(size_t n, std::vector<vec3>& points);



};

#endif
