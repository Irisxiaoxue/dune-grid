// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>

#include <dune/grid/uggrid.hh>
#include <dune/grid/uggrid/ugintersectionit.hh>

template<class GridImp>
inline const Dune::FieldVector<typename GridImp::ctype, GridImp::dimensionworld>&
Dune::UGGridLevelIntersectionIterator <GridImp>::outerNormal (const Dune::FieldVector<UGCtype, GridImp::dimension-1>& local) const
{
  // //////////////////////////////////////////////////////
  //   Implementation for 3D
  // //////////////////////////////////////////////////////

  if (dim == 3) {

    if (UG_NS<dim>::Corners_Of_Side(center_, neighborCount_) == 3) {

      // A triangular intersection.  The normals are constant
      const UGCtype* aPos = UG_NS<dim>::Corner(center_,UG_NS<dim>::Corner_Of_Side(center_, neighborCount_, 0))->myvertex->iv.x;
      const UGCtype* bPos = UG_NS<dim>::Corner(center_,UG_NS<dim>::Corner_Of_Side(center_, neighborCount_, 1))->myvertex->iv.x;
      const UGCtype* cPos = UG_NS<dim>::Corner(center_,UG_NS<dim>::Corner_Of_Side(center_, neighborCount_, 2))->myvertex->iv.x;

      FieldVector<UGCtype, 3> ba, ca;

      for (int i=0; i<3; i++) {
        ba[i] = bPos[i] - aPos[i];
        ca[i] = cPos[i] - aPos[i];
      }

      outerNormal_[0] = ba[1]*ca[2] - ba[2]*ca[1];
      outerNormal_[1] = ba[2]*ca[0] - ba[0]*ca[2];
      outerNormal_[2] = ba[0]*ca[1] - ba[1]*ca[0];

    } else {

      // A quadrilateral: compute the normal in each corner and do bilinear interpolation
      // The cornerNormals array uses UG corner numbering
      FieldVector<UGCtype,3> cornerNormals[4];
      for (int i=0; i<4; i++) {

        // Compute the normal on the i-th corner
        const UGCtype* aPos = UG_NS<dim>::Corner(center_,UG_NS<dim>::Corner_Of_Side(center_,neighborCount_,i))->myvertex->iv.x;
        const UGCtype* bPos = UG_NS<dim>::Corner(center_,UG_NS<dim>::Corner_Of_Side(center_,neighborCount_,(i+1)%4))->myvertex->iv.x;
        const UGCtype* cPos = UG_NS<dim>::Corner(center_,UG_NS<dim>::Corner_Of_Side(center_,neighborCount_,(i+3)%4))->myvertex->iv.x;

        FieldVector<UGCtype, 3> ba, ca;

        for (int j=0; j<3; j++) {
          ba[j] = bPos[j] - aPos[j];
          ca[j] = cPos[j] - aPos[j];
        }

        cornerNormals[i][0] = ba[1]*ca[2] - ba[2]*ca[1];
        cornerNormals[i][1] = ba[2]*ca[0] - ba[0]*ca[2];
        cornerNormals[i][2] = ba[0]*ca[1] - ba[1]*ca[0];
      }

      // Bilinear interpolation
      for (int i=0; i<3; i++)
        outerNormal_[i] = (1-local[0])*(1-local[1])*cornerNormals[0][i]
                          + local[0]     * (1-local[1]) * cornerNormals[1][i]
                          + local[0]     * local[1]     * cornerNormals[2][i]
                          + (1-local[0]) * local[1]     * cornerNormals[3][i];

    }

  } else {     // if (dim==3) ... else

    // //////////////////////////////////////////////////////
    //   Implementation for 2D
    // //////////////////////////////////////////////////////

    // Get the vertices of this side.
    const UGCtype* aPos = UG_NS<dim>::Corner(center_,UG_NS<dim>::Corner_Of_Side(center_, neighborCount_, 0))->myvertex->iv.x;
    const UGCtype* bPos = UG_NS<dim>::Corner(center_,UG_NS<dim>::Corner_Of_Side(center_, neighborCount_, 1))->myvertex->iv.x;

    // compute normal
    outerNormal_[0] = bPos[1] - aPos[1];
    outerNormal_[1] = aPos[0] - bPos[0];

  }

  return outerNormal_;
}

template< class GridImp>
inline const typename Dune::UGGridLevelIntersectionIterator<GridImp>::LocalGeometry&
Dune::UGGridLevelIntersectionIterator<GridImp>::
intersectionSelfLocal() const
{
  int numCornersOfSide = UG_NS<dim>::Corners_Of_Side(center_, neighborCount_);

  selfLocal_.setNumberOfCorners(numCornersOfSide);

  for (int i=0; i<numCornersOfSide; i++)
  {
    // get number of corner in UG's numbering system
    int cornerIdx = UG_NS<dim>::Corner_Of_Side(center_, neighborCount_, i);

    // we need a temporary to be filled
    FieldVector<UGCtype, dim> tmp;

    // get the corners local coordinates
    UG_NS<dim>::getCornerLocal(center_,cornerIdx,tmp);

    // and poke them into the Geometry
    selfLocal_.setCoords(i,tmp);
  }

  return selfLocal_;
}

template< class GridImp>
inline const typename Dune::UGGridLevelIntersectionIterator<GridImp>::Geometry&
Dune::UGGridLevelIntersectionIterator<GridImp>::
intersectionGlobal() const
{
  int numCornersOfSide = UG_NS<dim>::Corners_Of_Side(center_, neighborCount_);

  neighGlob_.setNumberOfCorners(numCornersOfSide);

  for (int i=0; i<numCornersOfSide; i++) {

    int cornerIdx = UG_NS<dim>::Corner_Of_Side(center_, neighborCount_, i);
    typename UG_NS<dim>::Node* node = UG_NS<dim>::Corner(center_, cornerIdx);

    neighGlob_.setCoords(i, node->myvertex->iv.x);

  }

  return neighGlob_;
}

template< class GridImp>
inline const typename Dune::UGGridLevelIntersectionIterator<GridImp>::LocalGeometry&
Dune::UGGridLevelIntersectionIterator<GridImp>::
intersectionNeighborLocal() const
{
  typename UG_NS<dim>::Element *other;

  // if we have a neighbor on this level, then return it
  if (UG_NS<dim>::NbElem(center_, neighborCount_)!=NULL)
    other = UG_NS<dim>::NbElem(center_, neighborCount_);
  else
    DUNE_THROW(GridError,"no neighbor found");

  // ///////////////////////////////////////
  // go on and get the local coordinates
  // ///////////////////////////////////////
  int numCornersOfSide = UG_NS<dim>::Corners_Of_Side(center_,neighborCount_);
  neighLocal_.setNumberOfCorners(numCornersOfSide);

  for (int i=0; i<numCornersOfSide; i++) {

    // get the node in this element
    int localCornerNumber = UG_NS<dim>::Corner_Of_Side(center_, neighborCount_, i);
    const typename UG_NS<dim>::Node* node = UG_NS<dim>::Corner(center_,localCornerNumber);

    // get this node's local index in the neighbor element
    int j;
    for (j=0; j<UG_NS<dim>::Corners_Of_Elem(other); j++)
      if (UG_NS<dim>::Corner(other, j) == node)
        break;

    assert(j<UG_NS<dim>::Corners_Of_Elem(other));

    // get the local coordinate there
    FieldVector<UGCtype, dim> tmp;
    UG_NS<dim>::getCornerLocal(other,j,tmp);

    // and poke them into the Geometry
    neighLocal_.setCoords(i,tmp);
  }

  return neighLocal_;
}

template< class GridImp>
inline int Dune::UGGridLevelIntersectionIterator<GridImp>::
numberInNeighbor () const
{
  const typename UG_NS<dim>::Element *other;

  // Look for a neighbor on this level
  if ((other = UG_NS<dim>::NbElem(center_, neighborCount_)) == NULL)
    DUNE_THROW(GridError,"There is no neighbor element!");

  // Find the corresponding side in the neighbor element
  const int nSides = UG_NS<dim>::Sides_Of_Elem(other);
  int i;
  for (i=0; i<nSides; i++)
    if (UG_NS<dim>::NbElem(other,i) == center_)
      break;

  // now we have to renumber the side i
  return UGGridRenumberer<dim>::facesUGtoDUNE(i, nSides);
}


// /////////////////////////////////////////////////////////////////////////////
//   Implementations for the class UGGridLeafIntersectionIterator
// /////////////////////////////////////////////////////////////////////////////

/** \todo Needs to be checked for the nonconforming case */
template<class GridImp>
inline const Dune::FieldVector<typename GridImp::ctype, GridImp::dimensionworld>&
Dune::UGGridLeafIntersectionIterator <GridImp>::outerNormal (const FieldVector<UGCtype, GridImp::dimension-1>& local) const
{
  // //////////////////////////////////////////////////////
  //   Implementation for 3D
  // //////////////////////////////////////////////////////

  if (dim == 3) {

    if (UG_NS<dim>::Corners_Of_Side(center_, neighborCount_) == 3) {

      // A triangular intersection.  The normals are constant
      const UGCtype* aPos = UG_NS<dim>::Corner(center_,UG_NS<dim>::Corner_Of_Side(center_, neighborCount_, 0))->myvertex->iv.x;
      const UGCtype* bPos = UG_NS<dim>::Corner(center_,UG_NS<dim>::Corner_Of_Side(center_, neighborCount_, 1))->myvertex->iv.x;
      const UGCtype* cPos = UG_NS<dim>::Corner(center_,UG_NS<dim>::Corner_Of_Side(center_, neighborCount_, 2))->myvertex->iv.x;

      FieldVector<UGCtype, 3> ba, ca;

      for (int i=0; i<3; i++) {
        ba[i] = bPos[i] - aPos[i];
        ca[i] = cPos[i] - aPos[i];
      }

      outerNormal_[0] = ba[1]*ca[2] - ba[2]*ca[1];
      outerNormal_[1] = ba[2]*ca[0] - ba[0]*ca[2];
      outerNormal_[2] = ba[0]*ca[1] - ba[1]*ca[0];

    } else {

      // A quadrilateral: compute the normal in each corner and do bilinear interpolation
      // The cornerNormals array uses UG corner numbering
      FieldVector<UGCtype,3> cornerNormals[4];
      for (int i=0; i<4; i++) {

        // Compute the normal on the i-th corner
        const UGCtype* aPos = UG_NS<dim>::Corner(center_,UG_NS<dim>::Corner_Of_Side(center_,neighborCount_,i))->myvertex->iv.x;
        const UGCtype* bPos = UG_NS<dim>::Corner(center_,UG_NS<dim>::Corner_Of_Side(center_,neighborCount_,(i+1)%4))->myvertex->iv.x;
        const UGCtype* cPos = UG_NS<dim>::Corner(center_,UG_NS<dim>::Corner_Of_Side(center_,neighborCount_,(i+3)%4))->myvertex->iv.x;

        FieldVector<UGCtype, 3> ba, ca;

        for (int j=0; j<3; j++) {
          ba[j] = bPos[j] - aPos[j];
          ca[j] = cPos[j] - aPos[j];
        }

        cornerNormals[i][0] = ba[1]*ca[2] - ba[2]*ca[1];
        cornerNormals[i][1] = ba[2]*ca[0] - ba[0]*ca[2];
        cornerNormals[i][2] = ba[0]*ca[1] - ba[1]*ca[0];
      }

      // Bilinear interpolation
      for (int i=0; i<3; i++)
        outerNormal_[i] = (1-local[0])*(1-local[1])*cornerNormals[0][i]
                          + local[0]     * (1-local[1]) * cornerNormals[1][i]
                          + local[0]     * local[1]     * cornerNormals[2][i]
                          + (1-local[0]) * local[1]     * cornerNormals[3][i];

    }

  } else {     // if (dim==3) ... else

    // //////////////////////////////////////////////////////
    //   Implementation for 2D
    // //////////////////////////////////////////////////////

    // Get the vertices of this side.
    const UGCtype* aPos = UG_NS<dim>::Corner(center_,UG_NS<dim>::Corner_Of_Side(center_, neighborCount_, 0))->myvertex->iv.x;
    const UGCtype* bPos = UG_NS<dim>::Corner(center_,UG_NS<dim>::Corner_Of_Side(center_, neighborCount_, 1))->myvertex->iv.x;

    // compute normal
    outerNormal_[0] = bPos[1] - aPos[1];
    outerNormal_[1] = aPos[0] - bPos[0];

  }

  return outerNormal_;
}

/** \todo Needs to be checked for the nonconforming case */
template< class GridImp>
inline const typename Dune::UGGridLeafIntersectionIterator<GridImp>::LocalGeometry&
Dune::UGGridLeafIntersectionIterator<GridImp>::
intersectionSelfLocal() const
{
  if (leafSubFaces_.size() == 1) {

    // //////////////////////////////////////////////////////
    //   The easy case: a conforming intersection
    // //////////////////////////////////////////////////////

    int numCornersOfSide = UG_NS<dim>::Corners_Of_Side(center_, neighborCount_);

    selfLocal_.setNumberOfCorners(numCornersOfSide);

    for (int i=0; i<numCornersOfSide; i++)
    {
      // get number of corner in UG's numbering system
      int cornerIdx = UG_NS<dim>::Corner_Of_Side(center_, neighborCount_, i);

      // we need a temporary to be filled
      FieldVector<UGCtype, dim> tmp;

      // get the corners local coordinates
      UG_NS<dim>::getCornerLocal(center_,cornerIdx,tmp);

      // and poke them into the Geometry
      selfLocal_.setCoords(i,tmp);
    }

  } else {
    DUNE_THROW(NotImplemented, "no intersectionSelfLocal() for nonconforming UGGrids");
  }

  return selfLocal_;
}

/** \todo Needs to be checked for the nonconforming case */
template< class GridImp>
inline const typename Dune::UGGridLeafIntersectionIterator<GridImp>::Geometry&
Dune::UGGridLeafIntersectionIterator<GridImp>::
intersectionGlobal() const
{
  if (leafSubFaces_.size() == 1) {

    // //////////////////////////////////////////////////////
    //   The easy case: a conforming intersection
    // //////////////////////////////////////////////////////

    int numCornersOfSide = UG_NS<dim>::Corners_Of_Side(center_, neighborCount_);

    neighGlob_.setNumberOfCorners(numCornersOfSide);

    for (int i=0; i<numCornersOfSide; i++) {

      int cornerIdx = UG_NS<dim>::Corner_Of_Side(center_, neighborCount_, i);
      typename UG_NS<dim>::Node* node = UG_NS<dim>::Corner(center_, cornerIdx);

      neighGlob_.setCoords(i, node->myvertex->iv.x);

    }

  } else {
    DUNE_THROW(NotImplemented, "no intersectionGlobal() for nonconforming UGGrids");
  }

  return neighGlob_;
}

/** \todo Needs to be checked for the nonconforming case */
template< class GridImp>
inline const typename Dune::UGGridLeafIntersectionIterator<GridImp>::LocalGeometry&
Dune::UGGridLeafIntersectionIterator<GridImp>::
intersectionNeighborLocal() const
{
  if (leafSubFaces_.size() == 1) {

    // //////////////////////////////////////////////////////
    //   The easy case: a conforming intersection
    // //////////////////////////////////////////////////////

    const typename UG_NS<dim>::Element *other = leafSubFaces_[subNeighborCount_].first;

    // ///////////////////////////////////////
    // go on and get the local coordinates
    // ///////////////////////////////////////
    int numCornersOfSide = UG_NS<dim>::Corners_Of_Side(center_,neighborCount_);
    neighLocal_.setNumberOfCorners(numCornersOfSide);

    for (int i=0; i<numCornersOfSide; i++) {

      // get the node in this element
      int localCornerNumber = UG_NS<dim>::Corner_Of_Side(center_, neighborCount_, i);
      const typename UG_NS<dim>::Node* node = UG_NS<dim>::Corner(center_,localCornerNumber);

      // get this node's local index in the neighbor element
      int j;
      for (j=0; j<UG_NS<dim>::Corners_Of_Elem(other); j++)
        // Compare vertices because the nodes may be on different levels, but the nodes are the same
        if (UG_NS<dim>::Corner(other, j)->myvertex == node->myvertex)
          break;

      assert(j<UG_NS<dim>::Corners_Of_Elem(other));

      // get the local coordinate there
      FieldVector<UGCtype, dim> tmp;
      UG_NS<dim>::getCornerLocal(other,j,tmp);

      // and poke them into the Geometry
      neighLocal_.setCoords(i,tmp);
    }

  } else {

    DUNE_THROW(NotImplemented, "no intersectionNeighborLocal for nonconforming UGGrids");

  }

  return neighLocal_;
}

template< class GridImp>
inline int Dune::UGGridLeafIntersectionIterator<GridImp>::
numberInNeighbor () const
{
  if (leafSubFaces_[subNeighborCount_].first == NULL)
    DUNE_THROW(GridError,"There is no neighbor!");

  const int nSides = UG_NS<dim>::Sides_Of_Elem(leafSubFaces_[subNeighborCount_].first);

  assert(leafSubFaces_[subNeighborCount_].second < nSides);

  // Renumber to DUNE numbering
  return UGGridRenumberer<dim>::facesUGtoDUNE(leafSubFaces_[subNeighborCount_].second, nSides);
}

template< class GridImp>
inline void Dune::UGGridLeafIntersectionIterator<GridImp>::constructLeafSubfaces() {

  // Do nothing if level neighbor doesn't exit
  typename UG_NS<dim>::Element* levelNeighbor = UG_NS<dim>::NbElem(center_, neighborCount_);

  if (levelNeighbor != NULL && UG_NS<dim>::isLeaf(levelNeighbor)) {
    leafSubFaces_.resize(1);
    leafSubFaces_[0] = Face(levelNeighbor, numberInNeighbor(center_, levelNeighbor));
    return;
  }

  // Go down
  if (levelNeighbor == NULL) {

    leafSubFaces_.resize(1);

    // I am a leaf and the neighbor does not exist: go down
    const typename UG_NS<dim>::Element* me = center_;
    const typename UG_NS<dim>::Element* father = UG_NS<GridImp::dimensionworld>::EFather(center_);

    int side = neighborCount_;
    int fatherSide;

    while (father != NULL) {

      // //////////////////////////////////////////////////////////////
      //   Find the topological father face
      // //////////////////////////////////////////////////////////////
      if (dim==2) {

        // Get the two nodes
        const typename UG_NS<dim>::Node* n0 = UG_NS<dim>::Corner(me,UG_NS<dim>::Corner_Of_Side(me, side, 0));
        const typename UG_NS<dim>::Node* n1 = UG_NS<dim>::Corner(me,UG_NS<dim>::Corner_Of_Side(me, side, 1));

        const typename UG_NS<dim>::Node* fatherN0, *fatherN1;

        if (UG::D2::ReadCW(n0, UG::D2::NTYPE_CE) == UG::D2::CORNER_NODE
            && UG::D2::ReadCW(n1, UG::D2::NTYPE_CE) == UG::D2::MID_NODE) {

          // n0 exists on the coarser level, but n1 doesn't
          const typename UG_NS<dim>::Edge* fatherEdge = (const typename UG_NS<dim>::Edge*)n1->father;
          fatherN0 = fatherEdge->links[0].nbnode;
          fatherN1 = fatherEdge->links[1].nbnode;

        } else if (UG::D2::ReadCW(n0, UG::D2::NTYPE_CE) == UG::D2::MID_NODE
                   && UG::D2::ReadCW(n1, UG::D2::NTYPE_CE) == UG::D2::CORNER_NODE) {

          // n1 exists on the coarser level, but n0 doesn't
          const typename UG_NS<dim>::Edge* fatherEdge = (const typename UG_NS<dim>::Edge*)n0->father;
          fatherN0 = fatherEdge->links[0].nbnode;
          fatherN1 = fatherEdge->links[1].nbnode;

        } else if (UG::D2::ReadCW(n0, UG::D2::NTYPE_CE) == UG::D2::CORNER_NODE
                   && UG::D2::ReadCW(n1, UG::D2::NTYPE_CE) == UG::D2::CORNER_NODE) {

          // This edge is a copy
          fatherN0 = (const typename UG_NS<dim>::Node*)n0->father;
          fatherN1 = (const typename UG_NS<dim>::Node*)n1->father;

        } else
          DUNE_THROW(GridError, "2d edge with two MID_NODES found!");

        // Find the corresponding side on the father element
        int i;
        for (i=0; i<UG_NS<dim>::Sides_Of_Elem(father); i++) {
          if ( (fatherN0 == UG_NS<dim>::Corner(father,UG_NS<dim>::Corner_Of_Side(father, i, 0))
                && fatherN1 == UG_NS<dim>::Corner(father,UG_NS<dim>::Corner_Of_Side(father, i, 1)))
               || (fatherN0 == UG_NS<dim>::Corner(father,UG_NS<dim>::Corner_Of_Side(father, i, 1))
                   && fatherN1 == UG_NS<dim>::Corner(father,UG_NS<dim>::Corner_Of_Side(father, i, 0))))
            break;
        }

        assert (i<UG_NS<dim>::Sides_Of_Elem(father));
        fatherSide = i;

      } else {

        // Get the nodes
        int nNodes = UG_NS<dim>::Corners_Of_Side(me,side);
        const typename UG_NS<dim>::Node* n[nNodes];
        for (int i=0; i<nNodes; i++)
          n[i] = UG_NS<dim>::Corner(me,UG_NS<dim>::Corner_Of_Side(me, side, i));

        const typename UG_NS<dim>::Node* fatherNode[4];          // No more than four father nodes

        //                 printf("NodeTypes: %d %d %d %d\n",
        //                        UG::D2::ReadCW(n[0], UG::D2::NTYPE_CE),
        //                        UG::D2::ReadCW(n[1], UG::D2::NTYPE_CE),
        //                        UG::D2::ReadCW(n[2], UG::D2::NTYPE_CE),
        //                        (nNodes==4) ? UG::D2::ReadCW(n[3], UG::D2::NTYPE_CE) : -1);

        /** \todo This is no good.  There are too many cases */
        if (UG::D3::ReadCW(n[0], UG::D3::NTYPE_CE) == UG::D3::CORNER_NODE
            && UG::D3::ReadCW(n[1], UG::D3::NTYPE_CE) == UG::D3::CORNER_NODE
            && UG::D3::ReadCW(n[2], UG::D3::NTYPE_CE) == UG::D3::CORNER_NODE
            && ((nNodes < 4) || UG::D3::ReadCW(n[3], UG::D3::NTYPE_CE) == UG::D3::CORNER_NODE)) {

          for (int i=0; i<nNodes; i++)
            fatherNode[i] = (const typename UG_NS<dim>::Node*)n[i]->father;
        } else {
          //DUNE_THROW(NotImplemented,"for 3d");
          // For conforming grids we know that we're at a domain boundary
          // when arriving here.
          leafSubFaces_[0] = Face(NULL, 0);
          return;
        }

        // Find the corresponding side on the father element
        int i;
        for (i=0; i<UG_NS<dim>::Sides_Of_Elem(father); i++) {
          int found = 0;
          for (int j=0; j<nNodes; j++)
            for (int k=0; k<UG_NS<dim>::Corners_Of_Side(father,i); k++)
              if (fatherNode[j] ==UG_NS<dim>::Corner(father,UG_NS<dim>::Corner_Of_Side(father, i, k))) {
                found++;
                break;
              }

          if (found==nNodes) {
            fatherSide = i;
            break;
          }

        }
      }

      // Do we have a neighbor on across this side?
      typename UG_NS<dim>::Element* otherElement = UG_NS<dim>::NbElem(father, fatherSide);
      if (otherElement) {
        const int nSides = UG_NS<dim>::Sides_Of_Elem(otherElement);
        for (int i=0; i<nSides; i++)
          if (UG_NS<dim>::NbElem(otherElement,i) == father) {
            leafSubFaces_[0] = Face(otherElement, i);
            return;
          }

      }

      // Go further down
      me     = father;
      father = UG_NS<dim>::EFather(father);
      side   = fatherSide;

    }

    // Nothing found
    leafSubFaces_[0] = Face(NULL, 0);
    return;
  }


  // ///////////////
  //   init list
  // ///////////////

  SLList<Face> list;
  int levelNeighborSide = numberInNeighbor(center_, levelNeighbor);

  int Sons_of_Side = 0;
  typename UG_NS<dim>::Element* SonList[UG_NS<dim>::MAX_SONS];
  int SonSides[UG_NS<dim>::MAX_SONS];

  int rv = Get_Sons_of_ElementSide(levelNeighbor,
                                   levelNeighborSide,
                                   &Sons_of_Side,
                                   SonList,        // the output elements
                                   SonSides,       // Output element side numbers
                                   true,          // Element sons are not precomputed
                                   false,          // ioflag: Obsolete debugging flag
                                   true);

  if (rv!=0)
    DUNE_THROW(GridError, "Get_Sons_of_ElementSide returned with error value " << rv);

  for (int i=0; i<Sons_of_Side; i++)
    list.push_back(Face(SonList[i],SonSides[i]));

  // //////////////////////////////////////////////////
  //   Traverse and collect all children of the side
  // //////////////////////////////////////////////////

  typename SLList<Face>::iterator f = list.begin();
  for (; f!=list.end(); ++f) {

    typename UG_NS<dim>::Element* theElement = f->first;

    int Sons_of_Side = 0;
    typename UG_NS<dim>::Element* SonList[UG_NS<dim>::MAX_SONS];
    int SonSides[UG_NS<dim>::MAX_SONS];

    if (!UG_NS<dim>::isLeaf(theElement)) {

      Get_Sons_of_ElementSide(theElement,
                              f->second,        // Input element side number
                              &Sons_of_Side,       // Number of topological sons of the element side
                              SonList,            // Output elements
                              SonSides,           // Output element side numbers
                              true,
                              false,
                              true);

      for (int i=0; i<Sons_of_Side; i++)
        list.push_back(Face(SonList[i],SonSides[i]));

    }

  }

  // //////////////////////////////
  //   Extract result from stack
  // //////////////////////////////
  leafSubFaces_.resize(0);

  for (f = list.begin(); f!=list.end(); ++f) {

    // Set element
    if (UG_NS<dim>::isLeaf(f->first))
      leafSubFaces_.push_back(*f);

  }

}

// Explicit template instantiations to compile the stuff in this file

template class Dune::UGGridLevelIntersectionIterator<const Dune::UGGrid<2> >;
template class Dune::UGGridLevelIntersectionIterator<const Dune::UGGrid<3> >;

template class Dune::UGGridLeafIntersectionIterator<const Dune::UGGrid<2> >;
template class Dune::UGGridLeafIntersectionIterator<const Dune::UGGrid<3> >;
