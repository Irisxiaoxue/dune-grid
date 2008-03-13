// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_BOUNDARY_EXTRACTOR_HH
#define DUNE_BOUNDARY_EXTRACTOR_HH

/** \file
    \brief Contains helper classes for the creation of UGGrid objects
    \author Oliver Sander
 */

#include <vector>
#include <set>
#include <dune/common/fvector.hh>


namespace Dune {

  /** \brief Boundary segments that can be compared

     This general implementation is empty.  Only specializations for dim==2 and dim==3 exist.
   */
  template <int dim>
  class UGGridBoundarySegment {};

  /** \brief Specialization of the boundary segment class for 2d */
  template <>
  class UGGridBoundarySegment<2> : public FieldVector<int,2> {

  public:

    /** \brief Compare the vertex lists modulo permutation */
    bool operator<(const UGGridBoundarySegment<2>& other) const {

      FieldVector<int,2> sorted1, sorted2;

      // ////////////////////////////////////////////////////////////////////////////
      // Sort the two arrays to get rid of cyclic permutations in mirror symmetry
      // ////////////////////////////////////////////////////////////////////////////
      if ((*this)[0] < (*this)[1])
        sorted1 = (*this);
      else {
        sorted1[0] = (*this)[1];
        sorted1[1] = (*this)[0];
      }

      if (other[0] < other[1])
        sorted2 = other;
      else {
        sorted2[0] = other[1];
        sorted2[1] = other[0];
      }

      // ////////////////////////////////////////////////////////////////////////////
      //   Compare the two sorted arrays
      // ////////////////////////////////////////////////////////////////////////////
      for (int i=0; i<2; i++) {
        if (sorted1[i]<sorted2[i])
          return true;
        else if (sorted1[i]>sorted2[i])
          return false;
      }

      // The sorted arrays are identical
      return false;
    }

  };

  /** \brief Specialization of the boundary segment class for 2d */
  template <>
  class UGGridBoundarySegment<3> : public FieldVector<int,4> {

  public:

    /** \brief Compare the vertex lists modulo permutation */
    bool operator<(const UGGridBoundarySegment<3>& other) const {
      UGGridBoundarySegment<3> sorted1 = (*this);
      UGGridBoundarySegment<3> sorted2 = other;

      // ////////////////////////////////////////////////////////////////////////////
      // Sort the two arrays to get rid of cyclic permutations in mirror symmetry
      // ////////////////////////////////////////////////////////////////////////////

      // bubble sort
      for (int i=3; i>=1; i--) {

        for (int j=0; j<i; j++) {

          if (sorted1[j] > sorted1[j+1]) {
            int tmp = sorted1[j];
            sorted1[j] = sorted1[j+1];
            sorted1[j+1] = tmp;
          }

          if (sorted2[j] > sorted2[j+1]) {
            int tmp = sorted2[j];
            sorted2[j] = sorted2[j+1];
            sorted2[j+1] = tmp;
          }

        }

      }

      // ////////////////////////////////////////////////////////////////////////////
      //   Compare the two sorted arrays
      // ////////////////////////////////////////////////////////////////////////////
      for (int i=0; i<4; i++) {
        if (sorted1[i]<sorted2[i])
          return true;
        else if (sorted1[i]>sorted2[i])
          return false;
      }

      // The sorted arrays are identical
      return false;
    }

  };

  class BoundaryExtractor {

    typedef std::set<UGGridBoundarySegment<2> >::iterator SetIterator2d;
    typedef std::set<UGGridBoundarySegment<3> >::iterator SetIterator3d;

  public:

    static void detectBoundarySegments(const std::vector<unsigned char>& elementTypes,
                                       const std::vector<unsigned int>& elementVertices,
                                       std::set<UGGridBoundarySegment<2> >& boundarySegments);

    static void detectBoundarySegments(const std::vector<unsigned char>& elementTypes,
                                       const std::vector<unsigned int>& elementVertices,
                                       std::set<UGGridBoundarySegment<3> >& boundarySegments);

    template <int dim>
    static int detectBoundaryNodes(const std::set<UGGridBoundarySegment<dim> >& boundarySegments,
                                   int noOfNodes,
                                   std::vector<int>& isBoundaryNode);

  };

}

#endif
