// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALBERTA_TREEITERATOR_CC
#define DUNE_ALBERTA_TREEITERATOR_CC

#include <dune/grid/albertagrid/treeiterator.hh>

namespace Dune
{

  // AlbertaMarkerVector
  // -------------------

  template< int dim, int dimworld >
  template< int codim >
  inline bool AlbertaMarkerVector< dim, dimworld >
  ::subEntityOnElement ( const ElementInfo &elementInfo, int subEntity ) const
  {
    assert( marker_[ codim ] != 0 );

    const int subIndex = dofNumbering_( elementInfo, codim, subEntity );
    const int markIndex = marker_[ codim ][ subIndex ];
    assert( (markIndex >= 0) );

    const int index = dofNumbering_( elementInfo, 0, 0 );
    return (markIndex == index);
  }


  template< int dim, int dimworld >
  template< int firstCodim, class Iterator >
  inline void AlbertaMarkerVector< dim, dimworld >
  ::markSubEntities ( const Iterator &begin, const Iterator &end )
  {
    clear();

    for( int codim = firstCodim; codim <= dimension; ++codim )
    {
      const int size = dofNumbering_.size( codim );
      marker_[ codim ] = new int[ size ];

      int *array = marker_[ codim ];
      for( int i = 0; i < size; ++i )
        array[ i ] = -1;
    }

    for( Iterator it = begin; it != end; ++it )
    {
      const ElementInfo &elementInfo = Grid::getRealImplementation( *it ).elementInfo();
      Alberta::ForLoop< MarkSubEntities, firstCodim, dimension >
      ::apply( dofNumbering_, marker_, elementInfo );
    }
  }



  template< int dim, int dimworld >
  inline void AlbertaMarkerVector< dim, dimworld >::print ( std::ostream &out ) const
  {
    for( int codim = 1; codim <= dimension; ++codim )
    {
      int *marker = marker_[ codim ];
      if( marker != 0 )
      {
        const int size = dofNumbering_.size( codim );
        out << std::endl;
        out << "Codimension " << codim << " (" << size << " entries)" << std::endl;
        for( int i = 0; i < size; ++i )
          out << "subentity " << i << " visited on Element " << marker[ i ] << std::endl;
      }
    }
  }



  // AlbertaMarkerVector::MarkSubEntities
  // ------------------------------------

  template< int dim, int dimworld >
  template< int codim >
  class AlbertaMarkerVector< dim, dimworld >::MarkSubEntities
  {
    static const int numSubEntities = Alberta::NumSubEntities< dimension, codim >::value;

    typedef Alberta::ElementInfo< dimension > ElementInfo;

  public:
    static void apply ( const DofNumbering &dofNumbering,
                        int *(&marker)[ dimension + 1 ],
                        const ElementInfo &elementInfo )
    {
      int *array = marker[ codim ];

      const int index = dofNumbering( elementInfo, 0, 0 );
      for( int i = 0; i < numSubEntities; ++i )
      {
        int &mark = array[ dofNumbering( elementInfo, codim, i ) ];
        mark = std::max( index, mark );
      }
    }
  };



  // AlbertaGridTreeIterator
  // -----------------------

  template< int codim, class GridImp, bool leafIterator >
  inline void AlbertaGridTreeIterator< codim, GridImp, leafIterator >::makeIterator ()
  {
    level_ = 0;
    subEntity_ = -1;
    marker_ = 0;

    entityImp().clearElement();
  }


  template< int codim, class GridImp, bool leafIterator >
  inline AlbertaGridTreeIterator< codim, GridImp, leafIterator >
  ::AlbertaGridTreeIterator ( const GridImp &grid,
                              const MarkerVector *marker,
                              int travLevel )
    : Base( grid ),
      level_( travLevel ),
      subEntity_( (codim == 0 ? 0 : -1) ),
      macroIterator_( grid.meshPointer().begin() ),
      marker_( marker )
  {
    ElementInfo elementInfo = *macroIterator_;
    nextElementStop( elementInfo );
    if( codim > 0 )
      goNext( elementInfo );
    // it is ok to set the invalid ElementInfo
    entityImp().setElement( elementInfo, subEntity_ );
  }


  // Make LevelIterator with point to element from previous iterations
  template< int codim, class GridImp, bool leafIterator >
  inline AlbertaGridTreeIterator< codim, GridImp, leafIterator >
  ::AlbertaGridTreeIterator ( const GridImp &grid,
                              int travLevel )
    : Base( grid ),
      level_( travLevel ),
      subEntity_( -1 ),
      macroIterator_( grid.meshPointer().end() ),
      marker_( 0 )
  {}


  // Make LevelIterator with point to element from previous iterations
  template< int codim, class GridImp, bool leafIterator >
  inline AlbertaGridTreeIterator< codim, GridImp, leafIterator >
  ::AlbertaGridTreeIterator( const This &other )
    : Base( other ),
      level_( other.level_ ),
      subEntity_( other.subEntity_ ),
      macroIterator_( other.macroIterator_ ),
      marker_( other.marker_ )
  {}


  // Make LevelIterator with point to element from previous iterations
  template< int codim, class GridImp, bool leafIterator >
  inline typename AlbertaGridTreeIterator< codim, GridImp, leafIterator >::This &
  AlbertaGridTreeIterator< codim, GridImp, leafIterator >::operator= ( const This &other )
  {
    Base::operator=( other );

    level_ = other.level_;
    subEntity_ =  other.subEntity_;
    macroIterator_ = other.macroIterator_;
    marker_ = other.marker_;

    return *this;
  }


  template< int codim, class GridImp, bool leafIterator >
  inline void AlbertaGridTreeIterator< codim, GridImp, leafIterator >::increment ()
  {
    ElementInfo elementInfo = entityImp().elementInfo_;
    goNext ( elementInfo );
    // it is ok to set the invalid ElementInfo
    entityImp().setElement( elementInfo, subEntity_ );
  }


  template< int codim, class GridImp, bool leafIterator >
  inline void AlbertaGridTreeIterator< codim, GridImp, leafIterator >
  ::nextElement ( ElementInfo &elementInfo )
  {
    if( elementInfo.isLeaf() || (elementInfo.level() >= level_) )
    {
      while( (elementInfo.level() > 0) && (elementInfo.indexInFather() == 1) )
        elementInfo = elementInfo.father();
      if( elementInfo.level() == 0 )
      {
        ++macroIterator_;
        elementInfo = *macroIterator_;
      }
      else
        elementInfo = elementInfo.father().child( 1 );
    }
    else
      elementInfo = elementInfo.child( 0 );
  }


  template< int codim, class GridImp, bool leafIterator >
  inline void AlbertaGridTreeIterator< codim, GridImp, leafIterator >
  ::nextElementStop ( ElementInfo &elementInfo )
  {
    while( !(!elementInfo || stopAtElement( elementInfo )) )
      nextElement( elementInfo );
  }


  template< int codim, class GridImp, bool leafIterator >
  inline bool AlbertaGridTreeIterator< codim, GridImp, leafIterator >
  ::stopAtElement ( const ElementInfo &elementInfo ) const
  {
    if( !elementInfo )
      return true;
    return (leafIterator ? elementInfo.isLeaf() : (level_ == elementInfo.level()));
  }


  template< int codim, class GridImp, bool leafIterator >
  inline void AlbertaGridTreeIterator< codim, GridImp, leafIterator >
  ::goNext ( ElementInfo &elementInfo )
  {
    Int2Type< codim > codimVariable;
    goNext( codimVariable, elementInfo );
  }

  template< int codim, class GridImp, bool leafIterator >
  inline void AlbertaGridTreeIterator< codim, GridImp, leafIterator >
  ::goNext ( const Int2Type< 0 > cdVariable, ElementInfo &elementInfo )
  {
    assert( stopAtElement( elementInfo ) );

    nextElement( elementInfo );
    nextElementStop( elementInfo );
  }

  template< int codim, class GridImp, bool leafIterator >
  inline void AlbertaGridTreeIterator< codim, GridImp, leafIterator >
  ::goNext ( const Int2Type< 1 > cdVariable, ElementInfo &elementInfo )
  {
    assert( stopAtElement( elementInfo ) );

    ++subEntity_;
    if( subEntity_ >= numSubEntities )
    {
      subEntity_ = 0;
      nextElement( elementInfo );
      nextElementStop( elementInfo );
      if( !elementInfo )
        return;
    }

    if( leafIterator )
    {
      const ALBERTA EL *neighbor = elementInfo.elInfo().neigh[ subEntity_ ];
      if( neighbor != NULL )
      {
        // face is reached from element with largest number
        const int elIndex = grid().dofNumbering() ( elementInfo, 0, 0 );
        const int nbIndex = grid().dofNumbering() ( neighbor, 0, 0 );
        if( elIndex < nbIndex )
          goNext( cdVariable, elementInfo );
      }
      // uncomment this assertion only if codimension 1 entities are marked
      // assert( marker_->template subEntityOnElement< 1 >( elementInfo, subEntity_ ) );
    }
    else
    {
      assert( marker_ != 0 );
      if( !marker_->template subEntityOnElement< 1 >( elementInfo, subEntity_ ) )
        goNext( cdVariable, elementInfo );
    }
  }

  template< int codim, class GridImp, bool leafIterator >
  template< int cd >
  inline void AlbertaGridTreeIterator< codim, GridImp, leafIterator >
  ::goNext ( const Int2Type< cd > cdVariable, ElementInfo &elementInfo )
  {
    assert( stopAtElement( elementInfo ) );

    ++subEntity_;
    if( subEntity_ >= numSubEntities )
    {
      subEntity_ = 0;
      nextElement( elementInfo );
      nextElementStop( elementInfo );
      if( !elementInfo )
        return;
    }

    assert( marker_ != 0 );
    if( !marker_->template subEntityOnElement< cd >( elementInfo, subEntity_ ) )
      goNext( cdVariable, elementInfo );
  }

}

#endif
