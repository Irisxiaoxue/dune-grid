// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALU3DINCLUDE_HH
#define DUNE_ALU3DINCLUDE_HH

#ifndef _ANSI_HEADER
#define _ANSI_HEADER
#endif

//#define _ALU3DGRID_PARALLEL_

// all methods and classes of the ALU3d-Grid are defined in the namespace
#define ALU3DSPACE ALU3dGridSpace ::

// if this is defined in bsgrid some methods are added which we only need
// for the Dune interface
#define _DUNE_USES_ALU3DGRID_

// if MPI was found include all headers
#ifdef _ALU3DGRID_PARALLEL_
#include <mpi.h>
#include <alu3dgrid_parallel.h>
#else
// if not, include only headers for serial version
#include <alu3dgrid_serial.h>
#endif
#undef _DUNE_USES_ALU3DGRID_

namespace ALU3dGridSpace {


#ifdef _ALU3DGRID_PARALLEL_

  typedef GatherScatter GatherScatterType;

  typedef GitterDunePll GitterType;
  typedef GitterDunePll GitterImplType;

  typedef Hbnd3PllInternal<GitterType::Objects::Hbnd3Default,
      BndsegPllBaseXClosure<GitterType::hbndseg3_GEO>,
      BndsegPllBaseXMacroClosure<GitterType::hbndseg3_GEO> > :: micro_t MicroType;

  // value for boundary to other processes
  static const int ProcessorBoundary_t = GitterImplType::hbndseg_STI::closure;

#else
  typedef GatherScatter GatherScatterType;

  // the header
  typedef Gitter GitterType;
  typedef GitterDuneImpl GitterImplType;
  typedef GitterType::hface_STI PLLFaceType;                     // Interface Face

#endif

  // rule for Dune LeafIterator
#include "leafwalk.hh"

  // typedefs of Element types
  typedef GitterType::helement_STI HElementType;               // Interface Element
  typedef GitterType::hface_STI HFaceType;                     // Interface Face
  typedef GitterType::hedge_STI HEdgeType;                     // Interface Edge
  typedef GitterType::vertex_STI VertexType;                   // Interface Vertex
  typedef GitterType::Geometric::hface3_GEO GEOFace3Type;     // Tetra Face
  typedef GitterType::Geometric::hface4_GEO GEOFace4Type; // Hexa Face
  typedef GitterType::Geometric::hedge1_GEO GEOEdgeT;     // * stays real Face
  typedef GitterType::Geometric::VertexGeo GEOVertexT;     // * stays real Face
  typedef GitterImplType::Objects::tetra_IMPL IMPLTetraElementType; //impl Element
  typedef GitterImplType::Objects::hexa_IMPL IMPLHexaElementType;
  typedef GitterType::Geometric::tetra_GEO GEOTetraElementType;  // real Element
  typedef GitterType::Geometric::hexa_GEO GEOHexaElementType;
  typedef GitterType::Geometric::hasFace3 HasFace3Type;    // has Face with 3 polygons
  typedef GitterType::Geometric::hasFace4 HasFace4Type;
  typedef GitterType::Geometric::Hface3Rule Hface3RuleType;
  typedef GitterType::Geometric::Hface4Rule Hface4RuleType;

  typedef GitterImplType::Objects::Hbnd3Default BNDFace3Type;    // boundary segment
  typedef GitterImplType::Objects::Hbnd4Default BNDFace4Type;
  typedef GitterImplType::Objects::hbndseg3_IMPL ImplBndFace3Type;    // boundary segment
  typedef GitterImplType::Objects::hbndseg4_IMPL ImplBndFace4Type;

  // * end new

  //*************************************************************
  //  definition of original LeafIterators of ALU3dGrid
  //
  // default is element (codim = 0)
  template <int codim>
  struct BSMacroIterator
  {
    typedef AccessIterator<GitterType::helement_STI>::Handle IteratorType;
  };

  //******************************************************************
  //  LevelIterators
  //******************************************************************
  template <int codim> struct ALUHElementType
  {
    typedef GitterType :: helement_STI ElementType;
  };

  template <> struct ALUHElementType<0> {
    typedef GitterType :: helement_STI ElementType;
  };
  template <> struct ALUHElementType<1> {
    typedef GitterType :: hface_STI ElementType;
  };
  template <> struct ALUHElementType<2> {
    typedef GitterType :: hedge_STI ElementType;
  };
  template <> struct ALUHElementType<3> {
    typedef GitterType :: vertex_STI ElementType;
  };

  template <int codim> struct BSIMPLElementType
  {
    typedef GitterImplType::Objects::tetra_IMPL ElementType; // impl Element
  };

  template <> struct BSIMPLElementType<0> {
    typedef GitterImplType::Objects::tetra_IMPL ElementType; // impl Element
  };
  template <> struct BSIMPLElementType<1> {
    typedef GitterImplType::Objects::hface3_IMPL ElementType; // impl Element
  };
  template <> struct BSIMPLElementType<2> {
    typedef GitterImplType::Objects::hedge1_IMPL ElementType; // impl Element
  };

  template <> struct BSIMPLElementType<3> {
    typedef GitterType::Geometric::VertexGeo ElementType;
  };

  //*********************************************************
  //  LevelIterator Wrapper
  //*********************************************************
  template <int codim> class ALU3dGridLevelIteratorWrapper;
  template <>
  class ALU3dGridLevelIteratorWrapper<0>
  {
    typedef ALUHElementType<0>::ElementType ElType;
    typedef Insert < AccessIterator < ElType >::Handle ,
        TreeIterator  < ElType ,  any_has_level < ElType > > > IteratorType;

    IteratorType it_;
    typedef IteratorType :: val_t val_t;
  public:
    template <class GridImp>
    ALU3dGridLevelIteratorWrapper (const GridImp & grid, int level )
      : it_(const_cast<GridImp &> (grid).myGrid().container(),level) {}

    int size  ()    { return it_.size(); }
    void next ()    { it_.next(); }
    void first()    { it_.first(); }
    int done ()     { return it_.done(); }
    val_t & item () { return it_.item(); }

  };

  template <>
  class ALU3dGridLevelIteratorWrapper<1>
  {
    typedef ALUHElementType<1>::ElementType ElType;
    typedef Insert < AccessIterator < ElType >::Handle ,
        TreeIterator  < ElType ,  any_has_level < ElType > > > IteratorType;

    IteratorType it_;
    typedef IteratorType :: val_t val_t;
  public:
    template <class GridImp>
    ALU3dGridLevelIteratorWrapper (const GridImp & grid, int level )
      : it_(const_cast<GridImp &> (grid).myGrid().container(),level) {}

    int size  ()    { return it_.size(); }
    void next ()    { it_.next(); }
    void first()    { it_.first(); }
    int done ()     { return it_.done(); }
    val_t & item () { return it_.item(); }

  };

  template <>
  class ALU3dGridLevelIteratorWrapper<2>
  {
    typedef ALUHElementType<2>::ElementType ElType;
    typedef Insert < AccessIterator < ElType >::Handle ,
        TreeIterator  < ElType ,  any_has_level < ElType > > > IteratorType;

    IteratorType it_;
    typedef IteratorType :: val_t val_t;
  public:
    template <class GridImp>
    ALU3dGridLevelIteratorWrapper (const GridImp & grid, int level )
      : it_(const_cast<GridImp &> (grid).myGrid().container(),level) {}

    int size  ()    { return it_.size(); }
    void next ()    { it_.next(); }
    void first()    { it_.first(); }
    int done ()     { return it_.done(); }
    val_t & item () { return it_.item(); }

  };

  template <>
  class ALU3dGridLevelIteratorWrapper<3>
  {
    typedef LeafIterator < GitterType::vertex_STI > IteratorType;

    IteratorType it_;
    typedef IteratorType :: val_t val_t;

    // level to walk
    int level_;
  public:
    template <class GridImp>
    ALU3dGridLevelIteratorWrapper (const GridImp & grid, int level )
      : it_(const_cast<GridImp &> (grid).myGrid()),
        level_(level)  {}

    int size  ()  { return it_->size(); }

    //! if level of item is larger then walk level, go next
    void next ()
    {
      it_->next();
      if(it_->done()) return ;
      if(it_->item().level() > level_) this->next();
      return ;
    }
    void first()    { it_->first(); }
    int done () const { return it_->done(); }
    val_t & item () { return it_->item(); }

  };

  template <int codim> class ALU3dGridLeafIteratorWrapper;
  //**********************************************************
  //  LeafIterator Wrapper
  //**********************************************************
  template <>
  class ALU3dGridLeafIteratorWrapper<0>
  {
    typedef ALUHElementType<0>::ElementType ElType;
    typedef Insert < AccessIterator < ElType >::Handle,
        TreeIterator < ElType , leaf_or_has_level < ElType > > > IteratorType;

    IteratorType it_;
    typedef IteratorType :: val_t val_t;
  public:
    template <class GridImp>
    ALU3dGridLeafIteratorWrapper (const GridImp & grid, int level )
      : it_(const_cast<GridImp &> (grid).myGrid().container(),level) {}

    int size  ()    { return it_.size(); }
    void next ()    { it_.next(); }
    void first()    { it_.first(); }
    int done ()     { return it_.done(); }
    val_t & item () { return it_.item(); }

  };

  template <>
  class ALU3dGridLeafIteratorWrapper<1>
  {
    typedef ALUHElementType<1>::ElementType ElType;
    typedef Insert < AccessIterator < ElType >::Handle,
        TreeIterator < ElType , leaf_or_has_level < ElType > > > IteratorType;

    IteratorType it_;
    typedef IteratorType :: val_t val_t;
  public:
    template <class GridImp>
    ALU3dGridLeafIteratorWrapper (const GridImp & grid, int level )
      : it_(const_cast<GridImp &> (grid).myGrid().container(),level) {}

    int size  ()    { return it_.size(); }
    void next ()    { it_.next(); }
    void first()    { it_.first(); }
    int done ()     { return it_.done(); }
    val_t & item () { return it_.item(); }

  };

  template <>
  class ALU3dGridLeafIteratorWrapper<2>
  {
    typedef ALUHElementType<2>::ElementType ElType;
    typedef Insert < AccessIterator < ElType >::Handle,
        TreeIterator < ElType , leaf_or_has_level < ElType > > > IteratorType;

    IteratorType it_;
    typedef IteratorType :: val_t val_t;
  public:
    template <class GridImp>
    ALU3dGridLeafIteratorWrapper (const GridImp & grid, int level )
      : it_(const_cast<GridImp &> (grid).myGrid().container(),level) {}

    int size  ()    { return it_.size(); }
    void next ()    { it_.next(); }
    void first()    { it_.first(); }
    int done ()     { return it_.done(); }
    val_t & item () { return it_.item(); }

  };

  template <>
  class ALU3dGridLeafIteratorWrapper<3>
  {
    typedef LeafIterator < GitterType::vertex_STI > IteratorType;
    IteratorType it_;
    typedef IteratorType :: val_t val_t;
  public:
    template <class GridImp>
    ALU3dGridLeafIteratorWrapper (const GridImp & grid, int level )
      : it_(const_cast<GridImp &> (grid).myGrid()) {}

    int size  ()    { return it_->size(); }
    void next ()    { it_->next(); }
    void first()    { it_->first(); }
    int done ()     { return it_->done(); }
    val_t & item () { return it_->item(); }

  };

  //*************************************************************
  typedef LeafIterator < GitterType::helement_STI > BSLeafIteratorMaxLevel;

} //end namespace BernhardSchuppGrid

#endif
