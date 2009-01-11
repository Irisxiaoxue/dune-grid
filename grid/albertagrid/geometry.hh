// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALBERTA_GEOMETRY
#define DUNE_ALBERTA_GEOMETRY

#include <dune/grid/common/geometry.hh>

namespace Dune
{

  // Forward Declarations
  // --------------------

  template< int dim, int dimworld >
  class AlbertaGrid;



  // AlbertaGridCoordinateReader
  // ---------------------------

  template< int codim, class GridImp >
  struct AlbertaGridCoordinateReader
  {
    typedef typename remove_const< GridImp >::type Grid;

    static const int dimension = Grid::dimension;
    static const int codimension = codim;
    static const int mydimension = dimension - codimension;
    static const int coorddimension = Grid::dimensionworld;

    typedef Alberta::Real ctype;

    typedef Alberta::ElementInfo< dimension > ElementInfo;
    typedef FieldVector< ctype, coorddimension > Coordinate;

  private:
    const Grid &grid_;
    const ElementInfo &elementInfo_;
    const int subEntity_;

  public:
    AlbertaGridCoordinateReader ( const GridImp &grid,
                                  const ElementInfo &elementInfo,
                                  int subEntity )
      : grid_( grid ),
        elementInfo_( elementInfo ),
        subEntity_( subEntity )
    {}

    void coordinate ( int i, Coordinate &x ) const
    {
      assert( !elementInfo_ == false );
      assert( (i >= 0) && (i <= mydimension) );

      const int k = mapVertices( subEntity_, i );
      const Alberta::GlobalVector &coord = grid_.getCoord( elementInfo_, k );
      for( int j = 0; j < coorddimension; ++j )
        x[ j ] = coord[ j ];
    }

    bool hasDeterminant () const
    {
      return ((codimension == 0) && elementInfo_.isLeaf());
    }

    ctype determinant () const
    {
      assert( hasDeterminant() );

      const Alberta::Element *el = elementInfo_.el();
      typedef typename Grid::LeafDataType::Data LeafData;
      LeafData *leafdata = (LeafData *)el->child[ 1 ];
      assert( leafdata != NULL );
      return leafdata->determinant;
    }

  private:
    static int mapVertices ( int subEntity, int i )
    {
      typedef AlbertHelp::MapVertices< mydimension, dimension > Mapper;
      return Mapper::mapVertices( subEntity, i );
    }
  };



  // AlbertaGridGeometry
  // -------------------

  /** \class AlbertaGridGeometry
   *  \brief geometry implementation for AlbertaGrid
   *
   *  Defines the geometry part of a mesh entity. Works for all dimensions,
   *  element types and dim of world. Provides reference element and mapping
   *  between local and global coordinates.
   *
   *  \tparam  mydim    dimension of the element (0 <= dim <= 3)
   *  \tparam  cdim     dimension of global coordinates
   *  \tparam  GridImp  grid implementation
   *                    (always const AlbertaGrid< dim, dimworld >)
   */
  template< int mydim, int cdim, class GridImp >
  class AlbertaGridGeometry
  {
    typedef AlbertaGridGeometry< mydim, cdim, GridImp > This;

    // remember type of the grid
    typedef GridImp Grid;

    // dimension of barycentric coordinates
    static const int dimbary = mydim + 1;

  public:
    //! type of coordinates
    typedef albertCtype ctype;

    static const int dimension = Grid :: dimension;
    static const int mydimension = mydim;
    static const int codimension = dimension - mydimension;
    static const int coorddimension = cdim;

  private:
    static const int numCorners = mydimension + 1;

    typedef FieldMatrix< ctype, numCorners, coorddimension > CoordMatrix;

  public:
    //! Default constructor
    AlbertaGridGeometry();

    AlbertaGridGeometry ( const CoordMatrix &coords );

    template< class CoordReader >
    AlbertaGridGeometry ( const CoordReader &coordReader );

    //! return the element type identifier
    //! line , triangle or tetrahedron, depends on dim
    GeometryType type () const;

    /** \brief obtain the number of corners of this element */
    int corners () const;

    //! access to coordinates of corners. Index is the number of the corner
    const FieldVector< ctype, cdim > &operator[] (int i) const;

    //! maps a local coordinate within reference element to
    //! global coordinate in element
    FieldVector< ctype, cdim >
    global ( const FieldVector< ctype, mydim> &local ) const;

    //! maps a global coordinate within the element to a
    //! local coordinate in its reference element
    FieldVector< ctype, mydim >
    local ( const FieldVector< ctype, cdim  > &global ) const;

    //! returns true if the point in local coordinates is inside reference element
    bool checkInside( const FieldVector< ctype, mydim > &local ) const;

    /*!
       Copy from sgrid.hh:

       Integration over a general element is done by integrating over the reference element
       and using the transformation from the reference element to the global element as follows:
       \f[\int\limits_{\Omega_e} f(x) dx = \int\limits_{\Omega_{ref}} f(g(l)) A(l) dl \f] where
       \f$g\f$ is the local to global mapping and \f$A(l)\f$ is the integration element.

       For a general map \f$g(l)\f$ involves partial derivatives of the map (surface element of
       the first kind if \f$d=2,w=3\f$, determinant of the Jacobian of the transformation for
       \f$d=w\f$, \f$\|dg/dl\|\f$ for \f$d=1\f$).

       For linear elements, the derivatives of the map with respect to local coordinates
       do not depend on the local coordinates and are the same over the whole element.

       For a structured mesh where all edges are parallel to the coordinate axes, the
       computation is the length, area or volume of the element is very simple to compute.

       Each grid module implements the integration element with optimal efficieny. This
       will directly translate in substantial savings in the computation of finite element
       stiffness matrices.
     */

    // A(l)
    ctype integrationElement ( const FieldVector< ctype, mydim > &local ) const;

    // volume if geometry
    ctype volume () const;

    //! can only be called for dim=dimworld!
    //! Note that if both methods are called on the same element, then
    //! call jacobianInverseTransposed first because integration element is calculated
    //! during calculation of the transposed of the jacobianInverse
    const FieldMatrix< ctype, cdim, mydim > &
    jacobianInverseTransposed ( const FieldVector< ctype, mydim > &local ) const;

    //***********************************************************************
    //  Methods that not belong to the Interface, but have to be public
    //***********************************************************************

    void invalidate ();

    template< class CoordReader >
    void build ( const CoordReader &coordReader );

    //! print internal data
    //! no interface method
    void print (std::ostream& ss) const;

  private:
    // calculate Matrix for Mapping from reference element to actual element
    void calcElMatrix () const;

    //! build the transposed of the jacobian inverse and store the volume
    void buildJacobianInverseTransposed () const;

    // calculates the volume of the element
    ctype elDeterminant () const;

    //! the vertex coordinates
    CoordMatrix coord_;

    mutable FieldMatrix< ctype, cdim, mydim > Jinv_; //!< storage for inverse of jacobian

    mutable FieldMatrix< ctype, cdim, mydim > elMat_; //!< storage for mapping matrix

    //! is true if elMat_ was calced
    mutable bool builtElMat_;
    //! is true if Jinv_ and volume_ is calced
    mutable bool builtinverse_;

    mutable bool calcedDet_; //!< true if determinant was calculated
    mutable ctype elDet_; //!< storage of element determinant
  };



  // AlbertaGridLocalGeometryProvider
  // --------------------------------

  template< class Grid >
  class AlbertaGridLocalGeometryProvider
  {
    typedef AlbertaGridLocalGeometryProvider< Grid > This;

  public:
    typedef typename Grid::ctype ctype;

    static const int dimension = Grid::dimension;

    template< int codim >
    struct Codim
    {
      typedef Geometry< dimension-codim, dimension, Grid, AlbertaGridGeometry >
      LocalGeometry;
    };

    typedef typename Codim< 0 >::LocalGeometry LocalElementGeometry;
    typedef typename Codim< 1 >::LocalGeometry LocalFaceGeometry;

    static const int numChildren = 2;
    static const int numFaces = dimension + 1;

  private:
    struct GeoInFatherCoordReader;

    const LocalElementGeometry *geometryInFather_[ numChildren ][ 2 ];
    const LocalFaceGeometry *faceGeometry_[ numFaces ];

    AlbertaGridLocalGeometryProvider ()
    {
      buildGeometryInFather();
      buildFaceGeometry();
    }

    ~AlbertaGridLocalGeometryProvider ()
    {
      for( int child = 0; child < numChildren; ++child )
      {
        delete geometryInFather_[ child ][ 0 ];
        delete geometryInFather_[ child ][ 1 ];
      }

      for( int i = 0; i < numFaces; ++i )
        delete faceGeometry_[ i ];
    }

    void buildGeometryInFather();
    void buildFaceGeometry();

  public:
    const LocalElementGeometry &
    geometryInFather ( int child, const int orientation = 1 ) const
    {
      assert( (child >= 0) && (child < numChildren) );
      assert( (orientation == 1) || (orientation == -1) );
      return *geometryInFather_[ child ][ (orientation + 1) / 2 ];
    }

    const LocalFaceGeometry &
    faceGeometry ( int face ) const
    {
      assert( (face >= 0) && (face < numFaces) );
      return *faceGeometry_[ face ];
    }

    static const This &instance ()
    {
      static This theInstance;
      return theInstance;
    }
  };

}

#endif
