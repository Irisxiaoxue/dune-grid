// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_AGMEMORY_HH
#define DUNE_AGMEMORY_HH

#if HAVE_ALBERTA

#include <stack>

#include <dune/grid/albertagrid/albertaheader.hh>

namespace Dune
{

  //! organize the memory management for entitys used by the NeighborIterator
  template <class Object>
  class AGMemoryProvider
  {
    std::stack < Object * > objStack_;

    typedef AGMemoryProvider < Object > MyType;
  public:
    typedef Object ObjectType;

    //! delete all objects stored in stack
    AGMemoryProvider() {};

    //! call deleteEntity
    ~AGMemoryProvider ();

    //! create object with empty constructor
    /*
       Object * getObject()
       {
       if( objStack_.empty() )
       {
        return ( new Object () );
       }
       else
       {
        ObjectType * obj = objStack_.top();
        objStack_.pop();
        return obj;
       }
       }
     */

    //! i.e. return pointer to Entity
    template <class GridType, class ObjectImp>
    ObjectType * getNewObjectEntity(const GridType &grid,
                                    const ObjectImp * fakePointer );

    //! i.e. return pointer to Entity
    template <class GridType>
    ObjectType * getObject(const GridType &grid, int level);

    //! i.e. return pointer to Entity with calling copy constructor
    ObjectType * getObjectCopy(const ObjectType & org);

    //! free, move element to stack, returns NULL
    void freeObjectEntity (ObjectType * obj);

    //! free, move element to stack, returns NULL
    void freeObject (ObjectType * obj) { freeObjectEntity(obj); }

  private:
    //! do not copy pointers
    AGMemoryProvider(const AGMemoryProvider<Object> & org);
  };


  //************************************************************************
  //
  //  AGMemoryProvider implementation
  //
  //************************************************************************
  template <class Object> template <class GridType, class ObjectImp>
  inline typename AGMemoryProvider<Object>::ObjectType *
  AGMemoryProvider< Object >
  ::getNewObjectEntity ( const GridType &grid, const ObjectImp *fakePointer )
  {
    if( objStack_.empty() )
    {
      return new Object( ObjectImp( grid ) );
    }
    else
    {
      ObjectType * obj = objStack_.top();
      objStack_.pop();
      return obj;
    }
  }

  template <class Object> template <class GridType>
  inline typename AGMemoryProvider<Object>::ObjectType *
  AGMemoryProvider<Object>::getObject
    (const GridType &grid, int level )
  {
    if( objStack_.empty() )
    {
      return ( new Object (grid,level) );
    }
    else
    {
      ObjectType * obj = objStack_.top();
      objStack_.pop();
      return obj;
    }
  }

  template <class Object>
  inline typename AGMemoryProvider<Object>::ObjectType *
  AGMemoryProvider<Object>::getObjectCopy
    (const ObjectType & org)
  {
    if( objStack_.empty() )
    {
      return ( new Object (org));
    }
    else
    {
      ObjectType * obj = objStack_.top();
      objStack_.pop();
      return obj;
    }
  }

  template <class Object>
  inline AGMemoryProvider<Object>::~AGMemoryProvider()
  {
    while ( !objStack_.empty() )
    {
      ObjectType * obj = objStack_.top();
      objStack_.pop();
      if( obj ) delete obj;
    }
  }

  template <class Object>
  inline void AGMemoryProvider<Object>::freeObjectEntity(Object * obj)
  {
    objStack_.push( obj );
  }

} //end namespace

#endif // HAVE_ALBERTA

#endif