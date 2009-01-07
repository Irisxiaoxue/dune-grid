// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALBERTA_ELEMENTINFO_HH
#define DUNE_ALBERTA_ELEMENTINFO_HH

#include <dune/grid/albertagrid/misc.hh>

#if HAVE_ALBERTA

namespace Dune
{

  namespace Alberta
  {

    // External Forward Declarations
    // -----------------------------

    template< int dim >
    class MeshPointer;



    // FillFlags
    // ---------

    template< int dim >
    struct FillFlags
    {
      typedef ALBERTA FLAGS Flags;

      static const Flags nothing = FILL_NOTHING;

      static const Flags coords = FILL_COORDS;

      static const Flags neighbor = FILL_NEIGH;

      static const Flags orientation = (dim == 3 ? FILL_ORIENTATION : FILL_NOTHING);

#if DUNE_ALBERTA_VERSION >= 0x201
      static const Flags elementType = FILL_NOTHING;
#else
      static const Flags elementType = (dim == 3 ? FILL_EL_TYPE : FILL_NOTHING);
#endif

#if DUNE_ALBERTA_VERSION >= 0x201
      static const Flags boundaryId = FILL_MACRO_WALLS;
#else
      static const Flags boundaryId = FILL_BOUND;
#endif

#if DUNE_ALBERTA_VERSION >= 0x201
      static const Flags nonPeriodic = FILL_NON_PERIODIC;
#else
      static const Flags nonPeriodic = FILL_NOTHING;
#endif

      static const Flags all = coords | neighbor | boundaryId | nonPeriodic
                               | orientation | elementType;

#if CALC_COORD
      static const Flags standard = all;
#else
      static const Flags standard = all & ~coords;
#endif
    };



#if DUNE_ALBERTA_VERSION < 0x200
    // BoundaryProvider
    // ----------------

    class BoundaryProvider
    {
      static const int interior = INTERIOR;

#if DIM > 1
      static const int numTypes = 256;
      static const int firstType = -127;

      Boundary boundaries[ numTypes ];
#endif

      BoundaryProvider ();

      // prohibit copying and assignment
      BoundaryProvider ( const BoundaryProvider & );
      BoundaryProvider &operator= ( const BoundaryProvider & );

    public:
      const Boundary *operator[] ( int type ) const;

      static const BoundaryProvider &instance ()
      {
        static BoundaryProvider provider;
        return provider;
      }

      static const Boundary *initBoundary ( Mesh *mesh, int type )
      {
        return instance()[ type ];
      }
    };


    inline BoundaryProvider::BoundaryProvider ()
    {
#if DIM > 1
      for( int i = 0; i < numTypes; ++i )
      {
        boundaries[ i ].param_bound = NULL;
        boundaries[ i ].bound = i+firstType;
      }
#endif
    }


    inline const Boundary *BoundaryProvider::operator[] ( int type ) const
    {
      int index = type-firstType;
      if( (type == interior) || (index < 0) || (index >= numTypes) )
        DUNE_THROW( AlbertaError, "Invalid boundary type: " << type << "." );
#if DIM > 1
      return &(boundaries[ index ]);
#else
      return NULL;
#endif
    }
#endif



    // ElementInfo
    // -----------

    template< int dim >
    class ElementInfo
    {
      class Instance;
      class Stack;

      typedef Instance *InstancePtr;

      InstancePtr instance_;

      explicit ElementInfo ( const InstancePtr &instance );

    public:
      static const int dimension = dim;

      static const int numVertices = NumSubEntities< dimension, dimension >::value;

      typedef Alberta::MeshPointer< dimension > MeshPointer;
      typedef Alberta::FillFlags< dimension > FillFlags;

#if DUNE_ALBERTA_VERSION >= 0x200
      static const int maxNeighbors = N_NEIGH_MAX;
#else
      static const int maxNeighbors = N_NEIGH;
#endif

      ElementInfo ();
      ElementInfo ( const MeshPointer &mesh, MacroElement &macroElement,
                    typename FillFlags::Flags fillFlags = FillFlags::standard );
      ElementInfo ( const ElementInfo &other );

      ~ElementInfo ();

      ElementInfo &operator= ( const ElementInfo &other );

      bool operator! () const;
      bool operator== ( const ElementInfo &other ) const;
      bool operator!= ( const ElementInfo &other ) const;

      ElementInfo father () const;
      int indexInFather () const;
      ElementInfo child ( int i ) const;
      bool isLeaf () const;

      bool mightVanish () const;

      int level () const;
      // see ALBERTA documentation for definition of element type
      // values are 0, 1, 2
      int type () const;

      int getMark () const;
      void setMark ( int refCount ) const;

      bool isBoundary ( int face ) const;
      int boundaryId ( int face ) const;

      bool hasCoordinates () const;
      const GlobalVector &coordinate ( int vertex ) const;

      template< class Functor >
      void hierarchicTraverse ( Functor &functor ) const;

      template< class Functor >
      void leafTraverse ( Functor &functor ) const;

      Element *el () const;
      ALBERTA EL_INFO &elInfo () const;

      static ElementInfo createFake ();

    private:
      static bool isLeaf ( Element *element );
      static bool mightVanish ( Element *element, int depth );

      void addReference () const;
      void removeReference () const;

      static InstancePtr null ();
      static Stack &stack ();
    };



    // ElementInfo::Instance
    // ---------------------

    template< int dim >
    struct ElementInfo< dim >::Instance
    {
      ALBERTA EL_INFO elInfo;
      unsigned int refCount;

      InstancePtr &parent ()
      {
        return parent_;
      }

    private:
      InstancePtr parent_;
    };



    // ElementInfo::Stack
    // ------------------

    template< int dim >
    class ElementInfo< dim >::Stack
    {
      InstancePtr top_;
      Instance null_;

    public:
      Stack ();
      ~Stack ();

      InstancePtr allocate ();
      void release ( InstancePtr &p );
      InstancePtr null ();
    };



    // Implementation of ElementInfo
    // -----------------------------

    template< int dim >
    inline ElementInfo< dim >::ElementInfo ( const InstancePtr &instance )
      : instance_( instance )
    {
      addReference();
    }


    template< int dim >
    inline ElementInfo< dim >::ElementInfo ()
      : instance_( null() )
    {
      addReference();
    }


    template< int dim >
    inline ElementInfo< dim >
    ::ElementInfo ( const MeshPointer &mesh, MacroElement &macroElement,
                    typename FillFlags::Flags fillFlags )
    {
      instance_ = stack().allocate();
      instance_->parent() = null();
      ++(instance_->parent()->refCount);

      addReference();

      elInfo().fill_flag = fillFlags;

      // Alberta fills opp_vertex only if there is a neighbor
      for( int k = 0; k < maxNeighbors; ++k )
        elInfo().opp_vertex[ k ] = -1;

      fill_macro_info( mesh, &macroElement, &elInfo() );
    }


    template< int dim >
    inline ElementInfo< dim >::ElementInfo ( const ElementInfo &other )
      : instance_( other.instance_ )
    {
      addReference();
    }


    template< int dim >
    inline ElementInfo< dim >::~ElementInfo ()
    {
      removeReference();
    }


    template< int dim >
    inline ElementInfo< dim > &
    ElementInfo< dim >::operator= ( const ElementInfo< dim > &other )
    {
      other.addReference();
      removeReference();
      instance_ = other.instance_;
      return *this;
    }


    template< int dim >
    inline bool ElementInfo< dim >::operator! () const
    {
      return (instance_ == null());
    }


    template< int dim >
    inline bool
    ElementInfo< dim >::operator== ( const ElementInfo< dim > &other ) const
    {
      return (instance_->elInfo.el == other.instance_->elInfo.el);
    }


    template< int dim >
    inline bool
    ElementInfo< dim >::operator!= ( const ElementInfo< dim > &other ) const
    {
      return (instance_->elInfo.el != other.instance_->elInfo.el);
    }


    template< int dim >
    inline ElementInfo< dim > ElementInfo< dim >::father () const
    {
      assert( !(*this) == false );
      return ElementInfo< dim >( instance_->parent() );
    }


    template< int dim >
    inline int ElementInfo< dim >::indexInFather () const
    {
      const Element *element = elInfo().el;
#if DUNE_ALBERTA_VERSION >= 0x201
      const Element *father = elInfo().parent->el;
#else
      const Element *father = elInfo().parent;
#endif
      assert( father != NULL );

      const int index = (father->child[ 0 ] == element ? 0 : 1);
      assert( father->child[ index ] == element );
      return index;
    }


    template< int dim >
    inline ElementInfo< dim > ElementInfo< dim >::child ( int i ) const
    {
      assert( !isLeaf() );

      InstancePtr child = stack().allocate();
      child->parent() = instance_;
      addReference();

      // Alberta fills opp_vertex only if there is a neighbor
      for( int k = 0; k < maxNeighbors; ++k )
        child->elInfo.opp_vertex[ k ] = -2;

#if DUNE_ALBERTA_VERSION >= 0x201
      ALBERTA fill_elinfo( i, FILL_ANY, &elInfo(), &(child->elInfo) );
#else
      ALBERTA fill_elinfo( i, &elInfo(), &(child->elInfo) );
#endif

      return ElementInfo< dim >( child );
    }


    template< int dim >
    inline bool ElementInfo< dim >::isLeaf () const
    {
      assert( !(*this) == false );
      return isLeaf( el() );
    }


    template< int dim >
    inline bool ElementInfo< dim >::mightVanish () const
    {
      return mightVanish( el(), 0 );
    }


    template< int dim >
    inline int ElementInfo< dim >::level () const
    {
      return instance_->elInfo.level;
    }


    template< int dim >
    inline int ElementInfo< dim >::type () const
    {
      return 0;
    }


#if (DUNE_ABLERTA_VERSION >= 0x200) || (DIM == 3)
    template<>
    inline int ElementInfo< 3 >::type () const
    {
      return instance_->elInfo.el_type;
    }
#endif


    template< int dim >
    inline int ElementInfo< dim >::getMark () const
    {
      return el()->mark;
    }


    template< int dim >
    inline void ElementInfo< dim >::setMark ( int refCount ) const
    {
      assert( isLeaf() );
      assert( (refCount >= -128) && (refCount < 127) );
      el()->mark = refCount;
    }


    template< int dim >
    inline bool ElementInfo< dim >::isBoundary ( int face ) const
    {
      assert( !(*this) == false );
      assert( (face >= 0) && (face < maxNeighbors) );
      return (elInfo().neigh[ face ] == 0);
    }


#if DUNE_ALBERTA_VERSION >= 0x201
    template< int dim >
    inline int ElementInfo< dim >::boundaryId ( int face ) const
    {
      assert( !(*this) == false );
      assert( (face >= 0) && (face < N_WALLS_MAX) );

      const int macroFace = elInfo().macro_wall[ face ];
      const int id = elInfo().macro_el->wall_bound[ macroFace ];
      // this assertion is only allowed, if FILL_BOUND is set
      // assert( id == elInfo().wall_bound[ face ] );
      return id;
    }
#endif // #if DUNE_ALBERTA_VERSION >= 0x201


#if DUNE_ALBERTA_VERSION == 0x200
    template<>
    inline int ElementInfo< 1 >::boundaryId ( int face ) const
    {
      assert( !(*this) == false );
      assert( (face >= 0) && (face < N_VERTICES_MAX) );
      return elInfo().vertex_bound[ face ];
    }

    template<>
    inline int ElementInfo< 2 >::boundaryId ( int face ) const
    {
      assert( !(*this) == false );
      assert( (face >= 0) && (face < N_EDGES_MAX) );
      return elInfo().edge_bound[ face ];
    }

    template<>
    inline int ElementInfo< 3 >::boundaryId ( int face ) const
    {
      assert( !(*this) == false );
      assert( (face >= 0) && (face < N_FACES_MAX) );
      return elInfo().face_bound[ face ];
    }
#endif // #if DUNE_ALBERTA_VERSION == 0x200


#if DUNE_ALBERTA_VERSION < 0x200
#if DIM == 1
    template<>
    inline int ElementInfo< 1 >::boundaryId ( int face ) const
    {
      assert( !(*this) == false );
      assert( (face >= 0) && (face < N_VERTICES) );
      return elInfo().bound[ face ];
    }
#endif // #if DIM == 1

#if DIM == 2
    template<>
    inline int ElementInfo< 2 >::boundaryId ( int face ) const
    {
      assert( !(*this) == false );
      assert( (face >= 0) && (face < N_EDGES) );
      return elInfo().boundary[ face ]->bound;
    }
#endif // #if DIM == 2

#if DIM == 3
    template<>
    inline int ElementInfo< 3 >::boundaryId ( int face ) const
    {
      assert( !(*this) == false );
      assert( (face >= 0) && (face < N_FACES) );
      return elInfo().boundary[ face ]->bound;
    }
#endif // #if DIM == 3
#endif // #if DUNE_ALBERTA_VERSION < 0x200


    template< int dim >
    inline bool ElementInfo< dim >::hasCoordinates () const
    {
      return ((elInfo().fill_flag & FILL_COORDS) != 0);
    }

    template< int dim >
    inline const GlobalVector &ElementInfo< dim >::coordinate ( int vertex ) const
    {
      assert( hasCoordinates() );
      assert( (vertex >= 0) && (vertex < numVertices) );
      return elInfo().coord[ vertex ];
    }


    template< int dim >
    template< class Functor >
    void ElementInfo< dim >::hierarchicTraverse ( Functor &functor ) const
    {
      functor( *this );
      if( !isLeaf() )
      {
        child( 0 ).hierarchicTraverse( functor );
        child( 1 ).hierarchicTraverse( functor );
      }
    }


    template< int dim >
    template< class Functor >
    void ElementInfo< dim >::leafTraverse ( Functor &functor ) const
    {
      if( !isLeaf() )
      {
        child( 0 ).leafTraverse( functor );
        child( 1 ).leafTraverse( functor );
      }
      else
        functor( *this );
    }


    template< int dim >
    inline Element *ElementInfo< dim >::el () const
    {
      return elInfo().el;
    }


    template< int dim >
    inline ALBERTA EL_INFO &ElementInfo< dim >::elInfo () const
    {
      return (instance_->elInfo);
    }


    template< int dim >
    inline ElementInfo< dim > ElementInfo< dim >::createFake ()
    {
      InstancePtr instance = stack().allocate();
      instance->parent() = null();
      ++(instance->parent()->refCount);
      return ElementInfo< dim >( instance );
    }


    template< int dim >
    inline bool ElementInfo< dim >::isLeaf ( Element *element )
    {
      return IS_LEAF_EL( element );
    }


    template< int dim >
    inline bool ElementInfo< dim >::mightVanish ( Alberta::Element *element, int depth )
    {
      if( isLeaf( element ) )
        return (element->mark < depth);
      else
        return (mightVanish( element->child[ 0 ], depth-1 ) && mightVanish( element->child[ 1 ], depth-1 ));
    }


    template< int dim >
    inline void ElementInfo< dim >::addReference () const
    {
      ++(instance_->refCount);
    }


    template< int dim >
    inline void ElementInfo< dim >::removeReference () const
    {
      // this loop breaks when instance becomes null()
      for( InstancePtr instance = instance_; --(instance->refCount) == 0; )
      {
        const InstancePtr parent = instance->parent();
        stack().release( instance );
        instance = parent;
      }
    }


    template< int dim >
    inline typename ElementInfo< dim >::InstancePtr
    ElementInfo< dim >::null ()
    {
      return stack().null();
    }


    template< int dim >
    inline typename ElementInfo< dim >::Stack &
    ElementInfo< dim >::stack ()
    {
      static Stack s;
      return s;
    }



    // Implementation of ElementInfo::Stack
    // ------------------------------------

    template< int dim >
    inline ElementInfo< dim >::Stack::Stack ()
      : top_( 0 )
    {
      null_.elInfo.el = NULL;
      null_.refCount = 1;
      null_.parent() = 0;
    }


    template< int dim >
    inline ElementInfo< dim >::Stack::~Stack ()
    {
      while( top_ != 0 )
      {
        InstancePtr p = top_;
        top_ = p->parent();
        delete p;
      }
    }


    template< int dim >
    inline typename ElementInfo< dim >::InstancePtr
    ElementInfo< dim >::Stack::allocate ()
    {
      InstancePtr p = top_;
      if( p != 0 )
        top_ = p->parent();
      else
        p = new Instance;
      p->refCount = 0;
      return p;
    }


    template< int dim >
    inline void ElementInfo< dim >::Stack::release ( InstancePtr &p )
    {
      assert( (p != null()) && (p->refCount == 0) );
      p->parent() = top_;
      top_ = p;
    }


    template< int dim >
    inline typename ElementInfo< dim >::InstancePtr
    ElementInfo< dim >::Stack::null ()
    {
      return &null_;
    }

  }

}

#endif // #if HAVE_ALBERTA

#endif
