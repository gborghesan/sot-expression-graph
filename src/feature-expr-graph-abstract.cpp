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
#include "feature-expr-graph-abstract.h"
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

/* --------------------------------------------------------------------- */
/* --- CLASS ----------------------------------------------------------- */
/* --------------------------------------------------------------------- */

FeatureExprGraphAbstract::
FeatureExprGraphAbstract( const string& name )
  : FeatureAbstract( name )
  ,w_T_o1_SIN( NULL,"FeatureExprGraphAbstract("+name+")::input(matrixHomo)::w_T_o1" )
  ,w_T_o2_SIN( NULL,"FeatureExprGraphAbstract("+name+")::input(matrixHomo)::w_T_o2" )
  ,w_J_o1_SIN( NULL,"sotFeatureExprGraphAbstract("+name+")::input(matrix)::w_J_o1" )
  ,w_J_o2_SIN( NULL,"sotFeatureExprGraphAbstract("+name+")::input(matrix)::w_J_o2" )
{
  //the jacobian depends by
  jacobianSOUT.addDependency( w_T_o1_SIN );
  jacobianSOUT.addDependency( w_T_o2_SIN );
  jacobianSOUT.addDependency( w_J_o1_SIN );
  jacobianSOUT.addDependency( w_J_o2_SIN );

  //the output depends by
  errorSOUT.addDependency( w_T_o1_SIN );
  errorSOUT.addDependency( w_T_o2_SIN );

  signalRegistration( w_T_o1_SIN<<w_T_o2_SIN
                      <<w_J_o1_SIN<<w_J_o2_SIN);

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
}


void
FeatureExprGraphAbstract::updateInputValues(KDL::Expression<double>::Ptr Soutput, int time)
{
  //read signals!
  const MatrixHomogeneous &  w_Tm_o1=  w_T_o1_SIN (time);
  const MatrixHomogeneous &  w_Tm_o2=  w_T_o2_SIN (time);
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
}


void FeatureExprGraphAbstract::
evaluateJacobian( ml::Matrix& res, KDL::Expression<double>::Ptr Soutput, int time )
{
  const MatrixHomogeneous &  w_Tm_o1=  w_T_o1_SIN (time);
  const MatrixHomogeneous &  w_Tm_o2=  w_T_o2_SIN (time);

  if(w_J_o1_SIN.isPlugged())
  {
    const ml::Matrix & w_J_o1 = w_J_o1_SIN(time);
    ml::Matrix Jtask1(1,6);
    ml::Matrix ad1(6,6);

    //compute the interaction matrices, that are expressed one in o1 applied
    for (int i=0;i<6;++i)
      Jtask1(0,i)=Soutput->derivative(i);

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

    for (int i=0;i<3;i++)
      for (int j=0;j<3;j++)
        ad1(i,j) = ad1(i+3,j+3) = w_Tm_o1(i,j);

    res = Jtask1*ad1*w_J_o1;
  }

  if(w_J_o2_SIN.isPlugged())
  {
    const ml::Matrix & w_J_o2 = w_J_o2_SIN(time);
    ml::Matrix Jtask2(1,6);
    ml::Matrix ad2(6,6);
    ad2.setZero();

    //compute the interaction matrices, that are expressed one in o1 applied
    for (int i=0;i<6;++i)
      Jtask2(0,i)=Soutput->derivative(i+6);

    for (int i=0;i<3;i++)
      for (int j=0;j<3;j++)
        ad2(i,j) = ad2(i+3,j+3) = w_Tm_o2(i,j);

    if(w_J_o1_SIN.isPlugged())
      res += Jtask2*ad2*w_J_o2;
    else
      res = Jtask2*ad2*w_J_o2;
  }
}

/*
 * readVersorVector:
 * as readPositionVector, BUT set as default the x axis (1 0 0) if the signal is not plugged
 *  */
bool FeatureExprGraphAbstract::readVersorVector(
		 dg::SignalPtr< ml::Vector,int >& SIN,
		unsigned int base,
		const KDL::Expression<double>::Ptr & exp,
		const int time)
{
	if(SIN.isPlugged())
	  {
	    const ml::Vector & p = SIN(time);
	    for( int i=0;i<3;++i )
	    	exp->setInputValue(base+i, p(i));
	    return true;
	  }
	  else
	  {
	    	exp->setInputValue(base,   1);
	    	exp->setInputValue(base+1, 0);
	        exp->setInputValue(base+2, 0);
	    return false;
	  }

}

