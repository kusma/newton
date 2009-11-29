#define _USE_MATH_DEFINES

#include "GbaSceneMtl.h"
#include "resource.h"
#include "stdmat.h"
#include "iparamm2.h"
#include <d3dx9.h>
#include "ihardwarematerial.h"
#include "ihardwareshader.h"
#include "ihardwarerenderer.h"
#include <map>

using namespace gbaSceneMtlStuff;

class GbaSceneMtlCD : public ClassDesc2 {
public:
	int		 		IsPublic(){ return GetAppID() != kAPP_VIZR; }
	void *			Create(BOOL loading);
	const TCHAR *	ClassName() {return _T("GBA Skene Mtl"); } 
	SClass_ID		SuperClassID() {return MATERIAL_CLASS_ID;}
	Class_ID 		ClassID() {return GBASCENEMATERIAL_CLASS_ID;}
	const TCHAR* 	Category() {return _T("");}
	const TCHAR*	InternalName() { return _T("GbaSceneMtl"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
};
static GbaSceneMtlCD classDesc;
ClassDesc2* GetMaterialDesc(){ return &classDesc; }







static const char* subtexNames[] = {
	"Primary", "Secondary"
};

#define PBLOCKOFFS      0
#define SUBTEXOFFS		1
#define SUBTEXCOUNT		2

// subtex
#define REFCOUNT		(SUBTEXCOUNT+1)








class GbaSceneMtl : public Mtl {
	std::map<GeomObject*, Mesh*> meshMap;
public:
	IParamBlock2*			pblock;
	Texmap*					subtex[SUBTEXCOUNT];

	GbaSceneMtl(BOOL loading);
	void NotifyChanged() {NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);}

	// From MtlBase and Mtl
	void SetAmbient(Color c, TimeValue t) {}		
	void SetDiffuse(Color c, TimeValue t) {}		
	void SetSpecular(Color c, TimeValue t) {}
	void SetShininess(float v, TimeValue t) {}				

	// dette er ad-hoc, og det skal det få være ei stund :)
	Color GetAmbient(int mtlNum=0, BOOL backFace=FALSE) { return Color(0,0,0); }
	Color GetDiffuse(int mtlNum=0, BOOL backFace=FALSE) { return Color(0,0,0); }
	Color GetSpecular(int mtlNum=0, BOOL backFace=FALSE) { return Color(0,0,0); }
	float GetXParency(int mtlNum=0, BOOL backFace=FALSE) { return 0.f; }
	float GetShininess(int mtlNum=0, BOOL backFace=FALSE) { return 0.f; }
	float GetShinStr(int mtlNum=0, BOOL backFace=FALSE) { return 0.f; }
	//BOOL  GetSelfIllumColorOn(int mtlNum, BOOL backFace) { return TRUE; }
	//Color GetSelfIllumColor(int mtlNum, BOOL backFace) { return pblock->GetColor(genDiffCol); }
	float GetSelfIllum(int mtlNum, BOOL backFace) { return 1.f; }

	void  Shade(ShadeContext& sc);


	ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);


	void Update(TimeValue t, Interval& valid);
	void Init();
	void Reset();
	Interval Validity(TimeValue t) { return FOREVER; }

	Class_ID ClassID()			{ return classDesc.ClassID(); }
	SClass_ID SuperClassID()	{ return classDesc.SuperClassID(); }
	void GetClassName(TSTR& s)	{ s=classDesc.ClassName(); }  
	void DeleteThis()			{ delete this; }	

	// Methods to access sub texture maps of material or texmap
	int NumSubTexmaps()	{ return SUBTEXCOUNT; }
	Texmap* GetSubTexmap(int i) { 
		if (i<SUBTEXCOUNT) return (Texmap*)GetReference(i+SUBTEXOFFS);
		return NULL;
	} 
	void SetSubTexmap(int i, Texmap *m)  { 
		if (i<SUBTEXCOUNT) {
			ReplaceReference(i+SUBTEXOFFS,m);
//			if (i==texPrimary && SupportTexDisplay()) {
//				SetMtlFlag(MTL_TEX_DISPLAY_ENABLED, TRUE);
//				GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
//			}
		}
	}
	TSTR GetSubTexmapSlotName(int i)	 { 
		if (i<SUBTEXCOUNT) return _T(subtexNames[i]); 
		return _T("");
	}

	// From ref
	int NumRefs() { return REFCOUNT; }
	RefTargetHandle GetReference(int i)  {
		if (i==PBLOCKOFFS) return pblock;
		int subtexi = i-SUBTEXOFFS;
		if (subtexi>=0 && subtexi<SUBTEXCOUNT) return subtex[subtexi];
		return NULL;
	}
	void SetReference(int i, RefTargetHandle rtarg) { 
		if (i==PBLOCKOFFS) pblock = (IParamBlock2*)rtarg;
		int subtexi = i-SUBTEXOFFS;
		if (subtexi>=0 && subtexi<SUBTEXCOUNT) {
			subtex[subtexi] = (Texmap*)rtarg;
//			trashTexHandles();
		}
	}
	RefTargetHandle Clone(RemapDir &remap = NoRemap());
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);

	// IO
	IOResult Save(ISave *isave); 
	IOResult Load(ILoad *iload); 

	// direct ParamBlock access
	int	NumParamBlocks()				{ return 1; }
	IParamBlock2* GetParamBlock(int i)	{ 
		if (i==PBLOCKOFFS) return (IParamBlock2*)GetReference(i); 
		return NULL;
	}	
	IParamBlock2* GetParamBlockByID(BlockID id) { 
		if (pblock->ID()==id) return pblock;
		return NULL;
	}


};
void* GbaSceneMtlCD::Create(BOOL loading){ return new GbaSceneMtl(loading); }

static ParamBlockDesc2 general_blk (pbGeneral, _T("general"),  0, &classDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, PBLOCKOFFS, 
	//rollout
	IDD_GBASCENEMTL, IDS_PARAMS_GENERAL, 0, 0, NULL,
	// params
	genFiller,			_T("genFiller"), TYPE_INT, 0, 0, 
		p_ui,			TYPE_RADIO,	3,	IDC_FILLER_TEX256, IDC_FILLER_ESPENBUMP, IDC_FILLER_NMAP,
		end,
	genUvGen,			_T("genUvGen"), TYPE_INT, 0, 0, 
		p_ui,			TYPE_RADIO,	2,	IDC_UVGEN_MAPCHAN, IDC_UVGEN_ENV,
		end,
	genTexPrimary,		_T("genTexPrimary"), TYPE_TEXMAP, P_OWNERS_REF, 0,
		p_subtexno,		texPrimary,	
		p_refno,		SUBTEXOFFS+texPrimary,
		p_ui,			TYPE_TEXMAPBUTTON, IDC_TEX_PRIMARY,
		end,
	genTexSecondary,	_T("genTexSecondary"), TYPE_TEXMAP, P_OWNERS_REF, 0,
		p_subtexno,		texSecondary,	
		p_refno,		SUBTEXOFFS+texSecondary,
		p_ui,			TYPE_TEXMAPBUTTON, IDC_TEX_SECONDARY,
		end,
	genBumpSrc,			_T("genBumpSrc"), TYPE_INODE, 0, 0,
		p_ui,			TYPE_PICKNODEBUTTON, IDC_BUMPSRC,
		end,
	genOneone,			_T("genOneone"), 	TYPE_BOOL, 	0,	0,
		p_default, 		FALSE, 
		p_ui,			TYPE_SINGLECHEKBOX, IDC_CHECK_ONEXONE,
		end,
	end
);

// stuff for updating the gui
namespace {
	ICustButton* bumpSrcCam=NULL;
	void updateBumpSrc(IParamBlock2* pb) {
		if (!bumpSrcCam) return;
		if (pb->GetINode(genBumpSrc)==NULL) {
			bumpSrcCam->SetCheck(true);
			bumpSrcCam->Enable(false);
		} else {
			bumpSrcCam->SetCheck(false);
			int filler = pb->GetInt(genFiller);
			bumpSrcCam->Enable(filler!=0);
		}
	}
	void updateGui(IParamBlock2* pb, HWND hWnd) {
		IParamMap2 *map = pb->GetMap();
		HWND texPrimaryCaption = GetDlgItem(hWnd,IDC_STATIC_PRIMARY);
		HWND texSecondaryCaption = GetDlgItem(hWnd,IDC_STATIC_SECONDARY);
		int filler = pb->GetInt(genFiller);

		if (filler==0) {
			SendMessage(texPrimaryCaption, WM_SETTEXT, 0, (LPARAM)"Texture");
			SendMessage(texSecondaryCaption, WM_SETTEXT, 0, (LPARAM)"(unused)");
			map->Enable(genTexSecondary, false);
			map->Enable(genBumpSrc, false);
			bumpSrcCam->Enable(false);
		} else {
			map->Enable(genTexSecondary, true);
			map->Enable(genBumpSrc, true);
			bumpSrcCam->Enable(true);
		}
		
		if (filler==1) {
			SendMessage(texPrimaryCaption, WM_SETTEXT, 0, (LPARAM)"Normalmap");
			SendMessage(texSecondaryCaption, WM_SETTEXT, 0, (LPARAM)"Envmap");
		} else if (filler==2) {
			SendMessage(texPrimaryCaption, WM_SETTEXT, 0, (LPARAM)"Normalmap");
			SendMessage(texSecondaryCaption, WM_SETTEXT, 0, (LPARAM)"Color Table (preview)");
		}
	}
};


//dialog stuff to get the Set Ref button
class GbaSceneMtlDlgProc : public ParamMap2UserDlgProc {
public:
	void DeleteThis() {delete this;}
	BOOL GbaSceneMtlDlgProc::DlgProc(TimeValue t,IParamMap2 *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
		int id = LOWORD(wParam);
		int code = HIWORD(wParam);
		GbaSceneMtl* mtl = (GbaSceneMtl*)map->GetParamBlock()->GetOwner();
		IParamBlock2* pb = map->GetParamBlock();

		switch (msg) {
			case WM_INITDIALOG:
				bumpSrcCam 	 = GetICustButton( GetDlgItem(hWnd,IDC_BUMPSRC_CAMERA) );
				bumpSrcCam->SetType(CBT_CHECK);
				updateGui(pb, hWnd);
				updateBumpSrc(pb);
				break;
			case WM_DESTROY:
				ReleaseICustButton(bumpSrcCam);
				bumpSrcCam=NULL;
				break;
			case WM_COMMAND:
			case WM_NOTIFY:
				if (id==IDC_BUMPSRC_CAMERA) {
					if (bumpSrcCam->IsChecked()) {
						pb->SetValue(genBumpSrc, 0, NULL);
					} else if (!pb->GetINode(genBumpSrc)) {
						bumpSrcCam->SetCheck(true);
					}
				}
				updateGui(pb, hWnd);
				updateBumpSrc(pb);
				break;
			default:;
		}
		return FALSE;
	}
};


GbaSceneMtl::GbaSceneMtl(BOOL loading) 
{	
	pblock=NULL;
	for (int i=0; i<SUBTEXCOUNT; i++) subtex[i]=NULL;
	if (!loading) {
		classDesc.MakeAutoParamBlocks(this);	// make and intialize paramblock2
		Init();
	}
}

void GbaSceneMtl::Init()
{
	classDesc.Reset(this, TRUE);	// reset all pb2's
}

void GbaSceneMtl::Reset()
{
	Init();
}
		
ParamDlg* GbaSceneMtl::CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp)
{
	// create the rollout dialogs
	IAutoMParamDlg* masterDlg = classDesc.CreateParamDlgs(hwMtlEdit, imp, this);
	GbaSceneMtlDlgProc* mpParamDlg = new GbaSceneMtlDlgProc();
	general_blk.SetUserDlgProc( mpParamDlg );
	Update(imp->GetTime(), FOREVER);
	return masterDlg;
}



RefTargetHandle GbaSceneMtl::Clone(RemapDir &remap)
{
	GbaSceneMtl *mtl = new GbaSceneMtl(FALSE);
	*((MtlBase*)mtl) = *((MtlBase*)this);  // copy superclass stuff
	for (int i=0; i<NumRefs(); i++) mtl->ReplaceReference(i, remap.CloneRef( GetReference(i) ) );
	BaseClone(this, mtl, remap);
	return mtl;
}

RefResult GbaSceneMtl::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message)
{
	switch (message) {
		case REFMSG_CHANGE:
			if (hTarget==pblock) {
				IParamBlock2* pb = (IParamBlock2*)hTarget;
				ParamID changing_param = pb->LastNotifyParamID();				
				pb->GetDesc()->InvalidateUI(changing_param);
				updateBumpSrc(pblock);
			}
			break;
	}
	return REF_SUCCEED;
}






/////////////////////////////////////////////////////////////////
//
//	Save & Load
//
// Note: ALL Materials and texmaps must have a Save and Load to save and load
// the MtlBase info.
#define HEADER_CHUNK 0x4000
#define VERSION_CHUNK 0x1000

#define CURRENT_VERSION 0x1

IOResult GbaSceneMtl::Save(ISave *isave) 
{ 
	IOResult res;

	// Save common stuff
	isave->BeginChunk(HEADER_CHUNK);
	res = MtlBase::Save(isave);
	if (res!=IO_OK) return res;
	isave->EndChunk();

	isave->BeginChunk(VERSION_CHUNK);
	int version = CURRENT_VERSION;
	ULONG	nBytes;
	isave->Write( &version, sizeof(version), &nBytes );			
	isave->EndChunk();

	return IO_OK;
}	
	  



IOResult GbaSceneMtl::Load(ILoad *iload) 
{ 
	IOResult res;
	int version;

	while (IO_OK==(res=iload->OpenChunk()))
	{
		switch( iload->CurChunkID() ){
			case HEADER_CHUNK:
				res = MtlBase::Load(iload);
				break;
			case VERSION_CHUNK:
				ULONG nb;
				res = iload->Read(&version, sizeof(version), &nb);
				break;
		} // end switch
		
		iload->CloseChunk();
	
		if( res != IO_OK ) 
			return res;
	
	} // end, while still chunks

	return IO_OK;
}











namespace {

	Color colErr(1,1,1);

	Color lookupTex(Texmap* texmap, float u, float v, bool clamp=false) {
		if (!texmap || texmap->ClassID()!=Class_ID(0x240, 0)) return colErr;
 		BitmapTex* bitmapTex = (BitmapTex*)texmap;
		Bitmap* bitmap=NULL;
		try {
			bitmap = bitmapTex->GetBitmap(0);
		} catch (...) {
			return colErr;
		}
		if (!bitmap) return colErr;

		int x = int(u*bitmap->Width());
		int y = int(v*bitmap->Height()) % bitmap->Height();
		if (clamp) {
			if (x<0) x=0;
			if (y<0) y=0;
			if (x>=bitmap->Width()) x=bitmap->Width()-1;
			if (y>=bitmap->Height()) y=bitmap->Height()-1;
		} else {
			x %= bitmap->Width();
			y %= bitmap->Height();
			if (x<0) x+= bitmap->Width();
			if (y<0) y+= bitmap->Height();
		}

		BMM_Color_fl bmmColor;
		bitmap->GetPixels(x, y, 1, &bmmColor);

		return bmmColor;
	}

	Color evalTex(ShadeContext& sc, Texmap* texmap, int mapChannel) {
		Point3 uvw = sc.UVW(mapChannel);		
		return lookupTex(texmap, uvw.x, 1.f-uvw.y);
	}


	Color evalEnvTex(ShadeContext& sc, Texmap* texmap) {
		Point3 uvw = sc.Normal();
		float u = (1.f+uvw.x)*0.5f;
		float v = (1.f-uvw.y)*0.5f;
	 	return lookupTex(texmap, u, v);
	}

	float reducePrecision(float op, int bits) {
		int tmp = int(op * (1<<bits)); // 1 sign, 4 bits
		return tmp * (1.f/(1<<bits));
	}
	Point3 reducePrecision(Point3 op, int bits) {
		op.x = reducePrecision(op.x, bits);
		op.y = reducePrecision(op.y, bits);
		op.z = reducePrecision(op.z, bits);
		return op;
	}

};

class NullView : public View {
public:
	Point2 ViewToScreen(Point3 p)
	{ return Point2(p.x,p.y); }

	NullView() {
		worldToView.IdentityMatrix();
		screenW=240.0f; screenH = 160.0f;
	}
};

void GbaSceneMtl::Update(TimeValue t, Interval& valid) {
	meshMap.clear();
}

void  GbaSceneMtl::Shade(ShadeContext& sc) { 
	if (gbufID) sc.SetGBufferID(gbufID);
	sc.out.Reset();
	sc.out.t = Color(0, 0, 0);
	sc.out.c = Color(0, 0, 0);
	sc.out.ior = 1.f;

// 	float nearPlane = sc.CamNearRange();
	float nearPlane = 0.1f;
	if (sc.globContext->GetViewParams()) {
		nearPlane = sc.globContext->GetViewParams()->hither;
 	}

 	Object* object = sc.GetEvalObject();
  	if (!object || object->SuperClassID()!=GEOMOBJECT_CLASS_ID) return;

	Mesh* mesh=NULL;
	if (meshMap.count((GeomObject*)object)) {
		mesh = meshMap[(GeomObject*)object];
	} else {
		assert(NULL != object);
		BOOL deleteThis = false;
 		mesh = ((GeomObject*)object)->GetRenderMesh(sc.CurTime(), sc.Node(), NullView(), deleteThis);
		if (NULL == mesh) return;
		meshMap[(GeomObject*)object] = mesh;
	}

 	int faceno = sc.FaceNumber();
	if (mesh && faceno >=0 && faceno < mesh->getNumFaces()) {
  		Face& face = mesh->faces[faceno];
  		for (int i=0; i<3; i++) {
 			Point3 p = mesh->verts[face.v[i]];
 			p = sc.PointFrom(p, REF_OBJECT);
			p = sc.PointTo(p, REF_CAMERA);
			if (-p.z < nearPlane) {
				sc.out.c = Color(1, 0, 0);
				return;
			}
		} 
	} else {
		sc.out.c = Color(1, 0, 0);
	}


	int filler = pblock->GetInt(genFiller);
	int uvGen  = pblock->GetInt(genUvGen);

	Color primary=colErr;
	if (subtex[texPrimary]) {
		if (uvGen==0) {
			primary = evalTex(sc, subtex[texPrimary], subtex[texPrimary]->GetMapChannel());
		} else {
			primary = evalEnvTex(sc, subtex[texPrimary]);
		}
	}

	Color outcol = colErr;
	

	Point3 lightVecObj;
	if (filler==1 || filler==2) {

		// find lightvec in object space
		Matrix3 nodeWorld = sc.Node()->GetObjectTM(sc.CurTime());
		INode* bumpSrcNode = pblock->GetINode(genBumpSrc);
		if (bumpSrcNode) {
			Point3 bumpSrcWorld = bumpSrcNode->GetObjectTM(sc.CurTime()).GetTrans();
			Point3 bumpSrcView = sc.PointFrom(bumpSrcWorld, REF_WORLD);
			Point3 bumpSrcObj = sc.PointTo(bumpSrcView, REF_OBJECT);
			lightVecObj = bumpSrcObj;
		} else {
			Point3 camObj = sc.PointTo(sc.CamPos(), REF_OBJECT);
			lightVecObj = camObj;
		}
 		lightVecObj.Unify(); 
		//lightVecObj = reducePrecision(lightVecObj, 4);
	}

	
	if (filler==0) {  
		outcol=primary;
	} else if (filler==1 && subtex[texSecondary]) {
		// espenbump
    	Point3 bumpNormal = primary*2.f - Point3(1.f, 1.f, 1.f);
		bumpNormal.Unify();

		float la = atan2(lightVecObj.x, lightVecObj.y) * (1.f/M_PI);
		 
 		// change bump to cylinder coords
   		float h  = -asin(bumpNormal.z) * (1.0/M_PI) + 0.5f;
  		float na =  atan2(bumpNormal.x, bumpNormal.y) * (0.5/M_PI) + 0.5f;
 		h = reducePrecision(h, 8);
		if (h<0) h=0;
		if (h>0.99f) h=0.99f;
   		na = reducePrecision(na, 8);
		na = (na-0.5f)*2.f;
 
		// draw 
 		float a = (la-na) + 0.5f;
  		outcol = lookupTex(subtex[texSecondary], a, h, false);
	} else if (filler==2) {
 		Point3 bumpNormal = reducePrecision(primary*2.f - Point3(1.f, 1.f, 1.f), 4);

		// linear normal map
		float dot = DotProd(bumpNormal, lightVecObj);

		// lookup
		if (subtex[texSecondary]) {
			outcol = lookupTex(subtex[texSecondary], dot*0.5f+0.5f, 0.f, true);
		} else {
			float d = max(0.f, dot);
			outcol = Color(d, d, d);
		}
	}
/* IKKE SÅNN
 	// shadows!
	for (int lighti=0; lighti<sc.nLights; lighti++) {
		LightDesc* l = sc.Light(lighti);

		// kan dette lyset kaste skygge egentlig?

 		Ray ray;
		ray.p = sc.P();
		ray.dir = l->LightPosition() - sc.P();
		ISect iSect;
		ISectList iSectList;
		sc.globContext->IntersectWorld(ray, sc.NodeID(), iSect, iSectList);
		if (iSect.t < 0.9f) {
			outcol *= 0.5f;
		}
	}
*/

	sc.out.c = outcol; 
}


