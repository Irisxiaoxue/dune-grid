// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALBERTA_INTERSECTIONITERATOR_HH
#define DUNE_ALBERTA_INTERSECTIONITERATOR_HH

#include <dune/grid/common/intersectioniterator.hh>

#include <dune/grid/albertagrid/intersection.hh>

/** \file
 *  \author Martin Nolte
 *  \brief  Implementation of the IntersectionIterator for AlbertaGrid
 */

namespace Dune
{

  // AlbertaGridLeafIntersectionIterator
  // -----------------------------------

  template< class GridImp >
  class AlbertaGridLeafIntersectionIterator
  {
    typedef AlbertaGridLeafIntersectionIterator< GridImp > This;

  public:
    typedef Dune::Intersection< GridImp, AlbertaGridIntersection > Intersection;

  private:
    typedef AlbertaGridIntersection< GridImp > IntersectionImp;

    Intersection *intersection_;

  public:
    template< class EntityImp >
    AlbertaGridLeafIntersectionIterator
      ( const GridImp &grid, const EntityImp &entity, int wLevel, bool end )
      : intersection_( new Intersection( IntersectionImp( grid, wLevel ) ) )
    {
      if( end )
        intersectionImp().done();
      else
        intersectionImp().first( entity, wLevel );
    }

    AlbertaGridLeafIntersectionIterator ( const This &other )
      : intersection_( new Intersection( other.intersectionImp() ) )
    {}

    This &operator= ( const This &other )
    {
      intersectionImp() = other.intersectionImp();
      return *this;
    }

    ~AlbertaGridLeafIntersectionIterator ()
    {
      delete intersection_;
    }

    const Intersection &dereference () const
    {
      return *intersection_;
    }

    bool equals ( const This &other ) const
    {
      return intersectionImp().equals( other.intersectionImp() );
    }

    void increment ()
    {
      intersectionImp().increment();
    }

  private:
    const IntersectionImp &intersectionImp () const
    {
      return GridImp::getRealImplementation( *intersection_ );
    }

    IntersectionImp &intersectionImp ()
    {
      return GridImp::getRealImplementation( *intersection_ );
    }
  };

}

#endif // #ifndef DUNE_ALBERTA_INTERSECTIONITERATOR_HH