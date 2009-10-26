// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_BOUNDARYPROJECTION_HH
#define DUNE_BOUNDARYPROJECTION_HH

//- system includes
#include <cmath>

//- Dune includes
#include <dune/common/fvector.hh>

#include <dune/grid/common/boundarysegment.hh>
#include <dune/grid/genericgeometry/mappingprovider.hh>
#include <dune/grid/genericgeometry/geometrytraits.hh>

namespace Dune
{

  /** \brief Interface class for vertex projection at the boundary.
   */
  template <int dimworld>
  struct DuneBoundaryProjection
  {
    //! \brief type of coordinate vector
    typedef FieldVector< double, dimworld> CoordinateType;
    //! \brief destructor
    virtual ~DuneBoundaryProjection() {}

    //! \brief projection operator projection a global coordinate
    virtual CoordinateType operator() (const CoordinateType& global) const = 0;
  };



  // BoundarySegmentWrapper
  // ----------------------

  template< int dim, int dimworld >
  class BoundarySegmentWrapper
    : public DuneBoundaryProjection< dimworld >
  {
    typedef BoundarySegmentWrapper< dim, dimworld > This;
    typedef DuneBoundaryProjection< dimworld > Base;

    typedef GenericGeometry::DefaultGeometryTraits< double, dim-1, dimworld > GeometryTraits;
    typedef GenericGeometry::HybridMapping< dim-1, GeometryTraits > FaceMapping;
    typedef GenericGeometry::MappingProvider< FaceMapping, 0 > FaceMappingProvider;

  public:
    typedef typename Base::CoordinateType CoordinateType;
    typedef Dune::BoundarySegment< dim, dimworld > BoundarySegment;

    /** constructor
     *
     *  \param[in]  type             geometry type of the boundary face
     *  \param[in]  vertices         vertices of the boundary face
     *  \param[in]  boundarySegment  geometric realization of the shaped boundary
     *
     *  \note The BoundarySegmentWrapper takes control of the boundary segment.
     */
    BoundarySegmentWrapper ( const GeometryType &type,
                             const std::vector< CoordinateType > &vertices,
                             const BoundarySegment *boundarySegment )
      : faceMapping_( FaceMappingProvider::mapping( type, vertices ) ),
        boundarySegment_( boundarySegment )
    {
      faceMapping_->referenceCount = 1;
    }

    BoundarySegmentWrapper ( const This &other )
      : faceMapping_( other.faceMapping_ ),
        boundarySegment_( other.boundarySegment_ )
    {
      ++(faceMapping_->referenceCount);
    }

    ~BoundarySegmentWrapper ()
    {
      // we abuse the mapping's reference count to determine when to
      // destruct the boundary segment
      if( --(faceMapping_->referenceCount) == 0 )
      {
        delete faceMapping_;
        delete boundarySegment_;
      }
    }

    This &operator= ( const This &other ) const
    {
      ++(other.faceMapping_->referenceCount);
      if( --(faceMapping_->referenceCount == 0) )
      {
        delete faceMapping_;
        delete boundarySegment_;
      }
      faceMapping_ = other.faceMapping_;
      boundarySegment_ = other.boundarySegment_;
      return *this;
    }

    CoordinateType operator() ( const CoordinateType &global ) const
    {
      return boundarySegment() ( faceMapping_->local( global ) );
    }

    const BoundarySegment &boundarySegment () const
    {
      return *boundarySegment_;
    }

  private:
    FaceMapping *faceMapping_;
    const BoundarySegment *boundarySegment_;
  };



  //////////////////////////////////////////////////////////////////////
  //
  // Example of boundary projection projection to a circle
  //
  //////////////////////////////////////////////////////////////////////
  template <int dimworld>
  struct CircleBoundaryProjection : public DuneBoundaryProjection< dimworld >
  {
    //! \brief type of coordinate vector
    typedef FieldVector< double, dimworld> CoordinateType;

    //! constructor taking radius of circle (default = sqrt( dimworld ) )
    CircleBoundaryProjection(const double radius = std::sqrt( (double)dimworld ))
      : radius_( radius ) {}

    //! \brief destructor
    virtual ~CircleBoundaryProjection() {}

    //! \brief projection operator projection a global coordinate
    virtual CoordinateType operator() (const CoordinateType& global) const
    {
      CoordinateType prj( global );
      // get adjustment factor
      const double factor = radius_  / global.two_norm();
      // adjust
      prj *= factor;
      return prj;
    }

  protected:
    //! radius of circ
    const double radius_;
  };

} // end namespace

#endif // #ifndef DUNE_BOUNDARYPROJECTION_HH