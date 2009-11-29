#include <assert.h>
#include "main.h"
#include "Vector3.h"
#include <set>
#include <list>
#include <algorithm>
#include "freeimage.h"
using namespace std;

#define SCALE_R 0.299
#define SCALE_G 0.587
#define SCALE_B 0.114

// this is the array of colors in the image
vector<Vector3> colors;

#define SPLITVEC_COUNT 7
Vector3 splitVecs[SPLITVEC_COUNT];


struct Node;
struct Node {
	int left, count;
	double count_rcp;
	Node* parent;
	Node* children[2];
	Vector3 avg;
	bool childrenCreated;

	// precalced error values
	double singleError; // error of approximation with this node
	double totalError;  // error of appriximation using the current subtree
	int leafCount;      // number of leaf nodes in the subtree of this node

	// NOTE on updating precalced values
	// singleError - constant. Calculated automatically.
	// totalError - changes whenever a descendant split.
	//   1. root.CalculateTotalErrorsAndDescendantCount()
	//   3. then, whenever you modify the tree, have updateTotalErrors = true

	void CalculateTotalErrorAndLeafCount() {
		if (children[0]) {
			children[0]->CalculateTotalErrorAndLeafCount();
			children[1]->CalculateTotalErrorAndLeafCount();
			totalError  = children[0]->totalError + children[1]->totalError;
			leafCount = children[0]->leafCount + children[1]->leafCount;
		} else {
			totalError = singleError;
			leafCount = 1;
		}
	}

private:
	void ModifyTotalErrorAndLeafCount_RecurseUp(double err, int leaves) {
		totalError += err;
		leafCount += leaves;
		if (parent) parent->ModifyTotalErrorAndLeafCount_RecurseUp(err, leaves);
	}
public:

	Node(Node* parent, int left, int count) {
		this->parent = parent;
		this->left = left;
		this->count = count;
		this->children[0] = NULL;
		this->children[1] = NULL;
		this->count_rcp = 1.0/count;

		// calculate avg
		avg = Vector3(0,0,0);
		for (int i=0; i<count; i++) {
			avg = avg + colors[left+i];
		}
		avg = avg * (1.f/count);

		// calculate singleError
		singleError=0;
		for (int i=0; i<count; i++) {
			singleError += (avg-colors[i+left]).magnitude();
		}
		totalError = singleError;
		leafCount = 1;
		childrenCreated = false;
	}



	struct LinearVectorElement {
		double value;
		int index;
		void UpdateValue(const Vector3& splitVector) {
			value = colors[index].dot(splitVector);
		}
		bool operator<(const LinearVectorElement& op) const {
			return value < op.value;
		}
	};

	void CreateChildren(bool updateTotalErrorAndLeafCount) {
		if (childrenCreated) return;
		childrenCreated=true;
		// optimize: if (singleError==0) return;

		static vector<LinearVectorElement> sortList;
		sortList.resize(count);
		for (int i=0; i<count; i++) {
			sortList[i].index = left+i;
		}

		// find splitvec (make new thing here)
		// suggestion: just find biggest gap using a few different splitvectors.


		// find the biggest gap on any axis
		double biggestGap=0;
		int biggestGap_index=-1;
		Vector3 biggestGap_splitVector(0,0,0);

		// search for the biggest gap
		// i think this metric is actually quite good
		// we don't necessarily want to minimize the error on a single cut,
		// we want to cut at large gaps so that the segmentation have as few
		// bad cuts as possible. bad cuts are very bad. i.e. cutting in the
		// middle will be very bad, so will greedy minimization of error
		for (int splitVec_index = 0; splitVec_index<SPLITVEC_COUNT; splitVec_index++) {
			const Vector3& splitVec = splitVecs[splitVec_index];

			// update the values of the sortlist with the appropriate splitVec and sort accordingly
			assert(sortList.size() == count);

			for (int i=0; i<count; i++) {
				sortList[i].UpdateValue(splitVec);
			}



			std::sort(sortList.begin(), sortList.end());

			// find the biggest gap in all the sorted lists, weighted by distance from middle
			for (int i=1; i<count; i++) {
				double val0 = sortList[i-1].value;
				double val1 = sortList[i].value;
				double gap = val1 - val0;
				// gap is scaled down by distance from center
				double dist2 = abs(2*i-count) * count_rcp;
				double dist = fabs(((i*count_rcp)-0.5)*2.0);
				gap *= 1.0 - dist;
				if (gap>biggestGap) {
					biggestGap = gap;
					biggestGap_index = i;
					biggestGap_splitVector = splitVec;
				}
			}
		}

		// they are all the same
		// don't make any children. leave childrenError as singleError
		if (biggestGap_index<0) return;
		
		// sort the colors by the chosen splitVector
		for (int i=0; i<count; i++) {
			sortList[i].UpdateValue(biggestGap_splitVector);
		}
		std::sort(sortList.begin(), sortList.end());
		
		// update the colors vector
		static vector<Vector3> tempBuffer;
		tempBuffer.resize(count);
		for (int i=0; i<count; i++) {
			tempBuffer[i] = colors[sortList[i].index];
		}
		for (int i=0; i<count; i++) {
			colors[left+i] = tempBuffer[i];
		}

		// make the children nodes
		children[0] = new Node(this, left, biggestGap_index);
		children[1] = new Node(this, left+biggestGap_index, count-biggestGap_index);

		// update totalErrors
		if (updateTotalErrorAndLeafCount) ModifyTotalErrorAndLeafCount_RecurseUp(GetErrorChange_Split(), 1);
		assert(GetErrorChange_Split() < 0);
	}

	// get error change from split
	double GetErrorChange_Split() const {
		//CreateChildren();
		assert(childrenCreated);
		if (!children[0]) return 0;
		return children[0]->singleError + children[1]->singleError - singleError;
	}

	void Consolidate(bool updateTotalErrorAndLeafCount) {
		if (!children[0]) return;

		if (updateTotalErrorAndLeafCount) {
			double childrenTotalError = children[0]->totalError + children[1]->totalError;
			assert(fabs(totalError-childrenTotalError) < 0.000000000001);
			ModifyTotalErrorAndLeafCount_RecurseUp(singleError - totalError, 1-leafCount);		
		}
		
		for (int i=0; i<2; i++) {
			children[i]->Consolidate(false);
			delete children[i];
			children[i]=NULL;
		}
	}

	void WhichShouldIConsolidate(Node*& bestErrorChange_Node, double& bestErrorChange, int reductionLimit) {
		if (!children[0]) return;

		int reductionCount = leafCount - 1;
		if (reductionCount <= reductionLimit) {
			double reductionCount_rcp = 1.0 / reductionCount;
			double childrenTotalError = children[0]->totalError + children[1]->totalError;
			assert(fabs(totalError-childrenTotalError) < 0.000000000001);
			double errorChange = (singleError - totalError) * reductionCount_rcp;
			if (errorChange < bestErrorChange) {
				bestErrorChange = errorChange;
				bestErrorChange_Node = this;
			}
		}
		children[0]->WhichShouldIConsolidate(bestErrorChange_Node, bestErrorChange, reductionLimit);
		children[1]->WhichShouldIConsolidate(bestErrorChange_Node, bestErrorChange, reductionLimit);
	}

	void GetLeaves(list<Node*>& leaves) {
		if (children[0]) {
			children[0]->GetLeaves(leaves);
			children[1]->GetLeaves(leaves);
		} else {
			leaves.push_back(this);
		}
	}
};



bool CompareErrorChange_Split(const Node* a, const Node* b) {
	return a->GetErrorChange_Split() < b->GetErrorChange_Split();
}

static list<Node*> generate(int palCount) {
	int bigPalCount = min(colors.size(), palCount*4);
	
	Node papa(NULL, 0, colors.size());
	papa.CreateChildren(false);
	list<Node*> leaves;
	leaves.push_back(&papa);

	// expand everything to count*count colors
	fprintf(stderr, "Starting out with %i colors\n", colors.size());
	
	fprintf(stderr, "Creating a palette of %i colors by splitting\n", bigPalCount);
	while (leaves.size()<bigPalCount) {

		// split the few worst ones
		int splitCount = leaves.size()/5 + 1;
		if (leaves.size() < bigPalCount/5) splitCount=leaves.size(); 
		fprintf(stderr, "%i, ", leaves.size());

		// do the splits: remove N elements from list of leaves, and add its children
		for (int i=0; i<splitCount; i++) {
			Node* n = *leaves.begin();
			if (!n->children[0]) break; // means there is no point in subdividing
			leaves.erase(leaves.begin());
			n->children[0]->CreateChildren(false);
			n->children[1]->CreateChildren(false);
			leaves.push_back(n->children[0]);
			leaves.push_back(n->children[1]);
		}
 		leaves.sort(CompareErrorChange_Split);
		if (!(*leaves.begin())->children[0]) break;
	}


	fprintf(stderr, "\nEnded up with a palette of %i elements\n", leaves.size());
	int countah=0;
	for(list<Node*>::iterator iter=leaves.begin(); iter!=leaves.end(); iter++) countah+=(*iter)->count;
	fprintf(stderr, "Ended up with a palette of unique %i colors\n", countah);


	fprintf(stderr, "Creating a palette of %i colors by consolidating\n", palCount);
	// consolidate all leaves
	for (list<Node*>::iterator node_iter = leaves.begin(); node_iter!=leaves.end(); node_iter++) {
		Node* n = *node_iter;
		n->Consolidate(false);
	}
	// calculate total errors
	papa.CalculateTotalErrorAndLeafCount();

	// jeg har: sett med alt som ikke er løv
	// liste med løv

	// find best nodes to consolidate
	// criterium: lowest increase in error pr color removed.
	while (1) {
		// recursive call to find the node which is best to consolidate
//		printf(stderr, "%i, ", papa.leafCount);
		Node* bestErrorChange_Node = NULL;
		double bestErrorChange = 100000000000000000000.0;
		papa.WhichShouldIConsolidate(bestErrorChange_Node, bestErrorChange, papa.leafCount-palCount);
		if (!bestErrorChange_Node) break;
		// consolidate that node
		bestErrorChange_Node->Consolidate(true);
	}


	leaves.clear();
	papa.GetLeaves(leaves);
	return leaves;
}





FIBITMAP* LoadBitchmap(const char* filename) {
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(filename);
	if (fif==FIF_UNKNOWN) fif = FreeImage_GetFIFFromFilename(filename);
	if (!FreeImage_FIFSupportsReading(fif)) error("File format not supported: %s", filename);
	if (fif==FIF_PNG) {
		return FreeImage_Load(fif, filename, PNG_IGNOREGAMMA);			
	} else {
		return FreeImage_Load(fif, filename);			
	}
}

void task_reduce() {
	splitVecs[0] = Vector3(0,  0,  1);
	splitVecs[1] = Vector3(0,  1,  0);
	splitVecs[2] = Vector3(1,  0,  0);
	splitVecs[3] = Vector3(0,  1,  1).normalize();
	splitVecs[4] = Vector3(1,  1,  0).normalize();
	splitVecs[5] = Vector3(1,  0,  1).normalize();
	splitVecs[6] = Vector3(1,  1,  1).normalize();
	assert(SPLITVEC_COUNT==7);
	colors.clear();
	FreeImage_Initialise();

	// fetch first parameters
	int out_count = 0;
	int reserve_count = 0;
	sscanf(args[0].c_str(), "%i", &out_count);
	sscanf(args[1].c_str(), "%i", &reserve_count);
	
	// read pixels of all images
	for (int argi=2; argi<args.size(); argi++) {

		FIBITMAP* dib_orig = LoadBitchmap(args[argi].c_str());
		FIBITMAP* dib = FreeImage_ConvertToRGBF(dib_orig);
		if (!dib) error("Couldn't convert to RGBF: %s", args[argi].c_str());

		int bpp    = FreeImage_GetBPP(dib);
		int width  = FreeImage_GetWidth(dib);
		int height = FreeImage_GetHeight(dib);

		for (int y=0; y<height; y++) {
			float* linep = (float*)FreeImage_GetScanLine(dib, y);
			for (int x=0; x<width; x++) {
				float* pixel = linep + x*3;

				// skip bright green
				int r = floor(pixel[0]*256 + 0.49);
				int g = floor(pixel[1]*256 + 0.49);
				int b = floor(pixel[2]*256 + 0.49);
				if (r==0 && g==255 && b==0) continue;

				Vector3 col(floor(pixel[0]*SCALE_R*0xFFFF), floor(pixel[1]*SCALE_G*0xFFFF), floor(pixel[2]*SCALE_B*0xFFFF));
				colors.push_back(col);
			}
		}
	}

	// do the palette generation (subtract 1 color for the transparent one)
	list<Node*> leaves = generate(out_count-reserve_count);

	// output first colors as bright green
	FORI(reserve_count) {
		int zero=0;
		int maximum=0xFF;
		fwrite(&zero, 2, 1, stdout);
		fwrite(&zero, 2, 1, stdout);
		fwrite(&maximum, 2, 1, stdout);
		fwrite(&zero, 2, 1, stdout);
	}

	// then the rest of the colors
	for (list<Node*>::iterator iter=leaves.begin(); iter!=leaves.end(); iter++) {
		Node* n = *iter;
		int r = n->avg.x * (1.0/SCALE_R);
		int g = n->avg.y * (1.0/SCALE_G);
		int b = n->avg.z * (1.0/SCALE_B);
		int avg = (r + g + b) * 0.3333333333;
		if (avg<0) avg=0;
		if (r<0) r=0; 
		if (g<0) g=0; 
		if (b<0) b=0; 
		if (avg>0xFFFF) avg=0xFFFF;
		if (r>0xFFFF) r=0xFFFF;
		if (g>0xFFFF) g=0xFFFF;
		if (b>0xFFFF) b=0xFFFF;
		// skriv det ut
		fwrite(&avg, 2, 1, stdout);
		fwrite(&r, 2, 1, stdout);
		fwrite(&g, 2, 1, stdout);
		fwrite(&b, 2, 1, stdout);
	}

	FreeImage_DeInitialise();
}







