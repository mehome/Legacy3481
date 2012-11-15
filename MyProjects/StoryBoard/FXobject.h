#ifndef FXOBJECT_H
#define FXOBJECT_H

//dve structs
class generalFX {
	public:
	BOOL reverse;
	//dofx parms
	struct imagelist *dve;
	ULONG *imagea;
	ULONG *imageb;
	ULONG *videobuf;
	ULONG dvetime;

	virtual void openprefs()=0;
	virtual void doFX(struct imagelist *dve,ULONG *imagea,ULONG *imageb,ULONG *videobuf,ULONG dvetime,BOOL field)=0;
	virtual ~generalFX();
	};

enum alphablendtypes {afx_soft,afx_medium,afx_hard};
	//medium will use the softness parm, hard is the gradient one
class alphaFX : public messagebase,public generalFX {
	public:
	int softness;
	alphablendtypes alphablendtype;

	int Callback(HWND w_ptr, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void openprefs();
	void doFX(struct imagelist *dve,ULONG *imagea,ULONG *imageb,ULONG *videobuf,ULONG dvetime,BOOL field);
	alphaFX(struct imagelist *mediaptr);
	~alphaFX();

	private:
	HWND window,softRD,mediumRD,hardRD;
	HWND reverseCHK;
	HGLOBAL mediaalpha;
	alphablendtypes tempabtype;
	BOOL tempreverse;
	};

#endif /* STRUCTGO_H */
