// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_COMBINEDGRAPEDISPLAY_HH
#define DUNE_COMBINEDGRAPEDISPLAY_HH

//- System includes
#include <list>
#include <vector>

//- Dune includes

#if HAVE_GRAPE
//- Local includes
#include "grapedatadisplay.hh"
#endif

/** @file
   @author Robert Kloefkorn
   @brief Combines several mesh scenes for visualisation of parallel data
 */

namespace Dune
{

  /*! \brief
   * Combines GrapeDataDisplays to one so that visualisation for parallel
   * grid is more easy to handle.
   */
  template<class DisplayType>
  class CombinedGrapeDisplay
  {

#if HAVE_GRAPE
    typedef CombinedGrapeDisplay < DisplayType > MyDisplayType;

    typedef typename DisplayType :: MyGridType GridType;
    enum { dim      = GridType :: dimension };
    enum { dimworld = GridType :: dimensionworld };

    // defined in griddisplay.hh
    typedef typename GrapeInterface<dim,dimworld>::DUNE_ELEM DUNE_ELEM;
    typedef typename GrapeInterface<dim,dimworld>::DUNE_FDATA DUNE_FDATA;
    typedef typename GrapeInterface<dim,dimworld>::DUNE_DAT DUNE_DAT;
    typedef typename GrapeInterface<dim,dimworld>::F_DATA F_DATA;
    typedef typename GrapeInterface<dim,dimworld>::STACKENTRY STACKENTRY;

  protected:
    typedef typename std::list< DisplayType * > DisplayListType;
    DisplayListType dispList_;

    typedef typename DisplayListType :: iterator DisplayListIteratorType;
    DisplayListIteratorType grditer_;
    DisplayListIteratorType enditer_;

    typedef typename std::list< void * > GridPartListType;
    GridPartListType gridPartList_;
    typedef typename GridPartListType :: iterator GridPartListIteratorType;
    GridPartListIteratorType partEnd_;
    GridPartListIteratorType partIter_;


    DUNE_ELEM * dhel_;

    // actual element data
    DUNE_ELEM hel_;

    // actual dat struct
    DUNE_DAT dune_;
#endif

    // pointer to actual display
    DisplayType * disp_;

  public:
    // no better way than this canot export HMESH structure to here
    //! pointer to hmesh
    void *hmesh_;

  public:
    //! Constructor, make a GrapeGridDisplay for given grid
    inline CombinedGrapeDisplay();

    //! Destructor for GrapeGridDisplay
    inline ~CombinedGrapeDisplay();

    //! add display to list of displays
    void addDisplay ( DisplayType & disp );

    //! Calls the display of the grid and draws the discrete function
    //! if discretefunction is NULL, then only the grid is displayed
    inline void display();

#if HAVE_GRAPE
    //! return pointer to Grape Hmesh
    inline void * getHmesh();

    inline void addMyMeshToGlobalTimeScene(double time, int proc);

  private:
    // generate hmesh
    inline void * setupHmesh();

    typedef typename DisplayType :: StackEntryType StackEntryType;
    StackEntryType stackEntry_;

  protected:
    std::vector < DUNE_FDATA * > vecFdata_;
    //****************************************************************
    //
    // --GrapeGridDisplay, Some Subroutines needed for display with GRAPE
    //
    //****************************************************************
    inline int callFirstMacro(DUNE_ELEM * he) ;
    // first and next macro element via LevelIterator level 0
    inline int first_leaf (DUNE_ELEM * he) ;
    inline int next_leaf (DUNE_ELEM * he) ;

    // first and next macro element via LevelIterator level 0
    inline int first_macro (DUNE_ELEM * he) ;
    inline int next_macro (DUNE_ELEM * he) ;

    // first and next child via HierarchicIterator with given maxlevel in Grape
    inline int first_child (DUNE_ELEM * he) ;
    inline int next_child (DUNE_ELEM * he) ;

    inline static void * copy_iterator (const void * i) ;

    // local to world
    inline void local_to_world (DUNE_ELEM * he, const double * c, double * w);

    // world to local
    inline int world_to_local (DUNE_ELEM * he, const double * w, double * c);

    // check inside reference element
    inline int checkInside (DUNE_ELEM * he, const double * w);

    // local to world
    inline static void ctow (DUNE_ELEM * he, const double * c, double * w);

    // world to local
    inline static int wtoc (DUNE_ELEM * he, const double * w, double * c);

    // check inside reference element
    inline static int check_inside (DUNE_ELEM * he, const double * w);

    // wrapper methods for first_child and next_child
    inline static int first_mac (DUNE_ELEM * he);
    inline static int next_mac (DUNE_ELEM * he);

    // wrapper methods for first_child and next_child
    inline static int fst_leaf (DUNE_ELEM * he);
    inline static int nxt_leaf (DUNE_ELEM * he);

    // wrapper methods for first_child and next_child
    inline static int fst_child (DUNE_ELEM * he);
    inline static int nxt_child (DUNE_ELEM * he);

    //! calles evalCoord for data of actual display
    inline void evalCoord (DUNE_ELEM *he, DUNE_FDATA *df,
                           const double *coord, double * val);
    //! calles evalDof for data of actual display
    inline void evalDof (DUNE_ELEM *he, DUNE_FDATA *df, int localNum, double * val);

    //! functions pointer for calling evaluate in ghmesh
    //! just calles evalCoord for actual display
    inline static void evalCoordWrap (DUNE_ELEM *he, DUNE_FDATA *df,
                                      const double *coord, double * val);
    //! functions pointer for calling evaluate in ghmesh
    //! just calles evalDof for actual display
    inline static void evalDofWrap (DUNE_ELEM *he, DUNE_FDATA *df, int localNum, double * val);

    // function to evaluate data
    inline static void func_real (DUNE_ELEM *he , DUNE_FDATA * fe,int ind,
                                  const double *coord, double *val);

    inline void setIterationMethods(DUNE_DAT *, DUNE_FDATA *);
    static inline void setIterationModus(DUNE_DAT *, DUNE_FDATA *);

    static void * getStackEn(DUNE_DAT *);
    static void freeStackEn(DUNE_DAT *, void *);
#endif

  }; // end class GrapeGridDisplay

} // end namespace Dune

#include "grape/combinedgrapedisplay.cc"
#endif
