#include "compress.h"	
#include <map>
#include <assert.h>
#include <set>
#include <algorithm>
using namespace std;


namespace comp {
#if 0
	namespace {


		/**************************************************************************

		REHEAP ()

		This function creates a "legal" heap from the current heap tree structure.
		**************************************************************************/
		template<class T>
			void reheap (unsigned short heap_entry, vector<T>& heap, const std::map<T, int>& frequency_count)
		{
			const int heap_length = heap.size()-1;
			register unsigned short  index;
			register unsigned short  flag = 1;

			unsigned long   heap_value;


			heap_value = heap[heap_entry];

			while ((heap_entry <= (heap_length >> 1)) && (flag))
			{
				index = heap_entry << 1;

				if (index < heap_length)
					if (frequency_count.find(heap[index])->second >= frequency_count.find(heap[index+1])->second)
						index++;

				if (frequency_count.find(heap_value)->second < frequency_count.find(heap[index])->second)
					flag--;
				else
				{
					heap[heap_entry] = heap[index];
					heap_entry       = index;
				}
			}

			heap[heap_entry] = heap_value;
		}


		/**************************************************************************

		BUILD_INITIAL_HEAP ()

		This function builds a heap from the initial frequency count data.
		**************************************************************************/

		template<class T>
		void build_initial_heap (std::vector<T>& heap, const std::map<T, int>& frequency_count)
		{
			heap.push_back(0);
//			for (int loop = 0; loop < 256; loop++)
//				if (frequency_count.find(loop)->second)
//					heap.push_back( (unsigned long) loop );
			std::map<T, int>::const_iterator fc_iter;
			for (fc_iter = frequency_count.begin(); fc_iter!=frequency_count.end(); fc_iter++) {
				if (fc_iter->second) {
					int hehe = fc_iter->first;
					heap.push_back(hehe);
				}
			}
			sort(heap.begin(), heap.end());

			for (unsigned int loop = heap.size()-1; loop > 0; loop--)
				reheap(loop, heap, frequency_count);
		}



		/**************************************************************************

		BUILD_CODE_TREE ()

		This function builds the compression code tree.
		**************************************************************************/

		template<class T>
		void build_code_tree (std::map<T, int>& father, std::vector<T>& heap, std::map<T, int>& frequency_count)
		{
			T   heap_value;

			int heap_length = heap.size()-1;
			while (heap_length != 1)
			{
				heap_value = heap[1];
				heap[1]    = heap.back();
				heap.pop_back();
				heap_length--;
				

				reheap (1, heap, frequency_count);
				unsigned short findex = heap_length + 255;

				int c0 = frequency_count.count(heap[1]) ? frequency_count[heap[1]] : 0;
				int c1 = frequency_count.count(heap_value) ? frequency_count[heap_value] : 0;

				frequency_count[findex] = c0 + c1;
				father[heap_value] =  findex;
				father[heap[1]]    = -findex;
				heap[1]            =  findex;

				reheap (1, heap, frequency_count);
			}

			father[256] = 0;
		}


		/**************************************************************************

		GENERATE_CODE_TABLE ()

		This function generates the compression code table.
		**************************************************************************/

		template<class T, typename S>
		void generate_code_table (std::map<T, S>& code, const std::map<T, int>& father, const std::vector<T>& heap, const std::map<T, int>& frequency_count)
		{
			for (int loop = 0; loop < 256; loop++) {
				T val = loop;

				if (frequency_count.count(val))
				{
					int fcount = frequency_count.find(val)->second;
					int current_length = 0;
					int bitcode = 0;
					int current_bit = 1;
					int parent = father.find(val)->second;

					while (parent)
					{
						if (parent < 0)
						{
							bitcode += current_bit;
							parent = -parent;
						}
						parent = father.find(parent)->second;
						current_bit <<= 1;
						current_length++;
					}


					if (current_length > (sizeof(S)*8)) {
						assert(0);
						fprintf(stderr, "biiiiiigness in huffman!\n");
						exit(1);
					}

					if (current_length) {
						code[val].code = bitcode;
						code[val].length = (unsigned char) current_length;
					}
				} else {
					code[val].code = 0;
					code[val].length = 0;
				}
			}
		}


		/**************************************************************************

		COMPRESS_IMAGE ()

		This function performs the actual data compression.
		**************************************************************************/

		template <class T, typename S>
			void compress_image (std::vector<unsigned char>& out, const map<T, HuffCode<S> >& code, std::vector<T> file)
		{
			int				curbyte = 0;
			int				curbit = 7;
			unsigned int    thebyte = 0;

			for (int filei=0; filei<file.size(); filei++)
			{
				T dvalue = file[filei];
				HuffCode<S> current_code   = code.find(dvalue)->second;

				for (int loop1 = current_code.length-1; loop1 >= 0; --loop1)
				{
					if ((current_code.code >> loop1) & 1)
						thebyte |= (char) (1 << curbit);

					if (--curbit < 0)
					{
						out.push_back(thebyte);
						thebyte = 0;
						curbyte++;
						curbit = 7;
					}
				}
			}
			out.push_back(thebyte);
		}


	};



	template<class T, typename S>
	void getCodeMap(std::map<T, HuffCode<S> >& codeMap, const std::map<T, int>& frequency_count_in) {
		std::map<T, int> frequency_count = frequency_count_in;
		std::vector<T> heap;
		std::map<T, int> father;
		build_initial_heap(heap, frequency_count);
		build_code_tree(father, heap, frequency_count);
		generate_code_table(codeMap, father, heap, frequency_count);
	}
#endif



	void rleImage(vector<unsigned short>& dst, const u8* src, const u8* src_end, int bpp, bool reserveMSB) {
		int altallMax = 0xFFFF;
		altallMax >>= bpp;
		if (reserveMSB) altallMax >>= 1;

		int sistecmd=0;
		int altall=0;
		while (src!=src_end) {
			int cmd = *src >> (8-bpp);
			if (sistecmd!=cmd || altall>=altallMax) {
				int out = (altall<<bpp) | sistecmd;
				dst.push_back(out);
				sistecmd = cmd;
				altall=0;
			} 
			altall++;
			src++;
		}
		int out = (altall<<bpp) | sistecmd;
		dst.push_back(out);
	}

	void updateListPFrame(vector<unsigned short>& dst, vector<unsigned short> b) {
		dst = b;
	}


	struct Candidate {
		int location;
		int direction;
		int count;
		bool operator<(const Candidate& op) const {
			return count>op.count;
		}
	};

	void updateList(vector<unsigned short>& dst, vector<unsigned short>& prev, vector<unsigned short>& curr) {
		multimap<int, int> locations_prev;
		multimap<int, int> locations_curr;
		// store in map elements of prev
		FORI(prev.size()) {
			if (i>0 && i<prev.size()-1 && prev[i-1]==prev[i] && prev[i+1]==prev[i]) continue;
			locations_prev.insert( multimap<int, int>::value_type(prev[i], i));     
		}
		FORI(curr.size()) {
			if (i>0 && i<curr.size()-1 && curr[i-1]==curr[i] && curr[i+1]==curr[i]) continue;
			locations_curr.insert( multimap<int, int>::value_type(curr[i], i));     
		}
		dst.reserve(locations_prev.size()+locations_curr.size()+10);

		vector<Candidate> candidates;
		FORI(curr.size()) {
			if (!locations_prev.count(curr[i]) && !locations_curr.count(curr[i])) {
				dst.push_back(curr[i]);
				continue;
			}
	

			//so fucking crazy syntax
			std::pair<multimap<int, int>::iterator, multimap<int, int>::iterator> matches_prev  = locations_prev.equal_range(curr[i]);
			std::pair<multimap<int, int>::iterator, multimap<int, int>::iterator> matches_curr  = locations_prev.equal_range(curr[i]);

			candidates.clear();
//			candidates.reserve(locations_prev.size()*2+10);

			// find candidates
			for (int direction_iter=-1; abs(direction_iter)==1; direction_iter+=2) {
				multimap<int, int>::iterator matches_iter;
				
				// find candidates using previous frame
				matches_iter = matches_prev.first;
				for (; matches_iter != matches_prev.second; ++matches_iter) {
					Candidate candidate;
					candidate.direction = direction_iter;
					candidate.location = matches_iter->second;
					candidate.count = 0;

					// dont refer backwards: this way we don't have to double-buffer in the playah
					if (candidate.location<i) continue;

					// search
					while (1) {
						int previ = candidate.location + candidate.count * candidate.direction;
						int curri = i + candidate.count;
						if (candidate.count>=0x7FFF) break;
						if (previ>=prev.size()) break;
						if (curri>=curr.size()) break;
						if (previ<curri) break;
						if (prev[previ]!=curr[curri]) break;
						candidate.count++;
					}

					if (candidate.count>2) candidates.push_back(candidate);
				}


				// find candidates using current frame
				matches_iter = matches_curr.first;
				for (; matches_iter != matches_curr.second; ++matches_iter) {
					Candidate candidate;
					candidate.direction = direction_iter;
					candidate.location = matches_iter->second;
					candidate.count = 0;

					// only refer backwards: this way we don't have to double-buffer in the playah
					if (candidate.location>=i) continue;

					// search
					while (1) {
						int previ = candidate.location + candidate.count * candidate.direction;
						int curri = i + candidate.count;
						if (candidate.count>=0x7FFF) break;
						if (previ>=i) break;
						if (curri>=curr.size()) break;
						if (previ>=curri) break;
						if (curr[previ]!=curr[curri]) break;
						candidate.count++;
					}

					if (candidate.count>2) candidates.push_back(candidate);
				}
			}

			// store best candidate
			if (candidates.size()) {
				// sort candidates
				sort(candidates.begin(), candidates.end());
				Candidate bestCandidate = candidates.front();
				dst.push_back(0x8000 | bestCandidate.location);
				dst.push_back(bestCandidate.count*bestCandidate.direction);
				i += bestCandidate.count-1;
			} else {
				dst.push_back(curr[i]);
			}



		}
	}



};

