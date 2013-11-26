#ifndef __SOT_FEATURE_EXPRESSIONGRAPH_HELPER_HH__
#define __SOT_FEATURE_EXPRESSIONGRAPH_HELPER_HH__

#include <sot/core/matrix-homogeneous.hh>
#include <kdl/expressiontree.hpp>

KDL::Rotation mlHom2KDLRot (const dynamicgraph::sot::MatrixHomogeneous &  M);

namespace geometric_primitive
{
using namespace KDL;

struct Point
{
	Expression<KDL::Frame>::Ptr o;
	Expression<KDL::Vector>::Ptr p;
};

//no check done on versor in building up
struct Versor
{
	Expression<Frame>::Ptr o;
	Expression<Vector>::Ptr v;
};
struct Line
{
	Expression<Frame>::Ptr o;
	Expression<Vector>::Ptr dir;
	Expression<Vector>::Ptr p;
};

struct Plane
{
	Expression<Frame>::Ptr o;
	Expression<Vector>::Ptr normal;
	Expression<Vector>::Ptr p;
};

/*for all the expressions we soppose that the frames are expressed in a common reference*/
Expression<double>::Ptr point_point_distance(const Point & p1,const Point& p2);
Expression<double>::Ptr line_point_distance(const Point&  p, const Line& l);
Expression<double>::Ptr projection_of_point_on_line(const Point& p, const Line& l);
Expression<double>::Ptr line_line_distance (const Line& l1, const Line& l2);
Expression<double>::Ptr surface_point_distance(const Point & p, const Plane & plan);

 /* Expression<double>::Ptr distance_o1_f1 (line l1, line l2);
 * Expression<double>::Ptr distance_o2_f2 (line l1, line l2);
 *
//angles
 * Expression<double>::Ptr angle_btw_versors (versor v1, versor v2);
 * Expression<double>::Ptr incident_angle (versor v, plane pl);
 * Expression<double>::Ptr angles_btw_planes (plane pl1, plane pl2);
 * */
Expression<double>::Ptr angle_btw_planes (const Plane & pl1, const Plane & pl2);
Expression<double>::Ptr angle_btw_versors (const Versor & v1, const Versor& v2);
Expression<double>::Ptr angle_btw_plane_and_versor (const Plane & p, const Versor& v);

//helper functions
Expression<Vector>::Ptr ExpressInBase(const Point & p);
Expression<Vector>::Ptr ExpressInBase(const Versor & v);
Expression<Vector>::Ptr ExpressOriginInBase(const Plane & p);
Expression<Vector>::Ptr ExpressOriginInBase(const Line & l);
Expression<Vector>::Ptr ExpressDirectionInBase(const Plane & p);
Expression<Vector>::Ptr ExpressDirectionInBase(const Line & l);
}
#endif // __SOT_FEATURE_EXPRESSIONGRAPH_HELPER_HH__


