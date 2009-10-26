// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>

#include <memory>

#include <dune/grid/albertagrid.hh>
#include <dune/grid/io/file/gmshreader.hh>

#if HAVE_GRAPE
#include <dune/grid/io/visual/grapegriddisplay.hh>
#endif

#ifndef GRIDDIM
#define GRIDDIM ALBERTA_DIM
#endif

typedef Dune::AlbertaGrid< GRIDDIM > GridType;

int main ( int argc, char **argv )
try
{
  if( argc < 2 )
  {
    std::cerr << "Usage: " << argv[ 0 ] << " <gmshfile> [refinements]" << std::endl;
    return 1;
  }

  std::auto_ptr< GridType > grid( Dune::GmshReader< GridType >::read( argv[ 1 ] ) );
  if( argc >= 3 )
    grid->globalRefine( atoi( argv[ 2 ] ) );

#if HAVE_GRAPE
  Dune::GrapeGridDisplay< GridType > grape( *grid );
  grape.display();
#endif

  return 0;
}
catch( const Dune::Exception &e )
{
  std::cerr << "Error: " << e << std::endl;
  return 1;
}
catch( ... )
{
  std::cerr << "Error: Generic exception." << std::endl;
  return 2;
}