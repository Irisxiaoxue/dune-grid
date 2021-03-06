// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_HOSTGRIDACCESS_HH
#define DUNE_GRID_HOSTGRIDACCESS_HH

#include <string>

#include <dune/grid/geometrygrid/intersection.hh>

namespace Dune
{

  // External Forward Declarations
  // -----------------------------

  template< class HostGrid, class CoordFunction, class Allocator >
  class GeometryGrid;



  // HostGridAccess
  // --------------

  template< class Grid >
  struct HostGridAccess;



  /** \class HostGridAccess
   *  \brief provides access to host grid objects
   *
   *  \tparam GeometryGrid
   *
   *  \nosubgrouping
   */
  template< class HG, class CoordFunction, class Allocator >
  struct HostGridAccess< GeometryGrid< HG, CoordFunction, Allocator > >
  {
    /** \name Exported Types
     * \{ */

    typedef GeometryGrid< HG, CoordFunction, Allocator > Grid;

    //! type of HostGrid
    typedef typename Grid::HostGrid HostGrid;

    /** \} */

    /** \brief A Traits struct that collects return types of class member methods.
     *
     *  \tparam codim codimension
     */
    template< int codim >
    struct Codim
    {
      //! type of the GeometryGrid entity
      typedef typename Grid::template Codim< codim >::Entity Entity;
      //! type of the GeometryGrid entity pointer
      typedef typename Grid::template Codim< codim >::EntityPointer EntityPointer;

      //! type of the host entity
      typedef typename HostGrid::template Codim< codim >::Entity HostEntity;
      //! type of the host entity pointer
      typedef typename HostGrid::template Codim< codim >::EntityPointer HostEntityPointer;
    };

    //! type of the GeometryGrid leaf intersection
    typedef typename Grid::Traits::LeafIntersection LeafIntersection;
    //! type of the GeometryGrid level intersection
    typedef typename Grid::Traits::LevelIntersection LevelIntersection;

    //! type of the host leaf intersection
    typedef typename HostGrid::Traits::LeafIntersection HostLeafIntersection;
    //! type of the host level intersection
    typedef typename HostGrid::Traits::LevelIntersection HostLevelIntersection;

    /** \brief Get underlying HostGrid.
     *  \param[in] grid  GeometryGrid
     *  \returns HostGrid
     */
    static const HostGrid &hostGrid ( const Grid &grid )
    {
      return grid.hostGrid();
    }

    template< class Entity >
    static const typename Codim< Entity::codimension >::HostEntity &
    hostEntity ( const Entity &entity )
    {
      return hostEntity< Entity::codimension >( entity );
    }

    template< int codim >
    static const typename Codim< codim >::HostEntity &
    hostEntity ( const typename Codim< codim >::Entity &entity )
    {
      return Grid::getRealImplementation( entity ).hostEntity();
    }

    template< class HostIntersection >
    static const HostIntersection &
    hostIntersection ( const Intersection< const Grid, GeoGrid::Intersection< const Grid, HostIntersection > > &intersection )
    {
      return Grid::getRealImplementation( intersection ).hostIntersection();
    }
  };

} // namespace Dune

#endif // #ifndef DUNE_GRID_HOSTGRIDACCESS_HH
