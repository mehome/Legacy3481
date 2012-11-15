/**************************************************************************
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 *  Copyright (c) 1993 - 1998  Microsoft Corporation.  All Rights Reserved.
 *
 **************************************************************************/

#include <streams.h>

#ifndef __Crossbar_h__
#define __Crossbar_h__

class CRouting {
public:
    class CRouting       *pLeftRouting;
    class CRouting       *pRightRouting;
    LONG                 VideoInputIndex;
    LONG                 VideoOutputIndex;
    LONG                 AudioInputIndex;
    LONG                 AudioOutputIndex;
    IAMCrossbar         *pXbar;
    LONG                 InputPhysicalType;
    LONG                 OutputPhysicalType;
    LONG                 Depth;

    CRouting () {};
    ~CRouting () {};
};

typedef CGenericList<CRouting> CRoutingList;

class CCrossbar
{
private:
    IPin                    *m_pStartingPin;
    CRouting                 m_RoutingRoot;
    CRoutingList            *m_RoutingList;
    int                      m_CurrentRoutingIndex;

    HRESULT BuildRoutingList (
                IPin     *pStartingInputPin,
                CRouting *pCRouting,
                int       Depth);
    HRESULT SaveRouting (CRouting *pRoutingNew);
    HRESULT DestroyRoutingList();
    BOOL    StringFromPinType (TCHAR *pc, int nSize, long lType);
    
    HRESULT GetCrossbarIPinAtIndex(
                IAMCrossbar *pXbar,
                LONG PinIndex,
                BOOL IsInputPin,
                IPin ** ppPin);
    HRESULT GetCrossbarIndexFromIPin (
                IAMCrossbar * pXbar,
                LONG * PinIndex,
                BOOL IsInputPin,
                IPin * pPin);

public:

    CCrossbar (IPin *pPin);             
    ~CCrossbar();

    HRESULT GetInputCount (LONG *pCount);
    HRESULT GetInputType  (LONG Index, LONG * PhysicalType);
    HRESULT GetInputName  (LONG Index, TCHAR * pName, LONG NameSize);
    HRESULT SetInputIndex (LONG Index);
    HRESULT GetInputIndex (LONG *Index);

};

#endif  // __Crossbar_h__
