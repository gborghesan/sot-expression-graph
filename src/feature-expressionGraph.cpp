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
#include "feature-expressionGraph.h"
#include "helper.h"
#include <sot/core/exception-feature.hh>

#include <sot/core/matrix-homogeneous.hh>
#include <sot/core/matrix-rotation.hh>
#include <sot/core/vector-utheta.hh>
#include <sot/core/factory.hh>

using namespace dynamicgraph::sot;
using namespace std;
using namespace dynamicgraph;

DYNAMICGRAPH_FACTORY_ENTITY_PLUGIN(FeatureExpressionGraph,"FeatureExpressionGraph");

/* --------------------------------------------------------------------- */
/* --- CLASS ----------------------------------------------------------- */
/* --------------------------------------------------------------------- */

FeatureExpressionGraph::
FeatureExpressionGraph( const string& ExpressionGraph )
  : FeatureAbstract( ExpressionGraph )
    ,w_T_o1_SIN( NULL,"sotFeaturePoint6d("+name+")::input(matrixHomo)::w_T_o1" )
    ,w_T_o2_SIN( NULL,"sotFeaturePoint6d("+name+")::input(matrixHomo)::w_T_o2" )
    ,w_J_o1_SIN( NULL,"sotFeatureExpressionGraph("+name+")::input(matrix)::w_J_o1" )
	,w_J_o2_SIN( NULL,"sotFeatureExpressionGraph("+name+")::input(matrix)::w_J_o2" )
    ,positionRefSIN( NULL,"sotFeatureExpressionGraph("+name+")::input(double)::positionRef" )
{
  //the jacobian depends by
  jacobianSOUT.addDependency( w_T_o1_SIN );
  jacobianSOUT.addDependency( w_T_o2_SIN );
  jacobianSOUT.addDependency( w_J_o1_SIN );
  jacobianSOUT.addDependency( w_J_o2_SIN );
  jacobianSOUT.addDependency( positionRefSIN );//this one is not probably necessary...
  //the output depends by
  errorSOUT.addDependency( w_T_o1_SIN );
  errorSOUT.addDependency( w_T_o2_SIN );
  errorSOUT.addDependency( positionRefSIN );

  signalRegistration( w_T_o1_SIN<<w_T_o2_SIN
                      <<w_J_o1_SIN<<w_J_o2_SIN<<positionRefSIN );

  /***
   * init of expression graph
   * */
  Expression<KDL::Vector>::Ptr w_p_o1=KDL::vector(input(0), input(1),input(2));
  Expression<KDL::Rotation>::Ptr w_R_o1 = inputRot(3);
  //frame of the robot ee, w.r.t a world frame
  w_T_o1= frame(w_R_o1,  w_p_o1);
  Expression<KDL::Vector>::Ptr w_p_o2=KDL::vector(input(6), input(7),input(8));
  Expression<KDL::Rotation>::Ptr w_R_O2= inputRot(9);
  //frame of the the object, w.r.t the same world frame
  w_T_o2= frame(w_R_O2,  w_p_o2);
  //in and out
  Sreference= input(12);

  //
  /*here goes te expressions!!!*/
  Soutput=Sreference-norm(w_p_o1-w_p_o2);

  //end init
}


/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */

unsigned int& FeatureExpressionGraph::
getDimension( unsigned int & dim, int /*time*/ )
{
  sotDEBUG(25)<<"# In {"<<endl;

  return dim=1;
}



/* --------------------------------------------------------------------- */
/** Compute the interaction matrix from a subset of
 * the possible features.
 */

ml::Matrix& FeatureExpressionGraph::
computeJacobian( ml::Matrix& J,int time )
{
  sotDEBUG(15)<<"# In {"<<endl;

  //read signals!
  const MatrixHomogeneous &  w_Tm_o1=  w_T_o1_SIN (time);
  const MatrixHomogeneous &  w_Tm_o2=  w_T_o2_SIN (time);
  const ml::Matrix & w_J_o1 = w_J_o1_SIN(time);
  const ml::Matrix & w_J_o2 = w_J_o2_SIN(time);
  const double & vectdes = positionRefSIN(time);
  //copy positions
  for( int i=0;i<3;++i )
  {
	  Soutput->setInputValue(i,w_Tm_o1.elementAt( i,3 ));
	  Soutput->setInputValue(i+6, w_Tm_o2.elementAt( i,3 ));
  }
  //copy rotations
  //TODO use variable type for not controllable objects
  Soutput->setInputValue(3,mlHom2KDLRot(w_Tm_o1));
  Soutput->setInputValue(9,mlHom2KDLRot(w_Tm_o2));
  //copy reference
  Soutput->setInputValue(12,vectdes);


  //evaluate once to update the tree
  Soutput->value();

  //resize the matrices
  J.resize(1,w_J_o1.nbCols());
  ml::Matrix Jtask1(1,6);
  ml::Matrix Jtask2(1,6);
  ml::Matrix ad1(6,6);
  ml::Matrix ad2(6,6);


  //compute the interaction matrices, that are expressed one in o1 applied

  for (int i=0;i<6;++i)
  {
	  Jtask1(0,i)=Soutput->derivative(i);
  	  Jtask2(0,i)=Soutput->derivative(i+6);
  }
  //multiplication!
  /*
   * we need the inverse of the adjoint matrix that brings
   * from o1(o2) frame to w
   * this is equal to the adjoint from w to o1(o2)
   * the matrix is then composed by
   * w_R_o1| 0
   * ------+-----
   *   0   |w_R_o1
   *    */
  ad1.setZero();//inverse of ad
  ad2.setZero();

  for (int i=0;i<3;i++)
	  for (int j=0;j<3;j++)
	  {
		  ad1(i,j)=w_Tm_o1(i,j);
		  ad1(i+3,j+3)=w_Tm_o1(i,j);
		  ad2(i,j)=w_Tm_o2(i,j);
		  ad2(i+3,j+3)=w_Tm_o2(i,j);
	  }


  J=Jtask1*ad1*w_J_o1+Jtask2*ad2*w_J_o2;

//return result
  sotDEBUG(15)<<"# Out }"<<endl;
  return J;
}

/** Compute the error between two visual features from a subset
*a the possible features.
 */
ml::Vector&
FeatureExpressionGraph::computeError( ml::Vector& res,int time )
{
  sotDEBUGIN(15);


  const MatrixHomogeneous &  w_Tm_o1=  w_T_o1_SIN (time);
  const MatrixHomogeneous &  w_Tm_o2=  w_T_o2_SIN (time);
  const double & val_desired = positionRefSIN(time);

  sotDEBUG(15) << "w_T_o1 = " << w_Tm_o1 << std::endl;
  sotDEBUG(15) << "w_T_o2 = " << w_Tm_o2 << std::endl;
  sotDEBUG(15) << "desired value = " << val_desired << std::endl;



  //copy positions
  for( int i=0;i<3;++i )
  {
	  Soutput->setInputValue(i,  w_Tm_o1.elementAt( i,3 ));
	  Soutput->setInputValue(i+6,w_Tm_o2.elementAt( i,3 ));
  }
  //copy rotations
  Soutput->setInputValue(3,mlHom2KDLRot(w_Tm_o1));
  Soutput->setInputValue(9,mlHom2KDLRot(w_Tm_o2));
  //copy reference
  Soutput->setInputValue(12,val_desired);


  res.resize(1);
  //evaluate the result.
  res(0)=Soutput->value();


  sotDEBUGOUT(15);
  return res ;
}

void FeatureExpressionGraph::
display( std::ostream& os ) const
{
  os <<"FeatureExpressionGraph <"<<name<<">";
}



void FeatureExpressionGraph::
commandLine( const std::string& cmdLine,
	     std::istringstream& cmdArgs,
	     std::ostream& os )
{
  if( cmdLine=="help" )
    {
      os << "FeatureExpressionGraph: "<<endl;
      Entity::commandLine( cmdLine,cmdArgs,os );
    }
  else  //FeatureAbstract::
    Entity::commandLine( cmdLine,cmdArgs,os );

}



/*
 * Local variables:
 * c-basic-offset: 2
 * End:
 */
