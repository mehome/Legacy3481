#include "StdAfx.h"

//****************************************************************************************************************************
double SBD_Item_Folder::Calculate(void)
{	// Expand the list to the correct length
	if (!MyChildren.NoItems) return 0.0; // Pedantic
	//Timing.SetSize(MyChildren.NoItems);

	// Are we on the upper row ?
	bool l_UpperRow=true;

	// Look at all children
	for(unsigned i=0;i<MyChildren.NoItems;i++)
	{	// Is it a clip or a transition
		if (!MyChildren[i]->m_Transition)
		{	// Get the item of interest
			SBD_Item_Folder_StartStopTimes *Item=Timing.New();
			
			// Setup the item of interest in this one
			Item->Item=MyChildren[i];
		
			// **** For Clips ****
			Item->UpperRow=l_UpperRow;
			l_UpperRow=!l_UpperRow;
			
			// What is the previous clip ?
			if (i==0)
			{	// There is no previous clip
				Item->Start=0.0;
			}
			else
			{	// The previous clip is another clip
				if (!MyChildren[i-1]->m_Transition)
						Item->Start=Timing[i-1].End;
				// The previous item was a transition
				else	Item->Start=Timing[i-1].Start;
			}

			Item->End=Item->Start+MyChildren[i]->m_StretchDuration;
		}
		else
		{	// **** Transitions ****			

			// What is the previous clip ?
			if ( (i==0)||
				((i>0)&&(MyChildren[i-1]->m_Transition))||
				 (i==MyChildren.NoItems-1) )
			{	continue;
			}
			else
			{	// Get the item of interest
				SBD_Item_Folder_StartStopTimes *Item=Timing.New();
			
				// Setup the item of interest in this one
				Item->Item=MyChildren[i];

				// Setup the row to use
				Item->UpperRow=Timing[i-1].UpperRow;
		
				Item->Start=Timing[i-1].End-MyChildren[i]->m_StretchDuration;
				Item->End  =Timing[i-1].End;
			}
		}		

	}

	// Compute information about all child clips
	for(i=0;i<MyChildren.NoItems;i++)
		MyChildren[i]->Calculate();

	// Return my total length
	return Timing.NoItems?Timing[Timing.NoItems-1].End:0;
}

//****************************************************************************************************************************
bool SBD_Item_Folder::DoRender(unsigned NoItems,SBD_Item_Render_Buffer **Items)
{	// Notify people of what we are doing

	// Check things have not got to funky ...
	if (NoItems>MaximumFieldsProcessedAtOnce) return false;

	// This is the list of handles that will be used to wait upon
	HandleCache_Item *HandlesToWaitFor=NULL;
	
	// These are the set of buffers that are used
	class _InternalBuffers
	{	public:		SBD_Item_Folder_StartStopTimes	*UpTimes;
					SBD_Item_Folder_StartStopTimes	*LoTimes;					
					SBD_Item_Folder_StartStopTimes	*TransitionTimes;
					SBD_Item_Render_Buffer			BackgroundSource;
					SBD_Item_Render_Buffer			TransitionSource;	// No memory is actually allocated in this one
					SBD_Item_Info_From_To			TransitionElements;
					double							l_Time[2];

					bool BGSourceIsLowerRow(void)
					{	if ((TransitionTimes)&&(TransitionTimes->UpperRow)) return true;
						return false;
					}

					bool BGSourceIsUpperRow(void)
					{	if ((TransitionTimes)&&(!TransitionTimes->UpperRow)) return true;
						return false;
					}

	} InternalBuffers[MaximumFieldsProcessedAtOnce];

	// Setup the variables
	for(unsigned i=0;i<NoItems;i++)
	{	InternalBuffers[i].LoTimes=InternalBuffers[i].UpTimes=InternalBuffers[i].TransitionTimes=NULL;
		InternalBuffers[i].l_Time[0]=Items[i]->GetStartTime();
		InternalBuffers[i].l_Time[1]=Items[i]->GetEndTime();
	}

	// Compute the LUT
	for(i=0;i<NoItems;i++)
	{	// Store a local copy of the rendering time to speed things up
		double l_RenderingTime=Items[i]->GetCentreTime();
		
		// We want to find the clip within the project
		for(unsigned j=0;j<Timing.NoItems;j++)
		{	// Have we stepped past the end of the possible region of interest
			if (Timing[j].Start > l_RenderingTime) break;

			if ((Timing[j].Start <= l_RenderingTime)&&
				(Timing[j].End   >  l_RenderingTime))
			{	// We have found one of the clips

				// It is a transition
				if (Timing[j].Item->m_Transition)
					InternalBuffers[i].TransitionTimes = &Timing[j];
				// It is a clip
				else
				{	// it is on the upper row
					if (Timing[j].UpperRow) 
							InternalBuffers[i].UpTimes = &Timing[j];
					// it is on the lower row
					else	InternalBuffers[i].LoTimes = &Timing[j];
				} // It is a clip
			} // if ((Timing[j].Start <= l_RenderingTime)&&
		} // for(unsigned j=0;j<Timing.NoItems;j++)
	} // for(unsigned i=0;i<NoItems;i++)

	// Blank buffers that are not use in any clips ...
	for(i=0;i<NoItems;i++)	
	if ( (!InternalBuffers[i].UpTimes) &&
		 (!InternalBuffers[i].LoTimes) &&
		 (!InternalBuffers[i].TransitionTimes) )
			Items[i]->Blank_Image();

	//**** ASYNC ON UPPER LAYER *****************************
	i=0;
	while(i<NoItems)
	{	if (InternalBuffers[i].UpTimes)
		{	// Store access to this item
			SBD_Item_Info *Item=InternalBuffers[i].UpTimes->Item;

			// Store the current tiem
			SBD_Item_Folder_StartStopTimes *TimerItem=InternalBuffers[i].UpTimes;

			// Store my linked list to render
			SBD_Item_Render_Buffer * List=NULL;
			SBD_Item_Render_Buffer **Prev=&List;

			// Look at things
			while(  (i<NoItems)&&								// While there are items on the list
					(InternalBuffers[i].UpTimes)&&				// There is an item.
					(InternalBuffers[i].UpTimes->Item==Item))	// And they are of the correct type
			{	// Compute the correct time for this item. This supports backwards playback !!!
				double InternalTime[2] = {	(InternalBuffers[i].l_Time[0] - TimerItem->Start)/(TimerItem->End - TimerItem->Start),
											(InternalBuffers[i].l_Time[1] - TimerItem->Start)/(TimerItem->End - TimerItem->Start) };
				double ChildTime[2] =	 {	(InternalTime[0] * ( Item->m_OutPoint - Item->m_InPoint ) + Item->m_InPoint),
											(InternalTime[1] * ( Item->m_OutPoint - Item->m_InPoint ) + Item->m_InPoint) };

				// Is this part of a transition
				if (InternalBuffers[i].BGSourceIsUpperRow())
				{	// And we are doing a transition TO me ...
					InternalBuffers[i].BackgroundSource.AllocateImage(	Items[i]->XRes,Items[i]->YRes,
																		ChildTime[0],ChildTime[1],
																		Item->IsOddField(0.5*(ChildTime[0]+ChildTime[1])),
																		Items[i]->ID);					

					// Insert this item into the linked list
					*Prev=&InternalBuffers[i].BackgroundSource;
					InternalBuffers[i].BackgroundSource.SetNext(NULL);
					Prev=InternalBuffers[i].BackgroundSource.GetNextRef();
				}
				else
				{	// Do a push on the rendering item !
					Items[i]->Push();

					// Setup the time value
					Items[i]->SetTime(ChildTime[0],ChildTime[1]);

					// Insert this item into the linked list
					*Prev=Items[i];
					Items[i]->SetNext(NULL);				
					Prev=Items[i]->GetNextRef();
				}			

				// Increment the position by one ...
				i++;
			}

			// We try making an async call on this item
			HandleCache_Item *L_Handle=List?Item->Render_Async(List):NULL;

			// If we did not success an asynchronouse read, then we
			// probably need to do a synchronous render :(
			if (!L_Handle) Item->Render(List);
			else 
			{	// Add it to the linkes list of items
				L_Handle->Next=HandlesToWaitFor;
				HandlesToWaitFor=L_Handle;
			}
		}
		else i++;
	}

	//**** ASYNC ON LOWER LAYER *****************************
	i=0;
	while(i<NoItems)
	{	if (InternalBuffers[i].LoTimes)
		{	// Store access to this item
			SBD_Item_Info *Item=InternalBuffers[i].LoTimes->Item;

			// Store the current tiem
			SBD_Item_Folder_StartStopTimes *TimerItem=InternalBuffers[i].LoTimes;

			// Store my linked list to render
			SBD_Item_Render_Buffer * List=NULL;
			SBD_Item_Render_Buffer **Prev=&List;

			// Look at things
			while(  (i<NoItems)&&								// While there are items on the list
					(InternalBuffers[i].LoTimes)&&				// Check that there is an item
					(InternalBuffers[i].LoTimes->Item==Item))	// And they are of the correct type
			{	// Compute the correct time for this item. This sLoports backwards playback !!!
				// Compute the correct time for this item. This supports backwards playback !!!
				double InternalTime[2] = {	(InternalBuffers[i].l_Time[0] - TimerItem->Start)/(TimerItem->End - TimerItem->Start),
											(InternalBuffers[i].l_Time[1] - TimerItem->Start)/(TimerItem->End - TimerItem->Start) };
				double ChildTime[2] =	 {	(InternalTime[0] * ( Item->m_OutPoint - Item->m_InPoint ) + Item->m_InPoint),
											(InternalTime[1] * ( Item->m_OutPoint - Item->m_InPoint ) + Item->m_InPoint) };

				// Is this part of a transition
				if (InternalBuffers[i].BGSourceIsLowerRow())
				{	// And we are doing a transition TO me ...
					InternalBuffers[i].BackgroundSource.AllocateImage(	Items[i]->XRes,Items[i]->YRes,
																		ChildTime[0],ChildTime[1],
																		Item->IsOddField(0.5*(ChildTime[0]+ChildTime[1])),
																		Items[i]->ID);

					// Insert this item into the linked list
					*Prev=&InternalBuffers[i].BackgroundSource;
					InternalBuffers[i].BackgroundSource.SetNext(NULL);
					Prev=InternalBuffers[i].BackgroundSource.GetNextRef();
				}
				else
				{	// Do a push on the rendering item !
					Items[i]->Push();

					// SetLo the time value
					Items[i]->SetTime(ChildTime[0],ChildTime[1]);

					// Insert this item into the linked list
					*Prev=Items[i];
					Items[i]->SetNext(NULL);				
					Prev=Items[i]->GetNextRef();
				}			

				// Increment the position by one ...
				i++;
			}

			// We try making an async call on this item
			HandleCache_Item *L_Handle=List?Item->Render_Async(List):NULL;

			// If we did not success an asynchronouse read, then we
			// probably need to do a synchronous render :(
			if (!L_Handle) Item->Render(List);
			else 
			{	// Add it to the linkes list of items
				L_Handle->Next=HandlesToWaitFor;
				HandlesToWaitFor=L_Handle;
			}
		}
		else i++;
	}

	//**** ASYNC ON TRANSITIONS *****************************
	i=0;
	while(i<NoItems)
	{	if (InternalBuffers[i].TransitionTimes)
		{	// Store access to this item
			SBD_Item_Info *Item=InternalBuffers[i].TransitionTimes->Item;

			// Store the current tiem
			SBD_Item_Folder_StartStopTimes *TimerItem=InternalBuffers[i].TransitionTimes;

			// Store my linked list to render
			SBD_Item_Render_Buffer * List=NULL;
			SBD_Item_Render_Buffer **Prev=&List;

			// Transitionok at things
			while(  (i<NoItems)&&								// While there are items on the list
					(InternalBuffers[i].TransitionTimes)&&				// Check that there is an item
					(InternalBuffers[i].TransitionTimes->Item==Item))	// And they are of the correct type
			{	// Compute the correct time for this item. This sTransitionports backwards playback !!!
				double InternalTime[2] = {	(InternalBuffers[i].l_Time[0] - TimerItem->Start)/(TimerItem->End - TimerItem->Start),
											(InternalBuffers[i].l_Time[1] - TimerItem->Start)/(TimerItem->End - TimerItem->Start) };
				double ChildTime[2] =	 {	(InternalTime[0] * ( Item->m_OutPoint - Item->m_InPoint ) + Item->m_InPoint),
											(InternalTime[1] * ( Item->m_OutPoint - Item->m_InPoint ) + Item->m_InPoint) };

				// Setup the information about this buffer
				InternalBuffers[i].TransitionSource.SetTime(ChildTime[0],ChildTime[1]);
				InternalBuffers[i].TransitionSource.ID	=Items[i]->ID;				
				InternalBuffers[i].TransitionSource.XRes=Items[i]->XRes;
				InternalBuffers[i].TransitionSource.YRes=Items[i]->YRes;
				*Prev=&InternalBuffers[i].TransitionSource;
				InternalBuffers[i].TransitionSource.SetNext(NULL);				
				Prev=InternalBuffers[i].TransitionSource.GetNextRef();

				// Increment the position by one ...
				i++;
			}

			// We try making an async call on this item
			HandleCache_Item *L_Handle=List?Item->Render_Async(List):NULL;

			// If we did not success an asynchronouse read, then we
			// probably need to do a synchronous render :(
			if (L_Handle)
			{	// Add it to the linkes list of items
				L_Handle->Next=HandlesToWaitFor;
				HandlesToWaitFor=L_Handle;
			}
		}
		else i++;
	}

	//**** WAIT FOR ASYNC DATA LOADS TO COMPLETE *****************************
	// Wait for all the events to complete ...
	HandleCache_WaitForHandles(HandlesToWaitFor);

	// We now release all the handles ...
	DriveReader_DisposeOfHandle_List(HandlesToWaitFor);

	//**** SYNCHRONOUS TRANSITION RENDERING **********************************
	// Finally, we render the transitions as required !

	i=0;
	while(i<NoItems)
	{	if (InternalBuffers[i].TransitionTimes)
		{	// Store access to this item
			SBD_Item_Info *Item=InternalBuffers[i].TransitionTimes->Item;

			// Store the current tiem
			SBD_Item_Folder_StartStopTimes *TimerItem=InternalBuffers[i].TransitionTimes;

			// Store my linked list to render
			SBD_Item_Info_From_To * List=NULL;
			SBD_Item_Info_From_To **Prev=&List;

			// Transitionok at things
			while(  (i<NoItems)&&										// While there are items on the list
					(InternalBuffers[i].TransitionTimes)&&				// Check that there is an item
					(InternalBuffers[i].TransitionTimes->Item==Item))	// And they are of the correct type
			{	// Setup the entry
				InternalBuffers[i].TransitionElements.To				=Items[i];
				InternalBuffers[i].TransitionElements.From				=&InternalBuffers[i].BackgroundSource;

				InternalBuffers[i].TransitionElements.SetStartTime(InternalBuffers[i].TransitionSource.GetStartTime());
				InternalBuffers[i].TransitionElements.SetEndTime  (InternalBuffers[i].TransitionSource.GetEndTime());

				*Prev=&InternalBuffers[i].TransitionElements;
				InternalBuffers[i].TransitionElements.Next=NULL;
				Prev=&InternalBuffers[i].TransitionElements.Next;
				
				// Increment the position by one ...
				i++;
			}

			// Preform the CPU intensive rendering
			if (List) Item->Render_Transition(List);
		}
		else i++;
	}

	//**** POP BUFFERS *************************************************************
	for(i=0;i<NoItems;i++)
	{	if (( InternalBuffers[i].UpTimes)&&
	        (!InternalBuffers[i].BGSourceIsUpperRow())) Items[i]->Pop();
		if (( InternalBuffers[i].LoTimes)&&
	        (!InternalBuffers[i].BGSourceIsLowerRow())) Items[i]->Pop();
	}

	return true;
}

//****************************************************************************************************************************
// The constructor
SBD_Item_Folder::SBD_Item_Folder(SBD_Item_Info *Parent)
:SBD_Item_Info(Parent)
{
}

//****************************************************************************************************************************
SBD_Item_Project::SBD_Item_Project(void) : SBD_Item_Folder(NULL)
{	// Default settings
}
