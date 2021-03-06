/*
 * Copyright 2010,
 * François Bleibel,
 * Olivier Stasse,
 *
 * CNRS/AIST
 *
 * This file is part of sot-core.
 * sot-core is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 * sot-core is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.  You should
 * have received a copy of the GNU Lesser General Public License along
 * with sot-core.  If not, see <http://www.gnu.org/licenses/>.
 */

/* --------------------------------------------------------------------- */
/* --- INCLUDE --------------------------------------------------------- */
/* --------------------------------------------------------------------- */

/* --- SOT --- */
//#define VP_DEBUG
//#define VP_DEBUG_MODE 45
#include <sot/core/debug.hh>
#include "feature-line-to-line-distance.h"
#include "helper.h"
#include <sot/core/exception-feature.hh>

#include <sot/core/matrix-homogeneous.hh>
#include <sot/core/matrix-rotation.hh>
#include <sot/core/vector-utheta.hh>
#include <sot/core/factory.hh>

using namespace dynamicgraph::sot;
using namespace dynamicgraph;
using namespace KDL;
using namespace std;

DYNAMICGRAPH_FACTORY_ENTITY_PLUGIN(FeatureLineToLineDistance,"FeatureLineToLineDistance");

/* --------------------------------------------------------------------- */
/* --- CLASS ----------------------------------------------------------- */
/* --------------------------------------------------------------------- */

FeatureLineToLineDistance::
FeatureLineToLineDistance( const string& name )
: FeatureExprGraphAbstract( name )
, p1_SIN( NULL,"FeatureLineToLineDistance("+name+")::input(vector)::p1" )
, p2_SIN( NULL,"FeatureLineToLineDistance("+name+")::input(vector)::p2" )
, dir1_SIN( NULL,"FeatureLineToLineDistance("+name+")::input(vector)::dir1" )
, dir2_SIN( NULL,"FeatureLineToLineDistance("+name+")::input(vector)::dir2" )
, referenceSIN( NULL,"FeatureLineToLineDistance("+name+")::input(vector)::reference" )
{
  //the Jacobian depends by
  jacobianSOUT.addDependency( p1_SIN );
  jacobianSOUT.addDependency( p2_SIN );
  jacobianSOUT.addDependency( dir1_SIN );
  jacobianSOUT.addDependency( dir2_SIN );

  //the output depends by
  errorSOUT.addDependency( p1_SIN );
  errorSOUT.addDependency( p2_SIN );
  errorSOUT.addDependency( dir1_SIN );
  errorSOUT.addDependency( dir2_SIN );
  errorSOUT.addDependency( referenceSIN );

  signalRegistration( p1_SIN << p2_SIN <<dir1_SIN<<dir2_SIN << referenceSIN);

  Expression<KDL::Vector>::Ptr p1 = KDL::vector(
		  input(EXP_GRAPH_BASE_INDEX),
		  input(EXP_GRAPH_BASE_INDEX+1),
		  input(EXP_GRAPH_BASE_INDEX+2));
  Expression<KDL::Vector>::Ptr p2 = KDL::vector(
		  input(EXP_GRAPH_BASE_INDEX+3),
		  input(EXP_GRAPH_BASE_INDEX+4),
		  input(EXP_GRAPH_BASE_INDEX+5));
  Expression<KDL::Vector>::Ptr dir1 = KDL::vector(
		  input(EXP_GRAPH_BASE_INDEX+6),
		  input(EXP_GRAPH_BASE_INDEX+7),
		  input(EXP_GRAPH_BASE_INDEX+8));
  Expression<KDL::Vector>::Ptr dir2 = KDL::vector(
		  input(EXP_GRAPH_BASE_INDEX+9),
		  input(EXP_GRAPH_BASE_INDEX+10),
		  input(EXP_GRAPH_BASE_INDEX+11));

  /*here goes the expressions!!!*/
  geometric_primitive::Line line1 ;
  line1.o=w_T_o1;line1.p=p1;line1.dir=dir1;

  geometric_primitive::Line line2 ;
  line2.o=w_T_o2;line2.p=p2;line2.dir=dir2;
  Soutput_ = geometric_primitive::line_line_distance(line1,line2);

  //end init
}


/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */

unsigned int& FeatureLineToLineDistance::
getDimension( unsigned int & dim, int /*time*/ )
{
  sotDEBUG(25)<<"# In {"<<endl;

  return dim=1;
}



/* --------------------------------------------------------------------- */
/** Compute the interaction matrix from a subset of
 * the possible features.
 */

void FeatureLineToLineDistance::updateInputValues(KDL::Expression<double>::Ptr Soutput, int time)
{
  FeatureExprGraphAbstract::updateInputValues(Soutput, time);
  //same order that in the constructor!
  readPositionVector(p1_SIN,	EXP_GRAPH_BASE_INDEX, 	Soutput,time);
  readPositionVector(p2_SIN,	EXP_GRAPH_BASE_INDEX+3,	Soutput,time);
  FeatureExprGraphAbstract::readVersorVector  (dir1_SIN,EXP_GRAPH_BASE_INDEX+6,	Soutput,time);
  FeatureExprGraphAbstract::readVersorVector  (dir2_SIN,EXP_GRAPH_BASE_INDEX+9,	Soutput,time);
}


ml::Matrix& FeatureLineToLineDistance::
computeJacobian( ml::Matrix& J,int time )
{
  sotDEBUG(15)<<"# In {"<<endl;
  updateInputValues(Soutput_, time);

  //evaluate once to update the tree
  Soutput_->value();

  //resize the matrices
  evaluateJacobian(J, Soutput_, time);

  sotDEBUG(15)<<"# Out }"<<endl;
  return J;
}

/** Compute the error between two visual features from a subset
*a the possible features.
 */
ml::Vector&
FeatureLineToLineDistance::computeError( ml::Vector& res,int time )
{
  sotDEBUGIN(15);
  const ml::Vector & reference = referenceSIN(time);

  updateInputValues(Soutput_, time);

  res.resize(1);
  //evaluate the result.
  res(0) = Soutput_->value() - reference(0);

  sotDEBUGOUT(15);
  return res ;
}
