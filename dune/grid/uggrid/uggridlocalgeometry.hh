// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_UGGRID_LOCALGEOMETRY_HH
#define DUNE_UGGRID_LOCALGEOMETRY_HH

/** \file
 * \brief The UGGridElement class and its specializations
 */

#include "uggridrenumberer.hh"
#include <dune/common/array.hh>
#include <dune/common/fmatrix.hh>
#include <dune/geometry/multilineargeometry.hh>

namespace Dune {


  /** \brief Geometry of an entity embedded in another element, not in the world space
   * \ingroup UGGrid

     \tparam mydim Dimension of the corresponding reference element
     \tparam coorddim Dimension of the coordinate space

     I suppose I could use GenericGeometry::LocalGeometry instead of this class.
     However several times I use that this UGGridLocalGeometry has a default constructor,
     which GenericGeometry::LocalGeometry doesn't have.
   */
  template<int mydim, int coorddim, class GridImp>
  class UGGridLocalGeometry :
    public MultiLinearGeometry<typename GridImp::ctype, mydim, coorddim>
  {

    typedef MultiLinearGeometry<typename GridImp::ctype, mydim, coorddim> Base;

  public:

    /** \brief Constructor from a given geometry type and a vector of corner coordinates */
    UGGridLocalGeometry(const GeometryType& type, const std::vector<FieldVector<typename GridImp::ctype,coorddim> >& coordinates)
      : Base(type, coordinates)
    {}

  };

  namespace FacadeOptions
  {

    /** \brief Switch on the new implementation for the Geometry interface class
     * \deprecated Eventually the new implementation will be hardwired,
     *             and this switch may disappear without prior warning!
     */
    template< int mydim, int cdim, class GridImp>
    struct StoreGeometryReference<mydim,cdim,GridImp,UGGridLocalGeometry>
    {
      static const bool v = false;
    };

  }


}  // namespace Dune

#endif
