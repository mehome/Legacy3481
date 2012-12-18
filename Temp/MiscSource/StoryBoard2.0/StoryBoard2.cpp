#include "StdAfx.h"


StoryBoard2::StoryBoard2() {
	m_TimeLine=&m_DefaultTimeLine;
	m_TimeLine->AddDependant(this);
	}

void StoryBoard2::SetTimeLineToEdit(TimeLine* timeLine) {
	// Remove any prior dependance
	if (m_TimeLine) m_TimeLine->DeleteDependant(this);

	// Setup any new dependancy
	if (timeLine) {
		m_TimeLine=timeLine;
		timeLine->AddDependant(this);
		DynamicCallback(0,"",NULL,timeLine);
		}
	else m_TimeLine=&m_DefaultTimeLine;
	}

void StoryBoard2::InitialiseWindow(void) {
	UtilLib_SimpleGrid_PutSpacingX(5);
	UtilLib_SimpleGrid_PutSpacingY(5);
	UtilLib_SimpleGrid::InitialiseWindow();
	}

StoryBoard2::~StoryBoard2() {
	m_TimeLine->DeleteDependant(this);
	}

bool StoryBoard2::DragNDrop_CanItemBeDroppedHere(HWND hWnd,Control_DragNDrop_DropInfo *Dropped) {
	FileButton* thisButton;
	if (thisButton = GetWindowInterface<FileButton>(Dropped->hWnd)) {
		FileProperties_Status status=(FileProperties_Status)thisButton->GetProperties()->GetFileStatus();
		if (status==FileProperties_File) return true;
		}
	return false;
	}

TimeLineElement *StoryBoard2::GetTLE(WindowLayout_Item *icon) {
	if (icon) {
		StoryBoard2_Crouton *SC=GetInterface<StoryBoard2_Crouton>(GetWindowInterface<BaseWindowClass>(icon->hWnd));
		if (SC) return SC->GetTLE();
		}
	return NULL;
	}

StoryBoard2::TLEtype StoryBoard2::GetTLEtype(ContentSource *source) {
	//Dynamic Casting is Good!   -Andrew  :)
	//Having one exit point is Good too -James ;)
	TLEtype TheType=TLE_Utility;
	TimeLineElement *tle=GetInterface<TimeLineElement>(source);
	bool overlay=0;
	if (tle) overlay=tle->GetOverlay();
	if (GetInterface<Transition_ContentSource>(source)) TheType=TLE_Trans;
	else {
		Audio_ContentSource *IsAudio=GetInterface<Audio_ContentSource>(source);
		Video_ContentSource *IsVideo=GetInterface<Video_ContentSource>(source);
		if ((IsAudio)&&(!IsVideo)) TheType=TLE_Audio;
		else if (IsVideo) TheType=TLE_Clip;
		}
	if (overlay) {
		if (TheType==TLE_Audio) TheType=TLE_OverAudio;
		else TheType=TLE_Over;
		}
	return TheType;
	}



  /***************************************************************************************/
 /*							This Section deals with Deletion							*/
/***************************************************************************************/



void StoryBoard2::Ripple(double StartTime,double Adjustment) {
	//Use negative numbers to cut time
	//I'm not going to renumber the TLE sbPos here.

	//Insertion operations will take care of filling in the holes...
	//and that operation can handle it a lot more efficiently

	TimeLine *l_TimeLine=m_TimeLine;
	TimeLineElement *tle;
	//it is probably faster to go through the entire list than to sort the elements
	unsigned eoi=l_TimeLine->GetNumTimeLineElements();
	for (unsigned i=0;i<eoi;i++) {
		tle=l_TimeLine->GetTimeLineElement(i);
		if (tle->GetContentInstance()->ContentInstance_GetClipLength()>=StartTime) 
			tle->SetGlobalStartPoint(tle->GetGlobalStartPoint()+Adjustment);
		}
	}

void StoryBoard2::DeleteTLE(TimeLineElement *tle) {
	if (!tle) return;
	m_TimeLine->ContentRegionEditor_StartRegionChanges();
	TLEtype type=GetTLEtype(tle->GetContentInstance()->GetContentSource());
	//TODO see if we have ripple mode... and for the transitions see if we have insert mode enabled
	if (type==TLE_Clip) Ripple(tle->GetGlobalStartPoint(),(tle->GetContentInstance()->ContentInstance_GetClipLength())*-1);
	else if (type==TLE_Trans) Ripple(tle->GetGlobalStartPoint(),tle->GetContentInstance()->ContentInstance_GetClipLength());
	m_TimeLine->RemoveTimeLineElement(tle);
	m_TimeLine->ContentRegionEditor_CompleteRegionChanges();
	}

void StoryBoard2::DeleteSelectedButtons() {
	WindowLayout_Item *Scan=GetLayout(GetWindowHandle());
	FileButton *button;
	StoryBoard2_Crouton *sc;
	TimeLineElement *tle;

	while (Scan) {
		BaseWindowClass *BWC=GetWindowInterface<BaseWindowClass>(Scan->hWnd);
		if (button=GetInterface<FileButton>(BWC)) {
			if (button->DragAndDrop_AmISelected()) {
				if (sc=GetInterface<StoryBoard2_Crouton>(button)) {
					//DebugOutput("Delete:%s\n",sc->GetProperties()->GetFileName());
					if (tle=sc->GetTLE()) DeleteTLE(tle);					
					}
				}
			}
		Scan=Scan->Next;
		}
	}



  /***************************************************************************************/
 /*							This Section deals with Insertion							*/
/***************************************************************************************/
//NumOrderPrev-This will number the previous crouton(s) of the inserted clip(s)
//FindLastEvent-Given a tList and valid type(s) it will find the last event
//ComputeStartingPoint-This calculates the starting point for the inserted/next element
//UseNewEventSPFromPrev-This determines if the previous clip can be used for ComputeStarting point
//GetPrevAcceptableEvent-This checks if the previous eventtype is compatible with the inserted clip
//DragNDrop_DropItemsHere-This is the area that handles insertion



int StoryBoard2::NumOrderPrev(tList<StoryBoard2_Crouton *> *SClist,int DefaultValue) {
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
			if ((!(Duration==PrevDuration))||(GetTLEtype(tle->GetContentInstance()->GetContentSource())==TLE_Clip)) break;
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
			(*SClist)[index]->GetTLE()->SetStoryBoardPosition(OrderNumber++);
			}
		return (OrderNumber);
		}
	return (DefaultValue+1);
	}

TimeLineElement *StoryBoard2::FindLastEvent(tList<StoryBoard2_Crouton *> *SClist,int flags,unsigned *index) {
	TimeLineElement *tle=NULL;
	for (unsigned i=SClist->NoItems;i>0;i--) {
		if ((GetTLEtype((tle=(*SClist)[i-1]->GetTLE())->GetContentInstance()->GetContentSource()))&flags) break;
		}
	if (index) *index=i; //this answer is one based
	return tle;
	}

double StoryBoard2::ComputeStartingPoint(TimeLineElement *PrevElement,ContentInstance *ci) {
	//Important Note... This function will return the same starting point as the previous element 
	//(work like an overlay) if the PrevElement is not a valid reference.  This means you will have to check 
	//that your PrevElement is valid.

	if ((!(PrevElement))||(!(ci))) return 0;
	TLEtype PrevElementType=GetTLEtype(PrevElement->GetContentInstance()->GetContentSource());
	TLEtype ElementType=GetTLEtype(ci->GetContentSource());
	double StartingPoint=PrevElement->GetGlobalStartPoint();
	double Duration=PrevElement->GetContentInstance()->ContentInstance_GetClipLength();
	//The computation for all overlay types and Ultility is already computed
	switch (ElementType) {
		case TLE_Clip:
			//CC start + duration
			if (PrevElementType==TLE_Clip) {
				StartingPoint+=Duration;
				break;
				}
			//TC start
			//if (PrevElementType==TLE_Trans) {}
			break;

		case TLE_Trans:
			//CT max(start,start+duration-transduration)
			if (PrevElementType==TLE_Clip) {
				//TODO in insert transition mode we'll need to recompute this
				StartingPoint=max(StartingPoint,Duration-ci->ContentInstance_GetClipLength());
				//TODO we'll have to check to clip the transition
				break;
				}
			//TT start + duration
			if (PrevElementType==TLE_Trans) {
				StartingPoint+=Duration;
				}
			break;

		case TLE_Audio:
			//AA start + duration
			if (PrevElementType==TLE_Clip) {
				StartingPoint+=Duration;
				break;
				}
			//CA start
			//if (PrevElementType==TLE_Audio) {}
			break;
		}

	return StartingPoint;
	}

bool StoryBoard2::UseNewEventSPFromPrev(ContentSource *PrevSource,ContentSource *Source) {
	if ((PrevSource==NULL)||(Source==NULL)) return false;
	TLEtype PrevElementType=GetTLEtype(PrevSource);
	TLEtype ElementType=GetTLEtype(Source);
	int flag=GetPrevAcceptableEvent(ElementType);
	return (flag&PrevElementType);
	}

int StoryBoard2::GetPrevAcceptableEvent(TLEtype EventType) {
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

WindowLayout_Item *StoryBoard2::DragNDrop_DropItemsHere
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

		m_TimeLine->ContentRegionEditor_StartRegionChanges();

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
					if (!(SC->GetTLE()->GetStoryBoardPosition()==-1)) SC->GetTLE()->SetStoryBoardPosition(lastposition++);
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
			ContentSource *source;

			while(Scan) {
				// We need to get the BaseWindowClass interface
				BaseWindowClass *BWC=GetWindowInterface<BaseWindowClass>(Scan->hWnd);
				if (BWC) {
					FB=GetInterface<FileButton>(BWC);
					SC=GetInterface<StoryBoard2_Crouton>(BWC);
					if (SC) {
						(newtle=SC->GetTLE())->SetStoryBoardPosition(lastposition++);
						if (FirstLoop==true) {
							FirstLoop=false;
							//We need to find the initial starting point of insertion
							//figure startingpoint of insertion
							source=newtle->GetContentInstance()->GetContentSource();
							PreviousElement=FindLastEvent(&SClist,GetPrevAcceptableEvent(GetTLEtype(source)));
							StartingPointofInsertion=ComputeStartingPoint(PreviousElement,newtle->GetContentInstance());
							}
						else {
							if (UseNewEventSPFromPrev(tle->GetContentInstance()->GetContentSource(),newtle->GetContentInstance()->GetContentSource()))
								PreviousElement=tle;
							}
						tle=newtle;
						tle->SetGlobalStartPoint(ComputeStartingPoint(PreviousElement,newtle->GetContentInstance()));
						}
					// If this is a FileButton, but NOT a crouton already
					else if (FB) {

						// We create a new window 
						HWND MyWindow=OpenChild("StoryBoard2_Crouton",0,0,100,100,WS_CHILD);
						SC=GetWindowInterface<StoryBoard2_Crouton>(MyWindow);

						if (SC) {
							// And place me in the original position
							Scan->hWnd=MyWindow;

							//Copy the contents of the filebutton into the crouton and delete the filebutton
							FB->CopyTo(SC);
							NewTek_Delete(FB);
							// We make sure all of our colors and size match
							Scan->XSize=SC->GetWindowWidth();
							Scan->YSize=SC->GetWindowHeight();
							}

						//create a new instance from the File Properties
						ContentInstance *ci=ContentInstance::GetContentInstanceFromFile(SC->GetProperties());
						//for now we'll check for NULL at this level... we may want to check this earlier
						//in the can items be dropped here function.  This ensures that the FileButton
						//can be a legal StoryBoard Crouton
						if (ci) {
							if (FirstLoop==true) {
								FirstLoop=false;
								//We need to find the initial starting point of insertion
								//figure startingpoint of insertion
								source=ci->GetContentSource();
								PreviousElement=FindLastEvent(&SClist,GetPrevAcceptableEvent(GetTLEtype(source)));
								StartingPointofInsertion=ComputeStartingPoint(PreviousElement,ci);
								}
							else {
								if (UseNewEventSPFromPrev(tle->GetContentInstance()->GetContentSource(),ci->GetContentSource()))
									PreviousElement=tle;
								}
							tle=m_TimeLine->AddTimeLineElement(ci,ComputeStartingPoint(PreviousElement,ci),0,lastposition++);
							}
						}
					}

				if (Scan==LastObjectDropped) {
					//we can make the adjustment here since we don't need adjustments if we hit NULL
					Scan=Scan->Next;
					if (Scan) {
						EndingPointofInsertion=ComputeStartingPoint(tle,GetTLE(Scan)->GetContentInstance());
						Adjustment=EndingPointofInsertion-StartingPointofInsertion;
						}
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

			//Renumber the data structs and ripple the time
			while (Scan) {

				SC=GetInterface<StoryBoard2_Crouton>(GetWindowInterface<BaseWindowClass>(Scan->hWnd));
				TimeLineElement *tle=SC->GetTLE();
				switch (RCStages) {
					case 0:
						//renumber everything always... when we find a clip then renumber it and
						//move on to the next stage
						RenumberCondition=true;
						if (GetTLEtype(tle->GetContentInstance()->GetContentSource())==TLE_Clip) {
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
					tle->SetStoryBoardPosition(sbPos++);
					}
				//TODO branch past this line to disable the ripple mode
				tle->SetGlobalStartPoint(tle->GetGlobalStartPoint()+Adjustment);
				Scan=Scan->Next;
				}
			}

		//Finally update everything
		m_TimeLine->ContentRegionEditor_CompleteRegionChanges();
		}

	// Success
	return Ret;
	}



  /***************************************************************************************/
 /*							This Section deals with Refresh								*/
/***************************************************************************************/
//SortCondition-Establishes the rules of how to sort TLE's
//SortTLEs-This uses the staight insertion algorithm to sort TimeLineElements
//DynamicCallback- Listens for Changed() messages and calls the DeferredMessage
//ReceiveDeferredMessage-  This is the main function which handles refreshing the StoryBoard



bool StoryBoard2::SortCondition(TimeLineElement *compvar,TimeLineElement *scanvar) {
	bool result=false;
	int sbPosA=compvar->GetStoryBoardPosition();
	int sbPosB=scanvar->GetStoryBoardPosition();
	if ((!(sbPosA==-1))&&(!(sbPosB==-1))) {
		//1) ClipA is numbered, and ClipB is numbered, and ClipA's number comes before ClipBs number
		if (sbPosA<sbPosB) result=true;
		}
	else {
		//2) If ClipA is not numbered or ClipB is nor numbered and ClipA starts before ClipB.
		double StartA=compvar->GetGlobalStartPoint();
		double StartB=scanvar->GetGlobalStartPoint();
		if (!(StartA==StartB)) {
			if (StartA<StartB) result=true;
			}
		else {
			//3) If the start times are identical then sort by types.  The types priority are as follows:
			//   Transition, Clip, OverlayVideo, Audio, OverlayAudio
			TLEtype typeA=GetTLEtype(compvar->GetContentInstance()->GetContentSource());
			TLEtype typeB=GetTLEtype(compvar->GetContentInstance()->GetContentSource());
			if (typeA>typeB) result=true;
			}
		}
	return result;
	}

void StoryBoard2::SortTLEs() {
	TimeLine *l_TimeLine=m_TimeLine; //we always have a timeline
	unsigned n=l_TimeLine->GetNumTimeLineElements();
	int i,j;
	TimeLineElement *a;

	for (j=2;j<=n;j++) {
		a=l_TimeLine->GetTimeLineElement(j-1);
		i=j-1;
		while (i>0 && SortCondition(a,l_TimeLine->GetTimeLineElement(i-1))) {
			l_TimeLine->SetTimeLineElement(i,l_TimeLine->GetTimeLineElement(i-1));
			i--;
			}
		l_TimeLine->SetTimeLineElement(i,a);
		}
	}


void StoryBoard2::DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging) {
	if (IsDeletion(String)) {
		if (ItemChanging==m_TimeLine) 
			m_TimeLine=&m_DefaultTimeLine;
		}

	if ((!(strcmp(String,ContentRegion_Changed)))&&(ItemChanging==m_TimeLine))
		DeferredMessage();
	//Call our predecessor
	UtilLib_Canvas::DynamicCallback(ID,String,args,ItemChanging);
	}

void StoryBoard2::ReceiveDeferredMessage(unsigned ID1,unsigned ID2) {
	SortTLEs();
	//Iterate through the timeline element and add/delete croutons as needed
	WindowLayout_Item *Scan=GetLayout(GetWindowHandle());
	unsigned eoi=m_TimeLine->GetNumTimeLineElements();

	// We do not want to do lots of layouts
	MultipleLayouts_Start();

	for (unsigned i=0;i<eoi;i++) {
		TimeLineElement *tle=m_TimeLine->GetTimeLineElement(i);
		StoryBoard2_Crouton	*SC=NULL;

		//Make sure we have a valid StoryBoard Crouton
		if (Scan) {
			BaseWindowClass *BWC=GetWindowInterface<BaseWindowClass>(Scan->hWnd);
			if (BWC) {	
				SC=GetInterface<StoryBoard2_Crouton>(BWC);
				}
			}
		else {
			//TODO pull a new window from a holding buffer
			HWND MyWindow=OpenChild("StoryBoard2_Crouton",0,0,100,100,WS_CHILD);
			SC=GetWindowInterface<StoryBoard2_Crouton>(MyWindow);
			}
		if (SC) {  //We really should always have a crouton here
			//Set our croutons attr to our data struct
			//TODO there are some cases where we'll have more croutons that represent the same data struct, but
			//not the other way around... this feature needs to be added for now we are doing a one for one
			char *filename=tle->GetContentInstance()->GetContentSource()->GetFileProperties()->GetFileName();
			SC->ChangeFileName(filename);
			SC->SetTLE(tle);
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
	MultipleLayouts_Finished(true);	
	}

