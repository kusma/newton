#ifndef BEATCOUNTER_H
#define BEATCOUNTER_H

class BeatCounter
{
public:
	BeatCounter(const fixed16 *beats, const size_t beat_count)
	: beats(beats), beat_count(beat_count), pos(0) {}
	
	int getPos(const fixed16 beat)
	{
		while ((pos < beat_count) && (beat > beats[pos]))
		{
			pos++;
		}
		return pos;
	}
	
private:
	const fixed16 *beats;
	const size_t   beat_count;
	int            pos;
};

#define BEATCOUNTER(array) BeatCounter(array, sizeof(array) / sizeof(array[0]));

#endif /* BEATCOUNTER_H */
