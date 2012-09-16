// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GEOGRID_GEOMETRY_HH
#define DUNE_GEOGRID_GEOMETRY_HH

#include <dune/common/nullptr.hh>
#include <dune/common/typetraits.hh>

#include <dune/geometry/referenceelements.hh>
#include <dune/geometry/genericgeometry/mappingprovider.hh>

#include <dune/grid/common/capabilities.hh>
#include <dune/grid/geometrygrid/cornerstorage.hh>

namespace Dune
{

  namespace GeoGrid
  {

    // GeometryTraits
    // --------------

    template< int cdim, class Grid >
    struct GeometryTraits
    {
      typedef typename remove_const< Grid >::type::Traits Traits;

      typedef GenericGeometry::DuneCoordTraits< typename Traits::ctype > CoordTraits;

      static const int dimWorld = cdim;

      template< class Topology >
      struct Mapping
      {
        typedef GeoGrid::CornerStorage< Topology, const Grid > CornerStorage;
        typedef GenericGeometry::CornerMapping< CoordTraits, Topology, dimWorld, CornerStorage > type;
      };

      struct Caching
      {
        static const GenericGeometry::EvaluationType evaluateJacobianTransposed = GenericGeometry::ComputeOnDemand;
        static const GenericGeometry::EvaluationType evaluateJacobianInverseTransposed = GenericGeometry::ComputeOnDemand;
        static const GenericGeometry::EvaluationType evaluateIntegrationElement = GenericGeometry::ComputeOnDemand;
      };

      struct UserData
      {
        UserData () : refCount_( 0 ) {}

        void addReference () { ++refCount_; }
        bool removeReference () { return (--refCount_ == 0); }

      private:
        unsigned int refCount_;
      };
    };



    // MappingFamily
    // -------------

    template< int cdim, class Grid >
    class MappingFamily
    {
      typedef typename remove_const< Grid >::type::Traits Traits;

    public:
      typedef GeoGrid::GeometryTraits< cdim, Grid > GeometryTraits;

      static const int dimension = Traits::dimension;

    private:
      template< bool >
      struct Hybrid
      {
        typedef GenericGeometry::HybridMapping< dimension, GeometryTraits > ElementMapping;
      };

      template< bool >
      struct NonHybrid
      {
        static const unsigned int topologyId = Capabilities::hasSingleGeometryType< Grid >::topologyId;
        typedef typename GenericGeometry::Topology< topologyId, dimension >::type Topology;
        typedef GenericGeometry::NonHybridMapping< Topology, GeometryTraits > ElementMapping;
      };

    public:
      typedef typename SelectType< Capabilities::hasSingleGeometryType< Grid >::v, NonHybrid< true >, Hybrid< false > >::Type::ElementMapping ElementMapping;

      template< int codim >
      struct Codim
      {
        typedef GenericGeometry::MappingProvider< ElementMapping, codim > MappingProvider;
        typedef typename MappingProvider::Mapping Mapping;

        static const unsigned int maxMappingSize = MappingProvider::maxMappingSize;
      };
    };



    // Geometry
    // --------

    template< int mydim, int cdim, class Grid >
    class Geometry
    {
      typedef Geometry< mydim, cdim, Grid > This;

      typedef GeoGrid::MappingFamily< cdim, Grid > MappingFamily;
      typedef typename remove_const< Grid >::type::Traits Traits;

      template< int, int, class > friend class Geometry;

    public:
      static const int mydimension = mydim;
      static const int coorddimension = cdim;
      static const int dimension = MappingFamily::dimension;
      static const int codimension = dimension - mydimension;

    protected:
      typedef typename MappingFamily::template Codim< codimension >::MappingProvider MappingProvider;
      typedef typename MappingFamily::template Codim< codimension >::Mapping Mapping;

    public:
      typedef typename Mapping::FieldType ctype;

      typedef FieldVector< ctype, mydimension > LocalCoordinate;
      typedef FieldVector< ctype, coorddimension > GlobalCoordinate;

      typedef typename Mapping::JacobianTransposed JacobianTransposed;
      typedef typename Mapping::JacobianInverseTransposed JacobianInverseTransposed;

      Geometry ( const Grid &grid )
        : grid_( &grid ),
          mapping_( nullptr )
      {}

      template< class CoordVector >
      Geometry ( const Grid &grid, const GeometryType &type, const CoordVector &coords )
        : grid_( &grid )
      {
        char *mappingStorage = grid.template allocateMappingStorage< codimension >( type );
        mapping_ = MappingProvider::construct( type.id(), coords, mappingStorage );
        mapping_->userData().addReference();
      }

      template< int fatherdim >
      Geometry ( const Geometry< fatherdim, cdim, Grid > &father, int i )
        : grid_( father.grid_ )
      {
        const unsigned int codim = fatherdim - mydim;
        char *mappingStorage = grid().template allocateMappingStorage< codimension >( type );
        mapping_ = father.mapping_->template trace< codim >( i, mappingStorage );
        mapping_->userData().addReference();
      }

      Geometry ( const This &other )
        : grid_( other.grid_ ),
          mapping_( other.mapping_ )
      {
        if( mapping_ )
          mapping_->userData().addReference();
      }

      ~Geometry ()
      {
        if( mapping_ && mapping_->userData().removeReference() )
          destroyMapping();
      }

      const This &operator= ( const This &other )
      {
        if( other.mapping_ )
          other.mapping_->userData().addReference();
        if( mapping_ && mapping_->userData().removeReference() )
          destroyMapping();
        grid_ = other.grid_;
        mapping_ = other.mapping_;
        return *this;
      }

      operator bool () const { return bool( mapping_ ); }

      bool affine () const { return mapping_->affine(); }
      GeometryType type () const { return mapping_->type(); }

      int corners () const { return mapping_->numCorners(); }
      GlobalCoordinate corner ( const int i ) const { return mapping_->corner( i ); }
      GlobalCoordinate center () const { return mapping_->center(); }

      GlobalCoordinate global ( const LocalCoordinate &local ) const { return mapping_->global( local ); }
      LocalCoordinate local ( const GlobalCoordinate &global ) const { return mapping_->local( global ); }

      ctype integrationElement ( const LocalCoordinate &local ) const { return mapping_->integrationElement( local ); }
      ctype volume () const { return mapping_->volume(); }

      const JacobianTransposed &jacobianTransposed ( const LocalCoordinate &local ) const { return mapping_->jacobianTransposed( local ); }
      const JacobianInverseTransposed &jacobianInverseTransposed ( const LocalCoordinate &local ) const { return mapping_->jacobianInverseTransposed( local ); }

      const Grid &grid () const { return *grid_; }

    private:
      void destroyMapping ()
      {
        const GeometryType gt = type();
        mapping_->~Mapping();
        grid().template deallocateMappingStorage< codimension >( gt, (char *)mapping_ );
      }

      const Grid *grid_;
      Mapping* mapping_;
    };

  } // namespace GeoGrid



  // FacadeOptions
  // -------------

  namespace FacadeOptions
  {

    template< int mydim, int cdim, class Grid >
    struct StoreGeometryReference< mydim, cdim, Grid, GeoGrid::Geometry >
    {
      static const bool v = false;
    };

  } // namespace FacadeOptions

} // namespace Dune

#endif // #ifndef DUNE_GEOGRID_GEOMETRY_HH
