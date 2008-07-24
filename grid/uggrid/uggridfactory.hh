// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UGGRID_FACTORY_HH
#define DUNE_UGGRID_FACTORY_HH

/** \file
    \brief The specialization of the generic GridFactory for UGGrid
    \author Oliver Sander
 */

#include <vector>

#include <dune/common/fvector.hh>

#include <dune/grid/common/boundarysegment.hh>
#include <dune/grid/common/gridfactory.hh>
#include <dune/grid/uggrid.hh>

namespace Dune {

  /** \brief Specialization of the generic GridFactory for UGGrid

     <p>
     If you want to write a routine that reads a grid from some
     file into a Dune UGGrid object you have to know how to use the UGGrid
     grid factory.  In the following we
     assume that you have a grid in some file format and an
     empty UGGrid object, created by one of its constructors.
     Hence, your file reader method signature may look like this:
     </p>

     <pre>
     UGGrid&lt;3&gt;&* readMyFileFormat(const std::string& filename)
     </pre>

     Now, in order to create a valid UGGrid object do the
     following steps:

     <h2> 1) Create a GridFactory Object </h2>
     <p>
     Get a new GridFactory object by calling
     First of all you have to call
     <pre>
     GridFactory<UGGrid<dim> > factory;
     </pre>

     <h2> 2)  Enter the Vertices </h2>

     <p>
     Now you have to enter the grid vertices.  Create a vertex by calling
     </p>

     <pre>
     factory.insertVertex(const FieldVector&lt;double,dimworld&gt;& position);
     </pre>

     <h2> 3) Enter the elements </h2>

     <p>
     For each element call
     </p>

     <pre>
     factory.insertElement(Dune::GeometryType type, const std::vector&lt;int&gt;& vertices);
     </pre>

     <p>
     The parameters are
     </p>

     <ul>
     <li> <b>type</b> - The element type.  UG supports the types <i>simplex</i> and
     <i>cube</i> in 2d, and <i>simplex, cube, prism</i>, and <i>pyramid</i> in 3d.
     <li> <b>vertices</b> - The Ids of the vertices of this element.</li>
     </ul>

     <p>
     The numbering of the vertices of each element is expected to follow the DUNE conventions.
     Refer to the page on reference elements for the details.

     <h2> 4) Parametrized Domains </h2>

     <p>
     UGGrid supports parametrized domains.  That means that you can provide a
     smooth description of your grid boundary.  The actual grid will always
     be piecewise linear; however, as you refine, the grid will approach your
     prescribed boundary.  You don't have to do this.  If your
     coarse grid boundary describes your domain well read on at Section 5.
     </p>

     <p>
     In order to create curved boundary segments, for each segment you have to write
     a class which implements the correct geometry.  These classes are then handed
     over to the UGGrid object.  Boundary segment implementations must be derived
     from
     <pre>
     template &lt;int dimworld&gt; Dune::BoundarySegment
     </pre>
     This is an abstract base class which requires you to overload the method

     <pre>
     virtual FieldVector&lt; double, dimworld &gt; operator() (const FieldVector&lt; double, dimworld-1 &gt; &local)
     </pre>

     <p>
     This methods must compute the world coordinates from the local ones on the
     boundary segment.  Give these classes to your grid by calling
     </p>
     <pre>
     grid.insertBoundarySegment(const std::vector&lt;int&gt;& vertices,
                             const BoundarySegment&lt;dimworld&gt; *boundarySegment);
     </pre>

     <p>
     Control over the allocated objects is taken from you, and the grid object
     will take care of their destruction.
     </p>

     <h2> 5) Finish construction </h2>

     <p>
     To finish off the construction of the UGGrid object call
     </p>

     <pre>
     UGGrid<dim>* grid = factory.createGrid();
     </pre>

     <p>
     This time it is you who gets full responsibility for the allocated object.
     </p>

     <h2> Loading a Grid on a Parallel Machine </h2>
     <p>
     If you're working on a parallel machine, and you want to set up a
     parallel grid, proceed as described on all processes.  This will
     create the grid on the master process and set up UG correctly on all
     other process.  Call <tt>loadBalance()</tt> to actually distribute the grid.
     </p>

     <p>
     <b>WARNING:</b> UG internally requests that all boundary vertices be
     inserted before the inner ones.  That means that if your input grid
     doesn't comply with this, it will have its vertices reordered by
     <tt>createGrid()</tt>.  So don't be surprised if you just read a grid and write
     it back to disk to find your vertex numberings changed.
     </p>

   */
  template <int dimworld>
  class GridFactory<UGGrid<dimworld> > : public GridFactoryInterface<UGGrid<dimworld> > {

    /** \brief Type used by the grid for coordinates */
    typedef typename UGGrid<dimworld>::ctype ctype;

    // UGGrid only in 2d and 3d
    dune_static_assert(dimworld==2 || dimworld || 3, "UGGrid only in 2d and 3d");

  public:

    /** \brief Default constructor */
    GridFactory();

    /** \brief Constructor for a given grid object

       If you already have your grid object constructed you can
       hand it over using this constructor.  A reason may be that
       you need a UGGrid object with a non-default heap size.

       If you construct your factory class using this constructor
       the pointer handed over to you by the method createGrid() is
       the one you supplied here.
     */
    GridFactory(UGGrid<dimworld>* grid);

    /** \brief Destructor */
    ~GridFactory();

    /** \brief Insert a vertex into the coarse grid */
    virtual void insertVertex(const FieldVector<ctype,dimworld>& pos);

    /** \brief Insert an element into the coarse grid
        \param type The GeometryType of the new element
        \param vertices The vertices of the new element, using the DUNE numbering
     */
    virtual void insertElement(const GeometryType& type,
                               const std::vector<unsigned int>& vertices);

    /** \brief Method to insert an arbitrarily shaped boundary segment into a coarse grid
        \param vertices The indices of the vertices of the segment
        \param boundarySegment Class implementing the geometry of the boundary segment.
        The grid object takes control of this object and deallocates it when destructing itself.
     */
    void insertBoundarySegment(const std::vector<unsigned int> vertices,
                               const BoundarySegment<dimworld>* boundarySegment);


    /** \brief Finalize grid creation and hand over the grid

       The receiver takes responsibility of the memory allocated for the grid
     */
    virtual UGGrid<dimworld>* createGrid();

  private:
    // Pointer to the grid being built
    UGGrid<dimworld>* grid_;

    // True if the factory allocated the grid itself, false if the
    // grid was handed over from the outside
    bool factoryOwnsGrid_;

  };

}

#endif
