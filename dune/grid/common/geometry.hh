// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_GEOMETRY_HH
#define DUNE_GRID_GEOMETRY_HH

/** \file
    \brief Wrapper and interface classes for element geometries
 */

#include <cassert>

#include <dune/common/fmatrix.hh>
#include <dune/common/typetraits.hh>

#include <dune/geometry/referenceelements.hh>

namespace Dune
{

  // External Forward Declarations
  // -----------------------------

  template< int dim, int dimworld, class ct, class GridFamily >
  class GridDefaultImplementation;



  namespace FacadeOptions
  {

    //! \brief Traits class determining whether the Dune::Geometry facade
    //!        class stores the implementation object by reference or by value
    /**
     * \ingroup GIGeometry
     *
     * Storing by reference is appropriate for grid managers that keep an
     * instance of each geometry around anyway.  Note that the reference to
     * that instance must be valid at least until the next grid modification.
     *
     * \note Even grid managers that let the facade class store a copy must
     *       take care to keep that copy valid until the next grid
     *       modification, e.g. if the geometry implementation object does not
     *       itself store the corner coordinates but only keeps references.
     */
    template< int mydim, int cdim, class GridImp, template< int, int, class > class GeometryImp >
    struct StoreGeometryReference
    {
      //! Whether to store by reference.
      static const bool v = true;
    };

  }



  //*****************************************************************************
  //
  // Geometry
  // forwards the interface to the implementation
  //
  //*****************************************************************************

  /**
     @brief Wrapper class for geometries


     \tparam mydim Dimension of the domain
     \tparam cdim Dimension of the range
     \tparam GridImp Type that is a model of Dune::Grid
     \tparam GeometryImp Class template that is a model of Dune::Geometry

     <H3>Maps</H3>

     A Geometry defines a map \f[ g : D \to W\f] where
     \f$D\subseteq\mathbf{R}^\textrm{mydim}\f$ and
     \f$W\subseteq\mathbf{R}^\textrm{cdim}\f$.
     The domain \f$D\f$ is one of a set of predefined convex polytopes, the
     so-called reference elements (\see Dune::ReferenceElement). The dimensionality
     of \f$D\f$ is <tt>mydim</tt>.
     In general \f$\textrm{mydim}\leq\textrm{cdim}\f$, i.e.
     the convex polytope may be mapped to a manifold. Moreover, we require that
     \f$ g\in \left( C^1(D) \right)^\textrm{cdim}\f$ and one-to-one.


     <H3>Engine Concept</H3>

     The Geometry class template wraps an object of type GeometryImp and forwards all member
     function calls to corresponding members of this class. In that sense Geometry
     defines the interface and GeometryImp supplies the implementation.

     The grid manager can instruct this facade class to either itself store an
     instance of the implementation class or keep a reference to an instance
     stored elsewhere as determined by FacadeOptions::StoreGeometryReference.
     In any case it is guaranteed that instances of Geometry are valid until
     the grid is changed (via any of adapt(), loadBalance() or
     globalRefine()).

     \ingroup GIGeometry
   */
  template< int mydim, int cdim, class GridImp, template< int, int, class > class GeometryImp >
  class Geometry
  {
  #if DUNE_GRID_EXPERIMENTAL_GRID_EXTENSIONS
  public:
  #else
  protected:
    // give the GridDefaultImplementation class access to the realImp
    friend class GridDefaultImplementation<
        GridImp::dimension, GridImp::dimensionworld,
        typename GridImp::ctype,
        typename GridImp::GridFamily> ;
  #endif
    // type of underlying implementation, for internal use only
    typedef GeometryImp< mydim, cdim, GridImp > Implementation;

#if 0
    //! return reference to the implementation
    Implementation &impl () { return realGeometry; }
#endif
    //! return reference to the implementation
    const Implementation &impl () const { return realGeometry; }

  public:
    //! @brief export grid dimension
    enum { dimension=GridImp::dimension /*!< grid dimension */ };
    //! @brief export geometry dimension
    enum { mydimension=mydim /*!< geometry dimension */ };
    //! @brief export coordinate dimension
    enum { coorddimension=cdim /*!< dimension of embedding coordsystem */ };

    //! @brief export dimension of world
    enum { dimensionworld=GridImp::dimensionworld /*!< dimension of world */ };
    //! define type used for coordinates in grid module
    typedef typename GridImp::ctype ctype;

    //! type of local coordinates
    typedef FieldVector<ctype, mydim> LocalCoordinate;

    //! type of the global coordinates
    typedef FieldVector< ctype, cdim > GlobalCoordinate;

    //! type of jacobian inverse transposed
    //typedef FieldMatrix< ctype, cdim, mydim > JacobianInverseTransposed;
    typedef typename Implementation::JacobianInverseTransposed JacobianInverseTransposed;

    // deprecated typedef for backward compatibility
    typedef JacobianInverseTransposed Jacobian DUNE_DEPRECATED_MSG ( "type Geometry::Jacobian is deprecated, use Geometry::JacobianInverseTransposed instead." );

    //! type of jacobian transposed
    //typedef FieldMatrix< ctype, mydim, cdim > JacobianTransposed;
    typedef typename Implementation::JacobianTransposed JacobianTransposed;

    /** \brief Return the name of the reference element. The type can
       be used to access the Dune::ReferenceElement.
     */
    GeometryType type () const { return impl().type(); }

    /** \brief Return true if the geometry mapping is affine and false otherwise */
    bool affine() const { return impl().affine(); }

    /** \brief Return the number of corners of the reference element.
     *
       Since a geometry is a convex polytope the number of corners is a well-defined concept.
       The method is redundant because this information is also available
       via the reference element. It is here for efficiency and ease of use.
     */
    int corners () const { return impl().corners(); }

    /** \brief Obtain a corner of the geometry
     *
     *  This method is for convenient access to the corners of the geometry. The
     *  same result could be achieved by by calling
     *  \code
     *  global( genericReferenceElement.position( i, mydimension ) )
     *  \endcode
     *
     *  \param[in]  i  number of the corner (with respect to the generic reference
     *                 element)
     *  \returns position of the i-th corner
     */
    GlobalCoordinate corner ( int i ) const
    {
      return impl().corner( i );
    }

    /** \brief Evaluate the map \f$ g\f$.
       \param[in] local Position in the reference element \f$D\f$
       \return Position in \f$W\f$
     */
    GlobalCoordinate global (const LocalCoordinate& local) const
    {
      return impl().global( local );
    }

    /** \brief Evaluate the inverse map \f$ g^{-1}\f$
       \param[in] global Position in \f$W\f$
       \return Position in \f$D\f$ that maps to global
     */
    LocalCoordinate local (const GlobalCoordinate& global) const
    {
      return impl().local( global );
    }

    /** \brief Return the factor appearing in the integral transformation formula

       Let \f$ g : D \to W\f$ denote the transformation described by the Geometry.
       Then the jacobian of the transformation is defined as the
       \f$\textrm{cdim}\times\textrm{mydim}\f$ matrix
       \f[ J_g(x) = \left( \begin{array}{ccc} \frac{\partial g_0}{\partial x_0} &
       \cdots & \frac{\partial g_0}{\partial x_{n-1}} \\
       \vdots & \ddots & \vdots \\ \frac{\partial g_{m-1}}{\partial x_0} &
       \cdots & \frac{\partial g_{m-1}}{\partial x_{n-1}}
       \end{array} \right).\f]
       Here we abbreviated \f$m=\textrm{cdim}\f$ and \f$n=\textrm{mydim}\f$ for ease of
       readability.

       The integration element \f$\mu(x)\f$ for any \f$x\in D\f$ is then defined as
       \f[ \mu(x) = \sqrt{|\det J_g^T(x)J_g(x)|}.\f]

       \param[in] local Position \f$x\in D\f$
       \return    integration element \f$\mu(x)\f$

       \note Each implementation computes the integration element with optimal
       efficieny. For example in an equidistant structured mesh it may be as
       simple as \f$h^\textrm{mydim}\f$.
     */
    ctype integrationElement (const LocalCoordinate& local) const
    {
      return impl().integrationElement( local );
    }

    /** \brief return volume of geometry */
    ctype volume () const
    {
      return impl().volume();
    }

    /** \brief return center of geometry
     *
     *  Note that this method is still subject to a change of name and semantics.
     *  At the moment, the center is not required to be the centroid of the
     *  geometry, or even the centroid of its corners. This makes the current
     *  default implementation acceptable, which maps the centroid of the
     *  reference element to the geometry.
     *  We may change the name (and semantic) of the method to centroid() if we
     *  find reasonably efficient ways to implement it properly.
     */
    GlobalCoordinate center () const
    {
      return impl().center();
    }

    /** \brief Return the transposed of the Jacobian
     *
     *  The Jacobian is defined in the documentation of
     *  \ref Dune::Geometry::integrationElement "integrationElement".
     *
     *  \param[in]  local  position \f$x\in D\f$
     *
     *  \return \f$J_g^T(x)\f$
     */
    const JacobianTransposed &
    jacobianTransposed ( const LocalCoordinate& local ) const
    {
      return impl().jacobianTransposed( local );
    }

    /** \brief Return inverse of transposed of Jacobian
     *
     *  The Jacobian is defined in the documentation of
     *  \ref Dune::Geometry::integrationElement "integrationElement".
     *
     *  \param[in]  local  position \f$x\in D\f$
     *  \return \f$J_g^{-T}(x)\f$
     *
     *  The use of this function is to compute the gradient of some function
     *  \f$f : W \to \textbf{R}\f$ at some position \f$y=g(x)\f$, where
     *  \f$x\in D\f$ and \f$g\f$ the transformation of the Geometry.
     *  When we set \f$\hat{f}(x) = f(g(x))\f$ and apply the chain rule we obtain
     *  \f[\nabla f(g(x)) = J_g^{-T}(x) \nabla \hat{f}(x).\f]
     *
     *  \note In the non-symmetric case \f$\textrm{cdim} \neq \textrm{mydim}\f$, the
     *        pseudoinverse of \f$J_g^T(x)\f$ is returned.
     *        This means that it is inverse for all tangential vectors in
     *        \f$g(x)\f$ while mapping all normal vectors to zero.
     */
    const JacobianInverseTransposed &
    jacobianInverseTransposed ( const LocalCoordinate &local ) const
    {
      return impl().jacobianInverseTransposed(local);
    }

    //! copy constructor from implementation
    explicit Geometry ( const Implementation &impl )
      : realGeometry( impl )
    {
      deprecationWarning ( integral_constant< bool, storeReference >() );
    }

  private:
    /** hide assignment operator */
    const Geometry &operator= ( const Geometry &rhs );

    void DUNE_DEPRECATED_MSG( "This Dune::Geometry is still a reference to its implementation." )
    deprecationWarning ( integral_constant< bool, true > ) {}

    void
    deprecationWarning ( integral_constant< bool, false > ) {}

  protected:
    static const bool storeReference = FacadeOptions::StoreGeometryReference< mydim, cdim, GridImp, GeometryImp >::v;

    typename conditional< storeReference, const Implementation &, Implementation >::type realGeometry;
  };



  //************************************************************************
  // GEOMETRY Default Implementations
  //*************************************************************************
  //
  // --GeometryDefault
  //
  //! Default implementation for class Geometry
  template<int mydim, int cdim, class GridImp, template<int,int,class> class GeometryImp>
  class GeometryDefaultImplementation
  {
  public:
    static const int mydimension = mydim;
    static const int coorddimension = cdim;

    // save typing
    typedef typename GridImp::ctype ctype;

    typedef FieldVector< ctype, mydim > LocalCoordinate;
    typedef FieldVector< ctype, cdim > GlobalCoordinate;

    //! type of jacobian inverse transposed
    typedef FieldMatrix< ctype, cdim, mydim > JacobianInverseTransposed;

    //! type of jacobian transposed
    typedef FieldMatrix< ctype, mydim, cdim > JacobianTransposed;

    //! return volume of the geometry
    ctype volume () const
    {
      GeometryType type = asImp().type();

      // get corresponding reference element
      const ReferenceElement< ctype , mydim > & refElement =
        ReferenceElements< ctype, mydim >::general(type);

      LocalCoordinate localBaryCenter ( 0 );
      // calculate local bary center
      const int corners = refElement.size(0,0,mydim);
      for(int i=0; i<corners; ++i) localBaryCenter += refElement.position(i,mydim);
      localBaryCenter *= (ctype) (1.0/corners);

      // volume is volume of reference element times integrationElement
      return refElement.volume() * asImp().integrationElement(localBaryCenter);
    }

    //! return center of the geometry
    GlobalCoordinate center () const
    {
      GeometryType type = asImp().type();

      // get corresponding reference element
      const ReferenceElement< ctype , mydim > & refElement =
        ReferenceElements< ctype, mydim >::general(type);

      // center is (for now) the centroid of the reference element mapped to
      // this geometry.
      return asImp().global(refElement.position(0,0));
    }

  private:
    //!  Barton-Nackman trick
    GeometryImp<mydim,cdim,GridImp>& asImp () {return static_cast<GeometryImp<mydim,cdim,GridImp>&>(*this);}
    const GeometryImp<mydim,cdim,GridImp>& asImp () const {return static_cast<const GeometryImp<mydim,cdim,GridImp>&>(*this);}
  }; // end GeometryDefault

  template<int cdim, class GridImp, template<int,int,class> class GeometryImp>
  class GeometryDefaultImplementation<0,cdim,GridImp,GeometryImp>
  {
    // my dimension
    enum { mydim = 0 };

  public:
    static const int mydimension = mydim;
    static const int coorddimension = cdim;

    // save typing
    typedef typename GridImp::ctype ctype;

    typedef FieldVector< ctype, mydim > LocalCoordinate;
    typedef FieldVector< ctype, cdim > GlobalCoordinate;

    //! type of jacobian inverse transposed
    typedef FieldMatrix< ctype, cdim, mydim > JacobianInverseTransposed;

    //! type of jacobian transposed
    typedef FieldMatrix< ctype, mydim, cdim > JacobianTransposed;

    //! return the only coordinate
    FieldVector<ctype, cdim> global (const FieldVector<ctype, mydim>& local) const
    {
      return asImp().corner(0);
    }

    //! return empty vector
    FieldVector<ctype, mydim> local (const FieldVector<ctype, cdim>& ) const
    {
      return FieldVector<ctype, mydim>();
    }

    //! checkInside here returns true
    bool checkInside (const FieldVector<ctype, mydim>& ) const
    {
      return true;
    }

    //! return volume of the geometry
    ctype volume () const
    {
      return 1.0;
    }

    //! return center of the geometry
    FieldVector<ctype, cdim> center () const
    {
      return asImp().corner(0);
    }

  private:
    // Barton-Nackman trick
    GeometryImp<mydim,cdim,GridImp>& asImp () {return static_cast<GeometryImp<mydim,cdim,GridImp>&>(*this);}
    const GeometryImp<mydim,cdim,GridImp>& asImp () const {return static_cast<const GeometryImp<mydim,cdim,GridImp>&>(*this);}
  }; // end GeometryDefault

} // namespace Dune

#endif // DUNE_GRID_GEOMETRY_HH
