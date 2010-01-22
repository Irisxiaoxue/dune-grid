## -*- autoconf -*-
AC_DEFUN([DUNE_GRID_CHECKS],[
  AC_REQUIRE([DUNE_GRID_DIMENSION])
  AC_REQUIRE([DUNE_PATH_GRAPE])
  AC_REQUIRE([DUNE_PATH_ALBERTA])
  AC_REQUIRE([DUNE_PATH_UG])
  AC_REQUIRE([DUNE_PATH_AMIRAMESH])
  AC_REQUIRE([DUNE_PATH_PSURFACE])
  AC_REQUIRE([DUNE_PATH_ALUGRID])
  AC_REQUIRE([DUNE_OLD_NUMBERING])
  AC_REQUIRE([DUNE_PATH_ALGLIB])
])

AC_DEFUN([DUNE_GRID_CHECK_MODULE],[
  DUNE_CHECK_MODULES([dune-grid], [grid/onedgrid.hh],[dnl
  std::vector<Dune::OneDGrid::ctype> coords;
  Dune::OneDGrid grid(coords);
  return grid.lbegin<0>(0) == grid.lend<0>(0);])
])
