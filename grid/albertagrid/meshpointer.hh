// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALBERTA_MESHPOINTER_HH
#define DUNE_ALBERTA_MESHPOINTER_HH

/** \file
 *  \author Martin Nolte
 *  \brief  provides a wrapper for ALBERTA's mesh structure
 */

#include <cassert>
#include <string>

#include <dune/grid/albertagrid/misc.hh>
#include <dune/grid/albertagrid/elementinfo.hh>
#include <dune/grid/albertagrid/albertaextra.hh>
#include <dune/grid/albertagrid/macrodata.hh>

#if HAVE_ALBERTA

namespace Dune
{

  namespace Alberta
  {

    // External Forward Declarations
    // -----------------------------

    template< int dim >
    class HierarchyDofNumbering;



    // MeshPointer
    // -----------

    template< int dim >
    class MeshPointer
    {
      Mesh *mesh_;

      typedef Alberta::ElementInfo< dim > ElementInfo;
      typedef typename ElementInfo::FillFlags FillFlags;

      class BoundaryProvider;
      class MacroIteratorBase;

    public:
      class MacroIterator;

      MeshPointer ()
        : mesh_( NULL )
      {}

      explicit MeshPointer ( Mesh *mesh )
        : mesh_( mesh )
      {}

      operator Mesh * () const
      {
        return mesh_;
      }

      bool operator! () const
      {
        return (mesh_ == NULL);
      }

      MacroIterator begin () const
      {
        return MacroIterator( *this, false );
      }

      MacroIterator end () const
      {
        return MacroIterator( *this, true );
      }

      std::string name () const
      {
        if( mesh_ != NULL )
          return mesh_->name;
        else
          return std::string();
      }

      int size ( int codim ) const;

      void create ( const MacroData< dim > &macroData, const std::string &name );
      void create ( const std::string &filename, const std::string &name, bool binary = false );

      void read ( const std::string &filename, Real &time );

      bool write ( const std::string &filename, Real time ) const;

      void release ()
      {
        if( mesh_ != NULL )
        {
#if (DUNE_ALBERTA_VERSION < 0x200) && (DIM == 3)
          // circumvent memory bug in ALBERTA 1.2
          ALBERTA RC_LIST_EL *list = ALBERTA get_rc_list( mesh_ );
          list = 0;
#endif
          ALBERTA free_mesh( mesh_ );
          mesh_ = NULL;
        }
      }

      template< class Functor >
      void hierarchicTraverse ( Functor &functor,
                                typename FillFlags::Flags fillFlags = FillFlags::standard ) const;

      template< class Functor >
      void leafTraverse ( Functor &functor,
                          typename FillFlags::Flags fillFlags = FillFlags::standard ) const;

      bool coarsen ( typename FillFlags::Flags fillFlags = FillFlags::nothing );

      bool refine ( typename FillFlags::Flags fillFlags = FillFlags::nothing );

#if DUNE_ALBERTA_VERSION < 0x200
    private:
      static void initDofAdmins ( Mesh *mesh )
      {
        mesh->preserve_coarse_dofs = 1;
        HierarchyDofNumbering< dim > dofNumbering;
        dofNumbering.create( MeshPointer< dim >( mesh ) );
      }
#endif
    };



    template<>
    inline int MeshPointer< 1 >::size( int codim ) const
    {
      assert( (codim >= 0) && (codim <= 1) );
      return (codim == 0 ? mesh_->n_elements : mesh_->n_vertices);
    }

#if (DUNE_ALBERTA_VERSION >= 0x200) || (DIM >= 2)
    template<>
    inline int MeshPointer< 2 >::size( int codim ) const
    {
      assert( (codim >= 0) && (codim <= 2) );
      if( codim == 0 )
        return mesh_->n_elements;
      else if( codim == 2 )
        return mesh_->n_vertices;
      else
        return mesh_->n_edges;
    }
#endif // #if (DUNE_ALBERTA_VERSION >= 0x200) || (ALBERTA_DIM >= 2)

#if (DUNE_ALBERTA_VERSION >= 0x200) || (DIM == 3)
    template<>
    inline int MeshPointer< 3 >::size( int codim ) const
    {
      assert( (codim >= 0) && (codim <= 3) );
      if( codim == 0 )
        return mesh_->n_elements;
      else if( codim == 3 )
        return mesh_->n_vertices;
      else if( codim == 1 )
        return mesh_->n_faces;
      else
        return mesh_->n_edges;
    }
#endif // #if (DUNE_ALBERTA_VERSION >= 0x200) || (ALBERTA_DIM == 3)


#if DUNE_ALBERTA_VERSION >= 0x200
    template< int dim >
    inline void MeshPointer< dim >
    ::create ( const MacroData< dim > &macroData, const std::string &name )
    {
      release();

#if DUNE_ALBERTA_VERSION >= 0x201
      mesh_ = GET_MESH( dim, name.c_str(), macroData, NULL, NULL );
#else
      mesh_ = GET_MESH( dim, name.c_str(), macroData, NULL );
#endif

      if( mesh_ != NULL )
      {
        typedef AlbertHelp::AlbertLeafData< dim, dim+1 > LeafData;
        ALBERTA init_leaf_data( mesh_, sizeof( typename LeafData::Data ),
                                LeafData::AlbertLeafRefine,
                                LeafData::AlbertLeafCoarsen );
      }
    }
#endif // #if DUNE_ALBERTA_VERSION >= 0x200

#if DUNE_ALBERTA_VERSION < 0x200
    template< int dim >
    inline void MeshPointer< dim >
    ::create ( const MacroData< dim > &macroData, const std::string &name )
    {
      release();

      typedef AlbertHelp::AlbertLeafData< dim, dim+1 > LeafData;
      mesh_ = ALBERTA get_mesh( name.c_str(), initDofAdmins, LeafData::initLeafData );
      if( mesh_ != NULL )
        ALBERTA macro_data2mesh( mesh_, macroData, BoundaryProvider::initBoundary );
    }
#endif // #if DUNE_ALBERTA_VERSION < 0x200


#if DUNE_ALBERTA_VERSION >= 0x200
    template< int dim >
    inline void MeshPointer< dim >
    ::create ( const std::string &filename, const std::string &name, bool binary )
    {
      MacroData< dim > macroData;
      macroData.read( filename, binary );
      create( macroData, name );
      macroData.release();
    }
#endif // #if DUNE_ALBERTA_VERSION >= 0x200

#if DUNE_ALBERTA_VERSION < 0x200
    template< int dim >
    inline void MeshPointer< dim >
    ::create ( const std::string &filename, const std::string &name, bool binary )
    {
      release();

      typedef AlbertHelp::AlbertLeafData< dim, dim+1 > LeafData;
      mesh_ = ALBERTA get_mesh( name.c_str(), initDofAdmins, LeafData::initLeafData );
      if( mesh_ != NULL )
      {
        if( binary )
          ALBERTA read_macro_xdr( mesh_, filename.c_str(), BoundaryProvider::initBoundary );
        else
          ALBERTA read_macro( mesh_, filename.c_str(), BoundaryProvider::initBoundary );
      }
    }
#endif // #if DUNE_ALBERTA_VERSION < 0x200



#if DUNE_ALBERTA_VERSION < 0x200
    template< int dim >
    inline void MeshPointer< dim >::read ( const std::string &filename, Real &time )
    {
      release();

      typedef AlbertHelp::AlbertLeafData< dim, dim+1 > LeafData;
      mesh_ = ALBERTA read_mesh_xdr( filename.c_str(), &time, LeafData::initLeafData, BoundaryProvider::initBoundary );
    }
#endif // #if DUNE_ALBERTA_VERSION < 0x200

#if DUNE_ALBERTA_VERSION >= 0x200
    template< int dim >
    inline void MeshPointer< dim >::read ( const std::string &filename, Real &time )
    {
      release();
#if DUNE_ALBERTA_VERSION >= 0x201
      mesh_ = ALBERTA read_mesh_xdr( filename.c_str(), &time, NULL, NULL );
#else
      mesh_ = ALBERTA read_mesh_xdr( filename.c_str(), &time, NULL );
#endif

      if( mesh_ != NULL )
      {
        typedef AlbertHelp::AlbertLeafData< dim, dim+1 > LeafData;
        ALBERTA init_leaf_data( mesh_, sizeof( typename LeafData::Data ),
                                LeafData::AlbertLeafRefine,
                                LeafData::AlbertLeafCoarsen );
      }
    }
#endif // #if DUNE_ALBERTA_VERSION >= 0x200


    template< int dim >
    inline bool MeshPointer< dim >::write ( const std::string &filename, Real time ) const
    {
      int success = ALBERTA write_mesh_xdr( mesh_, filename.c_str(), time );
      return (success == 0);
    }


    template< int dim >
    template< class Functor >
    inline void MeshPointer< dim >
    ::hierarchicTraverse ( Functor &functor,
                           typename FillFlags::Flags fillFlags ) const
    {
      const MacroIterator eit = end();
      for( MacroIterator it = begin(); it != eit; ++it )
      {
        const ElementInfo info = it.elementInfo( fillFlags );
        info.hierarchicTraverse( functor );
      }
    }


    template< int dim >
    template< class Functor >
    inline void MeshPointer< dim >
    ::leafTraverse ( Functor &functor,
                     typename FillFlags::Flags fillFlags ) const
    {
      const MacroIterator eit = end();
      for( MacroIterator it = begin(); it != eit; ++it )
      {
        const ElementInfo info = it.elementInfo( fillFlags );
        info.leafTraverse( functor );
      }
    }


#if DUNE_ALBERTA_VERSION >= 0x201
    template< int dim >
    inline bool MeshPointer< dim >::coarsen ( typename FillFlags::Flags fillFlags )
    {
      const bool coarsened = (ALBERTA coarsen( mesh_, fillFlags ) == meshCoarsened);
      if( coarsened )
        ALBERTA dof_compress( mesh_ );
      return coarsened;
    }
#endif // #if DUNE_ALBERTA_VERSION >= 0x201

#if DUNE_ALBERTA_VERSION <= 0x200
    template< int dim >
    inline bool MeshPointer< dim >::coarsen ( typename FillFlags::Flags fillFlags )
    {
      assert( fillFlags == FillFlags::nothing );
      const bool coarsened = (ALBERTA coarsen( mesh_ ) == meshCoarsened);
      if( coarsened )
        ALBERTA dof_compress( mesh_ );
      return coarsened;
    }
#endif // #if DUNE_ALBERTA_VERSION <= 0x200


#if DUNE_ALBERTA_VERSION >= 0x201
    template< int dim >
    inline bool MeshPointer< dim >::refine ( typename FillFlags::Flags fillFlags )
    {
      return (ALBERTA refine( mesh_, fillFlags ) == meshRefined);
    }
#endif // #if DUNE_ALBERTA_VERSION >= 0x201

#if DUNE_ALBERTA_VERSION <= 0x200
    template< int dim >
    inline bool MeshPointer< dim >::refine ( typename FillFlags::Flags fillFlags )
    {
      assert( fillFlags == FillFlags::nothing );
      return (ALBERTA refine( mesh_ ) == meshRefined);
    }
#endif // #if DUNE_ALBERTA_VERSION <= 0x200



    // MeshPointer::BoundaryProvider
    // -----------------------------

#if DUNE_ALBERTA_VERSION < 0x200
    template< int dim >
    class MeshPointer< dim >::BoundaryProvider
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


    template< int dim >
    inline MeshPointer< dim >::BoundaryProvider::BoundaryProvider ()
    {
#if DIM > 1
      for( int i = 0; i < numTypes; ++i )
      {
        boundaries[ i ].param_bound = NULL;
        boundaries[ i ].bound = i+firstType;
      }
#endif
    }


    template< int dim >
    inline const Boundary *
    MeshPointer< dim >::BoundaryProvider::operator[] ( int type ) const
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
#endif // #if DUNE_ALBERTA_VERSION < 0x200



    // MeshPointer::MacroIteratorBase
    // ------------------------------

#if DUNE_ALBERTA_VERSION >= 0x200
    template< int dim >
    class MeshPointer< dim >::MacroIteratorBase
    {
      friend class MacroIterator;

    public:
      typedef Alberta::MeshPointer< dim > MeshPointer;
      typedef Alberta::ElementInfo< dim > ElementInfo;

    private:
      MeshPointer mesh_;
      int index_;

      explicit MacroIteratorBase ( const MeshPointer &mesh, bool end = false )
        : mesh_( mesh ),
          index_( end ? numMacroElements() : 0 )
      {}

    public:
      bool done () const
      {
        return (index_ >= numMacroElements());
      }

      bool equals ( const MacroIterator &other ) const
      {
        return (index_ == other.index_);
      }

      void increment ()
      {
        assert( !done() );
        ++index_;
      }

      MacroElement &macroElement () const
      {
        assert( !done() );
        return mesh().mesh_->macro_els[ index_ ];
      }

      const MeshPointer &mesh () const
      {
        return mesh_;
      }

    private:
      int numMacroElements () const
      {
        return mesh().mesh_->n_macro_el;
      }
    };
#endif // #if DUNE_ALBERTA_VERSION >= 0x200

#if DUNE_ALBERTA_VERSION < 0x200
    template< int dim >
    class MeshPointer< dim >::MacroIteratorBase
    {
      friend class MeshPointer< dim >;

    public:
      typedef Alberta::MeshPointer< dim > MeshPointer;
      typedef Alberta::ElementInfo< dim > ElementInfo;

      typedef typename ElementInfo::FillFlags FillFlags;

    private:
      MeshPointer mesh_;
      MacroElement *element_;

      explicit MacroIteratorBase ( const MeshPointer &mesh, bool end = false )
        : mesh_( mesh ),
          element_( end ? NULL : mesh.mesh_->first_macro_el )
      {}

    public:
      bool done () const
      {
        return (element_ == NULL);
      }

      bool equals ( const MacroIterator &other ) const
      {
        return (element_ == other.element_);
      }

      void increment ()
      {
        assert( !done() );
        element_ = element_->next;
      }

      MacroElement &macroElement () const
      {
        assert( !done() );
        return *element_;
      }

      const MeshPointer &mesh () const
      {
        return mesh_;
      }
    };
#endif // #if DUNE_ALBERTA_VERSION < 0x200



    // MeshPointer::MacroIterator
    // --------------------------

    template< int dim >
    class MeshPointer< dim >::MacroIterator
      : public MeshPointer< dim >::MacroIteratorBase
    {
      typedef MacroIterator This;
      typedef MacroIteratorBase Base;

      friend class MeshPointer< dim >;

    public:
      typedef Alberta::MeshPointer< dim > MeshPointer;
      typedef Alberta::ElementInfo< dim > ElementInfo;

    private:
      explicit MacroIterator ( const MeshPointer &mesh, bool end = false )
        : Base( mesh, end )
      {}

    public:
      using Base::done;
      using Base::equals;
      using Base::increment;
      using Base::macroElement;
      using Base::mesh;

      This &operator++ ()
      {
        increment();
        return *this;
      }

      ElementInfo operator* () const
      {
        return elementInfo();
      }

      bool operator== ( const MacroIterator &other ) const
      {
        return equals( other );
      }

      bool operator!= ( const MacroIterator &other ) const
      {
        return !equals( other );
      }

      ElementInfo
      elementInfo ( typename FillFlags::Flags fillFlags = FillFlags::standard ) const
      {
        if( done() )
          return ElementInfo();
        else
          return ElementInfo( mesh(), macroElement(), fillFlags );
      }
    };

  }

}

#endif // #if HAVE_ALBERTA

#endif
