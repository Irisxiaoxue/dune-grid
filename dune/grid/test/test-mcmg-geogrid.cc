// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>

#include <dune/common/fvector.hh>

#include <dune/grid/geometrygrid.hh>
#include <dune/grid/geometrygrid/coordfunction.hh>
#include <dune/grid/sgrid.hh>
#include <dune/grid/common/mcmgmapper.hh>

const int dim=2;

using namespace Dune;

template <class GridView>
class DeformationFunction
  : public Dune :: DiscreteCoordFunction< double, dim, DeformationFunction<GridView> >
{
  typedef DeformationFunction<GridView> This;
  typedef Dune :: DiscreteCoordFunction< double, dim, This > Base;

public:

  DeformationFunction()
  {}

  void evaluate ( const typename GridView::template Codim<dim>::Entity& hostEntity, unsigned int corner,
                  FieldVector<double,dim> &y ) const
  {
    y = hostEntity.geometry().corner(0);
  }

  void evaluate ( const typename GridView::template Codim<0>::Entity& hostEntity, unsigned int corner,
                  FieldVector<double,dim> &y ) const
  {
    y = hostEntity.geometry().corner(corner);
  }

  void adapt() {}
};


int main (int argc, char *argv[]) try
{

  // make simple SGrid
  typedef SGrid<2,2,double> GridType;

  int cells[2] = {1,1};
  double extend[2] = {1,1};
  GridType grid(cells,extend);

  // make deformed grid with identity deformation
  typedef DeformationFunction<GridType::LeafGridView> DeformationFunction;
  typedef GeometryGrid<GridType,DeformationFunction> DeformedGridType;

  DeformationFunction deformation;
  DeformedGridType defGrid(grid,deformation);

  LeafMultipleCodimMultipleGeomTypeMapper<DeformedGridType,MCMGElementLayout > mapper(defGrid);

  //grid.globalRefine(1);
  //defGrid.update();

  mapper.update();

}
catch(Exception e)
{
  std::cout<<e<<std::endl;
}
