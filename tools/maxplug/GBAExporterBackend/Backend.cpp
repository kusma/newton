#define WIN32_LEAN_AND_MEAN
#define _USE_MATH_DEFINES
#include "../GBAPlugin/GbaSceneMtl.h"
#include <atlimage.h>
#include <windows.h>
#include "settings.h"
#include "resource.h"
#include <IGame.h>
#include <IGameObject.h>
#include <IGameProperty.h>
#include <IGameControl.h>
#include <IGameModifier.h>
#include <IConversionManager.h>
#include <IGameError.h>
#include "decomp.h"
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include "exstructs.h"
#include <GL/gl.h>
#include <GL/glu.h>
using namespace std;
#define FORI(a) for (int i=0; i<a; i++) 
#define FORJ(a) for (int j=0; j<a; j++) 
#define FORXY(a, b) for (int y=0; y<b; y++) for (int x=0; x<a; x++)  


char tempstr[256];

#define GBA_WIDTH  240
#define GBA_HEIGHT 160
// max st;rrelse - st;tter 3 supersample
#define MAX_SUPERSAMPLING 3
#define MAX_BUFFERSIZE (GBA_WIDTH*GBA_HEIGHT*MAX_SUPERSAMPLING*MAX_SUPERSAMPLING*3) 

//#define MAT4SKALERING 0.05f
HINSTANCE hInstance;
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	hInstance = hinstDLL;				// Hang on to this DLL's instance handle.
	static int controlsInit = FALSE;
	if (!controlsInit) {
		controlsInit = TRUE;
		InitCustomControls(hInstance);	// Initialize MAX's custom controls
		InitCommonControls();			// Initialize Win95 controls
	}
			
	return (TRUE);
}


//Error callback
class ExporterErrorCallback : public IGameErrorCallBack
{
public:
	void ErrorProc( IGameError Error )
	{
		TCHAR *pszErr = GetLastIGameErrorText();
		DebugPrint( "ErrorCode = %d ErrorText = %s\n", Error, pszErr );
//		message( pszErr );
	}
} kErrorCallback;


// Dummy function for progress bar
DWORD WINAPI fn2(LPVOID arg)
{
	return(0);
}

struct EdgeId{
	EdgeId(int vi0, int vi1) {
		if (vi0>vi1) swap(vi0,vi1);
		this->vi0 = vi0;
		this->vi1 = vi1;
	}
	int vi0, vi1;
	bool operator<(const EdgeId& op) const { 
		if (vi0!=op.vi0) return vi0<op.vi0;
		return vi1<op.vi1;
	}
};

struct WingedEdge {
	WingedEdge() : autoSmooth(false), visiblePolyCount(0), frontFacingPolyCount(0) {}
	std::vector<int> polys; // connected polys
	bool autoSmooth;
	int visiblePolyCount;
	int frontFacingPolyCount;
};

struct FrameState {
	Matrix3 view;
	Point3 viewPos;
	GMatrix proj;
	float n;
	std::vector<Matrix3> objecttm;
	std::vector<Point3> bumpvecs;
	std::vector<Point3> lights;
	std::vector<unsigned short> shadow_compr;
	//int activesilhouette_change;
	int shadow_compr_change;
	int lights_change;
	int bumpvecs_change;
	int view_change;
	std::vector<int> objecttm_change;
// precalc
	std::vector<GMatrix> mvp; // complete matrix
	std::vector<int>	 activeverts;
	std::vector<int>	 activefaces;
	std::vector<int>	 activefacemasks;
	std::vector<int>	 interleaved_faces;

//	std::vector<MyEdge>	 activesilhouette;


	std::vector<int>	 mvp_change;
//	int					 activefaces_change;
//	int					 activeverts_change;
	int pframe;
};

struct GbaMtl {
	UvGen uvGen;
	Filler filler;
	Texmap* texPrimary;
	Texmap* texSecondary;
	int oneone;
//	INode* bumpSrc;
};

struct FinalNode {
	IGameNode* base;
	IGameNode* bone;
	Matrix3 initSkinMat;
	Matrix3 initBoneMat;
	Matrix3 objOffsMat;
	void CalcStuff() {
		Matrix3 worldTM  = base->GetWorldTM(0).ExtractMatrix3();
		Matrix3 objectTM = base->GetObjectTM(0).ExtractMatrix3();
		Matrix3 invWorldTM = worldTM; 
		invWorldTM.Invert();
		Matrix3 invObjectTM = objectTM;
		invObjectTM.Invert();
		objOffsMat = objectTM * invWorldTM;
	}
	FinalNode(IGameNode* base) : base(base), bone(NULL) {
		initSkinMat.IdentityMatrix();
		initBoneMat.IdentityMatrix();
		CalcStuff();
	}
	FinalNode(IGameNode* base, IGameNode* bone, Matrix3 initSkinMat, Matrix3 initBoneMat) : base(base), bone(bone), initSkinMat(initSkinMat), initBoneMat(initBoneMat) {
		CalcStuff();	
	}
	bool operator==(const FinalNode& op) const {
		return base==op.base && bone==op.bone;
	}

	bool operator<(const FinalNode& op) const {
		if (base < op.base) return true;
		if (base > op.base) return false;
		return (bone < op.bone);
/*
		if     (base>op.base) return false;
		return (bone<op.bone); */
	}

	Matrix3 GetObjectTM(TimeValue t=TIME_NegInfinity) {
		assert(base);
		if (bone) {
#if 0
			// seems to work, but shouldnt (objOffsMat should be to the left)
			Matrix3 invInitBoneMat = initBoneMat;
			invInitBoneMat.Invert();
			// trial and failure led me here
			Matrix3 m = bone->GetObjectTM(t).ExtractMatrix3();
			return objOffsMat * invInitBoneMat * m;
#elif 0
			// doesnt work
			Matrix3 invInitBoneMat = initBoneMat;
			invInitBoneMat.Invert();

			Matrix3 m = bone->GetObjectTM(t).ExtractMatrix3();

			return objOffsMat * m * invInitBoneMat;
#elif 0
			Matrix3 invInitBoneMat = initBoneMat;
			invInitBoneMat.Invert();
			Matrix3 invInitSkinMat = initSkinMat;
			invInitSkinMat.Invert();

			Matrix3 poseBoneMat = bone->GetWorldTM(0).ExtractMatrix3();
			Matrix3 invPoseBoneMat = poseBoneMat;
			invPoseBoneMat.Invert();


			Matrix3 m = bone->GetWorldTM(t).ExtractMatrix3();
//			m = objOffsMat * m;
//			invInitBoneMat.NoScale();
//			m.NoScale();

			// er lik initSkinMat:
			// base->GetWorldTM(t).ExtractMatrix3();

		

			// objOffset
			// m
			// invPoseBoneMat
			// initSkinMat



			return objOffsMat * initSkinMat * invPoseBoneMat * m; // skal det være invInitBoneMat??
// 			return objOffsMat * invInitBoneMat * m;
#else
			Matrix3 invInitBoneMat = initBoneMat;
			invInitBoneMat.Invert();


			Matrix3 baseMat = base->GetWorldTM(t).ExtractMatrix3();
			Matrix3 boneMat = bone->GetWorldTM(t).ExtractMatrix3(); // World og object gir samma(?)
			

			return objOffsMat * initSkinMat * invInitBoneMat * boneMat;

#endif
		} else {
 			return objOffsMat * base->GetWorldTM(t).ExtractMatrix3();
		}
	}
};


namespace xpdb {
	IGameNode* skybox=NULL;
	std::map<std::string, IGameNode*> cameras;
	std::vector<IGameNode*> matteNodes;
	std::vector<IGameNode*> lights;
	std::vector<IGameNode*> splines;
	std::vector<GbaMtl>		mtls;
	std::vector<RomVertex>	verts;

	std::map<EdgeId, WingedEdge> wingedEdges;


	std::vector<RomUvs>		uvs;
	std::vector<RomFace>	faces;
	std::map<Mtl*, int>		mtl_lookup;
	std::map<RomUvs, int>	 uvs_lookup;
	std::map<RomVertex, int> vertex_lookup;
//	std::vector<IGameNode*>  meshnodes;
	std::vector<FinalNode>	 nodes;
	std::map<FinalNode, int> nodes_lookup;
	std::vector<FrameState>  framestates;
	int ss() {
		return min(global_settings.supersampling, MAX_SUPERSAMPLING);
	}
	int winw() { return GBA_WIDTH*ss(); }
	int winh() { return GBA_HEIGHT*ss(); }
	void reset() {
		skybox=NULL;
		splines.clear();
		lights.clear();
		cameras.clear();
		matteNodes.clear();
//		meshnodes.clear();
		nodes.clear();
		nodes_lookup.clear();
		verts.clear();
		faces.clear();
		vertex_lookup.clear();
		uvs_lookup.clear();
		mtl_lookup.clear();
		uvs.clear();
		mtls.clear();
		wingedEdges.clear();
	}
}

std::string formatfint(std::string op, bool filnavn=false) {
	// fjern alt foer slash
	if (filnavn) {
		size_t nPos = op.find_last_of("/\\");
		if (nPos>0) op = op.substr(nPos+1); 
	}

	// fjern tull
	std::string ret;
	for(const char* p = op.c_str(); *p; p++) {
		bool litenbokst = (*p >= 'a') && (*p <= 'z');
		bool storbokst  = (*p >= 'A') && (*p <= 'Z');
		bool tall  = (*p >= '0') && (*p <= '9');
		if (filnavn && *p=='.') break;
		if (!litenbokst && !storbokst && !tall) continue;
		char b = *p;
		if (storbokst) b = int(b) - int('A') + int('a');
		ret += b;
	}
	return ret;
}

std::string mekktexnavn(Texmap* tex) {
	assert(tex);
	if (tex->ClassID()==Class_ID(0x240, 0)) {
		std::string navn = ((BitmapTex*)tex)->GetMapName();
		return formatfint(navn, true);
	} else {
		std::string navn = tex->GetName();
		return formatfint(navn, false);
	}
}

bool isPFrame(int frame) {
	return (frame==0);
}





void cmdCompressPFrame(vector<int>& dst, vector<int> b) {
//	dst.push_back(CMD_INSERT | int(b.size()) );
	FORI(b.size()) dst.push_back(b[i]);
	return;
}

void cmdCompress(vector<int>& dst, vector<int>& prev, vector<int>& curr) {
	map<int, int> location;
	// store in map elements of prev
	FORI(prev.size()) location[prev[i]] = i; 

	FORI(curr.size()) {
		if (!location.count(curr[i])) {
			dst.push_back(curr[i]);
			continue;
		}
		int direction = 0;
		int match = location[curr[i]];

		// detect left or right search
		if (match+1<prev.size() && i+1<curr.size() && prev[match+1]==curr[i+1]) direction=1;
		else if (match-1>=0 && i-1>=0 && prev[match-1]==curr[i-1]) direction=-1;
		else {
			dst.push_back(curr[i]);
			continue;
		}

		int count = 0;

		// search
		while (1) {
			int previ = match+count*direction;
			int curri = i+count;
			if (previ>=prev.size() || previ<0) break;
			if (curri>=curr.size() || curri<0) break;
			if (prev[previ]!=curr[curri]) break;
			count++;
		}

		// store
		if (count>2) {
			assert(!(match&0x8000));
			dst.push_back(0x8000 | match);
			dst.push_back(count*direction);
			i += count-1;
		} else {
			dst.push_back(curr[i]);
		}
	}
}

// skriv den
void cmdUnCompress(vector<int>& dst, vector<int>& data, vector<int> prev) {
	int i=0;
	while (i<data.size()) {
		if (data[i]&0x8000) {
			int firstai = data[i++]&0x7FFF;
			int count = (signed short)data[i++];
			int direction = (count>0) ? 1 : -1;
			for (int j=firstai; j!=firstai+count; j+=direction) {
				dst.push_back(prev[j]);
			}
		} else {
			dst.push_back(data[i++]);
		}
	}
}



void check_range(float a, int b, const char* melding) {
	int tull = abs((int)a);
	int mask = ((1<<b)-1);
	if ((tull&mask) != tull) {
		global_settings.warning("Out of range!", melding);
 	}
}

#define CHECK_RANGE(a, b) check_range(a, b, __FUNCTION__)



RomUvs concat_uvs(Point2 p) {
	RomUvs uv;
	p.y = 1.f - p.y;
	uv.u = int(p.x*(1<<12))&0xFFFF;
	uv.v = int(p.y*(1<<12))&0xFFFF;
	return uv;
}

void concat_vert(RomVertex& vert, Point3 p, int prec) {
	p *= (1<<prec);
	CHECK_RANGE(p.x, 15);
	CHECK_RANGE(p.y, 15);
	CHECK_RANGE(p.z, 15);
	vert.x = int(p.x);
	vert.y = int(p.y);
	vert.z = int(p.z);
}

/*
void vert_from_world_space(RomVertex& vert, Point3 worldPos, bool use_object_space) {
	if (!use_object_space) {
		concat_vert(vert, worldPos);
	} else {
		Point3 objPos(0,0,0);
		for (int bonei=0; bonei<3; bonei++) {
			float w = vert.matrix[bonei].w * (1.0/255);
			if (w <= (1.0/256)) continue;
			IGameNode* node = xpdb::nodes[ vert.matrix[bonei].i - 1 ];
			Matrix3 world2obj = Inverse( node->GetObjectTM().ExtractMatrix3() );
			objPos += worldPos * world2obj * w;
		}
		concat_vert(vert, objPos);
	}
}
*/


/*
RomMatrix3 max2rom_matrix3(Matrix3 maxmat) {
	RomMatrix3 rommat;
	signed short* dst = rommat.m;
	for (int j=0; j<3; j++) {
		for (int i=0; i<4; i++) {
			Point3 row = maxmat.GetRow(i);
			float opp = (i==3) ? ((1<<PREC_VEC)) : ((1<<8));  // translasjon er 12:4, resten 8:8
			float boosted = row[j]*opp;
			CHECK_RANGE(boosted, 15);
			*dst++ = int(boosted);
		}
	}
	return rommat;
}
*/


RomMatrix4 max2rom_matrix4(GMatrix maxmat) {
	RomMatrix4 rommat;
	signed int* dst = rommat.m;
	float opp = (1<<PREC_MVP);
	for (int i=0; i<4; i++) {
 		Point4 row = maxmat.GetColumn(i);
		for (int j=0; j<4; j++) {
			float boosted = row[j]*opp;
			CHECK_RANGE(boosted, 31);
			*dst++ = int(boosted);
		}
	}
	return rommat;
}


bool needBumpVecMtl(Mtl* mtl) {
	if (!mtl) return false;
	if (mtl->ClassID()==GBASCENEMATERIAL_CLASS_ID) {
		Filler filler = (Filler)mtl->GetParamBlock(0)->GetInt(gbaSceneMtlStuff::genFiller);
		return filler!=FILLER_TEX256;
	} else {
		FORI(mtl->NumSubMtls()) {
			if (needBumpVecMtl(mtl->GetSubMtl(i))) return true;
		}
		return false;
	}
}

Mtl* findGbaMtl(Mtl* maxmtl) {
	if (!maxmtl) return NULL;
	if (maxmtl->ClassID()==GBASCENEMATERIAL_CLASS_ID) {
		return maxmtl;
	} else if (maxmtl->ClassID()==Class_ID(0x255, 0)) {
		return findGbaMtl(maxmtl->GetSubMtl(1));
	} else {
		if (maxmtl->NumSubMtls()) {
			return findGbaMtl(maxmtl->GetSubMtl(0));
		} else {
			return NULL;
		}
	}
}


/*
int analajs_material(Mtl* maxmtl, IGameNode* node) {
	if (xpdb::mtl_lookup.count(maxmtl)) {
		return xpdb::mtl_lookup[maxmtl];
	} else {
		if (maxmtl->ClassID()==Class_ID(0x255, 0)) {
			return analajs_material(maxmtl->GetSubMtl(1), node);
		} else if (maxmtl->ClassID()==GBASCENEMATERIAL_CLASS_ID) {
			IParamBlock2* pb = maxmtl->GetParamBlock(0);
			GbaMtl mtl;
			mtl.uvGen = pb->GetInt(gbaSceneMtlStuff::genUvGen);
			mtl.filler = pb->GetInt(gbaSceneMtlStuff::genFiller);
			mtl.texPrimary = pb->GetTexmap(gbaSceneMtlStuff::genTexPrimary);
			mtl.texSecondary = pb->GetTexmap(gbaSceneMtlStuff::genTexSecondary);
//			mtl.bumpSrc = pb->GetINode(gbaSceneMtlStuff::genBumpSrc);
			int mtli = (unsigned short)xpdb::mtls.size();
			xpdb::mtl_lookup[maxmtl] = mtli;
			xpdb::mtls.push_back(mtl);
			return mtli;
		} else {
			global_settings.warning("Tulling! Bruke GBA-material!", node->GetName());
			return -1;
		}
	}
}
*/

bool isMatteMtl(Mtl* mtl) {
	if (!mtl) return false;
	if (mtl->ClassID()==Class_ID(0x255, 0)) {
		return isMatteMtl(mtl->GetSubMtl(1));
	} else if (mtl->ClassID()==Class_ID(0x260, 0)) {
		return true;
	}
	return false;
}

bool isEnvMappedMtl(Mtl* mtl) {
	mtl = findGbaMtl(mtl);
	if (!mtl) return false;
	IParamBlock2* pb = mtl->GetParamBlock(0);
	UvGen uvGen = (UvGen)pb->GetInt(gbaSceneMtlStuff::genUvGen);
	return (uvGen == UVGEN_ENV);
}

int getNodeIndex(FinalNode node) {
	if (!node.base) return -1;

	if (xpdb::nodes_lookup.count(node)) {
		return xpdb::nodes_lookup[node];
	}
	int i = (int)xpdb::nodes.size()+1; // add 1 cause 0 has special meaning
	xpdb::nodes_lookup[node] = i;
	xpdb::nodes.push_back(node);
	return i;
}

int getVertexIndex(RomVertex& vert) {
	if ( xpdb::vertex_lookup.count(vert) ) {
		return xpdb::vertex_lookup[vert];
	} else {
		int index = (int)xpdb::verts.size();
		xpdb::vertex_lookup[vert] = index;
		xpdb::verts.push_back(vert);
		return index;
	}
}



int getMtlIndex(Mtl* maxmtl) {
	if (!maxmtl) return -1;
	if (xpdb::mtl_lookup.count(maxmtl)) {
		return xpdb::mtl_lookup[maxmtl];
	}
	IParamBlock2* pb = maxmtl->GetParamBlock(0);
	GbaMtl gbaMtl;
	gbaMtl.uvGen = (UvGen)pb->GetInt(gbaSceneMtlStuff::genUvGen);
	gbaMtl.filler = (Filler)pb->GetInt(gbaSceneMtlStuff::genFiller);
	gbaMtl.texPrimary = pb->GetTexmap(gbaSceneMtlStuff::genTexPrimary);
	gbaMtl.texSecondary = pb->GetTexmap(gbaSceneMtlStuff::genTexSecondary);
	gbaMtl.oneone = (pb->GetInt(gbaSceneMtlStuff::genOneone) ? 1 : 0);

	int i = (int)xpdb::mtls.size();
	xpdb::mtl_lookup[maxmtl] = i;
	xpdb::mtls.push_back(gbaMtl);
	return i;
}

int getMapChan(int mtli) {
	
	assert(mtli<xpdb::mtls.size());
	GbaMtl& gbaMtl = xpdb::mtls[mtli];
	if (gbaMtl.uvGen!=0 || !gbaMtl.texPrimary) return -1;
	return gbaMtl.texPrimary->GetMapChannel();
}


class BoneSorter {
public:
	BoneSorter(float w, IGameNode* n) : w(w), n(n) {}
	float w;
	IGameNode* n;
	bool operator<(const BoneSorter& op) const {
		return w > op.w;
	}
};

void analajs_node(IGameScene* igame, IGameNode* node, bool use_object_space=false)
{
	const char* name = node->GetName();

	// finn ut object/world space
	IGameControl* c = node->GetIGameControl();
	if (c && !use_object_space) {
		IGameKeyTab temp, temp2;
		c->GetQuickSampledKeys(temp, IGAME_POS);
		c->GetQuickSampledKeys(temp2, IGAME_ROT);
		if (temp.Count() || temp2.Count()) use_object_space=true;
	}
	if (global_settings.force_object_space) use_object_space = true;
/*	Control* maxc = node->GetMaxNode()->GetTMController();
	if (maxc && !use_object_space) {
		int poscount = 0;
		int rotcount = 0;
		if (IKeyControl* c = GetKeyControlInterface(maxc->GetPositionController())) poscount = c->GetNumKeys();
		if (IKeyControl* c = GetKeyControlInterface(maxc->GetRotationController())) rotcount = c->GetNumKeys();


		if (poscount || rotcount) use_object_space=true;
	}*/


	// almost forgot...
 	for (int i=0; i<node->GetChildCount(); i++) {
		analajs_node(igame, node->GetNodeChild(i), use_object_space);
	}

//	SelectObject(
//	node->GetMaxNode()->s

	IGameObject* obj = node->GetIGameObject();
	if (name && std::string(name)=="Skybox") xpdb::skybox=node;
	if (obj->GetIGameType()==IGameObject::IGAME_CAMERA) {
		xpdb::cameras[node->GetName()] = node;
	} else if (obj->GetIGameType()==IGameObject::IGAME_LIGHT) {
		xpdb::lights.push_back(node);
	} else if (obj->GetIGameType()==IGameObject::IGAME_SPLINE) {
		// add spline if it starts with a g :D
		IGameObject::MaxType maxType = node->GetIGameObject()->GetMaxType();

		if ((node->GetName()[0]=='g' || node->GetName()[0]=='G') && maxType==IGameObject::IGAME_MAX_SHAPE) {
			xpdb::splines.push_back(node);
		}
	} else if (obj->GetIGameType()==IGameObject::IGAME_MESH) {

		// dont export matte
		if (isMatteMtl(node->GetMaxNode()->GetMtl())) {
			xpdb::matteNodes.push_back(node);
			return;
		}
	
		// Is there skin??
		IGameSkin* skin=NULL;
		for (int i=0; i<obj->GetNumModifiers(); i++) {
			IGameModifier* modifier = obj->GetIGameModifier(i);
			if (modifier->GetModifierType()==IGameModifier::IGAME_SKINNING) {
				skin = (IGameSkin*)modifier;
				use_object_space = true;
			}
		}

		if (needBumpVecMtl(node->GetMaxNode()->GetMtl())) use_object_space = true;

		// get and init mesh object
		IGameMesh* mesh = (IGameMesh*)obj;  
		if (skin) mesh = skin->GetInitialPose();
		mesh->SetCreateOptimizedNormalList();
		mesh->SetUseWeightedNormals();
		mesh->InitializeData();

		// faces
		for (int facei=0; facei<mesh->GetNumberOfFaces(); facei++) {
			FaceEx* faceEx = mesh->GetFace(facei);
			RomFace f;
			memset(&f, 0, sizeof(f)); // fali?

			f.smGrp = faceEx->smGrp; // smuding group

			// material
			IGameMaterial* igMtl = mesh->GetMaterialFromFace(faceEx);
			if (!igMtl) {
				global_settings.warning("Mangler material!", node->GetName());
				continue;
			}
			Mtl* mtl = findGbaMtl(igMtl->GetMaxMaterial());
			if (!mtl) {
				global_settings.warning("Bruk GBA-material!", node->GetName());
				continue;
			}
			f.mtl = getMtlIndex(mtl);

			// need normals?
			bool isEnvMapped = isEnvMappedMtl(node->GetMaxNode()->GetMtl());


			// v
			for (int facevi=0; facevi<3; facevi++) {

				int meshvi = faceEx->vert[facevi];
				int meshni = faceEx->norm[facevi];

				RomVertex vert;
				vert.isNormal = 0;
				memset(vert.matrix, 0, sizeof(RomVertex::RomMatrixRef)*3);
				concat_vert(vert, mesh->GetVertex(meshvi, use_object_space), PREC_VEC);

				RomVertex norm;
				norm.isNormal = 1;
				memset(norm.matrix, 0, sizeof(RomVertex::RomMatrixRef)*3);
				if (isEnvMapped) {
					Point3 pnorm;
					bool gotNorm = mesh->GetNormal(meshni, pnorm, use_object_space);
					assert(gotNorm);
					concat_vert(norm, pnorm, PREC_NORM);
				}
				

				// insert matrices and get weights
//				assert(skin->GetNumOfSkinnedVerts() == mesh->GetNumberOfVerts());
				if (skin && skin->GetNumOfSkinnedVerts() == mesh->GetNumberOfVerts()) {
					int boneCount = skin->GetNumberOfBones(meshvi);


					// get weights
					static vector<BoneSorter> boneSorter;
					boneSorter.clear();
					for (int bonei=0; bonei<boneCount; bonei++ ) {
						float w = skin->GetWeight(meshvi, bonei);
						IGameNode* n = skin->GetIGameBone(meshvi, bonei);
						boneSorter.push_back(BoneSorter(w, n));
					}
					std::sort(boneSorter.begin(), boneSorter.end());


					// find total weight of 3 most important
					float totalWeight=0.f;
					for (int bonei=0; bonei<min(3,boneCount); bonei++ ) {
						totalWeight += boneSorter[bonei].w;
					}

					if (totalWeight<0.1) {
						vert.matrix[0].w = 255;
						vert.matrix[0].i = getNodeIndex( FinalNode(node) );
					} else {

						// store it
						GMatrix initSkinMatrix; // initial matrix for skinned meshes
						initSkinMatrix.SetIdentity();
						skin->GetInitSkinTM(initSkinMatrix);

						for (int bonei=0; bonei<min(3,boneCount); bonei++ ) {
							IGameNode* boneNode = boneSorter[bonei].n;
					
							GMatrix initBoneMatrix; // initial matrix for skinned meshes
							initBoneMatrix.SetIdentity();
							skin->GetInitBoneTM(boneNode, initBoneMatrix);

							vert.matrix[bonei].i = getNodeIndex( FinalNode(node, boneNode, initSkinMatrix.ExtractMatrix3(), initBoneMatrix.ExtractMatrix3()) );
							int w = (boneSorter[bonei].w*256) / totalWeight;
							vert.matrix[bonei].w = min(255, w);
						}
					}

				} else {
					vert.matrix[0].w = 255;
					if (use_object_space) {
						vert.matrix[0].i = getNodeIndex( FinalNode(node) );
					} else {
						vert.matrix[0].i = 0;
					}
				}

				// copy matrix settings to normal
				norm.matrix[0] = vert.matrix[0];
				norm.matrix[1] = vert.matrix[1];
				norm.matrix[2] = vert.matrix[2];

				f.v[facevi] = getVertexIndex(vert);
				if (isEnvMapped) f.uv[facevi] = getVertexIndex(norm);
			}

			// uv
			if (!isEnvMapped) {
				f.uv[0]=0;
				f.uv[1]=0;
				f.uv[2]=0;
				int mapChan = getMapChan(f.mtl);
				DWORD uvi1[3];
				if (mesh->GetMapFaceIndex(mapChan, facei, uvi1)) {
					for (int vi=0; vi<3; vi++) {
						Point3 p3 = mesh->GetMapVertex(mapChan, uvi1[vi]);
						Point2 p(p3.x, p3.y);
						RomUvs uv = concat_uvs( p );
						if ( xpdb::uvs_lookup.count(uv) ) {
							f.uv[vi] = xpdb::uvs_lookup[uv];
						} else {
							int index = (int)xpdb::uvs.size();
							f.uv[vi] = index;						
							xpdb::uvs_lookup[uv] = index;
							xpdb::uvs.push_back(uv);
						}
					}
				}
			}


			// now that we have our vertex indices, we can insert ourselves into the
			// winged edge list!
			int faceIndex = (int)xpdb::faces.size();
			for (int ei=0; ei<3; ei++) {
				int ei_next = (ei==2)?0:ei+1;
				EdgeId edgeId( f.v[ei], f.v[ei_next] );
				WingedEdge& wingedEdge = xpdb::wingedEdges[edgeId];
				wingedEdge.polys.push_back(faceIndex);
			}

			// push face
			xpdb::faces.push_back(f);
		}
	}
	node->ReleaseIGameObject();
}

#if 0
// should clone calculation in engine to prevent popping
Point4 vert_transform_mvp(RomVertex& v, FrameState& fs)
{
	Point4 res(0,0,0,0);
	FORI(3) {
		RomVertex::RomMatrixRef& mref = v.matrix[i];

		RomMatrix4 m_ = max2rom_matrix4( fs.mvp[mref.i] );
		int* m = m_.m;

		// gange dem i fixedpoint
		int x = (((long long)v.x*m[0x0] + (long long)v.y*m[0x1] + (long long)v.z*m[0x2])>>PREC_VEC)  + (m[0x3]);
		int y = (((long long)v.x*m[0x4] + (long long)v.y*m[0x5] + (long long)v.z*m[0x6])>>PREC_VEC)  + (m[0x7]);
		int z = (((long long)v.x*m[0x8] + (long long)v.y*m[0x9] + (long long)v.z*m[0xA])>>PREC_VEC)  + (m[0xB]);
		int w = (((long long)v.x*m[0xC] + (long long)v.y*m[0xD] + (long long)v.z*m[0xE])>>PREC_VEC)  + (m[0xF]);

		res += Point4(x, y, z, w) * float(mref.w) * (1.f/255) * (1.f/(1<<(PREC_MVP)));
	}

	return res;
}

#elif 1
Point4 vert_transform_mvp(RomVertex& v, FrameState& fs)
{
	Point4 res(0,0,0,0);
	FORI(3) {
		RomVertex::RomMatrixRef& mref = v.matrix[i];

		RomMatrix4 m_ = max2rom_matrix4( fs.mvp[mref.i] );
		int* m = m_.m;

		// gange dem i fixedpoint
 		int x = (((long long)v.x*m[0x0] + (long long)v.y*m[0x1] + (long long)v.z*m[0x2])>>PREC_VEC)  + (m[0x3]);
		int y = (((long long)v.x*m[0x4] + (long long)v.y*m[0x5] + (long long)v.z*m[0x6])>>PREC_VEC)  + (m[0x7]);
 		int z = (((long long)v.x*m[0x8] + (long long)v.y*m[0x9] + (long long)v.z*m[0xA])>>PREC_VEC)  + (m[0xB]);
		int w = (((long long)v.x*m[0xC] + (long long)v.y*m[0xD] + (long long)v.z*m[0xE])>>PREC_VEC)  + (m[0xF]);

		res += Point4(x, y, z, w) * float(mref.w) * (1.f/255) * (1.f/(1<<(PREC_MVP)));
	}

	return res;
}


Point3 vert_transform(RomVertex& v, FrameState& fs)
{
	float ner = 1.f/(1<<PREC_VEC);
	Point3 p(v.x*ner, v.y*ner, v.z*ner);

	Point3 res(0,0,0);
	FORI(3) {
		RomVertex::RomMatrixRef& m = v.matrix[i];
		float w =  m.w * (1.0/255);
		if (m.i==0) { 
			res += p * w;
		} else {
			res += (p * fs.objecttm[m.i-1]) * w;
		}
	}
	return res;
}
#else
Point4 vert_transform_mvp(RomVertex& v, FrameState& fs)
{
	Point4 res(0,0,0,0);

	RomVertex::RomMatrixRef& mref = v.matrix[0];

	RomMatrix4 m_ = max2rom_matrix4( fs.mvp[mref.i] );
	int* m = m_.m;

	// gange dem i fixedpoint
	int x = (((long long)v.x*m[0x0] + (long long)v.y*m[0x1] + (long long)v.z*m[0x2])>>PREC_VEC)  + (m[0x3]);
	int y = (((long long)v.x*m[0x4] + (long long)v.y*m[0x5] + (long long)v.z*m[0x6])>>PREC_VEC)  + (m[0x7]);
	int z = (((long long)v.x*m[0x8] + (long long)v.y*m[0x9] + (long long)v.z*m[0xA])>>PREC_VEC)  + (m[0xB]);
	int w = (((long long)v.x*m[0xC] + (long long)v.y*m[0xD] + (long long)v.z*m[0xE])>>PREC_VEC)  + (m[0xF]);

	res += Point4(x, y, z, w) * (1.f/(1<<PREC_MVP));

	return res;
}


Point3 vert_transform(RomVertex& v, FrameState& fs)
{
	float ner = 1.f/(1<<PREC_VEC);
	Point3 p(v.x*ner, v.y*ner, v.z*ner);

	Point3 res(0,0,0);

	RomVertex::RomMatrixRef& m = v.matrix[0];
	if (m.i==0) { 
		res += p;
	} else {
		res += p * fs.objecttm[m.i-1];
	}
	return res;
}
#endif

void multmatrix_gl(GMatrix mat) {
	float m[16];
	float* mp=m;
	for (int i=0; i<4; i++) {
		Point4 row = mat.GetRow(i);
		for (int j=0; j<4; j++) {
			*mp++ = row[j];
		}
	}
	glMultMatrixf(m);
}
void loadmatrix_gl(GMatrix mat) {
	glLoadIdentity();
	multmatrix_gl(mat);
}


void setfi_gl(int fi) {
	fi++;
	int r = (fi*9 + 45)&0xFF;
	int g = (fi>>8)&0xFF;
	int b = (fi)&0xFF;
	glColor3ub(r, g, b);
}

int getfi(GLubyte* p) {
	int g = p[1];
	int b = p[2];
	int fi = (g<<8)|b;
	return fi-1;
}

struct SortAf {
	int fi;
	float z;
	bool operator<(const SortAf& op) const {
		return z<op.z;
	}
};

#define LIGHTMAP_SIZE 512

PIXELFORMATDESCRIPTOR pfd = { 
	sizeof(PIXELFORMATDESCRIPTOR),  //  size of this pfd 
		1,                     // version number 
		PFD_DRAW_TO_WINDOW |   // support window 
		PFD_SUPPORT_OPENGL |   // support OpenGL 
		PFD_DOUBLEBUFFER,      // double buffered 
		PFD_TYPE_RGBA,         // RGBA type 
		24,                    // 24-bit color depth 
		0, 0, 0, 0, 0, 0,      // color bits ignored 
		0,                     // no alpha buffer 
		0,                     // shift bit ignored 
		0,                     // no accumulation buffer 
		0, 0, 0, 0,            // accum bits ignored 
		32,                    // 32-bit z-buffer     
		0,                     // no stencil buffer 
		0,                     // no auxiliary buffer 
		PFD_MAIN_PLANE,        // main layer 
		0,                     // reserved 
		0, 0, 0                // layer masks ignored 
};

// wgl pbuffer protos
DECLARE_HANDLE(HPBUFFERARB);
#define WGL_FRONT_LEFT_ARB                  0x2083
#define WGL_BACK_LEFT_ARB                   0x2085
#define		WGL_BIND_TO_TEXTURE_RGB_ARB    0x2070
#define		WGL_DRAW_TO_PBUFFER_ARB        0x202D
#define		WGL_STENCIL_BITS_ARB           0x2023
#define		WGL_DEPTH_BITS_ARB             0x2022
#define		WGL_SUPPORT_OPENGL_ARB         0x2010
#define WGL_TEXTURE_FORMAT_ARB             0x2072
#define WGL_TEXTURE_TARGET_ARB             0x2073
#define WGL_MIPMAP_TEXTURE_ARB             0x2074
#define WGL_TEXTURE_RGB_ARB                0x2075
#define WGL_TEXTURE_2D_ARB                 0x207A
#define WGL_PBUFFER_LARGEST_ARB        0x2033
#define WGL_PBUFFER_WIDTH_ARB          0x2034
#define WGL_PBUFFER_HEIGHT_ARB         0x2035

typedef BOOL (WINAPI * PFNWGLGETPIXELFORMATATTRIBIVARBPROC) (HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, int *piValues);
typedef BOOL (WINAPI * PFNWGLGETPIXELFORMATATTRIBFVARBPROC) (HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, FLOAT *pfValues);
typedef BOOL (WINAPI * PFNWGLCHOOSEPIXELFORMATARBPROC) (HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
typedef HPBUFFERARB (WINAPI * PFNWGLCREATEPBUFFERARBPROC) (HDC hDC, int iPixelFormat, int iWidth, int iHeight, const int *piAttribList);
typedef HDC (WINAPI * PFNWGLGETPBUFFERDCARBPROC) (HPBUFFERARB hPbuffer);
typedef int (WINAPI * PFNWGLRELEASEPBUFFERDCARBPROC) (HPBUFFERARB hPbuffer, HDC hDC);
typedef BOOL (WINAPI * PFNWGLDESTROYPBUFFERARBPROC) (HPBUFFERARB hPbuffer);
typedef BOOL (WINAPI * PFNWGLQUERYPBUFFERARBPROC) (HPBUFFERARB hPbuffer, int iAttribute, int *piValue);
typedef BOOL (WINAPI * PFNWGLBINDTEXIMAGEARBPROC) (HPBUFFERARB hPbuffer, int iBuffer);
typedef BOOL (WINAPI * PFNWGLRELEASETEXIMAGEARBPROC) (HPBUFFERARB hPbuffer, int iBuffer);
typedef BOOL (WINAPI * PFNWGLSETPBUFFERATTRIBARBPROC) (HPBUFFERARB hPbuffer, const int * piAttribList);
typedef BOOL (WINAPI * PFNWGLMAKECONTEXTCURRENTARBPROC) (HDC hDrawDC, HDC hReadDC, HGLRC hglrc);

/*
BOOL		WINAPI wglChoosePixelFormatARB (HDC, const int *, const FLOAT *, UINT, int *, UINT *);
HPBUFFERARB WINAPI wglCreatePbufferARB (HDC, int, int, int, const int *);
HDC		    WINAPI wglGetPbufferDCARB (HPBUFFERARB);
BOOL		WINAPI wglQueryPbufferARB (HPBUFFERARB, int, int *);
int			WINAPI wglReleasePbufferDCARB (HPBUFFERARB, HDC);
BOOL		WINAPI wglDestroyPbufferARB (HPBUFFERARB);
BOOL		WINAPI wglGetPixelFormatAttribivARB (HDC, int, int, UINT, const int *, int *);
BOOL		WINAPI wglGetPixelFormatAttribfvARB (HDC, int, int, UINT, const int *, FLOAT *);
BOOL		WINAPI wglBindTexImageARB (HPBUFFERARB, int);
BOOL		WINAPI wglReleaseTexImageARB (HPBUFFERARB, int);
BOOL		WINAPI wglSetPbufferAttribARB (HPBUFFERARB, const int *);
*/
int windowsfeil();

namespace gl{
	bool inited=false;
	HWND   hwnd;
	HDC    hdc;
	HGLRC  hglrc;
	HPBUFFERARB rt_pbuffer;
	HDC    rt_hdc;
	HGLRC  rt_hglrc;
	GLuint rt_tex;
	PFNWGLRELEASETEXIMAGEARBPROC	wglReleaseTexImageARB;
	PFNWGLBINDTEXIMAGEARBPROC		wglBindTexImageARB;
	PFNWGLMAKECONTEXTCURRENTARBPROC wglMakeContextCurrentARB;

	



	LRESULT CALLBACK MainWndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
		return DefWindowProc(hwnd,msg,wParam,lParam);
	}

	WNDCLASSEX wndClassDesc = {
		sizeof(WNDCLASSEX),
		0, // style
		(WNDPROC)MainWndProc, // WNDPROC lpfnWndProc;
		0, //int cbClsExtra;
		0, //int cbWndExtra;
		0, // HINSTANCE hInstance;
		NULL, // HICON hIcon;
		NULL, // HCURSOR hCursor;
		NULL, // HBRUSH hbrBackground;
		NULL, // LPCTSTR lpszMenuName;
		"hei", // LPCTSTR lpszClassName;
		NULL, // HICON hIconSm;
	};
	

	int init() {
		inited=true;											// endra her!!!
//		ATOM wndClass = RegisterClassEx(&wndClassDesc);

		RECT rect;
		rect.left = 0;
		rect.top = 0;
		rect.right = xpdb::winw();
		rect.bottom = xpdb::winh();
		unsigned int style = WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU;
		AdjustWindowRect(&rect, style, FALSE); /* adjust to fit border */
		//hwnd = CreateWindowEx(WS_EX_TOPMOST, "hei", "epilepsi-indikator", style|WS_VISIBLE, 0, 0, rect.right-rect.left, rect.bottom-rect.top, NULL, NULL, NULL, NULL); 
		hwnd = CreateWindowEx(WS_EX_STATICEDGE|WS_EX_TOOLWINDOW|WS_EX_TOPMOST, "STATIC", "epilepsi-indikator", WS_VISIBLE, 0, 0, rect.right-rect.left, rect.bottom-rect.top, NULL, NULL, NULL, NULL); 
		if (!hwnd) return windowsfeil();

		hdc = GetDC(hwnd);
		int iPixelFormat = ChoosePixelFormat(hdc, &pfd);
		SetPixelFormat(hdc, iPixelFormat, &pfd);
		hglrc = wglCreateContext(hdc); 
		if (!hglrc) return windowsfeil();
		wglMakeCurrent(hdc, hglrc);

		PFNWGLCHOOSEPIXELFORMATARBPROC	wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress( "wglChoosePixelFormatARB" );
		PFNWGLCREATEPBUFFERARBPROC		wglCreatePbufferARB = (PFNWGLCREATEPBUFFERARBPROC)wglGetProcAddress( "wglCreatePbufferARB" );
		PFNWGLGETPBUFFERDCARBPROC		wglGetPbufferDCARB = (PFNWGLGETPBUFFERDCARBPROC)wglGetProcAddress( "wglGetPbufferDCARB" );
		PFNWGLQUERYPBUFFERARBPROC		wglQueryPbufferARB = (PFNWGLQUERYPBUFFERARBPROC)wglGetProcAddress( "wglQueryPbufferARB" );
		wglReleaseTexImageARB = (PFNWGLRELEASETEXIMAGEARBPROC)wglGetProcAddress( "wglReleaseTexImageARB" );
		wglBindTexImageARB = (PFNWGLBINDTEXIMAGEARBPROC)wglGetProcAddress( "wglBindTexImageARB" );
		wglMakeContextCurrentARB = (PFNWGLMAKECONTEXTCURRENTARBPROC)wglGetProcAddress( "wglMakeContextCurrentARB" );

		// create a texture
		glGenTextures( 1, (GLuint*)&rt_tex );
		glBindTexture( GL_TEXTURE_2D, rt_tex );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, LIGHTMAP_SIZE, LIGHTMAP_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);


		// wtf
		const int pixelformat_attribs[] = {
				WGL_SUPPORT_OPENGL_ARB, TRUE,
				//WGL_STENCIL_BITS_ARB, 8,
				WGL_DRAW_TO_PBUFFER_ARB, TRUE,
				WGL_BIND_TO_TEXTURE_RGB_ARB, TRUE,
				0
		};
		int pixelformat, count;
		if (!wglChoosePixelFormatARB( hdc, pixelformat_attribs, 0, 1, &pixelformat, (UINT*)&count ) ) return windowsfeil();
//		if (count!=1) message("couldnt find suitable pixel format for pbuffer"); yeah well i'm not using shadowmaps anymore so who cares?
		
		
		const int pbuffer_attribs[] = {
				WGL_TEXTURE_FORMAT_ARB, WGL_TEXTURE_RGB_ARB,
				WGL_TEXTURE_TARGET_ARB, WGL_TEXTURE_2D_ARB,
				WGL_MIPMAP_TEXTURE_ARB, FALSE,
				WGL_PBUFFER_LARGEST_ARB, FALSE,
				0
		};

		rt_pbuffer = wglCreatePbufferARB( hdc, pixelformat, LIGHTMAP_SIZE, LIGHTMAP_SIZE, pbuffer_attribs );
		if (!rt_pbuffer) return windowsfeil();

		rt_hdc   = wglGetPbufferDCARB( rt_pbuffer );
		rt_hglrc = wglCreateContext( rt_hdc );
		if (!rt_hglrc) return windowsfeil();
		if (!wglShareLists( hglrc, rt_hglrc )) return windowsfeil();

		int w, h;
		wglQueryPbufferARB( rt_pbuffer, WGL_PBUFFER_WIDTH_ARB, (int*)&w );
		wglQueryPbufferARB( rt_pbuffer, WGL_PBUFFER_HEIGHT_ARB, (int*)&h );
		assert(w==LIGHTMAP_SIZE && h==LIGHTMAP_SIZE);

		gl::wglMakeContextCurrentARB(hdc, hdc, hglrc);
//		glDrawBuffer(GL_BACK);
//		glReadBuffer(GL_FRONT);
		glViewport(0, 0, xpdb::winw(), xpdb::winh());
		glEnable(GL_SCISSOR_TEST);
		glScissor(0, 0, xpdb::winw(), xpdb::winh());
		return 0;
	}

	void deinit() {
		if (!inited) return;
		inited=false;
		PFNWGLRELEASEPBUFFERDCARBPROC	wglReleasePbufferDCARB = (PFNWGLRELEASEPBUFFERDCARBPROC)wglGetProcAddress( "wglReleasePbufferDCARB" );
		PFNWGLDESTROYPBUFFERARBPROC		wglDestroyPbufferARB = (PFNWGLDESTROYPBUFFERARBPROC)wglGetProcAddress( "wglDestroyPbufferARB" );
		// pbuffer
		assert(wglGetCurrentContext() != rt_hglrc);
		wglDeleteContext( rt_hglrc );
		wglReleasePbufferDCARB( rt_pbuffer, rt_hdc );
		wglDestroyPbufferARB( rt_pbuffer );
		// resten
		wglDeleteContext(hglrc);
		ReleaseDC(hwnd, hdc);
		DestroyWindow(hwnd);
	}
}

// global
Interface* core;
IGameScene* igame;

void startthis(const TCHAR* name) {
	SetErrorCallBack( &kErrorCallback );
	core  = GetCOREInterface();
	igame = GetIGameInterface();
	xpdb::reset();
	global_settings.init( std::string(name) );
	global_settings.pre_export();
	igame->InitialiseIGame(false);
	igame->SetStaticFrame(0);
	GetConversionManager()->SetCoordSystem(IGameConversionManager::IGAME_OGL);
}
int endthis(int retval) {
	gl::deinit();
	core->ProgressEnd();
	igame->ReleaseIGame();
	global_settings.post_export();
	return retval;
}


int windowsfeil() {
	LPVOID lpMsgBuf;
	if (!FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL ))
	{
		message("Noe gikk galt da jeg prøvde å skaffe feilmelding fra winapi!!!!!");
		return endthis(IMPEXP_FAIL);
	}

	// Process any inserts in lpMsgBuf.
	// ...

	// Display the string.
	message((LPCTSTR)lpMsgBuf);
	LocalFree( lpMsgBuf );
	return endthis(IMPEXP_FAIL);
}

// camera er aligna i ei saer retning i 3ds...
Matrix3 unfuck_cam(Matrix3& mat) {
	Matrix3 ret;
	ret.SetRow(0,  mat.GetRow(0));
	ret.SetRow(1, -mat.GetRow(2));
	ret.SetRow(2,  mat.GetRow(1));
	ret.SetRow(3,  mat.GetRow(3));
	return ret;
}
Point3 unfuck_point(Point3& p) {
	return Point3(p.x, -p.z, p.y);
}

GMatrix makemat_proj(float yfov, float as, float n, float f) {
	GMatrix ret;
//	float sina = sin(a*0.5f);
//	float cosa = cos(a*0.5f);
	float a = 0.5f*(yfov/as);
	float cota = 1.f/tan(a);
	float Q = 1.f / (n-f);
	{
		// na i opengl-format... :-P
		Point4 row0(cota/as,	0.f,		 0.f,		0.f);
		Point4 row1(0.f,	    cota,		 0.f,		0.f);
		Point4 row2(0.f,		0.f,		 (f+n)*Q,  -1.f);
		Point4 row3(0.f,		0.f,		 2*f*n*Q,	0.f);
		ret.SetRow(0, row0);
		ret.SetRow(1, row1);
		ret.SetRow(2, row2);
		ret.SetRow(3, row3);
	}
	return ret;
}


int __declspec(dllexport) DoExport(const TCHAR *name, ExpInterface *ei, Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0) {


	startthis(name);
	if(!suppressPrompts) {
		// Prompt the user with our dialogbox, and get all the options.
		if (!DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_SETTINGS),
			i->GetMAXHWnd(), SettingsDlgProc, (LPARAM)&global_settings)) {
			return endthis(IMPEXP_CANCEL);
		}
	}


	core->ProgressStart(_T("Exporting.."), TRUE, fn2, NULL);

	// analajs nodes
	for (int nodei=0; nodei<igame->GetTopLevelNodeCount(); nodei++)
	{
		int progress = (100*nodei)/(igame->GetTopLevelNodeCount());
		core->ProgressUpdate(progress, false, "Exporting objects");
		analajs_node(igame, igame->GetTopLevelNode(nodei));
	}
	
	// set autosmooth parameters in wingedEdge lists
	for (map<EdgeId, WingedEdge>::iterator iter = xpdb::wingedEdges.begin(); iter!=xpdb::wingedEdges.end(); iter++) {
		WingedEdge& wingedEdge = iter->second;
		assert(wingedEdge.polys.size());
		wingedEdge.autoSmooth = (wingedEdge.polys.size()==1);
		const RomFace& firstFace = xpdb::faces[ wingedEdge.polys[0] ];
		for (int fi=1; fi<(int)wingedEdge.polys.size(); fi++) {
			const RomFace& face = xpdb::faces[ wingedEdge.polys[fi] ];
			// antialias the edge if different material
			if (firstFace.mtl != face.mtl) wingedEdge.autoSmooth=true;
			// antialias the edge if there is no common smoothing group
			if (firstFace.smGrp || face.smGrp) {
				int commonSmGrp = (firstFace.smGrp & face.smGrp);
				if (!commonSmGrp) wingedEdge.autoSmooth=true;
			}
		}
	}

	// analajs frames
	{
		//if (xpdb::cameras.count("Camera01")) cam = xpdb::cameras["Camera01"];
		//else if (xpdb::cameras.size()) cam = xpdb::cameras.begin()->second;
		int frame=0;
		std::map<std::string, IGameNode*>::iterator cameras_iter;
		for (cameras_iter=xpdb::cameras.begin(); cameras_iter!=xpdb::cameras.end(); cameras_iter++) {
			IGameNode* cam = cameras_iter->second;
			int lastPFrame=frame;
			TimeValue time = igame->GetSceneStartTime();
			for (; time<=igame->GetSceneEndTime(); frame++, time+=igame->GetSceneTicks()) {
			
			
				double prog = (time-igame->GetSceneStartTime()) / (double)(igame->GetSceneEndTime()-igame->GetSceneStartTime());
				core->ProgressUpdate(int(100*prog), true);

				FrameState fs;
				fs.pframe = lastPFrame;

				// igame fucks up this matrix' axes for some reason
				if (cam) {
					Matrix3 mat = cam->GetWorldTM(time).ExtractMatrix3();
					fs.viewPos = mat.GetTrans();
					fs.view = Inverse( unfuck_cam(mat) );
				} else {
					fs.view.IdentityMatrix();
				}

				for (int i=0; i<xpdb::lights.size(); i++) {
					IGameNode* node = xpdb::lights[i];
					Point3 p = node->GetWorldTM(time).ExtractMatrix3().GetRow(3);
					fs.lights.push_back(p);
				}

				{
					float n=0.1f;
					float f=1000.f;
					float a=float(M_PI/4.f);
					if (cam) {
						IGameCamera* c = (IGameCamera*)cam->GetIGameObject();
						if (IGameProperty* p = c->GetCameraNearClip()) p->GetPropertyValue(n, time);
						if (IGameProperty* p = c->GetCameraFarClip()) p->GetPropertyValue(f, time);
						if (IGameProperty* p = c->GetCameraFOV()) p->GetPropertyValue(a, time);
					}

					fs.n = n;
					fs.proj = makemat_proj(a/**1.04*/, GBA_WIDTH/(float)GBA_HEIGHT, n, f);

				}
				
				for (int i=0; i<(int)xpdb::nodes.size(); i++) {
					FinalNode node = xpdb::nodes[i];
					Matrix3 mat = node.GetObjectTM(time);
					// er det skybox?
					for (IGameNode* iter=node.base; iter; iter=iter->GetNodeParent()) {
						if (iter==xpdb::skybox) {
							// vi har skybox. flytt med kameraet
							mat *= TransMatrix(-fs.viewPos);
							break;
						}
					}
					fs.objecttm.push_back(mat);

					// lag bumpvec
					Mtl* gbaMtl = findGbaMtl( node.base->GetMaxNode()->GetMtl() );
					Point3 bumpVec(0,0,0);
					if (gbaMtl) {
						IParamBlock2* pb = gbaMtl->GetParamBlock(0);
						Filler filler = (Filler)pb->GetInt(gbaSceneMtlStuff::genFiller);
						if (filler==FILLER_NMAP || filler==FILLER_EBUMP) {
							INode* bumpSrc = pb->GetINode(gbaSceneMtlStuff::genBumpSrc);

							Point3 bumpSrcWorld;
							if (bumpSrc) {
								bumpSrcWorld = bumpSrc->GetNodeTM(time).GetTrans();
								bumpSrcWorld = unfuck_point(bumpSrcWorld) * Point3(1, -1, -1);
							} else {
								bumpSrcWorld = fs.viewPos;
							}

							// atten
							float attenuation = 1.f;
							
							if (filler==FILLER_NMAP && bumpSrc && bumpSrc->GetObjectRef() && bumpSrc->GetObjectRef()->SuperClassID()==LIGHT_CLASS_ID) {
								GenLight * maxLight = (GenLight*) bumpSrc->GetObjectRef();
								if (maxLight->GetUseAtten()) {
									float attstart = maxLight->GetAtten(time, ATTEN_START);
									float attend = maxLight->GetAtten(time, ATTEN_END);

									// scale attenuation to world space
									Matrix3 mat = bumpSrc->GetNodeTM(time);
									AffineParts ap;
									decomp_affine(mat, &ap); 
									float scale = (fabs(ap.k.x)+fabs(ap.k.y)+fabs(ap.k.z))*0.3333f;
									attstart *= scale;
									attend *= scale;

									Point3 worldVec = node.base->GetMaxNode()->GetNodeTM(time).GetTrans() - bumpSrc->GetNodeTM(time).GetTrans();
									float worldDist = worldVec.Length();

									if (worldDist < attstart) {
										attenuation=1.f;
									} else if (worldDist > attend) {
										attenuation=0.f;
									} else {
										attenuation= (attend-worldDist)/(attend-attstart);
										assert(attenuation>-0.1f && attenuation<1.1f);
									}
								}
							}

							Point3 bumpSrcObject = Inverse(mat) * bumpSrcWorld;
							Point3 realBumpVec = unfuck_point(bumpSrcObject).Normalize(); // no overflow
							if (filler==FILLER_NMAP) {
								bumpVec = realBumpVec;
								bumpVec *= min(attenuation, 1.f);
								bumpVec *= (1<<8)-16;
							} else if (filler==FILLER_EBUMP) {
								bumpVec.x = -atan2(realBumpVec.x, realBumpVec.y) * (0.5f/M_PI) + 0.5f;
								bumpVec.x = fmod(bumpVec.x+0.25f, 1.0f);
								bumpVec.x *= (1<<16)-1;
							}
								
						}
					}
					fs.bumpvecs.push_back( bumpVec );
				}
				
				xpdb::framestates.push_back(fs);
			}
		}
	}

	// sjekk for cancel
	if (core->GetCancel()) return endthis(IMPEXP_CANCEL);

	// precalc
	if (global_settings.precalc) {
		int initret = gl::init();
		if (initret) return initret;

		bool mask[GBA_WIDTH*GBA_HEIGHT];
		FORI(GBA_WIDTH*GBA_HEIGHT) mask[i]=false;
		// load mask
		{
			FILE* fil=NULL;

			string& maskefil = global_settings.maskefil;
			if (maskefil.size()) {
				fil = fopen(maskefil.c_str(), "rb");
				if (!fil) message("Mask file not found");
			}
			if (fil) {
				// load raw stuff
				unsigned char mask_char[GBA_WIDTH*GBA_HEIGHT*3];
				int ret = fread(mask_char, 1, GBA_WIDTH*GBA_HEIGHT*3, fil);
				if (ret!=GBA_WIDTH*GBA_HEIGHT*3) message("mask is too small. should be 240x160 24bit, with green representing transparency.");
				fclose(fil);
				FORI(GBA_WIDTH*GBA_HEIGHT) {
					int r = mask_char[i*3+0];
					int g = mask_char[i*3+1];
					int b = mask_char[i*3+2];
					mask[i] = (r==0) && (g==0xFF) && (b==0);
				}
			}
		}

		glEnable(GL_CULL_FACE);
		glClearColor(0, 0, 0, 0);

		//GMatrix devsize = ScaleMatrix( Point3(GBA_WIDTH, -GBA_HEIGHT, 1) );

		int time =0;
		for (int frame=0; frame<xpdb::framestates.size(); frame++, time+=global_settings.get_frametime()) {
			int progress = (100*frame)/(int)xpdb::framestates.size();
			core->ProgressUpdate(progress, true);
			FrameState& fs  = xpdb::framestates[frame]; 

			GMatrix device = GMatrix(fs.view) * fs.proj;
			// make complete matrices
			fs.mvp.push_back(device); // world space
			for (int i=0; i<(int)fs.objecttm.size(); i++) {
				GMatrix mvp = GMatrix(fs.objecttm[i]) * device;
				fs.mvp.push_back(mvp);
			}
		}



		std::vector<bool> finniface;
		std::vector<bool> finnivert;
		std::vector<int>  facefeil;
		std::vector<bool> verts_behind;
		finniface.resize(xpdb::faces.size());
		finnivert.resize(xpdb::verts.size());
		verts_behind.resize(xpdb::verts.size());
		facefeil.resize(xpdb::faces.size());
		time=0;
		for (int frame=0; frame<xpdb::framestates.size(); frame++, time+=global_settings.get_frametime()) {
			int progress = (100*frame)/(int)xpdb::framestates.size();
			core->ProgressUpdate(progress, true);
			FrameState& fs  = xpdb::framestates[frame]; 
			FrameState& nfs = xpdb::framestates[min(frame+1, xpdb::framestates.size()-1)]; 

			// finn ut hvilke verts som er _bak nearplane_
  			for (int i=0; i<xpdb::verts.size(); i++) {
				RomVertex& v = xpdb::verts[i];
				if (v.isNormal) {
					verts_behind[i] = false;
				} else {
 					Point4 p = vert_transform_mvp(v, fs);
//					verts_behind[i] = (p.z <= 1.0);// (p.z/p.w) <= 0.99; //FTW?
					verts_behind[i] = (p.w <= 0.01); // hmm
					//assert(!verts_behind[i]);
				}
			}
			

			// sett finnifaces til false igjen
			for (int i=0; i<finniface.size(); i++) finniface[ i ]=false;
			static std::vector<SortAf> afs;
			afs.clear();

			static GLubyte pixels_z_dbl[2][MAX_BUFFERSIZE];


			// kjør _neste frame_ så current frame
			for (int timesample=1; timesample>=0; timesample--) {
				FrameState* cfs = NULL;
				if (timesample==0) cfs = &fs;
				else cfs = &nfs;
				GLubyte* pixels_z = pixels_z_dbl[timesample];

				// last projection og view matrix
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();

				// render de sakene for a finne activeverts og activepolys
				glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
				glEnable(GL_DEPTH_TEST);
				// tegn trianglene
				glBegin(GL_TRIANGLES);
				for (int fi=0; fi<xpdb::faces.size(); fi++) {
					RomFace& face = xpdb::faces[fi];
					int vi0 = face.v[0];
					int vi1 = face.v[1];
					int vi2 = face.v[2];
					if (verts_behind[vi0] || verts_behind[vi1] || verts_behind[vi2]) {
						continue; // drep saker som z-klipper
					}
					setfi_gl(fi);
					Point4 p0 = vert_transform_mvp(xpdb::verts[vi0], *cfs);
					Point4 p1 = vert_transform_mvp(xpdb::verts[vi1], *cfs);
					Point4 p2 = vert_transform_mvp(xpdb::verts[vi2], *cfs);
					glVertex4f(p0.x, p0.y, p0.z, p0.w);
					glVertex4f(p1.x, p1.y, p1.z, p1.w);
					glVertex4f(p2.x, p2.y, p2.z, p2.w);
				}
				glEnd();

				// hent tilbake skjermen for a finne activefaces
				memset(pixels_z, 0, MAX_BUFFERSIZE);
				glReadPixels(0, 0, xpdb::winw(), xpdb::winh(), GL_RGB, GL_UNSIGNED_BYTE, pixels_z);
				SwapBuffers(gl::hdc);
				
				// apply mask
				for (int pi=0; pi<xpdb::winw()*xpdb::winh()*3; pi+=3) {
					int pikk = pi/3;
					int x = (pikk%xpdb::winw())/xpdb::ss();
					int y = (pikk/xpdb::winw())/xpdb::ss();
					if (mask[x+y*GBA_WIDTH]) {
						pixels_z[pi+0]=0;
						pixels_z[pi+1]=0;
						pixels_z[pi+2]=0;
					}
				}

				// finn activefaces
				{
					for (int pi=0; pi<xpdb::winw()*xpdb::winh()*3; pi+=3) {

						int fi = getfi(pixels_z+pi);
						if (fi<0 || fi>=xpdb::faces.size()) continue;
						if (!finniface[fi]) {
							finniface[fi]=true;
							RomFace& face = xpdb::faces[fi];
							SortAf af;
							af.fi = fi;
							af.z=0.f;
							for (int vi=0; vi<3; vi++) {
								Point3 p = vert_transform(xpdb::verts[face.v[vi]], *cfs) * cfs->view;
								af.z += p.z;
							}
							afs.push_back(af);
						}
					}
				}
			}

			// activefaces
			// nå skal vi lage ny faceliste basert på gamle ting
			std::sort(afs.begin(), afs.end());

			if (fs.pframe==frame) {
				fs.activefaces.resize(afs.size());
				FORI(afs.size()) {
					fs.activefaces[i]=afs[i].fi;							
				}
			} else {
				FrameState& ps = xpdb::framestates[frame-1]; 

				// detekter NYE faces - inn i liste
				fs.activefaces.clear();
				fs.activefaces.reserve(ps.activefaces.size()+afs.size());

				std::set<int> gamlesaker;
				std::set<int> nyesaker;

				FORI(ps.activefaces.size()) gamlesaker.insert(ps.activefaces[i]);
				FORI(afs.size()) nyesaker.insert(afs[i].fi);


				int gammeli = 0;
				// legg inn en viss prosent av de gamle tinga som fortsatt fins (typisk skyboxen :)
				for(; gammeli<ps.activefaces.size()*0.5; gammeli++) {
					int fi = ps.activefaces[gammeli];
					if (nyesaker.count(fi)) {
						fs.activefaces.push_back(fi);
					}
				}

				// legg til nye ting som ikke var der før
				FORI(afs.size()) {
					int fi = afs[i].fi;
					if (gamlesaker.count(fi)==0) {
						fs.activefaces.push_back(fi);
					}
				}

				// legg inn resten av de gamle tinga som fortsatt fins
				for(; gammeli<ps.activefaces.size(); gammeli++) {
					int fi = ps.activefaces[gammeli];
					if (nyesaker.count(fi)) {
						fs.activefaces.push_back(fi);
					}
				}
			}

			// finn activeverts
			{
				fs.activeverts.clear();
				for (int i=0; i<fs.activefaces.size(); i++) {
					RomFace& face = xpdb::faces[ fs.activefaces[i] ];
					for (int j=0; j<3; j++) {
						int vi = face.v[j];
						if (!finnivert[vi]) {
							finnivert[vi]=true;
							fs.activeverts.push_back(vi);
						}
						if (xpdb::mtls[face.mtl].uvGen == UVGEN_ENV) {
							assert(face.mtl>=0 && face.mtl<xpdb::mtls.size());
							int ni = face.uv[j];
							if (!finnivert[ni]) {
								finnivert[ni]=true;
								fs.activeverts.push_back(ni);
							}
						}
					}
				}
				// sett finniverts til false igjen
				for (int i=0; i<fs.activeverts.size(); i++) finnivert[ fs.activeverts[i] ]=false;

				std::sort(fs.activeverts.begin(), fs.activeverts.end());
				//				fs.activefaces_change = frame;
				//				fs.activeverts_change = frame;
			}


			// resolve sorting errors
			// kjør _neste frame_ så current frame
			for (int timesample=1; timesample>=0; timesample--) {
				FrameState* cfs = NULL;
				if (timesample==0) cfs = &fs;
				else cfs = &nfs;
				GLubyte* pixels_z = pixels_z_dbl[timesample];
				
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();
				glDisable(GL_DEPTH_TEST);
				for (int iters=4; iters; iters--) {
					glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
					// tegn trianglene
					glBegin(GL_TRIANGLES);
					for (int afi=0; afi<fs.activefaces.size(); afi++) {
						int fi = fs.activefaces[afi];
						RomFace& face = xpdb::faces[fi];
						setfi_gl(fi);
						Point4 p0 = vert_transform_mvp(xpdb::verts[face.v[0]], *cfs);
						Point4 p1 = vert_transform_mvp(xpdb::verts[face.v[1]], *cfs);
						Point4 p2 = vert_transform_mvp(xpdb::verts[face.v[2]], *cfs);
						glVertex4f(p0.x, p0.y, p0.z, p0.w);
						glVertex4f(p1.x, p1.y, p1.z, p1.w);
						glVertex4f(p2.x, p2.y, p2.z, p2.w);
					}
					glEnd();
					static GLubyte pixels_b2f[MAX_BUFFERSIZE];
					memset(pixels_b2f, 0, MAX_BUFFERSIZE);
					glReadPixels(0, 0, xpdb::winw(), xpdb::winh(), GL_RGB, GL_UNSIGNED_BYTE, pixels_b2f);
					SwapBuffers(gl::hdc);

					// finn feil
					// facefeil: antall feil pixels per face
					int korrigeringer=0; // totalt antall korrigeringer foretatt
					for (int pi=0; pi<xpdb::winw()*xpdb::winh()*3; pi+=3) {
						int fz_ = getfi(pixels_z+pi);
						int fs_ = getfi(pixels_b2f+pi);
						if (fz_==fs_) continue;
						// hmm? den ok?
						//if (fz_==0) continue;

						//if (facefeil[fs_]++<2) continue;
						facefeil[fs_]=1;

						// situasjon: fs_ har feilaktig kommet foran fz_. (fs_ er bak fz_ i lista)
						// fs_ skal flyttes bak.
						// den kan allerede ha blitt flytta.
						// vi ma finne dem i afs_. 
						// dersom z ligger foer s, ma noe gjoeres.

						int afsi = 0;
						bool action = false;
						while (afsi<fs.activefaces.size() && fs.activefaces[afsi]!=fs_) {
							if (fs.activefaces[afsi]==fz_) action=true;
							afsi++;
						}
						if (!action || afsi>=fs.activefaces.size()) continue;
						korrigeringer++;

						// fs_ ma foran fz_ i lista (bak i z-tenking)
						int temp=fs.activefaces[afsi];
						for (; fs.activefaces[afsi]!=fz_; afsi--) {
							fs.activefaces[afsi] = fs.activefaces[afsi-1];
						}
						fs.activefaces[afsi]=temp;
						// far --- z --- m --- b --- n --- c --- d --- s -- near
					}
					// sett facefeil til 0 igjen
					for (int i=0; i<fs.activefaces.size(); i++) facefeil[ fs.activefaces[i] ]=0;

					// break hvis ingen korrigeringer foretatt
					if (korrigeringer==0) break;
				}

			} // timestuff

			// nå: bestem silhouett stuff
			{
				// reset winged edge structs
				// and count frontfacing polys for each edge
				for (map<EdgeId, WingedEdge>::iterator iter=xpdb::wingedEdges.begin(); iter!=xpdb::wingedEdges.end(); iter++) {
					const EdgeId edgeId = iter->first;
					WingedEdge& wingedEdge = iter->second;
					wingedEdge.visiblePolyCount=0;
					// count frontfacing polys
					wingedEdge.frontFacingPolyCount=0;
					for (int fi=0; fi<wingedEdge.polys.size(); fi++) {
						RomFace& f = xpdb::faces[ wingedEdge.polys[fi] ];
						RomVertex& v0 = xpdb::verts[ f.v[0] ];
						RomVertex& v1 = xpdb::verts[ f.v[1] ];
						RomVertex& v2 = xpdb::verts[ f.v[2] ];
						Point4 p0 = vert_transform_mvp(v0, fs);
						Point4 p1 = vert_transform_mvp(v1, fs);
						Point4 p2 = vert_transform_mvp(v2, fs);
						Point3 pp0( p0.x/p0.w, p0.y/p0.w, 0.f );
						Point3 pp1( p1.x/p1.w, p1.y/p1.w, 0.f );
						Point3 pp2( p2.x/p2.w, p2.y/p2.w, 0.f );
						bool frontFacing = CrossProd(pp1-pp0, pp2-pp0).z > 0.f;
						if (frontFacing) wingedEdge.frontFacingPolyCount++;
					}
				}

				// count visible poly count for all edges
				for (int afi=0; afi<fs.activefaces.size(); afi++) {
					RomFace& f = xpdb::faces[ fs.activefaces[afi] ];
					for (int ei=0; ei<3; ei++) {
						int vid0 = f.v[ ei ];
						int vid1 = f.v[ (ei==2)?0:ei+1 ];
						EdgeId edgeId(vid0, vid1);
						WingedEdge& wingedEdge = xpdb::wingedEdges[ edgeId ];
						wingedEdge.visiblePolyCount++;
					}
				}

				// map of all silhouette edges.
				// stores an index to the Last Visible Polygon which uses this edge.
				std::map<EdgeId, int> sEdges;
				// map of all silhouette vertices 
				// stores an index to the Last Visible Polygon which uses this vertex.
				std::map<int, int> sVertices; 

				// go thru faces again.
				// * create sEdges
				// * create sVertices
				for (int afi=0; afi<fs.activefaces.size(); afi++) {
					RomFace& f = xpdb::faces[ fs.activefaces[afi] ];

					// culling
					for (int vii=0; vii<3; vii++) {
						RomVertex& v = xpdb::verts[ f.v[vii] ];
						Point4 p = vert_transform_mvp(v, fs);
						//Point3 pp0( p.x/p.w, p.y/p.w, 0.f );
						//negativeW[vii] = p.w<0.001;
					}

					

					// look for vertices in sVertices to update the lastVisiblePolyIndex
					for (int vii=0; vii<3; vii++) {
						int vi = f.v[vii];
						if (sVertices.count(vi)) sVertices[vi] = afi;
					}

					for (int ei=0; ei<3; ei++) {
						int ei_next = (ei==2)?0:ei+1;
						int vi0 = f.v[ei];
						int vi1 = f.v[ei_next];
						EdgeId edgeId( vi0, vi1 );
			
						// look for edge in sEdges to update the lastVisiblePolyIndex
						if (sEdges.count( edgeId )) sEdges[edgeId] = afi;

						// Now, see if this edge and its vertices need to be INSERTED
						// into the silhouette maps???
						WingedEdge& wingedEdge = xpdb::wingedEdges[ edgeId ];
						assert(wingedEdge.visiblePolyCount);
						//assert(wingedEdge.frontFacingPolyCount); not accurate
						if (wingedEdge.autoSmooth || wingedEdge.frontFacingPolyCount<wingedEdge.polys.size() ) {
							if (!verts_behind[vi0] && !verts_behind[vi1]) sEdges[edgeId] = afi;
							if (!verts_behind[vi0] && !sVertices.count(vi0)) sVertices[vi0] = afi;
							if (!verts_behind[vi1] && !sVertices.count(vi1)) sVertices[vi1] = afi;
						}
					}
				}

				// Finally, create aa masks for all the edges
				fs.activefacemasks.resize(fs.activefaces.size());
				for (int afi=0; afi<fs.activefaces.size(); afi++) {
					RomFace& f = xpdb::faces[ fs.activefaces[afi] ];
					int& mask = fs.activefacemasks[afi];
					mask = 0;

					// vertices?
					for (int vii=0; vii<3; vii++) {
						int vi = f.v[vii];
						if ( sVertices.count(vi) && sVertices[vi]==afi ) mask |= 1<<(vii+3);
					}
					// edges?
					for (int ei=0; ei<3; ei++) {
						int ei_next = (ei==2)?0:ei+1;
						EdgeId edgeId( f.v[ei], f.v[ei_next] );
						if ( sEdges.count(edgeId) && sEdges[edgeId]==afi ) mask |= 1<<ei;
					}
					
				}
			}

			// liten pause sa vi ser hva som skjer
		//	SwapBuffers(gl::hdc);
			// sjekk for cancel
			if (core->GetCancel()) return endthis(IMPEXP_CANCEL);
		}
		gl::deinit();
	}
 
	// interleave activefaces and their aamasks into a single list called interleaved_faces
	for (int frame=0; frame<xpdb::framestates.size(); frame++) {
		FrameState& fs = xpdb::framestates[frame];
		// this will contain activefacess as well as antialias masks
		// i will use the second most significant bit to denote an aa mask.
		std::vector<int>& interleaved_faces = fs.interleaved_faces;
		assert(fs.activefaces.size()==fs.activefacemasks.size());
		for (int i=0; i<fs.activefaces.size(); i++) {
			unsigned int facei = fs.activefaces[i];
			int mask = fs.activefacemasks[i];

			// the face
			assert(facei<0x4000);
			interleaved_faces.push_back( facei );

			// the mask?
			if (mask && global_settings.antialias_masks) {
				interleaved_faces.push_back( 0x4000 | mask );
			}
		}
	}

	// detect uniqueness
	for (int frame=0; frame<xpdb::framestates.size(); frame++) {
		FrameState& fs = xpdb::framestates[frame];
		fs.view_change=frame;
		fs.objecttm_change.resize( fs.objecttm.size() );
		fs.mvp_change.resize( fs.mvp.size() );
		FORI(fs.objecttm.size()) fs.objecttm_change[i]=frame;
		FORI(fs.mvp.size()) fs.mvp_change[i]=frame;
		fs.bumpvecs_change=frame;
		fs.lights_change=frame;
//		fs.activesilhouette_change=frame;
		fs.shadow_compr_change=frame;
		if (frame==0) continue;

		FrameState& ps = xpdb::framestates[frame-1];
		// view
		{
//			RomMatrix3 a = max2rom_matrix3(ps.view);
//			RomMatrix3 b = max2rom_matrix3(fs.view);
//			fs.view_change = (a!=b) ? frame : ps.view_change;
		}
		// objecttm
		for (int i=0; i<fs.objecttm.size(); i++) {
//			RomMatrix3 a = max2rom_matrix3(ps.objecttm[i]);
//			RomMatrix3 b = max2rom_matrix3(fs.objecttm[i]);
//			fs.objecttm_change[i] = (a!=b) ? frame : ps.objecttm_change[i];
		}
		// mvp
		for (int i=0; i<fs.mvp.size(); i++) {
			RomMatrix4 a = max2rom_matrix4(ps.mvp[i]);
			RomMatrix4 b = max2rom_matrix4(fs.mvp[i]);
			fs.mvp_change[i] = (a!=b) ? frame : ps.mvp_change[i];
		}
		// bumpvecs
		if (ps.bumpvecs.size()==fs.bumpvecs.size()) {
			fs.bumpvecs_change = ps.bumpvecs_change;
			FORI(fs.bumpvecs.size()) {
				Point3& a = ps.bumpvecs[i];
				Point3& b = fs.bumpvecs[i];
				int x = int(a.x-b.x);
				int y = int(a.y-b.y);
				int z = int(a.z-b.z);
				if (x||y||z) {
					fs.bumpvecs_change=frame;
					break;
				}
			} 
		}
		// lights
		{
			fs.lights_change = ps.lights_change;
			for (int i=0; i<xpdb::lights.size(); i++) {
				Point3 as = ps.lights[i] * (1<<PREC_VEC);
				Point3 bs = fs.lights[i] * (1<<PREC_VEC);
				if (int(as.x)!=int(bs.x) || int(as.y)!=int(bs.y) || int(as.z)!=int(bs.z) ) {
					fs.lights_change = frame;
					break;
				}
			}			 
		}
		// shadows_compr
		{
 			fs.shadow_compr_change = frame;
			for (int i=frame-1; i>=0; i--) {
				FrameState& ps = xpdb::framestates[i];
				if (ps.shadow_compr_change!=i) continue;
				if (ps.shadow_compr == fs.shadow_compr) {
					fs.shadow_compr_change = i;
					break;
				}
			}
		}
	}



	FILE* fil = fopen(global_settings.get_filename().c_str(), "wt");
	if (!fil) {
		message("kunne ikke lagre den");
		return IMPEXP_FAIL;
	}

	fprintf(fil, "#pragma once\n");

	// now, rules for makefile
//	fprintf(fil, "\n\n/*\n");
//	fprintf(fil, "scene.xpal: ");

	// export textures
	fprintf(fil, "\n");
	fprintf(fil, "const Material materials[] = {\n");
	for (int i=0; i<xpdb::mtls.size(); i++) {
		//std::string& tex = xpdb::textures[i];
//		fprintf(fil, "// %i: %s\n", i, tex.c_str());
		GbaMtl& mtl = xpdb::mtls[i];
		const char* fillers[] = {
			"FILLER_TEX256",
			"FILLER_EBUMP",
			"FILLER_NMAP",
		};
		const char* uvGens[] = {
			"UVGEN_UVS",
			"UVGEN_ENV",
		};
		// textures... dikt navn
		std::string tex1str("NULL");
		std::string tex2str("NULL");
		std::string subPalStr("0");
		if (mtl.texPrimary) {
			if (mtl.filler==FILLER_TEX256) {
				tex1str = mekktexnavn(mtl.texPrimary) + "_tex";
			} else if (mtl.filler==FILLER_NMAP) {
				tex1str = mekktexnavn(mtl.texPrimary) + "_nmap";
			} else if (mtl.filler==FILLER_EBUMP) {
				tex1str = mekktexnavn(mtl.texPrimary) + "_ebump";
			}
		}
		if (mtl.texSecondary) {
			std::string tmp = mekktexnavn(mtl.texSecondary);
			if (mtl.filler==FILLER_EBUMP) {
				tex2str = tmp + "_tex";
			} else if (mtl.filler==FILLER_NMAP) {
				subPalStr = tmp +  std::string("_subpal");
			}
		}
		fprintf(fil, "    { %s, %s, { %s, %s }, %s, %i }, \n", fillers[mtl.filler], uvGens[mtl.uvGen], tex1str.c_str(), tex2str.c_str(), subPalStr.c_str(), mtl.oneone);
	}
	fprintf(fil, "};\n\n");


	// export splines
	for (int si=0; si<xpdb::splines.size(); si++) {
		IGameNode* node = xpdb::splines[si];
		assert( node->GetIGameObject()->GetMaxType() == IGameObject::IGAME_MAX_SHAPE );
		ShapeObject* maxShape = (ShapeObject*)node->GetIGameObject()->GetMaxObject();
		PolyShape polyShape;
		// Create a PolyShape representation with 16 fixed steps & no optimization
		maxShape->MakePolyShape(0, polyShape, 15, FALSE);   

		vector<Point3> points;
		for (int li=0; li<polyShape.numLines; li++) {
			// for each line
			PolyLine& line = polyShape.lines[li];
			for (int pi=0; pi<line.numPts; pi++) {
				Point3 p = line.pts[pi].p;
				Point3 wp = node->GetMaxNode()->GetObjectTM(0) * p;
				// Point3 wp = node->GetMaxNode()->GetNodeTM(0) * p;
				// Point3 wp = node->GetWorldTM(0).ExtractMatrix3() * p; 
				points.push_back(wp);
			}
		}
		fprintf(fil, "const s16 spline%02i[] = { ", si);
		fprintf(fil, "(%i), ", points.size());
		for (int i=0; i<points.size(); i++) {
			RomVertex fixed_v; // exploit that nice concat_verts
			concat_vert(fixed_v, points[i], PREC_VEC);
			fprintf(fil, "0x%04x, 0x%04x, 0x%04x, ", fixed_v.x&0xFFFF, fixed_v.y&0xFFFF, fixed_v.z&0xFFFF);
		}
		fprintf(fil, " }; // %s\n", node->GetName());
	}

	fprintf(fil, "const s16* splines[] = { ", si);
	for (int si=0; si<xpdb::splines.size(); si++) {
		fprintf(fil, "spline%02i, ", si);
	}
	fprintf(fil, "};\n\n");

	// export verts
	fprintf(fil, "const RomVertex verts[] = {");
	for (int i=0; i<xpdb::verts.size(); i++) {
		fprintf(fil, "\n  ");
		RomVertex& vert = xpdb::verts[i];
		fprintf(fil, "{ %i, {{%2i, 0x%02x}, {%2i, 0x%02x}, {%2i, 0x%02x}}, 0x%04x, 0x%04x, 0x%04x }, ", vert.isNormal, (int)vert.matrix[0].i, (int)vert.matrix[0].w, (int)vert.matrix[1].i, (int)vert.matrix[1].w, (int)vert.matrix[2].i, (int)vert.matrix[2].w, (unsigned int)(unsigned short)vert.x, (unsigned int)(unsigned short)vert.y, (unsigned int)(unsigned short)vert.z);
	}
	fprintf(fil, "\n};\n\n");

	// export uvs
	fprintf(fil, "const RomUvs uvs[] = {");
	for (int i=0; i<xpdb::uvs.size(); i++) {
		if (i%4 == 0) fprintf(fil, "\n  ");
		RomUvs& uv = xpdb::uvs[i];
		fprintf(fil, "{0x%04x, 0x%04x}, ", uv.u, uv.v);
	}
	fprintf(fil, "\n};\n\n");



	// export faces
	fprintf(fil, "const RomFace faces[] = {\n");
	for (int i=0; i<xpdb::faces.size(); i++) {
		RomFace face = xpdb::faces[i];
		fprintf(fil, "  { %2i, { 0x%04x, 0x%04x, 0x%04x }, { 0x%04x, 0x%04x, 0x%04x } },\n", face.mtl, face.v[0],  face.v[1],  face.v[2], face.uv[0], face.uv[1], face.uv[2]);
	}
	fprintf(fil, "};\n\n");


	// eksporter framedata


	// bumpvecs
	for (int frame=0; frame<xpdb::framestates.size(); frame++) {
		FrameState& fs = xpdb::framestates[frame];
		if (fs.bumpvecs_change==frame) {
			fprintf(fil, "const signed short bumpvecs_%04i[][3] = { ", frame);
			for (int i=0; i<fs.bumpvecs.size(); i++) {
				Point3 p = fs.bumpvecs[i];
				fprintf(fil, "{ 0x%04x, 0x%04x, 0x%04x }, ", (unsigned short)p.x, (unsigned short)p.y, (unsigned short)p.z );
			}
			fprintf(fil, "};\n");
		}
	}



	// lights
	for (int frame=0; frame<xpdb::framestates.size(); frame++) {
		FrameState& fs = xpdb::framestates[frame];
		if (fs.lights_change==frame) {
			fprintf(fil, "const signed short lights_%04i[][3] = { ", frame);
			for (int i=0; i<fs.lights.size(); i++) {
				Point3 p = fs.lights[i] * (1<<PREC_VEC);
				CHECK_RANGE(p.x, 15);
				CHECK_RANGE(p.y, 15);
				CHECK_RANGE(p.z, 15);
				fprintf(fil, "{ 0x%04x, 0x%04x, 0x%04x }, ", (unsigned short)p.x, (unsigned short)p.y, (unsigned short)p.z );
			}
			fprintf(fil, "};\n");
		}
	}



#if 0
	// export skuggen
	for (int frame=0; frame<xpdb::framestates.size(); frame++) {
		FrameState& fs = xpdb::framestates[frame];
		std::vector<unsigned short>& skuggen = fs.shadow_compr;
		if (skuggen.size() && fs.shadow_compr_change==frame) {
			fprintf(fil, "const unsigned short shadow_%04i[] = { ", frame);
			for (int i=0; i<skuggen.size(); i++) fprintf(fil, "0x%04x, ", skuggen[i]);
			fprintf(fil, "};\n");
		} 
	}
#endif


	if (!global_settings.precalc) {
/*		for (int frame=0; frame<xpdb::framestates.size(); frame++) {
			FrameState& fs = xpdb::framestates[frame];
			if (fs.view_change==frame) {
				RomMatrix3 view = max2rom_matrix3(fs.view);
				fprintf(fil, "const RomMatrix3 view_____%04i    = { ", frame);
				for (int i=0; i<3*4; i++) fprintf(fil, "0x%04x, ", (unsigned short)view.m[i]);
				fprintf(fil, "};\n");
			}
		}


		for (int frame=0; frame<xpdb::framestates.size(); frame++) {
			FrameState& fs = xpdb::framestates[frame];
			for (int i=0; i<fs.objecttm.size(); i++) {
				if (fs.objecttm_change[i]==frame) {
					RomMatrix3 mat = max2rom_matrix3(fs.objecttm[i]);
					fprintf(fil, "const RomMatrix3 objtm%02x_%04i    = { ", i, frame);
					for (int i=0; i<3*4; i++) fprintf(fil, "0x%04x, ", (unsigned short)mat.m[i]);
					fprintf(fil, "};\n");
				}
			}
		}*/
	} else {
		for (int frame=0; frame<xpdb::framestates.size(); frame++) {
			FrameState& fs = xpdb::framestates[frame];
			for (int i=0; i<fs.mvp.size(); i++) {
				if (fs.mvp_change[i]==frame) {
					RomMatrix4 mat = max2rom_matrix4(fs.mvp[i]);
					fprintf(fil, "const RomMatrix4 mvp%02x_%04i = { ", i, frame);
					for (int i=0; i<4*4; i++) fprintf(fil, "0x%08x, ", mat.m[i]);
					fprintf(fil, "};\n");
				}
			}
		}

		for (int frame=0; frame<xpdb::framestates.size(); frame++) {
			FrameState& fs = xpdb::framestates[frame];
			// mvps liste
			fprintf(fil, "const RomMatrix4* const mvps_%04i[] = { ", frame);
			for (int i=0; i<fs.mvp.size(); i++) {
				fprintf(fil, "&mvp%02x_%04i, ", i, fs.mvp_change[i]);
			}
			fprintf(fil, "};\n");
		}

		// activeverts
		for (int frame=0; frame<xpdb::framestates.size(); frame++) {
			FrameState& fs = xpdb::framestates[frame];
			std::vector<int> stuff;

#if 0
			stuff = fs.activeverts;
#else
			if (fs.pframe==frame) {
				cmdCompressPFrame(stuff, fs.activeverts);
			} else {
				FrameState& ps = xpdb::framestates[frame-1];
				cmdCompress(stuff, ps.activeverts, fs.activeverts);

				// test
				{
					std::vector<int> unstuff;
					cmdUnCompress(unstuff, stuff, ps.activeverts);
					assert(unstuff==fs.activeverts);
				}

			}
#endif


			fprintf(fil, "const unsigned short activeverts_%04i[] = { (%i), ", frame, (int)stuff.size());
			FORI(stuff.size()) fprintf(fil, "0x%04x, ", stuff[i]);
			fprintf(fil, "};\n");
		}

		// activefaces (NOW: interleaved_faces)
		for (int frame=0; frame<xpdb::framestates.size(); frame++) {
			FrameState& fs = xpdb::framestates[frame];

			std::vector<int> stuff;
#if 0
			stuff = fs.activefaces;
#else
			if (fs.pframe==frame) {
				//cmdCompressPFrame(stuff, fs.activefaces);
				cmdCompressPFrame(stuff, fs.interleaved_faces);
			} else {
				FrameState& ps = xpdb::framestates[frame-1];
				//cmdCompress(stuff, ps.activefaces, fs.activefaces);
				cmdCompress(stuff, ps.interleaved_faces, fs.interleaved_faces);

				// test
				{
					std::vector<int> unstuff;
					//cmdUnCompress(unstuff, stuff, ps.activefaces);
					//assert(unstuff==fs.activefaces);
					cmdUnCompress(unstuff, stuff, ps.interleaved_faces);
					assert(unstuff==fs.interleaved_faces);
				}
			}
#endif

			fprintf(fil, "const unsigned short activefaces_%04i[] = { (%i), ", frame, (int)stuff.size());
			FORI(stuff.size()) fprintf(fil, "0x%04x, ", stuff[i]);
			fprintf(fil, "};\n");
		}
	}

	fprintf(fil, "\n");
	fprintf(fil, "\n");

	// na framedingsen
	fprintf(fil, "// her kommer framestates\n");
	if (global_settings.precalc) {
		fprintf(fil, "const RomFrame framestates[] = {\n");
		for (int frame=0; frame<xpdb::framestates.size(); frame++) {
			FrameState& fs = xpdb::framestates[frame];
	//		fprintf(fil, "  {  % 4i, activeverts_%04i, activefaces_%04i, activesilhouette_%04i, mvps_%04i, bumpvecs_%04i, lights_%04i, ", fs.pframe, frame, frame, fs.activesilhouette_change, frame, fs.bumpvecs_change, fs.lights_change);
			fprintf(fil, "  {  % 4i, activeverts_%04i, activefaces_%04i, mvps_%04i, bumpvecs_%04i, lights_%04i, ", fs.pframe, frame, frame,  frame, fs.bumpvecs_change, fs.lights_change);
			if (fs.shadow_compr.size()) {
				fprintf(fil, "shadow_%04i, ", fs.shadow_compr_change);
			} else {
				fprintf(fil, "NULL, ");
			}
			fprintf(fil, "},\n");
		}
		fprintf(fil, "};\n");
	} else {
		fprintf(fil, "// export av framestates ikke ferdig for ikke-precalc modus\n");
	}
	fprintf(fil, "\n");
	fprintf(fil, "\n");

	fprintf(fil, "const int mtlcount   = %i;\n", (int)xpdb::mtls.size());
	fprintf(fil, "const int splinecount= %i;\n", (int)xpdb::splines.size());
	fprintf(fil, "const int framecount = %i;\n", (int)xpdb::framestates.size());
	fprintf(fil, "const int camcount   = %i;\n", (int)xpdb::cameras.size());
	fprintf(fil, "const int lightcount = %i;\n", (int)xpdb::lights.size());
	fprintf(fil, "const int vertcount  = %i;\n", (int)xpdb::verts.size());
	fprintf(fil, "const int facecount  = %i;\n", (int)xpdb::faces.size());
	fprintf(fil, "const int uvcount    = %i;\n", (int)xpdb::uvs.size());
	fprintf(fil, "const int mvpcount   = %i;\n", (int)xpdb::nodes.size()+1);
	fprintf(fil, "\n");
	fprintf(fil, "\n");


	fclose(fil);

	return endthis(IMPEXP_SUCCESS);
}
