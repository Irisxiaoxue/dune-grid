// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
namespace Dune
{

  template<>
  inline ALUSimplexGrid< 3, 3 > *MacroGrid :: Impl< ALUSimplexGrid< 3, 3 > >
  :: generate ( MacroGrid &macroGrid,
                const char *filename,
                MPICommunicatorType communicator )
  {
    macroGrid.element = Simplex;

    int rank = 0;
#if ALU3DGRID_PARALLEL
    MPI_Comm_rank( communicator, &rank );
#endif

    std :: ifstream file( filename );
    if( macroGrid.readDuneGrid( file ) )
    {
      if( macroGrid.dimw != 3 )
        DUNE_THROW( DGFException, "Macrofile " << filename << " is for "
                                               << "dimension " << macroGrid.dimw
                                               << " and cannot be used to initialize an "
                                               << "ALUGrid of dimension 3." );
      macroGrid.setOrientation( 2, 3 );

      GridFactory< ALUSimplexGrid< 3, 3 > > factory( communicator );
      if( rank == 0 )
      {
        for( int n = 0; n < macroGrid.nofvtx; ++n )
        {
          FieldVector< double, 3 > pos;
          for( unsigned int i = 0; i < 3; ++i )
            pos[ i ] = macroGrid.vtx[ n ][ i ];
          factory.insertVertex( pos );
        }

        GeometryType elementType( GeometryType :: simplex, 3 );
        for( int n = 0; n < macroGrid.nofelements; ++n )
          factory.insertElement( elementType, macroGrid.elements[ n ] );

        GeometryType faceType( GeometryType :: simplex, 2 );
        for( facemap_t :: iterator it = macroGrid.facemap.begin();
             it != macroGrid.facemap.end(); ++it )
        {
          const unsigned int keySize = it->first.size();
          std :: vector< unsigned int > key( keySize );
          for( unsigned int i = 0; i < keySize; ++i )
          {
            key[ FaceTopologyMapping< tetra > :: alu2duneVertex( i ) ]
              = it->first.origKey( i );
          }
          factory.insertBoundaryId( faceType, key, it->second );
        }
      }

      return factory.createGrid();
    }
    else
    {
#if ALU3DGRID_PARALLEL
      std :: ostringstream tmps;
      tmps << filename << "." << rank;
      const std :: string &tmp = tmps.str();
#else
      std :: string tmp( filename );
#endif
      std :: ifstream testfile( tmp.c_str() );
      if( testfile )
      {
        testfile.close();
        return new ALUSimplexGrid< 3, 3 >( tmp.c_str(), communicator );
      }
    }
    DUNE_THROW( GridError, "Unable to create ALUSimplexGrid< 3, 3 > from '"
                << filename << "'." );
  }



  template<>
  inline ALUCubeGrid< 3, 3 > *MacroGrid :: Impl< ALUCubeGrid< 3, 3 > >
  :: generate ( MacroGrid &macroGrid,
                const char *filename,
                MPICommunicatorType communicator )
  {
    macroGrid.element = Cube;

    int rank = 0;
#if ALU3DGRID_PARALLEL
    MPI_Comm_rank( communicator, &rank );
#endif

    std :: ifstream file( filename );
    if( macroGrid.readDuneGrid( file ) )
    {
      if( macroGrid.dimw != 3 )
        DUNE_THROW( DGFException, "Macrofile " << filename << " is for "
                                               << "dimension " << macroGrid.dimw
                                               << " and cannot be used to initialize an "
                                               << "ALUGrid of dimension 3." );

      GridFactory< ALUCubeGrid< 3, 3 > > factory( communicator );
      if( rank == 0 )
      {
        for( int n = 0; n < macroGrid.nofvtx; ++n )
        {
          FieldVector< double, 3 > pos;
          for( unsigned int i = 0; i < 3; ++i )
            pos[ i ] = macroGrid.vtx[ n ][ i ];
          factory.insertVertex( pos );
        }

        GeometryType elementType( GeometryType :: cube, 3 );
        for( int n = 0; n < macroGrid.nofelements; ++n )
          factory.insertElement( elementType, macroGrid.elements[ n ] );

        GeometryType faceType( GeometryType :: cube, 2 );
        for( facemap_t :: iterator it = macroGrid.facemap.begin();
             it != macroGrid.facemap.end(); ++it )
        {
          const unsigned int keySize = it->first.size();
          std :: vector< unsigned int > key( keySize );
          for( unsigned int i = 0; i < keySize; ++i )
          {
            key[ FaceTopologyMapping< hexa > :: alu2duneVertex( i ) ]
              = it->first.origKey( i );
          }
          factory.insertBoundaryId( faceType, key, it->second );
        }
      }

      return factory.createGrid();
    }
    else
    {
#if ALU3DGRID_PARALLEL
      std :: stringstream tmps;
      tmps << filename << "." << rank;
      const std :: string &tmp = tmps.str();
#else
      std :: string tmp( filename );
#endif
      std :: ifstream testfile( tmp.c_str() );
      if( testfile )
      {
        testfile.close();
        return new ALUCubeGrid< 3, 3 >( tmp.c_str(), communicator );
      }
    }
    DUNE_THROW( GridError, "Unable to create ALUSimplexGrid< 3, 3 > from '"
                << filename << "'." );
  }



  // Previous implementations
  // ------------------------

#if 0
  template <>
  inline ALUSimplexGrid<3,3>*
  MacroGrid :: Impl<ALUSimplexGrid<3,3> >::
  generate(MacroGrid& mg,const char* filename, MPICommunicatorType MPICOMM )
  {
    mg.element=Simplex;
    std::string str(filename);
    std::string fn(filename);

#if ALU3DGRID_PARALLEL
    int myrank;
    MPI_Comm_rank(MPICOMM,&myrank);
    if(myrank == 0)
    {
#endif
    // note: in parallel only on proc 0 macro grid is generated
    MacroGrid::Impl<ALUSimplexGrid<3,3> >().
    generateAlu3d(mg,filename,str,MPICOMM);
#if ALU3DGRID_PARALLEL
  }

  // if equality, no ALUGrid file was generated from DGF file
  // this means we try to read parallel ALUGrid macro file
  if( (str == fn) )
  {
    std::stringstream tmp;
    // append filename by rank
    tmp << str << "." << myrank;

    // test if file exists of rank is zero
    std::ifstream testfile(tmp.str().c_str());
    if( testfile )
    {
      testfile.close();
      return new ALUSimplexGrid<3,3>(tmp.str().c_str(),MPICOMM);
    }
  }

  // otherwise proceed as normal
  // if rank 0 then return generated grid
  if ( myrank == 0 )
  {
    return new ALUSimplexGrid<3,3>(str.c_str(),MPICOMM);
  }
  else
  {
    return new ALUSimplexGrid<3,3>(MPICOMM);
  }
#else
    return new ALUSimplexGrid<3,3>(str.c_str());
#endif
}
#endif

#if 0
  template <>
  inline ALUCubeGrid<3,3>*
  MacroGrid :: Impl<ALUCubeGrid<3,3> >::generate
    (MacroGrid& mg,const char* filename, MPICommunicatorType MPICOMM )
  {
    mg.element=Cube;
    std::string str(filename);
    std::string fn(filename);

#if ALU3DGRID_PARALLEL
    int myrank;
    MPI_Comm_rank(MPICOMM,&myrank);
    if(myrank <= 0)
    {
#endif
    // note: in parallel only on proc 0 macro grid is generated
    MacroGrid::Impl<ALUCubeGrid<3,3> >().
    generateAlu3d(mg,filename,str,MPICOMM);
#if ALU3DGRID_PARALLEL
  }

  // if equality, no ALUGrid file was generated from DGF file
  // this means we try to read parallel ALUGrid macro file
  if( (str == fn) )
  {
    std::stringstream tmp;
    // append filename by rank
    tmp << str << "." << myrank;

    // test if file exists of rank is zero
    std::ifstream testfile(tmp.str().c_str());
    if( testfile )
    {
      testfile.close();
      return new ALUCubeGrid<3,3>(tmp.str().c_str(),MPICOMM);
    }
  }

  // otherwise proceed as normal
  // if rank 0 then return generated grid
  if ( myrank == 0 )
  {
    return new ALUCubeGrid<3,3>(str.c_str(),MPICOMM);
  }
  else
  {
    return new ALUCubeGrid<3,3>(MPICOMM);
  }
#else
    return new ALUCubeGrid<3,3>(str.c_str());
#endif
}
#endif

  template <>
  inline ALUSimplexGrid<2,2>*
  MacroGrid :: Impl<ALUSimplexGrid<2,2> >::
  generate(MacroGrid& mg,const char* filename, MPICommunicatorType MPICOMM )
  {
    mg.element=Simplex;
    std::string str(filename);
    MacroGrid::Impl<ALUSimplexGrid<2,2> >().
    generateAlu3d(mg,filename,str,MPICOMM);
    return new ALUSimplexGrid<2,2>(str.c_str());
  }

  /* needs newer version of alulib */
  template <>
  inline ALUConformGrid<2,2>*
  MacroGrid :: Impl<ALUConformGrid<2,2> >::generate
    (MacroGrid& mg,const char* filename, MPICommunicatorType MPICOMM ) {
    mg.element=Simplex;
    std::string str(filename);
    MacroGrid::Impl<ALUSimplexGrid<2,2> >().
    generateAlu3d(mg,filename,str,MPICOMM);
    return new ALUConformGrid<2,2>(str.c_str());
  }

  template <int dim,int dimworld>
  inline void
  MacroGrid :: Impl<ALUSimplexGrid<dim,dimworld> > ::
  generateAlu3d(MacroGrid& mg,const char* filename,
                std::string& str, MPICommunicatorType MPICOMM )
  {
    std::ifstream gridin(filename);
    if(mg.readDuneGrid(gridin))
    {
      if (mg.dimw != dimworld)
      {
        DUNE_THROW(DGFException,
                   "Macrofile " << filename << " is for dimension " << mg.dimw
                                << " and connot be used to initialize an ALUGrid of dimension "
                                << dimworld);
      }
      mg.setOrientation(dimworld-1,dimworld);
      // mg.setRefinement(dimworld-1,dimworld,-1,-1);
      str+=".ALUgrid";
      std::ofstream out(str.c_str());
      mg.writeAlu(out);
    }
  }

  template <int dim,int dimworld>
  inline void
  MacroGrid :: Impl<ALUCubeGrid<dim,dimworld> > :: generateAlu3d
    (MacroGrid& mg,const char* filename, std::string& str, MPICommunicatorType MPICOMM )
  {
    std::ifstream gridin(filename);
    if(mg.readDuneGrid(gridin))
    {
      if (mg.dimw != dimworld)
      {
        DUNE_THROW(DGFException,
                   "Macrofile " << filename << " is for dimension " << mg.dimw
                                << " and connot be used to initialize an ALUGrid of dimension "
                                << dimworld);
      }
      str+=".ALUgrid";
      std::ofstream out(str.c_str());
      mg.writeAlu(out);
    }
  }

} // end namespace Dune
