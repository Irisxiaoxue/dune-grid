// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALU2DGRID_CAPABILITIES_HH
#define DUNE_ALU2DGRID_CAPABILITIES_HH

// only include this code, if ENABLE_ALUGRID is defined
#ifdef ENABLE_ALUGRID

#include <dune/grid/alugrid/2d/alu2dinclude.hh>
#include <dune/grid/common/capabilities.hh>

/** @file
 *  @author Robert Kloefkorn
 *  @brief Capabilities for 2d ALUGrids
 */

namespace Dune
{

  template< int dim, int dimworld >
  class ALUSimplexGrid;

  template< int dim, int dimworld >
  class ALUConformGrid;

  namespace Capabilities
  {


    // Capabilities for ALUSimplexGrid
    // -------------------------------

    /** \struct isLeafwiseConforming
       \ingroup ALUSimplexGrid
     */

    /** \struct IsUnstructured
       \ingroup ALUSimplexGrid
     */

    /** \brief ALUSimplexGrid has entities for all codimension
       \ingroup ALUSimplexGrid
     */
    template< int cdim >
    struct hasEntity< ALUSimplexGrid< 2, 2 >, cdim >
    {
      static const bool v = true;
    };

    /** \brief ALUSimplexGrid is parallel
       \ingroup ALUSimplexGrid
     */
#if ALU2DGRID_PARALLEL
    //- default is false
    template<>
    struct isParallel< ALUSimplexGrid< 2, 2 > >
    {
      static const bool v = true;
    };
#endif

    /** \brief ALUSimplexGrid can communicate
       \ingroup ALUSimplexGrid
     */
#if ALU2DGRID_PARALLEL
    //- default is false
    template< int codim >
    struct canCommunicate< ALUSimplexGrid< 2, 2 >, codim >
    {
      static const bool v = true;
    };
#endif

    /** \brief ALUSimplexGrid has conforming level grids
       \ingroup ALUSimplexGrid
     */
    template<>
    struct isLevelwiseConforming< ALUSimplexGrid< 2, 2 > >
    {
      static const bool v = true;
    };

    /** \brief ALUSimplexGrid has supports hanging nodes
       \ingroup ALUSimplexGrid
     */
    template<>
    struct hasHangingNodes< ALUSimplexGrid< 2, 2 > >
    {
      static const bool v = true;
    };

    /** \brief ALUSimplexGrid has backup and restore facilities
       \ingroup ALUSimplexGrid
     */
    template<>
    struct hasBackupRestoreFacilities< ALUSimplexGrid< 2, 2 > >
    {
      static const bool v = true;
    };

    // Capabilities for ALUConformGrid
    // ----------------------------

    /** \struct isLeafwiseConforming
       \ingroup ALUConformGrid
     */

    /** \struct IsUnstructured
       \ingroup ALUConformGrid
     */

    /** \brief ALUConformGrid has entities for all codimension
       \ingroup ALUConformGrid
     */
    template< int cdim >
    struct hasEntity< ALUConformGrid< 2, 2 >, cdim >
    {
      static const bool v = true;
    };

    /** \brief ALUConformGrid is parallel
       \ingroup ALUConformGrid
     */
#if ALU2DGRID_PARALLEL
    //- default is false
    template<>
    struct isParallel< ALUConformGrid< 2, 2 > >
    {
      static const bool v = true;
    };
#endif

    /** \brief ALUConformGrid can communicate
       \ingroup ALUConformGrid
     */
#if ALU2DGRID_PARALLEL
    //- default is false
    template< int codim >
    struct canCommunicate< ALUConformGrid< 2, 2 >, codim >
    {
      static const bool v = true;
    };
#endif

    /** \brief ALUConformGrid has non-conforming level grids
       \ingroup ALUConformGrid
     */

    /** \brief ALUConformGrid has a conforming leaf grid
       \ingroup ALUConformGrid
     */
    template<>
    struct isLeafwiseConforming< ALUConformGrid< 2, 2 > >
    {
      static const bool v = true;
    };

    /** \brief ALUConformGrid has no support for hanging nodes
       \ingroup ALUConformGrid
     */

    /** \brief ALUConformGrid has backup and restore facilities
       \ingroup ALUConformGrid
     */
    template<>
    struct hasBackupRestoreFacilities< ALUConformGrid< 2, 2 > >
    {
      static const bool v = true;
    };


  } // end namespace Capabilities

} //end  namespace Dune

#endif // #ifdef ENABLE_ALUGRID

#endif