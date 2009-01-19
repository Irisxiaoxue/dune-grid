// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALBERTA_ENTITY_CC
#define DUNE_ALBERTA_ENTITY_CC

namespace Dune
{

  // AlbertaGridEntity (for codim > 0)
  // ---------------------------------

  template<int codim, int dim, class GridImp>
  inline AlbertaGridEntity< codim, dim, GridImp >
  ::AlbertaGridEntity ( const GridImp &grid )
    : grid_( grid ),
      elementInfo_(),
      subEntity_( -1 ),
      geo_( GeometryImp() )
  {}


  template<int codim, int dim, class GridImp>
  inline AlbertaGridEntity< codim, dim, GridImp >
  :: AlbertaGridEntity ( const This &other )
    : grid_( other.grid_ ),
      elementInfo_( other.elementInfo_ ),
      subEntity_( other.subEntity_ ),
      geo_( other.geo_ )
  {}


  template<int codim, int dim, class GridImp>
  inline PartitionType AlbertaGridEntity <codim,dim,GridImp>::
  partitionType () const
  {
    return InteriorEntity;
  }


  template< int codim, int dim, class GridImp >
  inline bool
  AlbertaGridEntity< codim, dim, GridImp >::equals ( const This &other ) const
  {
    const ALBERTA EL *e1 = getElement();
    const ALBERTA EL *e2 = other.getElement();

    // if both element null then they are equal
    if( (e1 == NULL) && (e2 == NULL) )
      return true;
    return ((e1 == e2) && (subEntity_ == other.subEntity_));
  }


  template<int codim, int dim, class GridImp>
  inline ALBERTA EL_INFO *
  AlbertaGridEntity< codim, dim, GridImp >::getElInfo () const
  {
    return &(elementInfo_.elInfo());
  }


  template< int codim, int dim, class GridImp >
  inline ALBERTA EL *
  AlbertaGridEntity< codim, dim, GridImp >::getElement() const
  {
    return elementInfo_.el();
  }


  template< int codim, int dim, class GridImp >
  inline void
  AlbertaGridEntity< codim, dim, GridImp >::clearElement ()
  {
    elementInfo_ = ElementInfo();
  }


  template< int codim, int dim, class GridImp >
  inline void AlbertaGridEntity< codim, dim, GridImp >
  ::setElement ( const ElementInfo &elementInfo, int subEntity )
  {
    elementInfo_ = elementInfo;
    subEntity_ = subEntity;
    if( !elementInfo_ == false )
    {
      typedef AlbertaGridCoordinateReader< codim, GridImp > CoordReader;
      const CoordReader coordReader( grid_, elementInfo_, subEntity_ );
      geoImp().build( coordReader );
    }
  }


  template< int codim, int dim, class GridImp >
  inline void
  AlbertaGridEntity< codim, dim, GridImp >::setEntity( const This &other )
  {
    setElement( other.elementInfo_, other.subEntity_ );
  }


  template<int codim, int dim, class GridImp>
  inline int AlbertaGridEntity<codim,dim,GridImp>::level() const
  {
    assert( elementInfo_.level() == grid_.levelProvider() ( elementInfo_ ) );
    return elementInfo_.level();
  }


  template<int codim, int dim, class GridImp>
  inline int AlbertaGridEntity<codim,dim,GridImp>::getFEVnum() const
  {
    return subEntity_;
  }


  template<int cd, int dim, class GridImp>
  inline const typename AlbertaGridEntity<cd,dim,GridImp>::Geometry &
  AlbertaGridEntity<cd,dim,GridImp>::geometry() const
  {
    assert( !elementInfo_ == false);
    return geo_;
  }


  template< int codim, int dim, class GridImp >
  inline GeometryType AlbertaGridEntity< codim, dim, GridImp >::type () const
  {
    return GeometryType( GeometryType::simplex, mydimension );
  }



  // AlbertaGridEntity (for codim = 0)
  // ---------------------------------

  template< int dim, class GridImp >
  inline AlbertaGridEntity< 0, dim, GridImp >
  ::AlbertaGridEntity( const GridImp &grid )
    : grid_(grid),
      elementInfo_(),
      geo_( GeometryImp() ),
      builtgeometry_( false )
  {}


  template< int dim, class GridImp >
  inline AlbertaGridEntity< 0, dim, GridImp >
  ::AlbertaGridEntity ( const This &other )
    : grid_( other.grid_ ),
      elementInfo_( other.elementInfo_ ),
      geo_( other.geo_ ),
      builtgeometry_ ( false )
  {}


  template< int dim, class GridImp >
  inline int AlbertaGridEntity< 0, dim, GridImp >::boundaryId() const
  {
    // elements are always inside of our Domain
    return 0;
  }


  template< int dim, class GridImp >
  inline bool AlbertaGridEntity< 0, dim,GridImp >::isNew () const
  {
    return grid_.levelProvider().isNew( elementInfo_ );
  }


  template< int dim, class GridImp >
  inline bool AlbertaGridEntity< 0, dim, GridImp>::mightVanish () const
  {
    return elementInfo_.mightVanish();
  }


  template< int dim, class GridImp >
  inline bool
  AlbertaGridEntity< 0, dim, GridImp >::hasBoundaryIntersections () const
  {
    assert( !elementInfo_ == false );
    bool isBoundary = false;
    for( int i = 0; i < dim+1; ++i )
      isBoundary |= elementInfo_.isBoundary( i );
    return isBoundary;
  }


  template< int dim, class GridImp >
  inline PartitionType
  AlbertaGridEntity< 0, dim, GridImp >::partitionType () const
  {
    return InteriorEntity;
  }


  template< int dim, class GridImp >
  inline bool AlbertaGridEntity< 0, dim,GridImp >::isLeaf () const
  {
    return elementInfo_.isLeaf();
  }


  template< int dim, class GridImp >
  inline bool
  AlbertaGridEntity< 0, dim, GridImp >::equals ( const This &other ) const
  {
    // element pointers are unique
    return (getElement() == other.getElement());
  }


  //*****************************************************************
  // count
  template <class GridImp, int dim, int cc> struct AlbertaGridCount {
    static int count () { return dim+1; }
  };

  // specialisation for codim 0
  template <class GridImp, int dim> struct AlbertaGridCount<GridImp,dim,0> {
    static int count () { return 1; }
  };

  // specialisation for edges in 3d
  template <class GridImp> struct AlbertaGridCount<GridImp,3,2> {
    static int count () { return 6; }
  };

  template<int dim, class GridImp> template <int cc>
  inline int AlbertaGridEntity <0,dim,GridImp>::count () const
  {
    return AlbertaGridCount<GridImp,dim,cc>::count();
  }

  //*****************************************************************

  template< int dim, class GridImp >
  template< int codim >
  inline typename AlbertaGridEntity< 0, dim, GridImp >
  ::template Codim< codim >::EntityPointer
  AlbertaGridEntity< 0, dim, GridImp >::entity ( int i ) const
  {
    typedef AlbertaGridEntityPointer< codim, GridImp > EntityPointerImpl;
    return EntityPointerImpl( grid_, elementInfo_, i );
  }


  template< int dim, class GridImp >
  inline ALBERTA EL_INFO *
  AlbertaGridEntity< 0, dim, GridImp >::getElInfo () const
  {
    return &(elementInfo_.elInfo());
  }


  template< int dim, class GridImp >
  inline ALBERTA EL *
  AlbertaGridEntity< 0, dim, GridImp >::getElement () const
  {
    return elementInfo_.el();
  }


  template< int dim, class GridImp >
  inline int AlbertaGridEntity< 0, dim, GridImp >::level () const
  {
    assert( elementInfo_.level() == grid_.levelProvider() ( elementInfo_ ) );
    return elementInfo_.level();
  }


  template< int dim, class GridImp >
  inline void
  AlbertaGridEntity< 0, dim, GridImp >::clearElement ()
  {
    elementInfo_ = ElementInfo();
    builtgeometry_ = false;
  }


  template< int dim, class GridImp >
  inline void AlbertaGridEntity< 0, dim, GridImp >
  ::setElement ( const ElementInfo &elementInfo, int subEntity )
  {
    elementInfo_ = elementInfo;
    builtgeometry_ = false;
  }


  template< int dim, class GridImp >
  inline void
  AlbertaGridEntity< 0, dim, GridImp >::setEntity( const This &other )
  {
    setElement( other.elementInfo_, 0 );
  }


  template< int dim, class GridImp >
  inline const typename AlbertaGridEntity< 0, dim, GridImp >::Geometry &
  AlbertaGridEntity< 0, dim, GridImp >::geometry () const
  {
    assert( !elementInfo_ == false );
    // geometry is only build on demand
    if( !builtgeometry_ )
    {
      typedef AlbertaGridCoordinateReader< 0, GridImp > CoordReader;
      const CoordReader coordReader( grid_, elementInfo_, 0 );
      geoImp().build( coordReader );
      builtgeometry_ = true;
    }
    assert( builtgeometry_ );
    return geo_;
  }


  template< int dim, class GridImp >
  inline GeometryType AlbertaGridEntity< 0, dim, GridImp>::type () const
  {
    return GeometryType( GeometryType::simplex, mydimension );
  }


  template< int dim, class GridImp >
  inline typename AlbertaGridEntity< 0, dim, GridImp >::EntityPointer
  AlbertaGridEntity< 0, dim, GridImp >::father () const
  {
    typedef AlbertaGridEntityPointer< 0, GridImp > EntityPointerImpl;

    assert( !elementInfo_ == false );
    const ElementInfo fatherInfo = elementInfo_.father();

    return EntityPointerImpl( grid_, fatherInfo, 0 );
  }


  template< int dim, class GridImp >
  inline int AlbertaGridEntity< 0, dim, GridImp >::nChild () const
  {
    return elementInfo_.indexInFather();
  }


  template< int dim, class GridImp >
  inline const typename AlbertaGridEntity< 0, dim, GridImp >::LocalGeometry &
  AlbertaGridEntity< 0, dim, GridImp >::geometryInFather() const
  {
    typedef AlbertaGridLocalGeometryProvider< GridImp > LocalGeoProvider;
    const int indexInFather = elementInfo_.indexInFather();
    const int orientation = (elementInfo_.type() == 1 ? -1 : 1);
    return LocalGeoProvider::instance().geometryInFather( indexInFather, orientation );
  }


  template< int dim, class GridImp >
  inline typename AlbertaGridEntity< 0, dim, GridImp >::HierarchicIterator
  AlbertaGridEntity< 0, dim, GridImp >::hbegin( int maxlevel ) const
  {
    assert( !elementInfo_ == false );
    typedef AlbertaGridHierarchicIterator< GridImp > IteratorImp;
    return IteratorImp( grid_, elementInfo_, maxlevel );
  }


  template< int dim, class GridImp >
  inline typename AlbertaGridEntity< 0, dim, GridImp >::HierarchicIterator
  AlbertaGridEntity< 0, dim, GridImp>::hend( int maxlevel ) const
  {
    assert( !elementInfo_ == false );
    typedef AlbertaGridHierarchicIterator< GridImp > IteratorImp;
    return IteratorImp( grid_, level(), maxlevel );
  }


  template< int dim, class GridImp >
  inline typename AlbertaGridEntity< 0, dim, GridImp >::AlbertaGridLeafIntersectionIteratorType
  AlbertaGridEntity< 0, dim, GridImp >::ileafbegin() const
  {
    assert( !elementInfo_ == false );
  #ifndef NDEBUG
    for( int i = 0; i <= dimension; ++i )
    {
      // std::cout << "Opposite vertex " << i << ": "
      //           << (int)(getElInfo()->opp_vertex[ i ]) << std::endl;
      if( getElInfo()->opp_vertex[ i ] == 127 )
      {
        assert( false );
        DUNE_THROW( NotImplemented, "AlbertaGrid: Intersections on outside "
                    "entities are not fully implemented, yet." );
      }
    }
  #endif
    return AlbertaGridLeafIntersectionIteratorType( grid_, *this, level(), false );
  }


  template< int dim, class GridImp >
  inline typename AlbertaGridEntity< 0, dim, GridImp >::AlbertaGridLeafIntersectionIteratorType
  AlbertaGridEntity< 0, dim, GridImp >::ileafend() const
  {
    assert( !elementInfo_ == false );
    return AlbertaGridLeafIntersectionIteratorType( grid_, *this, level(), true );
  }

}

#endif
