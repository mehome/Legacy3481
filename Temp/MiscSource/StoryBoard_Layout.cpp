#include "StdAfx.h"

void StoryBoard_Layout::InitialiseWindow(void) {
	UtilLib_SimpleGrid_PutSpacingX(5);
	UtilLib_SimpleGrid_PutSpacingY(5);
	VideoEditor_Layout::InitialiseWindow();
	}

void StoryBoard_Layout::Child_AboutToLeave(HWND hWnd) {
	//call the predecessor
	VideoEditor_Layout::Child_AboutToLeave(hWnd);

	StoryBoard2_Crouton *sc=GetWindowInterface<StoryBoard2_Crouton>(hWnd);
	if (sc) {
		//now we can remove the tle
		TimeLineElement *tle=sc->GetTLE();
		if (tle) 
			m_timeLine->RemoveTimeLineElement(tle);
		if (sc->GetStartedContentRange()) {
			m_timeLine->ContentRegionEditor_CompleteRegionChanges();
			sc->SetStartedContentRange(false);
			}
		}
	}

TimeLineElement *StoryBoard_Layout::GetTLE(WindowLayout_Item *icon) {
	ContentInstance *ci;
	if (icon) {
		StoryBoard2_Crouton *SC=GetInterface<StoryBoard2_Crouton>(GetWindowInterface<BaseWindowClass>(icon->hWnd));
		if (SC) ci=SC->VideoEditor_Crouton_GetContentInstance();
		if (ci) return ci->ContentInstance_GetTimeLineElement();
		}
	return NULL;
	}

StoryBoard_Layout::TLEtype StoryBoard_Layout::GetTLEtype(TimeLineElement *tle) {
	if (!tle) return TLE_Unknown;
	return GetTLEtype(tle->GetContentInstance());
	}

StoryBoard_Layout::TLEtype StoryBoard_Layout::GetTLEtype(ContentInstance *ci) {
	if (!ci) return TLE_Unknown;
	TLEtype DestType=TLE_Utility;
	eContentInstance_SourceTypes ciType;
	TimeLineElement *tle=ci->ContentInstance_GetTimeLineElement();
	if (tle) ciType=(eContentInstance_SourceTypes)tle->TimeLineElement_GetSourceType();
	else ciType=(eContentInstance_SourceTypes)ci->ContentInstance_GetSourceType();
	switch (ciType) {
		case ContentInstance_Utility:
			DestType=TLE_Utility;
			break;
		case ContentInstance_Effect:
			DestType=TLE_Over;
			break;
		case ContentInstance_Audio:
			DestType=TLE_Audio;
			break;
		case ContentInstance_Video:
			DestType=TLE_Clip;
			break;
		case ContentInstance_Video+ContentInstance_Audio:
			DestType=TLE_Clip;
			break;
		case ContentInstance_Transition:
			DestType=TLE_Trans;
			break;
		default:
			DestType=TLE_Over;
		}
	if (ciType&ContentInstance_Overlay) {
		if (ciType&ContentInstance_Video) DestType=TLE_Over;
		else DestType=TLE_OverAudio;
		}
	return DestType;
	}

  /***************************************************************************************/
 /*							This Section deals with Deletion							*/
/***************************************************************************************/



void StoryBoard_Layout::DumbRipple(double StartTime,double Adjustment,unsigned i) {
	if (!Adjustment) return;
	//Use negative numbers to cut time
	//I'm not going to renumber the TLE sbPos here.

	//Insertion operations will take care of filling in the holes...
	//and that operation can handle it a lot more efficiently

	VideoEditor_Layout_ChildItem *scan;
	unsigned eoi=m_childItems.NoItems;
	unsigned usingindex=i;
	for (;i<eoi;i++) {
		scan=m_childItems[i];
		if ((scan->start>=StartTime)||(usingindex)) {
			scan->SetStartingPoint(scan->GetStartingPoint()+Adjustment);
			}
		}
	}

void StoryBoard_Layout::SmartRipple(unsigned index) { //The main entry
	//Note: We are assuming at this point that the elements are sorted
	//Pedantic bounds check
	if (index>m_childItems.NoItems) return;
	unsigned startindex=index;
	VideoEditor_Layout_ChildItem *scan=m_childItems[index];
	TimeLineElement *tle=scan->GetTimeLineElement();
	TLEtype type=GetTLEtype(tle->GetContentInstance());

	{//From index position to the next defined clip we have to analyze each crouton and compute its new starting point
		double offset=0,startingpoint;
		unsigned eoi=m_childItems.NoItems,deletedindex=index;
		VideoEditor_Layout_ChildItem *scanstart=m_childItems[index];
		VideoEditor_Layout_ChildItem *scan;
		index++;

		for (;index<eoi;index++) {
			//Note I'm exanding all of this out for now to debug---
			scan=m_childItems[index];
			TimeLineElement *tle=scan->GetTimeLineElement();
			TLEtype type=GetTLEtype(tle->GetContentInstance());
			if (type==TLE_Clip) break;
			//Find the correct previous clip to compute starting point
			unsigned previndex=FindLastEvent(index,GetPrevAcceptableEvent(type),deletedindex);
			if (previndex==-1) startingpoint=ComputeStartingPoint(previndex,index,offset);
			else
				startingpoint=ComputeStartingPoint(previndex,index,offset,m_childItems[previndex]->instanceOrder);
			m_childItems[index]->SetStartingPoint(startingpoint);
			}

		if (index>=eoi) index=0; //no need to ripple if we reached the end
		}
	{//If the previous clip has multiple ranges then we need not dumb ripple
		unsigned prev=FindLastEvent(startindex,TLE_Clip,startindex);
		if (!(prev==-1)) 
			if (m_childItems[prev]->GetTimeLineElement()->m_activeGlobalRegions.GetNumRanges()>1) return;
		}
	if (index) {
		double Adjustment=0,newstartingpoint;
		unsigned prev=FindLastEvent(index,GetPrevAcceptableEvent(TLE_Clip),startindex);
		if (!(prev==-1)) newstartingpoint=ComputeStartingPoint(prev,index,m_childItems[prev]->instanceOrder); //ToDo add offset
		else newstartingpoint=0;
		Adjustment=newstartingpoint-(m_childItems[index]->GetTimeLineElement()->GetGlobalStartPoint());
		DumbRipple(scan->start,Adjustment,index);
		}
	}

void StoryBoard_Layout::SmartRipple(TimeLineElement *tle) {
	//The elements must be in order (We may want to see if we can keep in order without the sort)
	SortTLEs();
	//Search for the correct index entry
	unsigned eoi=m_childItems.NoItems;
	for (unsigned i=0;i<eoi;i++) {
		if (m_childItems[i]->GetTimeLineElement()==tle) {
			SmartRipple(i);
			break;
			}
		}

	}

void StoryBoard_Layout::DeleteTLE(TimeLineElement *tle) {
	if (!tle) return;
	m_timeLine->ContentRegionEditor_StartRegionChanges();
	SmartRipple(tle);
	VideoEditor_Layout_ChildItem* childItem = VideoEditor_Layout_GetChildItem(tle);

	m_timeLine->RemoveTimeLineElement(tle);

	VideoEditor_Layout_RemoveChildItem(childItem);
	m_timeLine->ContentRegionEditor_CompleteRegionChanges();
	}

  /***************************************************************************************/
 /*							This Section deals with Insertion							*/
/***************************************************************************************/
//SetStoryBoardPosition- filter out what needs to be numbered
//NumOrderPrev-This will number the previous crouton(s) of the inserted clip(s)
//FindLastEvent-Given a tList and valid type(s) it will find the last event
//ComputeStartingPoint-This calculates the starting point for the inserted/next element
//UseNewEventSPFromPrev-This determines if the previous clip can be used for ComputeStarting point
//GetPrevAcceptableEvent-This checks if the previous eventtype is compatible with the inserted clip
//DragNDrop_DropItemsHere-This is the area that handles insertion
void StoryBoard_Layout::SetStoryBoardPosition(tList<StoryBoard2_Crouton *> *SClist,TimeLineElement *tle,int sbPos) {
	//Only pure StoryBoard clips (with non multiple ranges) will be numbered
	if ((tle->m_activeGlobalRegions.GetNumRanges()<=1)&&(!(GetTLEtype(tle)==TLE_Trans)))
		tle->SetStoryBoardPosition(sbPos);
	else {
		//Only Transitions whose previous clip don't have multiple ranges
		if (GetTLEtype(tle)==TLE_Trans) {
			//I don't like this one but we have to know if a transition is part of an active region kind of clip
			TimeLineElement *PreviousElement;
			PreviousElement=FindLastEvent(SClist,TLE_Clip);
			if (PreviousElement) {
				if (PreviousElement->m_activeGlobalRegions.GetNumRanges()<=1)
					tle->SetStoryBoardPosition(sbPos);
				}
			else tle->SetStoryBoardPosition(sbPos);
			}
		}
	}

int StoryBoard_Layout::NumOrderPrev(tList<StoryBoard2_Crouton *> *SClist,int DefaultValue) {
	double PrevDuration,Duration;
	unsigned index,eoi=SClist->NoItems;
	int LastOrderNumber,OrderNumber;

	FindLastEvent(SClist,TLE_Clip,&index);
	if (index) {
		TimeLineElement *tle;
		Duration=(tle=(*SClist)[index-1]->GetTLE())->GetContentInstance()->ContentInstance_GetClipLength();
		//Grab last order number
		LastOrderNumber=tle->GetStoryBoardPosition();
		if (LastOrderNumber==-1) LastOrderNumber=DefaultValue;
		//FPTN
		for (index--;index>0;index--) {
			PrevDuration=(tle=(*SClist)[index-1]->GetTLE())->GetContentInstance()->ContentInstance_GetClipLength();
			if ((!(Duration==PrevDuration))||(GetTLEtype(tle->GetContentInstance())==TLE_Clip)) break;
			}
		//Since index is 1 based... and our result is -1 of what it was suppose to be, the correct position
		//becomes zero based which is what we want
		/*
		NumOrderPrev--FPTN-----------NULL------------------------------------Add 1----number to ins point--->
							\GetNumber-- -1 ---find last order#--default-/
									   \- >0-----------------     \- # --/
															\------------/
		*/
		//GetNumber
		OrderNumber=(*SClist)[index]->GetTLE()->GetStoryBoardPosition();
		if (OrderNumber==-1) OrderNumber=LastOrderNumber;
		//Number the index from this point to the end of the list using OrderNumber
		//note index is zero based now
		for (;index<eoi;index++) {
			SetStoryBoardPosition(SClist,(*SClist)[index]->GetTLE(),OrderNumber++);
			}
		return (OrderNumber);
		}
	return (DefaultValue+1);
	}

TimeLineElement *StoryBoard_Layout::FindNextEvent(WindowLayout_Item *scan,int flags) {
	TimeLineElement *tle=NULL;
	while (scan) {
		StoryBoard2_Crouton *sc=GetWindowInterface<StoryBoard2_Crouton>(scan->hWnd);
		//Yeah this means it will skip over all new inserted filebuttons
		if (sc) tle=sc->GetTLE();
		if (GetTLEtype(tle)&flags) break;
		scan=scan->Next;
		}
	if (!scan) tle=NULL;
	return tle;
	}

TimeLineElement *StoryBoard_Layout::FindLastEvent(tList<StoryBoard2_Crouton *> *SClist,int flags,unsigned *index) {
	TimeLineElement *tle=NULL;
	for (unsigned i=SClist->NoItems;i>0;i--) {
		if ((GetTLEtype((tle=(*SClist)[i-1]->GetTLE())))&flags) break;
		}
	if (index) *index=i; //this answer is one based
	return tle;
	}

unsigned StoryBoard_Layout::FindLastEvent(unsigned index,int flags,unsigned deletedindex) {
	for (;index>0;index--) {
		if (((GetTLEtype(m_childItems[index-1]->GetTimeLineElement()))&flags)&&(!(index==deletedindex+1))) break;
		}
	return (--index); //this answer is zero based (to be used in an array
	}

unsigned StoryBoard_Layout::FindNextEvent(unsigned index,int flags,unsigned deletedindex) {
	unsigned eoi=m_childItems.NoItems;
	for (;index<eoi;index++) {
		if (((GetTLEtype(m_childItems[index]->GetTimeLineElement()))&flags)&&(!(index==deletedindex))) break;
		}
	if (index==eoi) index=-1;
	return (index);
	}

double StoryBoard_Layout::ComputeStartingPoint(unsigned PrevIndex,unsigned Index,double offset,unsigned PrevInstanceOrder) {
	if (PrevIndex==-1) return offset;
	TimeLineElement *PrevElement=m_childItems[PrevIndex]->GetTimeLineElement();
	ContentInstance *ci=m_childItems[Index]->GetTimeLineElement()->GetContentInstance();
	if (PrevElement&&ci)
		return (ComputeStartingPoint(PrevElement,ci,PrevInstanceOrder)+offset);
	else return 0;
	}

double StoryBoard_Layout::ComputeStartingPoint(TimeLineElement *PrevElement,ContentInstance *ci,StoryBoard2_Crouton *PrevSC) {
	unsigned PrevInstanceOrder=0;
	if (PrevSC) PrevInstanceOrder=PrevSC->m_activeRegion;
	return ComputeStartingPoint(PrevElement,ci,PrevInstanceOrder);
	}

double StoryBoard_Layout::ComputeStartingPoint(TimeLineElement *PrevElement,ContentInstance *ci,unsigned PrevInstanceOrder) {
	//Important Note... This function will return the same starting point as the previous element 
	//(work like an overlay) if the PrevElement is not a valid reference.  This means you will have to check 
	//that your PrevElement is valid.

	if ((!(PrevElement))||(!(ci))) return 0;
	TLEtype PrevElementType=GetTLEtype(PrevElement->GetContentInstance());
	TLEtype ElementType=GetTLEtype(ci);
	double StartingPoint=PrevElement->GetGlobalStartPoint();
	double Duration=PrevElement->GetContentInstance()->ContentInstance_GetClipLength();
	//The computation for all overlay types and Ultility is already computed
	switch (ElementType) {
		case TLE_Clip:
			//CC start + duration
			if (PrevElementType==TLE_Clip) {
				//Here is the active region support.. .we'll pull the end range from the indexed crouton
				if (PrevElement->m_activeGlobalRegions.GetNumRanges()>1) {
					double startrange;
					PrevElement->m_activeGlobalRegions.GetRange(PrevInstanceOrder,startrange,StartingPoint);
					}
				else 
					StartingPoint+=Duration;
				break;
				}
			//TC start
			//if (PrevElementType==TLE_Trans) {}
			break;

		case TLE_Trans:
			//Here is the active region support.. .we'll pull the end range from the indexed crouton
			if (PrevElement->m_activeGlobalRegions.GetNumRanges()>1) {
				double endrange;
				PrevElement->m_activeGlobalRegions.GetRange(PrevInstanceOrder,StartingPoint,endrange);
				Duration=endrange-StartingPoint;
				}
			//CT max(start,start+duration-transduration)
			if (PrevElementType==TLE_Clip) {
				//TODO in insert transition mode we'll need to recompute this
				StartingPoint=max(StartingPoint,StartingPoint+(Duration-ci->ContentInstance_GetClipLength()));
				//TODO we'll have to check to clip the transition
				break;
				}
			//TT start + duration
			if (PrevElementType==TLE_Trans) {
				StartingPoint+=Duration;
				}
			break;

		case TLE_Audio:
			//Here is the active region support.. .we'll pull the end range from the indexed crouton
			if (PrevElement->m_activeGlobalRegions.GetNumRanges()>1) {
				double endrange;
				PrevElement->m_activeGlobalRegions.GetRange(PrevInstanceOrder,StartingPoint,endrange);
				Duration=endrange-StartingPoint;
				}
			//AA start + duration
			if (PrevElementType==TLE_Audio) {
				StartingPoint+=Duration;
				break;
				}
			//CA start
			//if (PrevElementType==TLE_Audio) {}
			break;
		}

	return StartingPoint;
	}

bool StoryBoard_Layout::UseNewEventSPFromPrev(ContentInstance *Prevci,ContentInstance *ci) {
	if ((Prevci==NULL)||(ci==NULL)) return false;
	TLEtype PrevElementType=GetTLEtype(Prevci);
	TLEtype ElementType=GetTLEtype(ci);
	int flag=GetPrevAcceptableEvent(ElementType);
	return (flag&PrevElementType);
	}

int StoryBoard_Layout::GetPrevAcceptableEvent(TLEtype EventType) {
	/*
	Here is all valid previous elements... given an element:
	C-C,T
	T-C,T
	A-A,C
	O-C
	*/
	int flag=0;
	switch (EventType) {
		case TLE_Clip:
			flag=TLE_Clip+TLE_Trans;
			break;
		case TLE_Trans:
			flag=TLE_Clip+TLE_Trans;
			break;
		case TLE_Audio:
			flag=TLE_Audio+TLE_Clip;
			break;
		default: flag=TLE_Clip;
		}
	return flag;
	}

void StoryBoard_Layout::ProvideClipBtoTimeLine(WindowLayout_Item *scan) {
	while (scan) {
		//Layout needs to have been updated to ensure all new inserted filebuttons are croutons
		StoryBoard2_Crouton	*SC=GetWindowInterface<StoryBoard2_Crouton>(scan->hWnd);
		if (SC) {
			TimeLineElement *tle=SC->GetTLE();
			if (tle) {
				TransitionContentInstance *tci=GetInterface<TransitionContentInstance>(tle->GetContentInstance());
				if (tci) {
					//search for clipb or trans whichever occurs first
					TimeLineElement *whatwasfound=FindNextEvent(scan->Next,TLE_Clip|TLE_Trans);
					if (GetTLEtype(whatwasfound)==TLE_Trans) whatwasfound=NULL;
					tci->Transition_SetClipB(whatwasfound);
					}
				}
			}
		//This should never happen
		else DebugOutput("StoryBoard_Layout::ProvideClipBtoTimeLine has invalid WindowLayout_Item(s)");
		scan=scan->Next;
		}
	}

void StoryBoard_Layout::Insertion(tList<VideoEditor_Layout_ChildItem*> *ChildItems,tList<VideoEditor_Layout_DraggingItem*>	*ItemsDragged,unsigned insertbeforehere) {
	}

WindowLayout_Item *StoryBoard_Layout::DragNDrop_DropItemsHere
			(	int Width,int Height,				// Window size
				int MousePosnX,int MousePosnY,		// Mouse posn
				WindowLayout_Item *ObjectsInWindow,	// The objects already in the window
				WindowLayout_Item *ObjectsDropped,	// The objects that have been dropped in the window
				bool Resizing,bool DroppedHere
				) {

	WindowLayout_Item *LastObjectDropped,*Scan;
	//Lets find our last Dropped Item before merging the lists
	Scan=ObjectsDropped;
	while (Scan) {
		LastObjectDropped=Scan;
		Scan=Scan->Next;
		}

	// Merge the lists
	WindowLayout_Item *Ret=UtilLib_SimpleGrid::DragNDrop_DropItemsHere(Width,Height,MousePosnX,MousePosnY,ObjectsInWindow,ObjectsDropped,Resizing,DroppedHere);

	// Have the items now actually been dropped ?
	if (DroppedHere) {
		FileButton *FB=NULL;
		StoryBoard2_Crouton	*SC;
		int lastposition=0;
		double StartingPointofInsertion=0,EndingPointofInsertion=0,Adjustment=0;
		tList<StoryBoard2_Crouton *> SClist;

		// Pedantic, but useful
		OutputDebugString("StoryBoard2::Items have been dropped into the storyboard\n");	

		m_timeLine->ContentRegionEditor_StartRegionChanges();

		// We do not want to do lots of layouts
		MultipleLayouts_Start();

		// We scan through all the items looking for where the filebutton gets inserted... to find the starting
		//point and order number

		{  //First Iteration preparion to insert clips

			Scan=Ret;
			StoryBoard2_Crouton	*SCmoved=GetInterface<StoryBoard2_Crouton>(GetWindowInterface<BaseWindowClass>(ObjectsDropped->hWnd));

			while(Scan) {
				BaseWindowClass *BWC=GetWindowInterface<BaseWindowClass>(Scan->hWnd);
				if (BWC) {
					FB=GetInterface<FileButton>(BWC);
					SC=GetInterface<StoryBoard2_Crouton>(BWC);
					if ((!SC)&&(FB)) break;
					if ((SC)&&(SC==SCmoved)) break;
					SClist.Add(SC);
					//Keep track of the last ordered number
					if (!(SC->GetTLE()->GetStoryBoardPosition()==-1)) SetStoryBoardPosition(&SClist,SC->GetTLE(),lastposition++);
					}
				Scan=Scan->Next;
				}

			//number our previous neighbors
			lastposition=NumOrderPrev(&SClist,lastposition-1);
			}
		// We scan through all the items that were dropped in and create new TimeLineElement structures
		if (!(Scan==ObjectsDropped)) Scan=NULL;  //This shouldn't happen

		{// Second Iteration actual insertion of the clip(s)
			bool FirstLoop=true;
			TimeLineElement *tle=NULL,*newtle=NULL,*PreviousElement=NULL;
			ContentInstance *newci=NULL;
			unsigned PreviousIndex;

			while(Scan) {
				// We need to get the BaseWindowClass interface
				BaseWindowClass *BWC=GetWindowInterface<BaseWindowClass>(Scan->hWnd);
				if (BWC) {
					FB=GetInterface<FileButton>(BWC);
					SC=GetInterface<StoryBoard2_Crouton>(BWC);
					if (SC) {
						VideoEditor_Layout_RemoveDraggingItem
							(VideoEditor_Layout_GetDraggingItem(SC->GetWindowHandle()), false);
						if (!(newtle=SC->GetTLE()))
							newtle=m_timeLine->AddTimeLineElement(SC->VideoEditor_Crouton_GetContentInstance(),0,0,0,0);
						SetStoryBoardPosition(&SClist,newtle,lastposition++);
						if (FirstLoop==true) {
							FirstLoop=false;
							//We need to find the initial starting point of insertion
							//figure startingpoint of insertion
							newci=newtle->GetContentInstance();
							PreviousElement=FindLastEvent(&SClist,GetPrevAcceptableEvent(GetTLEtype(newci)),&PreviousIndex);
							if (PreviousIndex)
								StartingPointofInsertion=ComputeStartingPoint(PreviousElement,newtle->GetContentInstance(),SClist[PreviousIndex-1]);
							else
								StartingPointofInsertion=ComputeStartingPoint(PreviousElement,newtle->GetContentInstance());
							}
						else {
							if (UseNewEventSPFromPrev(tle->GetContentInstance(),newtle->GetContentInstance())) {
								PreviousElement=tle;
								PreviousIndex=0; //Luckily we need not monitor the Crouton since all new Croutons will never have active regions
								}
							}

						tle=newtle;
						if (PreviousIndex)
							tle->SetGlobalStartPoint(ComputeStartingPoint(PreviousElement,newtle->GetContentInstance(),SClist[--PreviousIndex]));
						else
							tle->SetGlobalStartPoint(ComputeStartingPoint(PreviousElement,newtle->GetContentInstance()));
						}
					// If this is a FileButton, but NOT a crouton already
					else if (FB) {

						//create a new instance from the File Properties
						VideoEditor_Layout_DraggingItem* dragItem = this->VideoEditor_Layout_GetDraggingItem(Scan->hWnd);
						ContentInstance *ci= dragItem->contentInstance;
						VideoEditor_Layout_RemoveDraggingItem(dragItem, false);
						dragItem = NULL;

						// We create a new window 
						HWND MyWindow=OpenChild("StoryBoard2_Crouton");
						SC=GetWindowInterface<StoryBoard2_Crouton>(MyWindow);

						if (SC) {
							// And place me in the original position
							Scan->hWnd=MyWindow;

							//Copy the contents of the filebutton into the crouton and delete the filebutton
							SC->VideoEditor_Crouton_SetContentInstance(ci);
							NewTek_Delete(FB);
							// We make sure all of our colors and size match
							Scan->XSize=SC->GetWindowWidth();
							Scan->YSize=SC->GetWindowHeight();
							}

						//for now we'll check for NULL at this level... we may want to check this earlier
						//in the can items be dropped here function.  This ensures that the FileButton
						//can be a legal StoryBoard Crouton
						if (ci) {
							if (FirstLoop==true) {
								FirstLoop=false;
								//We need to find the initial starting point of insertion
								//figure startingpoint of insertion
								PreviousElement=FindLastEvent(&SClist,GetPrevAcceptableEvent(GetTLEtype(ci)),&PreviousIndex);
								if (PreviousIndex) 
									StartingPointofInsertion=ComputeStartingPoint(PreviousElement,ci,SClist[PreviousIndex-1]);
								else 
									StartingPointofInsertion=ComputeStartingPoint(PreviousElement,ci);
								}
							else {
								if (UseNewEventSPFromPrev(tle->GetContentInstance(),ci)) {
									PreviousElement=tle;
									PreviousIndex=0; //Luckily we need not monitor the Crouton since all new Croutons will never have active regions
									}
								}
							if (PreviousIndex)
								tle=m_timeLine->AddTimeLineElement(ci,ComputeStartingPoint(PreviousElement,ci,SClist[--PreviousIndex]),0,lastposition++,PreviousElement);
							else
								tle=m_timeLine->AddTimeLineElement(ci,ComputeStartingPoint(PreviousElement,ci),0,lastposition++,PreviousElement);
							VideoEditor_Layout_AddChildItem(SC->GetWindowHandle(), tle);
							}
						}
					}

				if (SC) SClist.Add(SC); //yes keep adding to the list so that we can determine the correct starting point for the elements that exist after the inserted icons

				if (Scan==LastObjectDropped) {
					//we can make the adjustment here since we don't need adjustments if we hit NULL
					Scan=Scan->Next;
					break;
					}
				// Look at the next item
				Scan=Scan->Next;
				}
			}

		MultipleLayouts_Finished(true);

		{//Third and final Iteration the remaining clips
			int sbPos=lastposition;

			double ClipsStartingPoint=0;
			int RCStages=0;
			bool RenumberCondition;
			bool Ripple;
			//Renumber the data structs and ripple the time
			while (Scan) {

				SC=GetWindowInterface<StoryBoard2_Crouton>(Scan->hWnd);
				TimeLineElement *tle=SC->GetTLE();
				TLEtype tletype;
				Ripple=true;

				switch (RCStages) {
					case 0:
						if (Scan) {
							TimeLineElement *nexttle,*PreviousElement;
							double StartingPointofRipple;
							PreviousElement=FindLastEvent(&SClist,GetPrevAcceptableEvent(GetTLEtype(nexttle=GetTLE(Scan))));
							EndingPointofInsertion=ComputeStartingPoint(PreviousElement,nexttle->GetContentInstance());
							StartingPointofRipple=nexttle->GetGlobalStartPoint();
							Adjustment=EndingPointofInsertion-StartingPointofRipple;
							}

						if (SC) SClist.Add(SC); //yes keep adding to the list so that we can determine the correct starting point for the elements that exist after the inserted icons

						//renumber everything always... when we find a clip then renumber it and
						//move on to the next stage
						RenumberCondition=true;
						if (GetTLEtype(tle->GetContentInstance())==TLE_Clip) {
							ClipsStartingPoint=tle->GetGlobalStartPoint();
							RCStages++;
							}
						break;
					case 1:
						//renumber everything always while our starting points are matching to the
						//clip found in stage 0
						RenumberCondition=true;
						if (tle->GetGlobalStartPoint()==ClipsStartingPoint) break;
						RCStages++;
					case 2:
						//And finally renumber all events which are valid
						RenumberCondition=(!(tle->GetStoryBoardPosition()==-1));
						break;
					}

				if (RenumberCondition) {
					SetStoryBoardPosition(&SClist,tle,sbPos++);
					}
				//This is left expanded for debugging
				if (!(SC->m_activeRegion==0)) Ripple=false;
				tletype=GetTLEtype(tle);
				if ((tletype==TLE_Clip)||(tletype==TLE_Trans)||(tletype==TLE_Audio)) {
					if (tle->GetGlobalStartPoint()<StartingPointofInsertion) goto ExitNow;
					}
				//I don't like this one but we have to know if a transition is part of an active region kind of clip
				if (tletype==TLE_Trans) {
					TimeLineElement *PreviousElement;
					PreviousElement=FindLastEvent(&SClist,TLE_Clip);
					if (PreviousElement) 
						if ((PreviousElement->m_activeGlobalRegions.GetNumRanges()>1)&&(PreviousElement->GetGlobalStartPoint()<=StartingPointofInsertion)) 
							goto ExitNow;
					}
				//TODO branch past this line to disable the ripple mode
				if (Ripple) tle->SetGlobalStartPoint(tle->GetGlobalStartPoint()+Adjustment);
				Scan=Scan->Next;
				}
			}

		//Help Rick by giving him the ClipB's to transitions
		//ProvideClipBtoTimeLine(Ret);
		//Finally update everything
ExitNow:
		m_timeLine->ContentRegionEditor_CompleteRegionChanges();
		}

	// Success
	return Ret;
	}

  /***************************************************************************************/
 /*							This Section deals with Refresh								*/
/***************************************************************************************/
//SortCondition-Establishes the rules of how to sort TLE's
//SortTLEs-This uses the staight insertion algorithm to sort TimeLineElements
//VideoEditor_Layout_Deferred_VerifyAllChildren-  This is the main function which handles refreshing the StoryBoard

bool StoryBoard_Layout::SortCondition(VideoEditor_Layout_ChildItem *compvar,VideoEditor_Layout_ChildItem *scanvar) {
	bool result=false;
	// Pedantic checking
	if ((compvar->GetTimeLineElement()==NULL)||(scanvar->GetTimeLineElement()==NULL)) return false;
	int sbPosA=compvar->GetTimeLineElement()->GetStoryBoardPosition();
	int sbPosB=scanvar->GetTimeLineElement()->GetStoryBoardPosition();
	//if ((!(sbPosA==-1))&&(!(sbPosB==-1))&&(compvar->instanceOrder==0)&&(scanvar->instanceOrder==0)) {
	if ((!(sbPosA==-1))&&(!(sbPosB==-1))) {
		//1) ClipA is numbered, and ClipB is numbered, and ClipA's number comes before ClipBs number
		if (sbPosA<sbPosB) result=true;
		}
	else {
		//2) If ClipA is not numbered or ClipB is nor numbered and ClipA starts before ClipB.
		double StartA=compvar->start;
		double StartB=scanvar->start;
		if (!(StartA==StartB)) {
			if (StartA<StartB) result=true;
			}
		else {
			//3) If the start times are identical then sort by types.  The types priority are as follows:
			//   Transition, Clip, OverlayVideo, Audio, OverlayAudio
			TLEtype typeA=GetTLEtype(compvar->GetTimeLineElement()->GetContentInstance());
			TLEtype typeB=GetTLEtype(scanvar->GetTimeLineElement()->GetContentInstance());
			if (typeA<typeB) result=true;
			}
		}
	return result;
	}

void StoryBoard_Layout::SortTLEs() {
	unsigned n=m_childItems.NoItems;
	int i,j;
	VideoEditor_Layout_ChildItem *a;

	for (j=2;j<=n;j++) {
		a=m_childItems[j-1];
		i=j-1;
		while (i>0 && SortCondition(a,m_childItems[i-1])) {
			m_childItems[i]=m_childItems[i-1];
			i--;
			}
		m_childItems[i]=a;
		}
	//Keep This for test purposes Very Useful!
	/*
	for (i=0;i<n;i++) {
		a=m_childItems[i];
		DebugOutput("%d,%d\n",i,a->GetTimeLineElement()->GetStoryBoardPosition());
		}
	*/
	}

void StoryBoard_Layout::VideoEditor_Layout_Deferred_VerifyAllChildren() {
	if (!this->m_timeLine) return;

	// Do not do any layouts until we are done!
	MultipleLayouts_Start();


	// Find out my proper list of children
	StoryBoard_Layout_OrderElements();

	//Iterate through the timeline element and add/delete croutons as needed
	WindowLayout_Item *startingScan = GetLayout(GetWindowHandle());
	WindowLayout_Item *Scan = startingScan;
	unsigned eoi=m_childItems.NoItems;

	for (unsigned i=0;i<eoi;i++) {
		TimeLineElement *tle=(m_childItems[i])->GetTimeLineElement();
		StoryBoard2_Crouton	*SC=NULL;

		//Make sure we have a valid StoryBoard Crouton
		if (Scan) {
			BaseWindowClass *BWC=GetWindowInterface<BaseWindowClass>(Scan->hWnd);
			if (BWC) {	
				SC=GetInterface<StoryBoard2_Crouton>(BWC);
				m_childItems[i]->hWnd = SC->GetWindowHandle();
				}
			}
		else {
			//TODO pull a new window from a holding buffer
			HWND MyWindow=OpenChild("StoryBoard2_Crouton");
			m_childItems[i]->hWnd=MyWindow;
			SC=GetWindowInterface<StoryBoard2_Crouton>(MyWindow);
			}
		if (SC) {  //We really should always have a crouton here
			//Set our croutons attr to our data struct
			//TODO there are some cases where we'll have more croutons that represent the same data struct, but
			//not the other way around... this feature needs to be added for now we are doing a one for one
			SC->m_activeRegion = (m_childItems[i])->instanceOrder;
			SC->VideoEditor_Crouton_SetContentInstance(tle->GetContentInstance());
			}
		if (Scan) Scan=Scan->Next;
		}
	while (Scan) {
		//TODO send excess to a holding buffer
		//for now we purge..
		BaseWindowClass *BWC=GetWindowInterface<BaseWindowClass>(Scan->hWnd);
		NewTek_Delete(BWC);
		Scan=Scan->Next;
		}

	// Clean up the WindowLayout Items
	this->FreeLayout(startingScan);

	// Go ahead and finish the layout
	MultipleLayouts_Finished(true);
	// Layout_PerformLayout(this->GetWindowHandle(), true);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
bool StoryBoard_Layout::VideoEditor_Layout_IsChangeRelevant(ContentRegion* change)
{
	if (!change) return false;
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void StoryBoard_Layout::VideoEditor_Layout_RemoveTimeLineElement(TimeLineElement* removeMe)
{
	if (removeMe && this->m_timeLine)
		this->m_timeLine->RemoveTimeLineElement(removeMe);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**/
//We'll activate this again once the active regions are resolved

void StoryBoard_Layout::StoryBoard_Layout_OrderElements()
{
	// Pedantic
	if (!this->m_timeLine) return;

	// Iterate through the list of TimeLineElements
	unsigned currentChild = 0;
	for (unsigned i = 0; i < this->m_timeLine->GetNumTimeLineElements(); i++)
	{
		TimeLineElement* thisElement = this->m_timeLine->GetTimeLineElement(i);
		
		// Iterate through each active region
		for (unsigned j = 0; j < thisElement->m_activeGlobalRegions.GetNumRanges(); j++)
		{
			// Get a VideoEditor_Layout_ChildItem (from the list or a new one)
			VideoEditor_Layout_ChildItem* item = NULL;
			if (this->m_childItems.NoItems > currentChild)
				item = m_childItems[currentChild];
			else
			{
				item = new VideoEditor_Layout_ChildItem();
				m_childItems.Add(item);
			}
			currentChild++;
			
			// Set all of the proper values
			item->hWnd = NULL;
			item->instanceOrder = j;
			item->SetTimeLineElement(thisElement);
			thisElement->m_activeGlobalRegions.GetRange(j, item->start, item->end);
		}
	}

	// Remove all extra items from my list
	for (unsigned babyKiller = currentChild; babyKiller < m_childItems.NoItems; babyKiller++)
		m_childItems.DeleteEntry(currentChild);

	// Order this completed list of Child elements
	SortTLEs();
	}
/**/
/*
//This version is a temporary fix to list all timeline elements bypassing the active regions
void StoryBoard_Layout::StoryBoard_Layout_OrderElements() {
	// Pedantic
	if (!this->m_timeLine) return;

	unsigned eoi=m_timeLine->GetNumTimeLineElements();
	unsigned currentChild = 0;

	for (unsigned i=0;i<eoi;i++) {
		TimeLineElement* thisElement = this->m_timeLine->GetTimeLineElement(i);
		// Get a VideoEditor_Layout_ChildItem (from the list or a new one)
		VideoEditor_Layout_ChildItem* item = NULL;

		if (this->m_childItems.NoItems > currentChild)
			item = m_childItems[currentChild];
		else {
			item = new VideoEditor_Layout_ChildItem();
			m_childItems.Add(item);
			}
		currentChild++;
		
		// Set all of the proper values
		item->hWnd = NULL;
		item->instanceOrder = 0;
		item->SetTimeLineElement(thisElement);
		thisElement->m_activeGlobalRegions.GetRange(0, item->start, item->end);
		}
	// Remove all extra items from my list
	for (unsigned babyKiller = currentChild; babyKiller < m_childItems.NoItems; babyKiller++)
		m_childItems.DeleteEntry(currentChild);

	// Order this completed list of Child elements
	SortTLEs();
	}
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
