// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <algorithm>

#include <dune/grid/io/file/dgfparser/entitykey.hh>

namespace Dune
{

  // DGFEntityKey
  // ------------

  template< class A >
  DGFEntityKey< A > :: DGFEntityKey ( const std :: vector< A > &key, bool setOrigKey )
    : key_( key.size() ),
      origKey_( key.size() ),
      origKeySet_( setOrigKey )
  {
    for (size_t i=0; i<key_.size(); i++)
    {
      key_[i]=key[i];
      origKey_[i]=key_[i];
    }
    std :: sort( key_.begin(), key_.end() );
  }


  template< class A >
  DGFEntityKey< A > :: DGFEntityKey ( const std :: vector< A > &key,
                                      int N, int offset, bool setOrigKey )
    : key_( N ),
      origKey_( N ),
      origKeySet_( setOrigKey )
  {
    for (size_t i=0; i<key_.size(); i++)
    {
      key_[i]=key[(i+offset)%key.size()];
      origKey_[i]=key[(i+offset)%key.size()];
    }
    std :: sort( key_.begin(), key_.end() );
  }


  template< class A >
  DGFEntityKey< A > :: DGFEntityKey ( const DGFEntityKey< A > &k )
    : key_( k.key_.size() ),
      origKey_( k.key_.size() ),
      origKeySet_( k. origKeySet_ )
  {
    for (size_t i=0; i<key_.size(); i++)
    {
      key_[i]=k.key_[i];
      origKey_[i]=k.origKey_[i];
    }
  }


  template< class A >
  DGFEntityKey< A > &DGFEntityKey< A > :: operator= ( const DGFEntityKey< A > &k )
  {
    assert(key_.size()==k.key_.size());
    for (size_t i=0; i<key_.size(); i++) {
      key_[i]=k.key_[i];
      origKey_[i]=k.origKey_[i];
    }
    origKeySet_ = k.origKeySet_;
    return *this;
  }


  template< class A >
  void DGFEntityKey< A >
  :: orientation ( int base, std::vector< std :: vector< double > > &vtx )
  {
    if (key_.size()==3)  {
      assert( (size_t) origKey_[0] < vtx.size() );
      std::vector<double>& p0 = vtx[origKey_[0]];
      assert( (size_t) origKey_[1] < vtx.size() );
      std::vector<double>& p1 = vtx[origKey_[1]];
      assert( (size_t) origKey_[2] < vtx.size() );
      std::vector<double>& p2 = vtx[origKey_[2]];
      assert( (size_t) base < vtx.size() );
      std::vector<double>& q  = vtx[base];
      double n[3];
      n[0] = (p1[1]-p0[1])*(p2[2]-p0[2])-(p2[1]-p0[1])*(p1[2]-p0[2]);
      n[1] = (p1[2]-p0[2])*(p2[0]-p0[0])-(p2[2]-p0[2])*(p1[0]-p0[0]);
      n[2] = (p1[0]-p0[0])*(p2[1]-p0[1])-(p2[0]-p0[0])*(p1[1]-p0[1]);
      double test = n[0]*(q[0]-p0[0])+n[1]*(q[1]-p0[1])+n[2]*(q[2]-p0[2]);
      bool reorient = (test>0);
      if (reorient) {
        A key1=origKey_[1];
        origKey_[1]=origKey_[2];
        origKey_[2]=key1;
      }
    }
  }


  template< class A >
  void DGFEntityKey< A > :: print ( std :: ostream &out ) const
  {
    for( size_t i = 0; i < key_.size(); ++i )
      out << key_[ i ] << " ";
    out << std :: endl;
  }



  // ElementFaceUtil
  // ---------------

  template< int dimworld >
  DGFEntityKey< unsigned int >
  ElementFaceUtil :: generateCubeFace
    ( const std :: vector< unsigned int > &element, int f )
  {
    ReferenceCube< double, dimworld > ref;
    const unsigned int size = ref.size( f, 1, dimworld );
    std :: vector< unsigned int > k( size );
    /*
       for (int i=0;i<size;i++) {
       k[i] = element[ref.subEntity(f,1,i,dimworld)];
       }
       if (dimworld==3) {
       if (f==2 || f==1 || f==5) {
        int ktmp=k[0];
        k[0]=k[1];
        k[1]=ktmp;
       }
       else {
        int ktmp=k[2];
        k[2]=k[3];
        k[3]=ktmp;
       }
       }
     */
    const int face = ElementTopologyMapping< hexa > :: dune2aluFace( f );
    for( unsigned int i = 0; i < size; ++i )
    {
      // int idxdune = ref.subEntity(f,1,i,dimworld);
      int idx = ElementTopologyMapping< hexa > :: alu2duneFaceVertex( face, i );
      int idxdune = ref.subEntity( f, 1, idx, dimworld );
      k[ size - (i+1) ] = element[ idxdune ];
    }
    return DGFEntityKey< unsigned int >( k );
  }


  template< int dimworld >
  DGFEntityKey< unsigned int >
  ElementFaceUtil :: generateSimplexFace
    ( const std :: vector< unsigned int > &element, int f )
  {
    ReferenceSimplex< double, dimworld > ref;
    const unsigned int size = ref.size( f, 1, dimworld );
    std :: vector< unsigned int > k( size );
    for( unsigned int i = 0; i < size; ++i )
      k[ i ] = element[ ref.subEntity( f, 1, i, dimworld ) ];
    return DGFEntityKey< unsigned int >( k );
  }


  DGFEntityKey< unsigned int >
  ElementFaceUtil :: generateFace
    ( int dimw, const std :: vector< unsigned int > &element, int f )
  {
    if (element.size()==size_t(dimw+1)) { // Simplex element
      if (dimw==3)
        return generateSimplexFace<3>(element,f);
      else if (dimw==2)
        return generateSimplexFace<2>(element,f);
      else if (dimw==1)
        return generateSimplexFace<1>(element,f);
    }
    else { // Cube element
      if (dimw==3)
        return generateCubeFace<3>(element,f);
      else if (dimw==2)
        return generateCubeFace<2>(element,f);
      else if (dimw==1)
        return generateCubeFace<1>(element,f);
    }
    DUNE_THROW(DGFException,"WRONG DIMENSION");
    return generateCubeFace<1>(element,f);
  }



  // Instantiations
  // --------------

  template class DGFEntityKey< unsigned int >;

} // end namespace Dune
