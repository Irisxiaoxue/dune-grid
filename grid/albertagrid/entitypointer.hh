// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALBERTA_ENTITYPOINTER_HH
#define DUNE_ALBERTA_ENTITYPOINTER_HH

#include <dune/grid/common/entitypointer.hh>

#include <dune/grid/albertagrid/elementinfo.hh>

namespace Dune
{

  /** \class   AlbertaGridEntityPointer
   *  \ingroup AlbertaGrid
   *  \brief   EntityPointer implementation for AlbertaGrid
   */
  template< int codim, class GridImp >
  class AlbertaGridEntityPointer
  {
    typedef AlbertaGridEntityPointer< codim, GridImp > This;

    friend class AlbertaGrid< GridImp::dimension, GridImp::dimensionworld >;

  public:
    static const int dimension = GridImp::dimension;
    static const int codimension = codim;
    static const int mydimension = dimension - codimension;
    static const int dimensionworld = GridImp::dimensionworld;

    typedef typename GridImp::template Codim< codimension >::Entity Entity;

  protected:
    typedef MakeableInterfaceObject< Entity > EntityObject;
    typedef typename EntityObject::ImplementationType EntityImp;

  public:
    typedef AlbertaGridEntityPointer< codimension, GridImp > EntityPointerImp;

    typedef typename EntityImp::ElementInfo ElementInfo;

    //! make an EntityPointer that points to an element
    AlbertaGridEntityPointer ( const GridImp &grid,
                               const ElementInfo &elementInfo,
                               int subEntity );

    //! constructor for invalid EntityPointer
    AlbertaGridEntityPointer ( const GridImp &grid );

    //! make entity pointer from entity
    AlbertaGridEntityPointer ( const EntityImp &entity );

    //! copy constructor
    AlbertaGridEntityPointer ( const This &other );

    //! Destructor
    ~AlbertaGridEntityPointer();

    //! assignment operator
    This &operator= ( const This &other );

    //! equality
    bool equals ( const This &other ) const;

    //! dereferencing
    Entity &dereference () const;

    //! ask for level of entities
    int level () const;

    //! reduce memory
    void compactify ();

  protected:
    //! obtain reference to internal entity implementation
    EntityImp &entityImp ();

    //! obtain const reference to internal entity implementation
    const EntityImp &entityImp () const;

    //! obtain a reference to the grid
    const GridImp &grid () const;

  private:
    EntityObject *entity_;
  };

}

#endif
