#include <stddef.h>
#include <stdlib.h>
#include <list>
#include "Goal.h"

void* Goal::operator new ( const size_t size )
{	return malloc( size );
}

void  Goal::operator delete ( void* ptr )
{	free( ptr );
}

void* Goal::operator new [] ( const size_t size )
{	return malloc( size );
}

void  Goal::operator delete [] ( void* ptr )
{	free( ptr );
}

  /***************************************************************************************************************/
 /*												CompositeGoal													*/
/***************************************************************************************************************/

void CompositeGoal::RemoveAllSubgoals()
{
	for (SubgoalList::iterator it = m_SubGoals.begin(); it!=m_SubGoals.end(); ++it)
	{
		(*it)->Terminate();
		delete *it;
	}
	m_SubGoals.clear();
}

CompositeGoal::~CompositeGoal()
{
	RemoveAllSubgoals();
}

Goal::Goal_Status CompositeGoal::ProcessSubgoals(double dTime_s)
{
	Goal_Status StatusOfSubGoals;
	//Remove all completed and failed goals from the front of the subgoal list
	while (!m_SubGoals.empty() && (m_SubGoals.front()->GetStatus()==eCompleted || m_SubGoals.front()->GetStatus()==eFailed))
	{
		m_SubGoals.front()->Terminate();
		delete m_SubGoals.front();
		m_SubGoals.pop_front();
	}
	//If any subgoals remain, process the one at the front of the list
	if (!m_SubGoals.empty())
	{
		//grab the status of the front-most subgoal
		StatusOfSubGoals = m_SubGoals.front()->Process(dTime_s);

		//we have to test for the special case where the front-most subgoal reports "completed" and the subgoal list contains additional goals.
		//When this is the case, to ensure the parent keeps processing its subgoal list, the "active" status is returned.
		if (StatusOfSubGoals == eCompleted && m_SubGoals.size() > 1)
			StatusOfSubGoals=eActive;
	}
	else
		StatusOfSubGoals=eCompleted;
	return StatusOfSubGoals;
}



  /***************************************************************************************************************/
 /*												MultitaskGoal													*/
/***************************************************************************************************************/


void MultitaskGoal::RemoveAllGoals()
{
	for (GoalList::iterator it = m_GoalsToProcess.begin(); it!=m_GoalsToProcess.end(); ++it)
	{
		(*it)->Terminate();
		delete *it;
	}
	m_GoalsToProcess.clear();
}

MultitaskGoal::~MultitaskGoal()
{
	RemoveAllGoals();
}

void MultitaskGoal::Activate()
{
	for (GoalList::iterator it = m_GoalsToProcess.begin(); it!=m_GoalsToProcess.end(); ++it)
		(*it)->Activate();
}
Goal::Goal_Status MultitaskGoal::Process(double dTime_s)
{
	ActivateIfInactive();
	Goal_Status status;
	size_t NonActiveCount=0;
	for (GoalList::iterator it = m_GoalsToProcess.begin(); it!=m_GoalsToProcess.end(); ++it)
	{
		status=(*it)->Process(dTime_s);
		if (status!=eActive)
			NonActiveCount++;
	}
	if (NonActiveCount<m_GoalsToProcess.size())
		m_Status=eActive;
	else
	{
		status=eCompleted;
		//Check the final status it is completed unless any goal failed
		for (GoalList::iterator it = m_GoalsToProcess.begin(); it!=m_GoalsToProcess.end(); ++it)
		{
			if ((*it)->GetStatus()==eFailed)
				status=eFailed;
		}
		m_Status=status;
	}
	return status;
}

void MultitaskGoal::Terminate()
{
	//ensure its all clean
	RemoveAllGoals();
	m_Status=eInactive; //make this inactive
}
