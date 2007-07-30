// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
//***************************************************************
//
//  --UGGridHierarchicIterator
//  --HierarchicIterator
//
//***************************************************************

template<class GridImp>
void UGGridHierarchicIterator<GridImp>::increment()
{
  if (elementStack_.empty())
    return;

  const int dim = GridImp::dimension;

  //StackEntry old_target = elemStack.pop();
  const typename UG_NS<dim>::Element* oldTarget = elementStack_.pop();

  // Traverse the tree no deeper than maxlevel
  if (UG_NS<dim>::myLevel(oldTarget) < maxlevel_) {

    typename UG_NS<dim>::Element* sonList[UG_NS<dim>::MAX_SONS];
    UG_NS<dim>::GetSons(oldTarget, sonList);

    // Load sons of old target onto the iterator stack
    for (int i=0; i<UG_NS<dim>::nSons(oldTarget); i++)
      elementStack_.push(sonList[i]);

  }

  if (elementStack_.empty())
    this->virtualEntity_.setToTarget(NULL);
  else
    this->virtualEntity_.setToTarget(elementStack_.top());

}
