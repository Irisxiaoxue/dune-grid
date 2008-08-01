// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id:$

#include "config.h" // autoconf defines, needed by the dune headers

// dune headers
#include <dune/grid/sgrid.hh>
#include <dune/grid/io/file/vtk/vtkwriter.hh>

#include <vector>
#include <unistd.h>

const char* VTKDataMode(Dune::VTKOptions::DataMode dm)
{
  switch(dm)
  {
  case Dune::VTKOptions::conforming :
    return "conforming";
  case Dune::VTKOptions::nonconforming :
    return "nonconforming";
  }
  return "";
}

template< class GridView >
class VTKVectorFuction
  : public Dune :: VTKWriter< typename GridView :: Grid, GridView > :: VTKFunction
{
  // extract types
  enum { n = GridView :: dimension };
  enum { w = GridView :: dimensionworld };
  typedef typename GridView :: Grid :: ctype DT;
  typedef typename GridView :: template Codim< 0 > :: Entity Entity;
public:
  //! return number of components
  virtual int ncomps () const { return n; };

  //! evaluate single component comp in the entity e at local coordinates xi
  /*! Evaluate the function in an entity at local coordinates.
     @param[in]  comp   number of component to be evaluated
     @param[in]  e      reference to grid entity of codimension 0
     @param[in]  xi     point in local coordinates of the reference element of e
     \return            value of the component
   */
  virtual double evaluate (int comp, const Entity& e, const Dune::FieldVector<DT,n>& xi) const
  {
    return comp*0.1;
  }

  // get name
  virtual std::string name () const
  {
    char _name[256];
    snprintf(_name, 256, "vector-%iD", ncomps());
    return std::string(_name);
  };


};

template< class GridView >
void doWrite( const GridView &gridView, Dune :: VTKOptions :: DataMode dm )
{
  enum { dim = GridView :: dimension };

  const typename GridView :: IndexSet &is = gridView.indexSet();
  std::vector<int> vertexdata(is.size(dim),dim);
  std::vector<int> celldata(is.size(0),0);

  Dune :: VTKWriter< typename GridView :: Grid, GridView > vtk( gridView, dm );
  vtk.addVertexData(vertexdata,"vertexData");
  vtk.addCellData(celldata,"cellData");

  VTKVectorFuction< GridView > *vectordata = new VTKVectorFuction< GridView >;
  vtk.addVertexData(vectordata);

  char name[256];
  snprintf(name,256,"vtktest-%iD-%s-ascii", dim, VTKDataMode(dm));
  vtk.write(name);

  snprintf(name,256,"vtktest-%iD-%s-binary", dim, VTKDataMode(dm));
  vtk.write(name, Dune::VTKOptions::binaryappended);
}

template<int dim>
void vtkCheck(int* n, double* h)
{
  const Dune :: PartitionIteratorType VTK_Partition = Dune :: InteriorBorder_Partition;
  std::cout << std::endl << "vtkCheck dim=" << dim << std::endl << std::endl;
  Dune::SGrid<dim,dim> g(n, h);
  g.globalRefine(1);

  doWrite( g.template leafView< VTK_Partition >(), Dune :: VTKOptions :: conforming );
  doWrite( g.template leafView< VTK_Partition >(), Dune :: VTKOptions :: nonconforming );
  doWrite( g.template levelView< VTK_Partition >( 0 ), Dune :: VTKOptions :: conforming );
  doWrite( g.template levelView< VTK_Partition >( 0 ), Dune :: VTKOptions :: nonconforming );
  doWrite( g.template levelView< VTK_Partition >( g.maxLevel() ), Dune :: VTKOptions :: conforming );
  doWrite( g.template levelView< VTK_Partition >( g.maxLevel() ), Dune :: VTKOptions :: nonconforming );
}

int main(int argc, char **argv)
{
  try {

    int n[] = { 5, 5, 5, 5 };
    double h[] = { 1.0, 2.0, 3.0, 4.0 };

    vtkCheck<1>(n,h);
    vtkCheck<2>(n,h);
    vtkCheck<3>(n,h);

  } catch (Dune::Exception &e) {
    std::cerr << e << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Generic exception!" << std::endl;
    return 2;
  }

  return 0;
}
