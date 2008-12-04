// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALBERTA_INTERSECTION_CC
#define DUNE_ALBERTA_INTERSECTION_CC

#include <dune/grid/albertagrid/intersection.hh>

namespace Dune
{

  template< class GridImp >
  inline AlbertaGridIntersectionIterator< GridImp >
  ::AlbertaGridIntersectionIterator ( const GridImp &grid, int level )
    : grid_( grid ),
      neighborCount_( dimension+1 ),
      elementInfo_(),
      fakeNeighObj_(LocalGeometryImp()),
      fakeSelfObj_ (LocalGeometryImp()),
      neighGlobObj_(LocalGeometryImp()),
      neighborInfo_()
  {}

  template< class GridImp >
  inline void
  AlbertaGridIntersectionIterator< GridImp >
  ::first ( const EntityImp &entity, int level )
  {
    neighborCount_ = 0;
    neighborInfo_ = ElementInfo();
    elementInfo_ = entity.elementInfo_;
    this->leafIt_  = entity.leafIt();

    assert( !elementInfo_ == false );
    assert( elementInfo_.level() == level );
  }

  template< class GridImp >
  inline void AlbertaGridIntersectionIterator< GridImp >::done ()
  {
    neighborCount_ = dimension+1;
    neighborInfo_ = ElementInfo();
    elementInfo_ = ElementInfo();
  }


  // copy constructor
  template< class GridImp >
  inline AlbertaGridIntersectionIterator< GridImp >
  ::AlbertaGridIntersectionIterator ( const This &other )
    : grid_( other.grid_ ),
      neighborCount_( other.neighborCount_ ),
      leafIt_( other.leafIt_ ),
      elementInfo_( other.elementInfo_ ),
      fakeNeighObj_( LocalGeometryImp() ),
      fakeSelfObj_ ( LocalGeometryImp() ),
      neighGlobObj_( LocalGeometryImp() ),
      neighborInfo_()
  {}


  // assignment operator
  template< class GridImp >
  inline void
  AlbertaGridIntersectionIterator< GridImp >::assign ( const This &other )
  {
    // only assign iterators from the same grid
    assert( &(this->grid_) == &(other.grid_) );

    neighborCount_ = other.neighborCount_;
    elementInfo_ = other.elementInfo_;
    neighborInfo_ = ElementInfo();
    leafIt_ = other.leafIt_;
  }


  template< class GridImp >
  inline bool
  AlbertaGridIntersectionIterator< GridImp >::equals ( const This &other ) const
  {
    const ALBERTA EL *e1 = elementInfo_.el();
    const ALBERTA EL *e2 = other.elementInfo_.el();
    return (e1 == e2) && (neighborCount_ == other.neighborCount_);
  }

  template< class GridImp >
  inline void AlbertaGridIntersectionIterator<GridImp>::increment()
  {
    neighborInfo_ = ElementInfo();
    ++neighborCount_;
    if( neighborCount_ > dimension )
      this->done();
  }

  template< class GridImp >
  inline typename AlbertaGridIntersectionIterator< GridImp >::EntityPointer
  AlbertaGridIntersectionIterator< GridImp >::outside () const
  {
    typedef AlbertaGridEntityPointer< 0, GridImp > EntityPointerImp;

    if( !neighborInfo_ )
    {
      assert( !elementInfo_ == false );
      neighborInfo_ = ElementInfo::createFake();
      std::memcpy( &(neighborInfo_.elInfo()), &(elementInfo_.elInfo()), sizeof( ALBERTA EL_INFO ) );

      setupVirtEn();

      assert( leafIt() || (elementInfo_.level() == neighborInfo_.level()) );
    }

    assert( !neighborInfo_ == false );
    assert( neighborInfo_.el() != NULL );
    const int neighborLevel = grid_.getLevelOfElement( neighborInfo_.el() );
    return EntityPointerImp( grid_, neighborLevel, neighborInfo_, 0 );
  }

  template< class GridImp >
  inline typename AlbertaGridIntersectionIterator< GridImp >::EntityPointer
  AlbertaGridIntersectionIterator< GridImp >::inside () const
  {
    typedef AlbertaGridEntityPointer< 0, GridImp > EntityPointerImp;
    assert( !elementInfo_ == false );
    return EntityPointerImp( grid_, elementInfo_.level(), elementInfo_, 0 );
  }

  template< class GridImp >
  inline int
  AlbertaGridIntersectionIterator<GridImp>::boundaryId () const
  {
    assert( !elementInfo_ == false );

    // id of interior intersections is 0
    if( !boundary() )
      return 0;

    const int id = elementInfo_.boundaryId( neighborCount_ );
    assert( id != 0 );
    return id;
  }

  template< class GridImp >
  inline bool AlbertaGridIntersectionIterator<GridImp>::conforming() const
  {
    return true;
  }

  template< class GridImp >
  inline bool AlbertaGridIntersectionIterator< GridImp >::boundary() const
  {
    assert( !elementInfo_ == false );
    return elementInfo_.isBoundary( neighborCount_ );
  }

  template< class GridImp >
  inline bool AlbertaGridIntersectionIterator< GridImp >::neighbor () const
  {
    assert( !elementInfo_ == false );
    const ALBERTA EL_INFO &elInfo = elementInfo_.elInfo();
    return (elInfo.neigh[ neighborCount_ ] != NULL);
  }

  template<class GridImp>
  inline const typename AlbertaGridIntersectionIterator< GridImp >::NormalVector
  AlbertaGridIntersectionIterator< GridImp >
  ::unitOuterNormal ( const LocalCoordType &local ) const
  {
    NormalVector normal;
    calcOuterNormal( normal );
    normal *= (1.0 / normal.two_norm());
    return normal;
  }

  template< class GridImp >
  inline const typename AlbertaGridIntersectionIterator< GridImp >::NormalVector
  AlbertaGridIntersectionIterator< GridImp >
  ::integrationOuterNormal ( const LocalCoordType &local ) const
  {
    NormalVector normal;
    calcOuterNormal( normal );
    return normal;
  }

  template< class GridImp >
  inline const typename AlbertaGridIntersectionIterator< GridImp >::NormalVector
  AlbertaGridIntersectionIterator< GridImp >
  ::outerNormal( const LocalCoordType &local ) const
  {
    return integrationOuterNormal( local );
  }

  template< class GridImp >
  inline void
  AlbertaGridIntersectionIterator<GridImp>::calcOuterNormal( NormalVector &n ) const
  {
    DUNE_THROW( NotImplemented, "AlbertaGrid: outer normal for dim != dimworld "
                "has not been implemented, yet." );
  }


  template<>
  inline void
  AlbertaGridIntersectionIterator< const AlbertaGrid< 2, 2 > >
  ::calcOuterNormal ( NormalVector &n ) const
  {
    assert( !elementInfo_ == false );
    const ALBERTA EL_INFO &elInfo = elementInfo_.elInfo();
    const ALBERTA REAL_D & coordOne = grid_.getCoord( &elInfo, (neighborCount_+1)%3 );
    const ALBERTA REAL_D & coordTwo = grid_.getCoord( &elInfo, (neighborCount_+2)%3 );

    n[ 0 ] = -(coordOne[ 1 ] - coordTwo[ 1 ]);
    n[ 1 ] =   coordOne[ 0 ] - coordTwo[ 0 ];
  }


  template<>
  inline void
  AlbertaGridIntersectionIterator< const AlbertaGrid< 3, 3 > >
  ::calcOuterNormal ( NormalVector &n ) const
  {
    assert( !elementInfo_ == false );
    const ALBERTA EL_INFO &elInfo = elementInfo_.elInfo();

    // in this case the orientation is negative, multiply by -1
#if (DUNE_ALBERTA_VERSION >= 0x200) || (DIM == 3)
    const albertCtype val = (elInfo.orientation > 0) ? 1.0 : -1.0;
#else
    const albertCtype val = 1.0;
#endif

    // neighborCount_ is the local face number
    const int *localFaces = ALBERTA AlbertHelp::localAlbertaFaceNumber[ neighborCount_ ];

    const ALBERTA REAL_D &coordZero = grid_.getCoord( &elInfo, localFaces[0] );
    const ALBERTA REAL_D &coordOne  = grid_.getCoord( &elInfo, localFaces[1] );
    const ALBERTA REAL_D &coordTwo  = grid_.getCoord( &elInfo, localFaces[2] );

    FieldVector< ctype, dimensionworld > u;
    FieldVector< ctype, dimensionworld > v;
    for( int i = 0; i < dimension; ++i )
    {
      v[ i ] = coordOne[ i ] - coordZero[ i ];
      u[ i ] = coordTwo[ i ] - coordOne[ i ];
    }

    // outNormal_ has length 3
    for( int i = 0; i < dimension; ++i )
    {
      const int j = (i+1)%dimension;
      const int k = (i+2)%dimension;
      n[ i ] = val * (u[ j ] * v[ k ] - u[ k ] * v[ j ]);
    }
  }


  template< class GridImp >
  inline const typename AlbertaGridIntersectionIterator< GridImp >::LocalGeometry &
  AlbertaGridIntersectionIterator< GridImp >::intersectionSelfLocal () const
  {
    assert( !elementInfo_ == false );
    ALBERTA EL_INFO &elInfo = elementInfo_.elInfo();

    LocalGeometryImp &geo = GridImp::getRealImplementation( fakeSelfObj_ );
    if( !geo.builtLocalGeom( inside()->geometry(), intersectionGlobal(), &elInfo, neighborCount_ ) )
      DUNE_THROW( AlbertaError, "internal error in intersectionSelfLocal." );
    return fakeSelfObj_;
  }


  template< class GridImp >
  inline const typename AlbertaGridIntersectionIterator< GridImp >::LocalGeometry &
  AlbertaGridIntersectionIterator< GridImp >::intersectionNeighborLocal () const
  {
    assert( neighbor() );
    assert( !neighborInfo_ == false );
    ALBERTA EL_INFO &nbInfo = neighborInfo_.elInfo();

    LocalGeometryImp &geo = GridImp::getRealImplementation( fakeNeighObj_ );
    if( !geo.builtLocalGeom( outside()->geometry(), intersectionGlobal(), &nbInfo, neighborCount_ ) )
      DUNE_THROW( AlbertaError, "internal error in intersectionNeighborLocal." );
    return fakeNeighObj_;
  }


  template< class GridImp >
  inline const typename AlbertaGridIntersectionIterator< GridImp >::Geometry &
  AlbertaGridIntersectionIterator< GridImp >::intersectionGlobal () const
  {
    assert( !elementInfo_ == false );
    ALBERTA EL_INFO &elInfo = elementInfo_.elInfo();

    GeometryImp &geo = GridImp::getRealImplementation( neighGlobObj_ );
    if( !geo.builtGeom( grid_, &elInfo, neighborCount_ ) )
      DUNE_THROW( AlbertaError, "internal error in intersectionGlobal." );
    return neighGlobObj_;
  }


  template< class GridImp >
  inline int AlbertaGridIntersectionIterator<GridImp>::
  numberInSelf () const
  {
    return neighborCount_;
  }

  template< class GridImp >
  inline int AlbertaGridIntersectionIterator<GridImp>::
  numberInNeighbor () const
  {
    assert( !elementInfo_ == false );
    const ALBERTA EL_INFO &elInfo = elementInfo_.elInfo();
    return elInfo.opp_vertex[ neighborCount_ ];
  }


  template< class GridImp >
  inline int
  AlbertaGridIntersectionIterator< GridImp >::twistInSelf () const
  {
    // always 0 for indside
    return 0;
  }


  template< class GridImp >
  inline int
  AlbertaGridIntersectionIterator< GridImp >::twistInNeighbor () const
  {
    return twist_;
  }


  // setup neighbor element with the information of elInfo_
  template< class GridImp >
  inline bool AlbertaGridIntersectionIterator<GridImp>::neighborHasSameLevel () const
  {
    assert( neighbor() );
    assert( !elementInfo_ == false );
    const ALBERTA EL_INFO &elInfo = elementInfo_.elInfo();

    const ALBERTA EL * myEl    = elementInfo_.el();
    const ALBERTA EL * neighEl = elInfo.neigh[ neighborCount_ ];

    return this->grid_.getLevelOfElement( myEl ) ==
           this->grid_.getLevelOfElement( neighEl );
  }

  //*****************************************
  //  setup for 2d
  //*****************************************
  template <class GridImp, int dimworld , int dim >
  struct SetupVirtualNeighbour;


  #if DIM == 2
  template <class GridImp>
  struct SetupVirtualNeighbour<GridImp,2,2>
  {
    enum { dim      = 2 };
    enum { dimworld = 2 };
    static int setupNeighInfo(GridImp & grid, const ALBERTA EL_INFO * elInfo,
                              const int vx, const int nb, ALBERTA EL_INFO * neighInfo)
    {

  #ifdef CALC_COORD
      // vx is the face number in the neighbour
      const int (& neighmap)[dim] = ALBERTA AlbertHelp :: localTriangleFaceNumber[vx];
      // neighborCount is the face number in the actual element
      const int (& selfmap) [dim] = ALBERTA AlbertHelp :: localTriangleFaceNumber[nb];

      // copy the two edge vertices
      for(int i=0; i<dim; i++)
      {
        const ALBERTA REAL_D & coord = elInfo->coord[ selfmap[i] ];
        // here the twist is simple, just swap the vertices, and do this by hand
        ALBERTA REAL_D & newcoord    = neighInfo->coord[ neighmap[(dim-1) - i] ];
        for(int j=0; j<dimworld; j++) newcoord[j] = coord[j];
      }
  #endif

      //****************************************
      // twist is always 1
      return 1;
    }
  };
  #endif

  //******************************************
  //  setup for 3d
  //******************************************
  #if DIM == 3
  template <class GridImp>
  struct SetupVirtualNeighbour<GridImp,3,3>
  {
    enum { dim      = 3 };
    enum { dimworld = 3 };
    static int setupNeighInfo(GridImp & grid, const ALBERTA EL_INFO * elInfo,
                              const int vx, const int nb, ALBERTA EL_INFO * neighInfo)
    {
      // the face might be twisted when look from different elements
      // default is no, and then rthe orientation is -1
      int facemap[dim]   = {0,1,2};
      bool rightOriented = false;
      {
        int myvx[dim];
        int neighvx[dim];

        const int * vxmap = ALBERTA AlbertHelp :: localAlbertaFaceNumber[vx];
        const int * nbmap = ALBERTA AlbertHelp :: localAlbertaFaceNumber[nb];

        bool allRight = true;
        for(int i=0; i<dim; i++)
        {
          myvx[i]    = grid.getVertexNumber(elInfo->el   , nbmap[i] );
          neighvx[i] = grid.getVertexNumber(neighInfo->el, vxmap[i] );
          if( myvx[i] != neighvx[i] ) allRight = false;
        }

        // note: if the vertices are equal then the face in the neighbor
        // is not oriented right, because all face are oriented math. pos when
        // one looks from the outside of the element.
        // if the vertices are the same, the face in the neighbor is therefore
        // wrong oriented
        if( !allRight )
        {
          for(int i=0; i<dim; i++)
          {
            if(myvx[i] != neighvx[i])
            {
              for(int k=1; k<dim; k++)
              {
                int newvx = (i+k) % dim;
                if( myvx[i] == neighvx[newvx] ) facemap[i] = newvx;
              }
            }
          }
          rightOriented = true;
        }
      }

      // TODO check infulence of orientation
      // is used when calculation the outerNormal
      neighInfo->orientation = ( rightOriented ) ? elInfo->orientation : -elInfo->orientation;

  #ifdef CALC_COORD
      // vx is the face number in the neighbour
      const int * neighmap = ALBERTA AlbertHelp :: localAlbertaFaceNumber[vx];
      // neighborCount is the face number in the actual element
      const int * selfmap  = ALBERTA AlbertHelp :: localAlbertaFaceNumber[nb];

      // copy the three face vertices
      for(int i=0; i<dim; i++)
      {
        const ALBERTA REAL_D & coord = elInfo->coord[selfmap[i]];
        // here consider possible twist
        ALBERTA REAL_D & newcoord    = neighInfo->coord[ neighmap[ facemap[i] ] ];
        for(int j=0; j<dimworld; j++) newcoord[j] = coord[j];
      }
  #endif

      //****************************************
      if (facemap[1] == (facemap[0]+1)%3) {
        return facemap[0];
      }
      // twist
      return facemap[1]-3;
    }
  };
  #endif

  // setup neighbor element with the information of elInfo_
  template< class GridImp >
  inline void AlbertaGridIntersectionIterator< GridImp >::setupVirtEn () const
  {
    // if this assertion fails then outside was called without checking
    // neighbor first
    assert( neighbor() );
    assert( neighborCount_ < dimension+1 );

    assert( !elementInfo_ == false );
    const ALBERTA EL_INFO &elInfo = elementInfo_.elInfo();

    // set the neighbor element as element
    // use ALBERTA macro to get neighbour
    assert( !neighborInfo_ == false );
    ALBERTA EL_INFO &nbInfo = neighborInfo_.elInfo();

    nbInfo.el = elInfo.neigh[ neighborCount_ ];
    assert( nbInfo.el != NULL );

    const int vx = elInfo.opp_vertex[ neighborCount_ ];
    assert( (vx >= 0) && (vx < ElementInfo::maxNeighbors) );

    // reset neighbor information
    for( int i = 0; i <= dimension; ++i )
    {
      nbInfo.neigh[ i ] = NULL;
      nbInfo.opp_vertex[ i ] = 127;
    }

    // set origin
    nbInfo.neigh[ vx ] = elInfo.el;
    nbInfo.opp_vertex[ vx ] = neighborCount_;


  #ifdef CALC_COORD
    // copy the one opposite vertex
    // the same for 2d and 3d
    {
      const ALBERTA REAL_D &coord = elInfo.opp_coord[ neighborCount_ ];
      ALBERTA REAL_D &newcoord = nbInfo.coord[ vx ];
      for( int j = 0; j < dimensionworld; ++j )
        newcoord[ j ] = coord[ j ];
    }
  #endif

    // setup coordinates of neighbour elInfo
    twist_ = SetupVirtualNeighbour<GridImp,dimensionworld,dimension>::
             setupNeighInfo( this->grid_, &elInfo, vx, neighborCount_, &nbInfo );
  }

}

#endif
