if(ALBERTA_2D_FOUND)
  add_executable(dune-gridinfo-gmsh-alberta-2d ridinfo-gmsh-alberta-2d.cc)
  add_dune_alberta_flags(dune-gridinfo-gmsh-alberta-2d GRIDDIM 2)
  install(dune-gridinfo-gmsh-alberta-2d ${CMAKE_INSTALL_BINDIR})
endif(ALBERTA_FOUND)

if(ALBERTA_3D_FOUND)
  add_executable(dune-gridinfo-gmsh-alberta-3d ridinfo-gmsh-alberta-3d.cc)
  add_dune_alberta_flags(dune-gridinfo-gmsh-alberta-3d GRIDDIM 3)
  install(dune-gridinfo-gmsh-alberta-2d ${CMAKE_INSTALL_BINDIR})
endif(ALBERTA_FOUND)

if(ALUGRID_FOUND)
  add_executable(dune-gridinfo-gmsh-alucube-3d gridinfo-gmsh-alucube-3d.cc)
  add_dune_alugrid_flags(dune-gridinfo-gmsh-alucube-3d)
  install(dune-gridinfo-gmsh-alucube-3d ${CMAKE_INSTALL_BINDIR})

  add_executable(dune-gridinfo-gmsh-alusimplex-3d gridinfo-gmsh-alusimplex-3d.cc)
  add_dune_alugrid_flags(dune-gridinfo-gmsh-alusimplex-3d)
  install(dune-gridinfo-gmsh-alusimplex-3d ${CMAKE_INSTALL_BINDIR})
endif(ALUGRID_FOUND)

if(UG_FOUND)
  add_executable(dune-gridinfo-gmsh-ug-2d dune-gridinfo-gmsh-ug-2d.cc)
  add_dune_ug_flags(dune-gridinfo-gmsh-ug-2d)
  add_executable(dune-gridinfo-gmsh-ug-3d dune-gridinfo-gmsh-ug-3d.cc)
  add_dune_ug_flags(dune-gridinfo-gmsh-ug-3d)
  install(dune-gridinfo-gmsh-ug-2d dune-gridinfo-gmsh-ug-3d ${CMAKE_INSTALL_BINDIR})
endif(UG_FOUND)
