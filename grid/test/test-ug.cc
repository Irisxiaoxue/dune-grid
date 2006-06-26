// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#include <config.h>

#include <iostream>

/*

   Instantiate UG-Grid and feed it to the generic gridcheck()

   Currently UGGrid can only be initialized via the AmiraMeshReader

 */

#include <dune/grid/uggrid.hh>
#include <dune/grid/io/file/amirameshreader.hh>

#include "gridcheck.cc"
#include "checkgeometryinfather.cc"
#include "checkintersectionit.cc"

class ArcOfCircle : public Dune::BoundarySegment<2>
{
public:

  ArcOfCircle(const Dune::FieldVector<double,2>& center, double radius,
              double fromAngle, double toAngle)
    : center_(center), radius_(radius), fromAngle_(fromAngle), toAngle_(toAngle)
  {}

  Dune::FieldVector<double,2> operator()(const Dune::FieldVector<double,1>& local) const {

    double angle = fromAngle_ + local[0]*(toAngle_ - fromAngle_);

    Dune::FieldVector<double,2> result = center_;
    result[0] += radius_ * std::cos(angle);
    result[1] += radius_ * std::sin(angle);

    return result;
  }

  Dune::FieldVector<double,2> center_;

  double radius_;

  double fromAngle_;

  double toAngle_;
};


void makeHalfCircleQuad(Dune::UGGrid<2,2>& grid, bool parametrization)
{
  using namespace Dune;

  grid.createBegin();

  // /////////////////////////////
  //   Create boundary segments
  // /////////////////////////////
  if (parametrization) {

    FieldVector<double,2> center(0);
    center[1] = 15;

    std::vector<unsigned int> vertices(2);

    vertices[0] = 1;  vertices[1] = 2;
    grid.insertBoundarySegment(vertices, new ArcOfCircle(center, 15, M_PI, M_PI*4/3));

    vertices[0] = 2;  vertices[1] = 3;
    grid.insertBoundarySegment(vertices, new ArcOfCircle(center, 15, M_PI*4/3, M_PI*5/3));

    vertices[0] = 3;  vertices[1] = 0;
    grid.insertBoundarySegment(vertices, new ArcOfCircle(center, 15, M_PI*5/3, M_PI*2));

  }

  // ///////////////////////
  //   Insert vertices
  // ///////////////////////
  FieldVector<double,2> pos;

  pos[0] = 15;  pos[1] = 15;
  grid.insertVertex(pos);

  pos[0] = -15; pos[1] = 15;
  grid.insertVertex(pos);

  pos[0] = -7.5; pos[1] = 2.00962;
  grid.insertVertex(pos);

  pos[0] = 7.5; pos[1] = 2.00962;
  grid.insertVertex(pos);
  // /////////////////
  // Insert elements
  // /////////////////

  std::vector<unsigned int> cornerIDs(4);
  cornerIDs[0] = 0;
  cornerIDs[1] = 1;
  cornerIDs[2] = 3;
  cornerIDs[3] = 2;

  grid.insertElement(GeometryType(GeometryType::cube,2), cornerIDs);

  // //////////////////////////////////////
  //   Finish initialization
  // //////////////////////////////////////
  grid.createEnd();

}


template <class GridType >
void markOne ( GridType & grid , int num , int ref )
{
  typedef typename GridType::template Codim<0>::LeafIterator LeafIterator;

  int count = 0;

  LeafIterator endit = grid.template leafend  <0> ();
  for(LeafIterator it = grid.template leafbegin<0> (); it != endit ; ++it )
  {
    if(num == count) grid.mark( ref, it );
    count++;
  }

  grid.preAdapt();
  grid.adapt();
  grid.postAdapt();
}

int main () try
{

  // ////////////////////////////////////////////////////////////////////////
  //  Do the standard grid test for a 2d UGGrid
  // ////////////////////////////////////////////////////////////////////////
  // extra-environment to check destruction
  {

    std::cout << "Testing UGGrid<2> with grid file: ug-testgrid-2.am" << std::endl;
    std::cout << "Testing UGGrid<3> with grid file: ug-testgrid-3.am" << std::endl;

    Dune::UGGrid<2> grid2d;
    Dune::UGGrid<3> grid3d;

    Dune::AmiraMeshReader<Dune::UGGrid<2> >::read(grid2d, "ug-testgrid-2.am");
    Dune::AmiraMeshReader<Dune::UGGrid<3> >::read(grid3d, "ug-testgrid-3.am");

    // check macro grid
    gridcheck(grid2d);
    gridcheck(grid3d);

    // create hybrid grid
    markOne(grid2d,0,1) ;
    markOne(grid3d,0,1) ;
    gridcheck(grid2d);
    gridcheck(grid3d);

    grid2d.globalRefine(1);
    grid3d.globalRefine(1);
    gridcheck(grid2d);
    gridcheck(grid3d);

    // check the method geometryInFather()
    checkGeometryInFather(grid2d);
    checkGeometryInFather(grid3d);

    // check the intersection iterator
    checkIntersectionIterator(grid2d);
    checkIntersectionIterator(grid3d);

  }

  // ////////////////////////////////////////////////////////////////////////
  //   Check whether geometryInFather returns equal results with and
  //   without parametrized boundaries
  // ////////////////////////////////////////////////////////////////////////

  Dune::UGGrid<2> gridWithParametrization, gridWithoutParametrization;

  // make grids
  makeHalfCircleQuad(gridWithoutParametrization, false);
  makeHalfCircleQuad(gridWithParametrization, true);

  // make grids again just to check this is possible
#if 0
  makeHalfCircleQuad(gridWithoutParametrization, false);
  makeHalfCircleQuad(gridWithParametrization, true);
#endif

  gridWithParametrization.globalRefine(1);
  gridWithoutParametrization.globalRefine(1);

  typedef Dune::UGGrid<2>::Codim<0>::LevelIterator ElementIterator;
  ElementIterator eIt    = gridWithParametrization.lbegin<0>(1);
  ElementIterator eWoIt  = gridWithoutParametrization.lbegin<0>(1);
  ElementIterator eEndIt = gridWithParametrization.lend<0>(1);

  for (; eIt!=eEndIt; ++eIt, ++eWoIt) {

    // The grids where constructed identically and they are traversed identically
    // Thus their respective output from geometryInFather should be the same
    for (int i=0; i<eIt->geometry().corners(); i++) {

      Dune::FieldVector<double,2> diff = eIt->geometryInFather()[i] - eWoIt->geometryInFather()[i];

      if ( diff.two_norm() > 1e-5 )
        DUNE_THROW(Dune::GridError, "output of geometryInFather() depends on boundary parametrization!");

    }

  }

  return 0;
}
catch (Dune::Exception& e) {
  std::cerr << e << std::endl;
  return 1;
} catch (...) {
  std::cerr << "Generic exception!" << std::endl;
  return 2;
}
