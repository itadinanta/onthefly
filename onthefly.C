/*
  ontheflY - an OpenGL techdemo
  version 1.0
  
  Copyright (C) 2003 Nicola Orrù
  You can read how to contact the author at http://www.itadinanta.it
  ------------------------------------------------------------------

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <sys/timeb.h>
#include <sys/time.h>
#include <time.h>
#ifdef __WIN32__
#  include <windows.h>
#  undef RGB
#endif

using namespace std;

#define FULLSCREEN
#define FLOAT float
#define LIST_PARTICLE 1
#define LIST_BODY 2
#define LIST_WING1 3
#define LIST_WING2 4
#define LIST_TAIL 5
#define LIST_PARTICLESHADE 6

#define FLAG_MOSQUITO    0x0001
#define FLAG_WINGS       0x0002
#define FLAG_PERSISTENT  0x0004
#define FLAG_IDLE        0x0008
#define FLAG_EVEN        0x0010
#define FLAG_COLLISION   0x0020
#define FLAG_TAIL        0x0040
#define FLAG_MATE        0x0080
#define FLAG_PREDATOR    0x0100
#define FLAG_PREY        0x0200
#define FLAG_INNERFADE   0x0400
#define FLAG_CAMSMOOTH   0x0400

#define EFFECT_BOUNCE     0x0001
#define EFFECT_SPRINKLER  0x0002
#define EFFECT_FOUNTAIN   0x0004
#define EFFECT_FIREWORKS  0x0008
#define EFFECT_SNOW       0x0010
#define EFFECT_SWARM_1    0x0020
#define EFFECT_SWARM_2    0x0040
#define EFFECT_SWARM_3    0x0080
#define EFFECT_SWARM_4     0x0100
#define EFFECT_FIRE        0x0200
#define DIR_RANDOM_1       0x0400
#define DIR_RANDOM_2       0x0800
#define EFFECT_PERSIST     0x1000
#define EFFECT_PERSIST_SUB 0x2000
#define DRAW_FPS           0x4000
#define EFFECT_PERSIST_ADD 0x8000
#define DIR_UP             0x00010000
#define DIR_DOWN           0x00020000
#define DIR_LEFT           0x00040000
#define DIR_RIGHT          0x00080000
#define DIR_TOP            0x00100000
#define DIR_BOTTOM         0x00200000
#define TURN_LEFT          0x00400000
#define TURN_RIGHT         0x00800000
#define DIR_ALL            0x00ff0000 
#define DRAW_HELP          0x80000000
#define EFFECT_RAND_MASK   0x00ff0dff

#define PLANE_FLOOR -2.0
#define EPS 1.0e-5
#define EPS_CAM 1

#define MAX_PARTICLES     1024
#define MAX_COLLISION_PARTICLES    1024
#define MAX_BYTE 255
#define BITS_CHAR 8
#define SIZE_IN_BITS(d) (sizeof(d)*BITS_CHAR)
#define ACC_GRAVITY (-9.8)
#define MAX_CORE_SIZE 3.0


#define TIME_START 1706000
#define MSECS_IN_SEC 1000
#define USECS_IN_MSEC 1000

#define GENERATOR_DIST 5.0
#define GENERATOR_HEIGHT 0.5
#define GENERATOR_VEL 1
#define GENERATOR_SIZE 2.0

#define SOLID_RAY 0.125
static char * copyright="'ontheflY', an oPenGl techdemo (C) 2003 nicola orru' www.itadinanta.it";
static float timeSeq = 0;
static float accum = 0.0;
static int doLive = 1;
static int odd = 1;
static float sightAngle = -90;
static bool interactiveMode = false;
static int randSeq = -1;
static int randDelta;
static int randCamera = 0;
static int EffectBits = EFFECT_FIRE;
static float fps=30;
static float frametime = 1.0/fps; // 30 msec ~33fps
static float framebase = 1.4/0.03;
static float timeunit = 1; // frametime * framebase
static float zoom = 1;
static int nFrames = 0;
static int camMode = 0;
static int tgtMode = 0;

char * message = 0;
float messageTTL;

enum _camModes {
	CAM_INTERACTIVE,
	CAM_LIGHT,
	CAM_QUEEN,
	CAM_RANDOM,
	CAM_SLIDER,
	CAM_BIRDFLY,
	CAM_CENTER,
	CAM_G0,
	CAM_G1,
	CAM_G2,
	CAM_G3,
	CAM_G4,
	CAM_G5,
	CAM_MODES
} camRandModes[] = { CAM_SLIDER, CAM_QUEEN, CAM_LIGHT, CAM_G0, CAM_G2, CAM_G4, CAM_RANDOM, CAM_BIRDFLY };
const char *camName[] ={"interactive","liGht","Queen","flYcam","Glider","bird's eYe","oriGin","fire 1","fire 2","fire 3","fire 4","fire 5","fire 6"};
enum _tgtModes {
	TGT_INTERACTIVE,
	TGT_LIGHT,
	TGT_QUEEN,
	TGT_RANDOM,
	TGT_SLIDER,
	TGT_CENTER,
	TGT_G0,
	TGT_G1,
	TGT_G2,
	TGT_G3,
	TGT_G4,
	TGT_G5,
 	TGT_MODES
} tgtRandModes[] = { TGT_RANDOM, TGT_QUEEN, TGT_LIGHT, TGT_CENTER };
const char *tgtName[]={"interactive","liGht","Queen","anYflY","Glider","oriGin","fire 1","fire 2","fire 3","fire 4","fire 5","fire 6"};
class M44;
static void Draw();
static void Key(unsigned char key, int x, int y);
static void Auto();
static float Width, Height;
static float distBody= 2000; 
static float distWings= 7500;
static float distInner= 7500;
static float distPoints= 14000;
static float distNothing= 20000;
static float distZoom2x= 10000;
static float distZoom4x= 10000;

const char * helpmsg[] = {
	"ontheflY bY nicola orru'",
	"------------------------------- helP screen",
	"'Q' to Quit",
	"'h' toGGles helP",
	"<sPacebar> stoPs simulation",
	"'0' toGGles fire",
	"'1' to '4'/F1 to F4 toGGles/triGGers swarm 1-4 Generation",
	"'5' to '9'/F5 to F8 toGGles/triGGers eYe candies",
	"'='/'@' starts/stoP animation",
	"'f' toGGles fPs info",
	"'e','E'/<INS>,<END> toGGles/triGGers escaPe modes",
	"'u','j','n',''g'/'U','J','N','G' starts/stoPs automatic movement",
	"<arrows>,<shift+arrows>,pGup,pGdown interactive camera movement",
	"'a' resets interactive camera",
	"'c','C' chanGes camera",
	"'t','T' chanGes camera tarGet",
	"'p' toGGles Particle aGinG",
	"'x' stoPs all effects",
	"'z','Z' zoom +/-",
	""
};

FLOAT rand(float min, float max) {
	return min+(max-min)*((float)rand() / (float)RAND_MAX);
}

bool prob(int num, int den=100) {
	return (rand()%den) < num;
}

int getTick() {
#ifdef __WIN32__
	return GetTickCount();
#else
        static struct timeval tv;	
	gettimeofday(&tv,0);
	return (tv.tv_sec-TIME_START)*MSECS_IN_SEC+tv.tv_usec/USECS_IN_MSEC;
#endif
}

int msecTime() {
	static int timeInit = 1;
	static int time0;

	if (timeInit) {
		timeInit = 0;
		time0 = getTick();
	}
	return getTick() - time0;
}

inline float POWT(float a) {
	return (expf(a*timeunit));
}

#define MIN(a,b) (((a)<(b))?(a):(b))
#define ONOFF(x) ((x)?"on":"off")

static const float msgFontSize = 0.0004;
static void FireMessage(const char *msg, const char *msgArg) {
	static char fmtMessage[1024];
	sprintf(fmtMessage,msg,msgArg);
	message = fmtMessage;
	messageTTL = 70;
}


template <class WORD>
class BitMask {
	const short nbits;
	const short ilen;
	const short size;
        WORD *bits;
	const short bitsperword;
public:
	BitMask(int nBits=MAX_COLLISION_PARTICLES, void *Bits = 0): 
		bits((WORD*)Bits), 
		nbits(nBits),ilen(nbits/SIZE_IN_BITS(WORD)), 
		bitsperword(SIZE_IN_BITS(WORD)),size(nBits/BITS_CHAR) {}
	inline void setBits(WORD *Bits = 0) { bits = Bits; };
	inline void toggleBit(int bit) {
		if (!bits) return;
		bits[bit/bitsperword] ^= (1L << (bit % bitsperword));
	}
	inline void setBit(int bit) {
		if (!bits) return;
		bits[bit/bitsperword] |= (1L << (bit % bitsperword));
	}
	inline void setBits(int firstBit, int lastBit) {
		// should be optimized
		for (register int i=firstBit; i<lastBit; i++) {		
			setBit(i);
		}
	}
	inline void clearBit(int bit) {
		if (!bits) return;
		bits[bit/bitsperword] &= ~(1L << (bit % bitsperword));
	}
	inline int testBit(int bit) {
		if (!bits) return false;
		return !!(bits[bit/bitsperword] & (1L << (bit % bitsperword)));
	}
	inline int isZero() {
		if (!bits) return true;
		register WORD * bd = bits;
		register int n=ilen;
		while (n--) if (*bd--) return 0;
		return 1;
	}
	inline void clearAll() {
		if (!bits) return;
		memset(bits,0,size);
	}
	inline void setAll() {
		if (!bits) return;
		memset(bits,0xff,size);
	}
	inline void toggleAll() {
		if (bits) {
			register WORD * bd = bits;
			register int n=ilen;
			while (n--) {
				*bd=~*bd;
				bd++;
			}
		}
	}
	inline BitMask& operator = (const BitMask &src) {
		if (!bits) return *this;
		if (!src.bits) clearAll();
		else memcpy(bits,src.bits,size);
		return *this;
	}
	inline BitMask& operator |= (const BitMask &src) {
		if (!bits || !src.bits) return *this;
		register WORD * bd = bits, *bs = src.bits;
		register int n=ilen;
		while (n--) *bd++|=*bs++;
		return *this;
	}
	inline BitMask& operator ^= (const BitMask &src) {
		if (!bits || !src.bits) return *this;
		register WORD * bd = bits, *bs = src.bits;
		register int n=ilen;
		while (n--) *bd++^=*bs++;
		return *this;
	}
	inline BitMask& operator &= (const BitMask &src) {
		if (!bits) return *this;
		if (!src.bits) clearAll();
		else {
			register WORD * bd = bits, *bs = src.bits;
			register int n=ilen;
			while (n--) *bd++&=*bs++;
		}
		return *this;
	}
	inline BitMask& operator -= (const BitMask &src) {
		if (!bits || !src.bits) return *this;
		register WORD * bd = bits, *bs = src.bits;
		register int n=ilen;
		while (n--) *bd++&=~*bs++;
		return *this;
	}
	int listBits(int *dest) {
		if (!bits) return 0;
		int bitsout=0;
		register WORD bitl;
		register WORD * bs = bits;
		register int n=ilen;
		register int j,i=0;
		while (n--) {
			if (bitl=*bs) {
				j=i;
				while (bitl) {
					if (bitl & 1) {
						*dest++=j;
						bitsout++;
					}
					j++;
					bitl>>=1;
				}
			}
			i+=bitsperword;
			bs++;
		}
		return bitsout;
	}
};

template <class WORD, int BUCKET_SIZE, int BUCKET_COUNT>
class CollisionSlicer {
private:
	class CollisionBucket {
	public:
		int n;
		class BucketPair {
		public:
			int key;
			WORD* datablock;
		};
		inline WORD * findBlock(int searchkey) {
			BucketPair* block=find(searchkey);
			if (block) return block->datablock;
			else return 0;
		}
		BucketPair *add(int key, WORD* datablock) {
			if (n>=BUCKET_SIZE) return 0; // no space left				
			struct BucketPair* block=blocks+(n++);			
			block->key=key;
			block->datablock=datablock;
			return block;
		}
		BucketPair *find(int searchkey) {
			struct BucketPair* block=blocks;
			for (register int i=0; i<n; i++, block++)
				if (searchkey==block->key)
					return block;
			return 0;
		}
		BucketPair blocks[BUCKET_SIZE];
	} *hash;
	int particlesPerSlice;
	int maxSlices;
	int slicesUsed;
	WORD *bitBlock;
	WORD *nextFree;
	WORD* allocSlice() {
		if (slicesUsed>=maxSlices) {
//			cerr << "Out of slices!\n";
			return 0;
		}
		WORD * block=nextFree;
		nextFree+=sliceSize();
		slicesUsed++;
		return block;
	}
	inline int sliceSize() {
		return (particlesPerSlice/SIZE_IN_BITS(WORD));
	}

	inline unsigned int toHash(int hashindex) {
		return ((unsigned int)hashindex) % BUCKET_COUNT; // trivial hash
	}
public:
	CollisionSlicer(int particles=MAX_COLLISION_PARTICLES, 
			int maxslices=MAX_COLLISION_PARTICLES) {
		particlesPerSlice=particles;
		maxSlices=maxslices;
		bitBlock=(WORD*)calloc(maxSlices,sliceSize()*BITS_CHAR);
		hash=(CollisionBucket*)calloc(BUCKET_COUNT,sizeof(CollisionBucket));
		nextFree=bitBlock;
		slicesUsed=0;
	}
	void clearAll() {
		nextFree=bitBlock;
		memset(bitBlock,0,maxSlices*sliceSize()*BITS_CHAR);
		memset(hash,0,BUCKET_COUNT*sizeof(CollisionBucket));
		slicesUsed=0;
	}
	~CollisionSlicer() {
		free(bitBlock);
		free(hash);
	}
	BitMask<WORD>* getSlice(int key, BitMask<WORD>* cursor) {
		unsigned int hashidx = toHash(key);
		CollisionBucket *b=hash+hashidx;
		WORD * datablock = b->findBlock(key);
		cursor->setBits(datablock);
		return cursor;
	}
	void addToSlice(int key, int particleIndex) {
		unsigned int hashidx = toHash(key);
		CollisionBucket *b=hash+hashidx;
		if (!b) {
#ifdef DEBUG
			fprintf(stderr,"Bucket full: %d\n",hashidx);
#endif
			return;
		}
		WORD * datablock = b->findBlock(key);
		if (!datablock) {
			datablock = allocSlice();
			b->add(key, datablock);
		}
		BitMask<WORD> bmsk(particlesPerSlice,datablock);
		bmsk.setBit(particleIndex);
		
	}
};

typedef BitMask<unsigned long> LBitMask;
class V3 {
public:
	static const V3 vZero, aX, aY, aZ;
public:
	FLOAT x,y,z;
	inline V3() {
	}
	inline V3(FLOAT _x, FLOAT _y, FLOAT _z) {
		x = _x;
		y = _y;
		z = _z;
	}
	inline void set(FLOAT _x, FLOAT _y, FLOAT _z) {
		x = _x;
		y = _y;
		z = _z;
	}
	inline void set(const V3& o) {
		x = o.x;
		y = o.y;
		z = o.z;
	}
	inline void setScalar(const V3& o, FLOAT timev) {
		x=o.x*timev;
		y=o.y*timev;
		z=o.z*timev;
	}
	inline 	void setDiff(const V3& o, const V3& r) {
		x = r.x - o.x;
		y = r.y - o.y;
		z = r.z - o.z;
	}
	inline void setSum(const V3& o, const V3& r) {
		x=r.x+o.x;
		y=r.y+o.y;
		z=r.z+o.z;
	}
	inline void setVec(const V3& a, const V3& s) {
		x = a.y*s.z-s.y*a.z;
		y = a.z*s.x-s.z*a.x;
		z = a.x*s.y-s.x*a.y;		
	}
	inline void reflect(V3& d, const V3 &s) const {
		float coef = 2*(x*s.x+y*s.y+z*s.z);
		d.x=s.x-coef*x;
		d.y=s.y-coef*y;
		d.z=s.z-coef*z;
	}
	inline void zero() {
		x=0;
		y=0;
		z=0;
	}
	inline FLOAT norm() const {
		return x*x+y*y+z*z;
	}
	inline float len() const {
		return sqrtf(x*x+y*y+z*z);
	}
	void normalize() {
		FLOAT l=len();
		if (l) {
			l = 1.0/l;
			x *=l;
			y *=l;
			z *=l;
		}
	}
	inline void orig(const V3& o) {
		x-=o.x;
		y-=o.y;
		z-=o.z;
	}
	inline void moveBy(const V3& v, const V3& a, FLOAT timev) {
		float timev2 = 0.5*timev;
		x+=(v.x+a.x*timev2)*timev;
		y+=(v.y+a.y*timev2)*timev;
		z+=(v.z+a.z*timev2)*timev;
	}
	inline void moveBy(const V3& o, FLOAT timev) {
		x+=o.x*timev;
		y+=o.y*timev;
		z+=o.z*timev;
	}
	inline void moveBy(const V3& o) {
		x+=o.x;
		y+=o.y;
		z+=o.z;
	}
	inline void multBy(FLOAT scale) {
		x*=scale;
		y*=scale;
		z*=scale;
	}
	inline void divBy(FLOAT scale) {
		if (scale) {
			scale = 1.0/scale;
			x*=scale;
			y*=scale;
			z*=scale;
		}
	}
	inline FLOAT dot(const V3& s) const {
		return	x * s.x + 
			y * s.y + 
			z * s.z;
	}
	inline void vec(V3& d, const V3& s) const {
		d.x = y*s.z-s.y*z;
		d.y = z*s.x-s.z*x;
		d.z = x*s.y-s.x*y;
	}
	inline FLOAT& operator[](int idx) {
		return *(((FLOAT*)this)+idx);
	}
	V3& normal(V3 &n, const V3 & p1, const V3 &p2) const {
		V3 p1n = p1;
		V3 p2n = p2;
		p1n.orig(*this);
		p2n.orig(*this);
		p1n.vec(n,p2n);
		n.normalize();
		return n;
	}
	inline GLfloat* GLf() {
		return  (GLfloat*)this;
	}
	inline const GLfloat* GLf() const {
		return  (GLfloat*)this;
	}
};

class V4: public V3 {
public:
	static const V4 iX;
	static const V4 iY;
	static const V4 iZ;
	static const V4 iW;
	static const V4 up;
public:
	FLOAT w;
	V4() {
	}
	inline V4(FLOAT _x, FLOAT _y, FLOAT _z, FLOAT _w): V3(_x,_y,_z) {
		w = _w;
	}
	inline V4(FLOAT _x, FLOAT _y, FLOAT _z): V3(_x,_y,_z) {
		w = 1.0;
	}
	inline V4& operator = (const V3& s) {
		x=s.x;
		y=s.y;
		z=s.z;
		w=1.0;
		return *this;
	}
	inline void set(FLOAT _x, FLOAT _y, FLOAT _z, FLOAT _w) {
		x = _x;
		y = _y;
		z = _z;
		w = _w;
	}
	inline void set(const V4& s) {
		x = s.x;
		y = s.y;
		z = s.z;
		w = s.w;
	}

	inline FLOAT dot(const V4& s) const {
		return  
			x * s.x + 
			y * s.y + 
			z * s.z + 
			w * s.w;
	}
	inline FLOAT dot(const V3& s) const {
		return  
			x * s.x + 
			y * s.y + 
			z * s.z + 
			w;
	}
	inline GLfloat* GLf() {
		return  (GLfloat*)this;
	}
	inline const GLfloat* GLf() const {
		return  (GLfloat*)this;
	}
};

class M44 {
public:
	V4 r1, r2, r3, r4;
	inline void mult(V4& d, const V4& s) const {
		d.x=r1.dot(s);
		d.y=r2.dot(s);
		d.z=r3.dot(s);
		d.w=r4.dot(s);
	}
	inline void mult(V4& d, const V3& s) const {
		d.x=r1.dot(s);
		d.y=r2.dot(s);
		d.z=r3.dot(s);
		d.w=r4.dot(s);
	}
	inline void mult3(V3& d, const V3& s) const {
		d.x=r1.dot(s);
		d.y=r2.dot(s);
		d.z=r3.dot(s);
	}
	inline void setR(int r, const V4& s) {
		(*this)[r].set(s);
	}
	inline void setC(int col, const V3&s) {
		r1[col]=s.x;
		r2[col]=s.y;
		r3[col]=s.z;
	}
	inline void setC(int col, const V4&s) {
		r1[col]=s.x;
		r2[col]=s.y;
		r3[col]=s.z;
		r4[col]=s.w;
	}
	inline V4& operator[](int c) {
		return *(((V4*)this)+c);
	}
	inline void transpose() {
		swap(r1.y,r2.x);
		swap(r1.z,r3.x);
		swap(r1.w,r4.x);
		swap(r2.z,r3.y);
		swap(r2.w,r4.y);
		swap(r3.w,r4.z);
	}
	inline void identity() {
		r1.set(V4::iX);
		r2.set(V4::iY);
		r3.set(V4::iZ);
		r4.set(V4::iW);
	}
	inline void rotY(FLOAT angle) {
		float sa = sin(angle);
		float ca = cos(angle);
		identity();
		r1.x=ca;
		r1.z=sa;
		r3.x=-sa;
		r3.z=ca;
	}
	inline GLfloat* GLf() {
		return  (GLfloat*)this;
	}
	inline const GLfloat* GLf() const {
		return  (GLfloat*)this;
	}
private:
	float sw;
	inline void swap(FLOAT& a, FLOAT& b) {
		sw=a; a=b; b=sw;
	}
};

class ROT {
public:
	V3 n;
	FLOAT disp;
	inline ROT(FLOAT _x, FLOAT _y, FLOAT _z, FLOAT _disp): n(_x,_y,_z) {
		disp = _disp;
	}
	inline void set(FLOAT _x, FLOAT _y, FLOAT _z, FLOAT _disp) {
		n.set(_x, _y, _z);
		disp = _disp;
	}
};

class QUAT {
public:
	FLOAT s;
	V3 n;
};

class LOC {
public:
	V4 pos;
	ROT dir;
	inline LOC(FLOAT _px, FLOAT _py, FLOAT _pz, FLOAT _dx, FLOAT _dy, FLOAT _dz, FLOAT _disp):
		pos(_px, _py, _pz),
		dir(_dx, _dy, _dz, _disp) {};
};

class RGB {
public:
	unsigned char r,g,b;
	RGB() {}
	RGB(unsigned char R, unsigned char G, unsigned char B) {
		r=R;
		g=G;
		b=B;
	}
	inline void set(unsigned char R, unsigned char G, unsigned char B) {	      
		r=R;
		g=G;
		b=B;
	}
};

class RGBA: public RGB {
public:
	unsigned char a;
	RGBA() {}
	RGBA(unsigned char R, unsigned char G, unsigned char B, unsigned char A):
		RGB(R,G,B),a(A) {		
		}
	inline void set(unsigned char R, unsigned char G, unsigned char B, unsigned char A) {
		RGB::set(R,G,B);
		a=A;
	}
};

class LA {
public:
	unsigned char l,a;
	LA(unsigned char L, unsigned char A) {
		l=L;
		a=A;
	}
};

class ParticleImage {
public:
	int w;
	int h;
	LA * la;
	ParticleImage(int l, float attPow=3, float intCenter=300.0, float den=256, float offset=3) {
		w=l;
		h=l;
		la = (LA*)malloc(w*h*sizeof(LA));
		int i,j;
		unsigned char a;
		float att,att2;
    
		unsigned char lum, alpha;
		float distFromCenterX, distFromCenterY,ray2x,ray2y,ray;
		int w2=w/2;
		int h2=h/2;
		for (i=0; i<h; i++) {
			distFromCenterY=(i-h2);
			ray2y=distFromCenterY*distFromCenterY;
			for (j=0; j<w; j++) {
				distFromCenterX=(j-w2);
				ray2x = distFromCenterX*distFromCenterX;
				ray = sqrtf(ray2x + ray2y);
				a=0;
				if (ray<w2) {
					att=intCenter/(1+pow((double)ray,(double)attPow)/den)-offset+0.5;
					if (att<0) att=0;
					a=(unsigned char)(att>=(float)MAX_BYTE?(float)MAX_BYTE:att);
				}
				pSet(j, i,MAX_BYTE,a);
			}
		}
	}
	void pSet(int x, int y, unsigned char l,unsigned char a) {
		LA *pixel = la+y*w+x;
		pixel->l = l;
		pixel->a = a;
	}
  
	~ParticleImage() {
		free(la);
	}
};

class Particle;
class Behavior {
public:
	virtual void live(Particle * item) = 0;
	virtual void collide(Particle *a, int nColl, Particle *b[]) = 0;
	virtual void mate(Particle *a, Particle *b, Particle *p) = 0;
	virtual void eat(Particle *a, Particle *b) = 0;
	virtual ~Behavior() {}
};

class Genotype {
public:
	static const int geneLength = 16*SIZE_IN_BITS(unsigned long);
	union {
		unsigned long int uiBits[geneLength/SIZE_IN_BITS(unsigned long)];
		int iBits[geneLength/SIZE_IN_BITS(int)];
		unsigned char  cBits[geneLength/SIZE_IN_BITS(unsigned char)];
	};
	inline float geneRepulsion() { return sfGene(0); }
	inline float geneAttraction() { return sfGene(2); }
	inline float geneFriction() { return sfGene(4); }
	inline float geneRandomMove() { return sfGene(6); }
	inline float geneMaxSize() { return sfGene(8);}
	inline int   geneMaturity() { return cGene(10); }

	inline int cGene(int i) { return (int)cBits[i]; }
	inline float sfGene(int i) { return (float)*(((short*)cBits)+i)/32768.0; }
	inline float cfGene(int i) { return (float)cBits[i]/(float)MAX_BYTE; }
	inline void mutate(int probPerCent, int nBits) { 
		if (prob(probPerCent)) 
			mutate(nBits);
	}
	void mutate(int nBits) {
		LBitMask m(geneLength,uiBits);
		while (nBits--)
			m.toggleBit(rand() % geneLength);
	}
	void shuffle() {
		int * ib=iBits;
		for (int i=0; i<sizeof(iBits)/sizeof(int); i++)
			*ib++=rand();
	}
	void crossWith(Genotype &partner) {
		int crossPoint1 = rand() % geneLength;
		int crossPoint2 = rand() % geneLength;
		unsigned long int maskBits[geneLength/SIZE_IN_BITS(unsigned long)];
		LBitMask m1(geneLength,uiBits);
		LBitMask m2(geneLength,partner.uiBits);
		LBitMask msk(geneLength,maskBits);
		int i;
		msk.clearAll();
		if (crossPoint1 <= crossPoint2) 
			msk.setBits(crossPoint1,crossPoint2);
		else {
			msk.setBits(crossPoint2,crossPoint1);
			msk.toggleAll();
		}
		m1  -= msk;
		msk &= m2;
		m1  |= msk;
	}
};

class Particle {
public:
	float distFromCamera;
	V3 pos,lpos;
	V4 tpos, vel, acc;
	float size, ttl, age;
	unsigned int flags;
	RGBA color;
	RGBA color2;
	Particle * chain;
	Behavior * behavior;
	Genotype gt;
	Particle() {}
	void reset() {
		memset(this,0,sizeof(Particle));
		acc.y=ACC_GRAVITY;
		color.set(MAX_BYTE,155,155,MAX_BYTE);
		color2.set(MAX_BYTE,MAX_BYTE,MAX_BYTE,MAX_BYTE);
		ttl=500;
		size = 1.0;
	}
	~Particle() {
	}
	void live() {
		lpos=pos;
		if (behavior) behavior->live(this);
	}
	void collideWith(int countCollision, Particle *d[]) {
		if (behavior) behavior->collide(this,countCollision,d);
	}
	void transform(M44& t) {
		t.mult3(tpos, pos);
	}
	inline int hashX() const {
		return (int)(lpos.x+.5);
	}
	inline int hashY() const {
		return (int)(lpos.y+.5);
	}
	inline int hashZ() const {
		return (int)(lpos.z+.5);
	}
	void getM44(M44& dest) const {
		V3 zveln = vel;
		V3 xveln;
		V3 yveln;
		zveln.normalize();
		xveln.setVec(V4::up, zveln); xveln.normalize();
		yveln.setVec(zveln, xveln);  yveln.normalize();
		dest.setC(3,pos);
		dest.setC(2,zveln);
		dest.setC(1,yveln);
		dest.setC(0,xveln);
		dest.r4.set(V4::iW);
	}
	
	friend class ParticleEngine;
	friend class Behavior;
};


static Particle *cam;
static Particle *light = 0;
static Particle *birdfly = 0;
static Particle *queen = 0;
static Particle *center = 0;
static Particle *slidingCam = 0;
static const int nGen = 6;
static int iGen = 0;
static Particle *generators[nGen];
static Particle *cameras[CAM_MODES];
static Particle *targets[TGT_MODES];

class StandardBehavior: public Behavior {
	float sd;
	int ttlNew;
	int mutationProbability;
	float sizePrizeOnEat;
	float ttlPrizeOnEat;
	float interactionDistance2;
	float minMateAge;
	float minMateSize;
	float collisionRepulsionFactor;
public:
	int fade;
	int fadeInner;
	float minTTL;
	StandardBehavior(int _fade = 800, int _fadeInner = 750, float _minTTL = 25): sd(-PLANE_FLOOR)  {
		fade=_fade;
		fadeInner=_fadeInner;
		minTTL=_minTTL;
		ttlPrizeOnEat=75;
	        sizePrizeOnEat=0.5;
		mutationProbability=25;
		ttlNew=500;
		interactionDistance2=0.2;
		minMateAge=10;
		minMateSize=0.5;
		collisionRepulsionFactor=35.0;
	}
	inline void bounceY(Particle *p) {
		float sd=this->sd-SOLID_RAY*p->size;
		float sd2=sd+sd;
		if (p->pos.y < -sd && p->vel.y < 0) {
			p->pos.y = -p->pos.y-sd2;
			p->vel.y = -p->vel.y;
		}
	}
	virtual void live(Particle * p) {
		p->flags ^= FLAG_EVEN;
		p->pos.moveBy(p->vel,p->acc,frametime);
		p->vel.moveBy(p->acc,frametime);
		if (p->flags & FLAG_INNERFADE) {
			float aFade=fadeInner/1000.0;
			float cFade=1-aFade;
			p->color.r=(unsigned char)(p->color.r*aFade+p->color2.r*cFade+0.5);
			p->color.g=(unsigned char)(p->color.g*aFade+p->color2.g*cFade+0.5);
			p->color.b=(unsigned char)(p->color.b*aFade+p->color2.b*cFade+0.5);
			p->color.a=(unsigned char)(p->color.a*aFade+p->color2.a*cFade+0.5);
		}
		if (p->ttl<minTTL) {
			p->color.a=((int)p->color.a*fade)/1000;
			p->color2.a=((int)p->color.a*fadeInner)/1000;
		}
		if (!(p->flags & FLAG_PERSISTENT) && (!(EffectBits & EFFECT_PERSIST))) 
			p->ttl-=timeunit;
		if (p->pos.y < PLANE_FLOOR-p->size)
			p->ttl=0;
		p->age+=timeunit;
	}
	virtual void collide(Particle *a, int collCount, Particle *b[]);
	virtual void mate(Particle *a, Particle *b, Particle *p);
	virtual void eat(Particle *a, Particle *b);
} BStandard(800,750,25), BFire(750,750,25);

class BounceBehavior: public StandardBehavior {
	float sd, sd2, ray, ray2, efficiency;
public:
	BounceBehavior(float side = 2,float r=5, float eff=0.8) {
		sd=side;
		sd2=side*2;
		ray=r;
		ray2=r*r;
		efficiency=eff;
	}
	virtual void live(Particle *p);
} BBounce(-PLANE_FLOOR,5.5);

#define ANGLE_CAMERA_ROT 0.04
class GeneratorBehavior: public StandardBehavior {
	M44 rotY;
	float Rangle;
public:
	GeneratorBehavior(float rangle) {
		Rangle=rangle;
	}
	virtual void live(Particle* p) {	
		rotY.rotY(Rangle*frametime);
		rotY.mult3(p->pos,p->lpos);
		V3 lvel=p->vel;
		rotY.mult3(p->vel,lvel);
	}
} BGen(ANGLE_CAMERA_ROT);


#define CORE_BASE_EFF 0.995
#define PREY_BASE_RAY 3.2

class SpringMassBehavior: public StandardBehavior {
	float efficiency;
	float sd;
	float maxCoreSize;
	float growthBase;
	float lnGrowthBase;
	float lnEfficiency;
public:
	SpringMassBehavior(float eff = 0.995, float maxcore = MAX_CORE_SIZE):
		sd(-PLANE_FLOOR),
		efficiency(eff),
		maxCoreSize(maxcore),
		growthBase(1.2) {
			lnGrowthBase=logf(growthBase);
			lnEfficiency=logf(efficiency);
		}
	virtual void live(Particle* p) {
		StandardBehavior::live(p);
		bounceY(p);
		p->size*=POWT(lnGrowthBase);
		if (p->size>maxCoreSize) p->size=maxCoreSize;
		p->acc=p->pos;
		p->acc.multBy(-1);
		p->vel.multBy(POWT(lnEfficiency));
	}
} BSpringMass(CORE_BASE_EFF);

class SpringFieldBehavior: public StandardBehavior {
	float efficiency;
	float r0;
	float maxSpeed,maxSpeed2;
	float minSpeed,minSpeed2;
	float minRand;
	float maxRand;
	float maxSize;
	float minSize;
	float scaleRand;
	float rayMagnitudo;
	float rayFoodScale;
	double depletionBase;
	float lnDepletionBase;
	float lnEfficiency;
public:
	SpringFieldBehavior(float eff = 0.97, float R0=4):
		efficiency(eff),
		r0(R0),
		minRand(0.7),
		maxRand(1.3),
		maxSpeed(5.0),
		minSpeed(1.0),
		maxSize(1),
		minSize(0.5),
		scaleRand(0.5),
		rayMagnitudo(1.5),
		rayFoodScale(0.1),
		depletionBase(0.99) {
			maxSpeed2=maxSpeed*maxSpeed;
			minSpeed2=minSpeed*minSpeed;
			lnDepletionBase=logf(depletionBase);
			lnEfficiency=logf(efficiency);
		}
	virtual void live(Particle* p) {
		StandardBehavior::live(p);
		bounceY(p);
		if (p->chain) {
			V3 norm = p->pos;
			norm.orig(p->chain->pos);
			float r=norm.len();
			float rRatio = r/r0;
			norm.multBy(-r);
			p->acc=norm;
			p->acc.multBy(-ACC_GRAVITY/r);
			if (rRatio<1) {
				float dist=r0-r;
				float gr=p->gt.geneRandomMove()*scaleRand;
				p->vel.x*=rand(minRand+gr, maxRand-gr);
				p->vel.y*=rand(minRand+gr, maxRand-gr);
				p->vel.z*=rand(minRand+gr, maxRand-gr);
				p->acc.multBy(-dist*dist*(1+p->gt.geneRepulsion()));
			}
			else if (rRatio<rayMagnitudo) {
				if (p->chain->size>1) {
					float q=1+(rayMagnitudo-rRatio)*rayFoodScale;
					if (q>0) {
						float feed=POWT(logf(q));
						p->ttl+=timeunit/p->size;
						p->size*=feed;
						p->chain->size/=feed;
						if (p->size > maxSize) p->size=maxSize;
					}
				}
			}
			else {
				p->size*=POWT(lnDepletionBase);
				if (p->size < minSize) p->size=minSize;
			}
			float lnEfficiency;
			p->vel.multBy(POWT(lnEfficiency+logf(1+p->gt.geneFriction())));
			r=p->vel.norm();
			if (r>maxSpeed2) p->vel.multBy(maxSpeed/sqrtf(r));
			else if (r<minSpeed2 && r) p->vel.multBy(minSpeed/sqrtf(r));
		}

	}
} BSpringField(0.995, PREY_BASE_RAY);


class MothParticleBehavior:public StandardBehavior {
	float r0;
	float attFactor;
	float efficiency;
	float maxSpeed,maxSpeed2;
	float minSpeed,minSpeed2;
	float minRand;
	float maxRand;
	float maxSize;
	float minSize;
	float scaleRand;
	float rayMagnitudo;
	float rayFoodScale;
	float maxAttraction,maxAttraction2;
	double depletionBase;
	float lnDepletionBase;
	float lnEfficiency;
public:
	MothParticleBehavior(float R0=2, float AttFactor=3, float eff=0.9999):
		efficiency(eff),
		r0(R0),
		attFactor(AttFactor),
		minRand(0.7),
		maxRand(1.3),
		maxSpeed(6.0),
		minSpeed(1.0),
		scaleRand(0.5),
		maxAttraction(30) {
			maxSpeed2=maxSpeed*maxSpeed;
			minSpeed2=minSpeed*minSpeed;
			maxAttraction2=maxAttraction*maxAttraction;
			lnDepletionBase=logf(depletionBase);
			lnEfficiency=logf(efficiency);			
		}
	virtual void live(Particle* p) {
		StandardBehavior::live(p);
		bounceY(p);
		if (p->chain) {
			V3 apos = p->chain->pos;
			apos.orig(p->pos);
			float r=apos.len();
			p->acc=apos;
			if (r>r0) 
				p->acc.multBy(attFactor/(r*(1+p->gt.geneAttraction())));
			else {
				float gr=p->gt.geneRandomMove()*scaleRand;
				p->acc.multBy(-1.0/(r*(1+p->gt.geneRepulsion())));
				p->vel.x*=rand(minRand+gr, maxRand-gr);
				p->vel.y*=rand(minRand+gr, maxRand-gr);
				p->vel.z*=rand(minRand+gr, maxRand-gr);
			}
			r=p->acc.norm();
			if (r>maxAttraction2) p->acc.multBy(maxAttraction/sqrtf(r));
			p->acc.multBy(1.0/(attFactor*p->size));
			p->vel.multBy(POWT(lnEfficiency+logf(1+p->gt.geneFriction())));
			r=p->vel.norm();
			if (r>maxSpeed2) p->vel.multBy(maxSpeed/sqrtf(r));
			else if (r<minSpeed2 && r) p->vel.multBy(minSpeed/sqrtf(r));
		}

	}
} BMoth(2,3,0.995), BQueen(1,1,0.95);

const V3 V3::vZero(0,0,0);
const V3 V3::aX(1,0,0);
const V3 V3::aY(0,1,0);
const V3 V3::aZ(0,0,1);
const V4 V4::iX(1,0,0,0);
const V4 V4::iY(0,1,0,0);
const V4 V4::iZ(0,0,1,0);
const V4 V4::iW(0,0,0,1);
const V4 V4::up(0,1,0,1);

class ParticleEngine {
	int n;
	int max;
	int free;
	Particle* particleBlock;
	Particle** particles;
	Particle** idleParticles;
public:	
	ParticleEngine(int nmax) {
		max = nmax;
		init();
	}
	void init() {
		n = 0;
		free = 0;
		particles =      (Particle**)calloc(max+1,sizeof(Particle*));
		idleParticles =  (Particle**)calloc(max,sizeof(Particle*));
		particleBlock =  (Particle*) calloc(max,sizeof(Particle));
		for (int i=0; i<max; i++)
			idleParticle(particleBlock + i);
	}
	inline Particle *getParticle() {
		Particle *r = free?idleParticles[--free]:0;
		if (r) r->reset();
		return r;
	}
	inline void idleParticle(Particle *p) {
		p->flags |= FLAG_IDLE;
		idleParticles[free++]=p;
	}
	Particle* activateParticle() {
		return activateParticle(getParticle());
	}
	inline int getNumParticles() const {
		return n;
	}
	inline Particle *getParticle(int i) const {
		return particles[i];
	}
	inline Particle** getCollisionParticles() {
		return particles;
	}
	inline const Particle** getParticles() const {
		return (const Particle**)particles;
	}
	void run() {
		for (Particle **p = particles; *p; p++)
			(*p)->live();
	}
	void cleanup() {
		Particle **p = particles;
		while (*p) {
			if ((*p)->ttl <= 0)
				kill(p-particles);
			else
				p++;
		}
	}
	void kill(int i) {
		idleParticle(particles[i]);
		particles[i]=particles[--n];
		particles[n]=0;
	}
	void transform(M44& t) {
		Particle **p = particles;
		while (*p) {
			(*p)->transform(t);
			p++;
		}
	}
	void sortByDist(const V3& cam) {
		Particle **p=particles;
		V3 d;
		int m=0;
		while (*p) {
			d.setDiff(cam,(*p)->pos);
			(*p)->distFromCamera = d.norm();
			p++;
			m++;
		}
		if (n)
			QuickSort(0,n-1);
	}
	void scaleDistance(float zoom) {
		register Particle **p, *pp;
		register float distf;
		for (p=particles;*p;p++) {
			pp=*p;	
			distf = pp->size * zoom;
			if (distf) pp->distFromCamera /= (distf*distf);
		}
	}

	~ParticleEngine() {
		::free(particles);
		::free(idleParticles);
		::free(particleBlock);
	}
private:
	Particle* activateParticle(Particle *p) {
		if (!p || n>=max) 
			return 0;
		return particles[n++] = p;
	}
	inline float Compare(Particle *a, Particle *b) {
		return (b->distFromCamera)-(a->distFromCamera);
	}
	void QuickSort(int L, int R) {
		int I, J;
		Particle *A, *B;
		Particle **Table = particles;
		I = L;
		J = R;
		A = Table[(L + R) >>1];
		do {
			while (Compare(Table[I], A)<0) I++;
			while (Compare(Table[J], A)>0) J--;
			if (I <= J)
			{
				B = Table[I];
				Table[I] = Table[J];
				Table[J] = B;
				I++;
				J--;
			}
		} while (!(I > J));
		if (L < J) QuickSort(L, J);
		if (I < R) QuickSort(I, R);
		
	}
} engine(MAX_PARTICLES);

class InteractionDetector {
	unsigned long *bitsX;
	unsigned long *bitsY;
	unsigned long *bitsZ;
	LBitMask bX, cX, bY, cY, bZ, cZ;
	int nParticles;
	int collCount;
	int *collPairs;
	Particle **collPointers;
	CollisionSlicer<unsigned long, 32, 256> CollX, CollY, CollZ;
public:
	InteractionDetector(int nparticles = MAX_COLLISION_PARTICLES):
		bX(nparticles), cX(nparticles),
		bY(nparticles), cY(nparticles),
		bZ(nparticles), cZ(nparticles) {
			nParticles=nparticles;
			collPairs = (int*)malloc(nparticles*sizeof(int));
			collPointers=(Particle**)malloc(nparticles*sizeof(Particle*));
			bitsX = (unsigned long*)malloc(nparticles/BITS_CHAR);
			bitsY = (unsigned long*)malloc(nparticles/BITS_CHAR);
			bitsZ = (unsigned long*)malloc(nparticles/BITS_CHAR);
			bX.setBits(bitsX);
			bY.setBits(bitsY);
			bZ.setBits(bitsZ);
		}
	~InteractionDetector() {
		free(collPairs);
		free(collPointers);
		free(bitsX);
		free(bitsY);
		free(bitsZ);
	}

	void detect(ParticleEngine *engine) {
		Particle **p,**p0;
		p=p0=engine->getCollisionParticles();
		CollX.clearAll();
		CollY.clearAll();
		CollZ.clearAll();
		int i;
		int j;
		for (p=p0; *p; p++) if ((*p)->flags & FLAG_COLLISION) {
			i=p-p0;
			CollX.addToSlice((*p)->hashX(),i);
			CollY.addToSlice((*p)->hashY(),i);
			CollZ.addToSlice((*p)->hashZ(),i);
		}
		for (p=p0; *p; p++) if ((*p)->flags & FLAG_COLLISION) {
			i=p-p0;
			bX = *(CollX.getSlice((*p)->hashX(),&cX));
			bX |= *CollX.getSlice((*p)->hashX()-1, &cX);
			bX |= *CollX.getSlice((*p)->hashX()+1, &cX);
			
			bY = *(CollY.getSlice((*p)->hashY(),&cY));
			bY |= *CollY.getSlice((*p)->hashY()-1, &cY);
			bY |= *CollY.getSlice((*p)->hashY()+1, &cY);

			bZ = *(CollZ.getSlice((*p)->hashZ(),&cZ));
			bZ |= *CollZ.getSlice((*p)->hashZ()-1,&cZ); 
			bZ |= *CollZ.getSlice((*p)->hashZ()+1, &cZ);

			bX.clearBit(i); // removes itself
			bX &= bY;
			bX &= bZ;
			
			if (collCount=bX.listBits(collPairs)) {
				for (j=0; j<collCount; j++)
					collPointers[j]=p0[collPairs[j]];
				(*p)->collideWith(collCount,collPointers);
			}

		}
	}
} detector(MAX_COLLISION_PARTICLES);

void StandardBehavior::collide(Particle *a, int collCount, Particle *b[]) {
	int j;
	V3 pos;
	Particle *bj;
	for (j=0; j<collCount; j++) {
		bj=b[j];
		pos = a->lpos;
		pos.orig(bj->lpos);
		float l=pos.norm();
		if (l) pos.multBy(1.0/(l*collisionRepulsionFactor));
		a->acc.moveBy(pos);
		if ((!(a->flags & FLAG_PERSISTENT)) &&
		     (a->flags & bj->flags & FLAG_MATE) && 
		     l<interactionDistance2) {
			if (a->flags & bj->flags & (FLAG_PREDATOR | FLAG_PREY)) {
				if ((a->age > minMateAge) && 
				    (a->size > minMateSize) &&
				    (bj->age > minMateAge))
					if(Particle *p=engine.activateParticle()) 
						mate(a,bj,p);
			}
			else if ((a->flags & FLAG_PREDATOR) && a->size>=bj->size)
				eat(a,bj);
		}
	}
}

void StandardBehavior::eat(Particle *a, Particle *b) {	
	a->ttl+=b->size*ttlPrizeOnEat;
	a->size+=b->size*sizePrizeOnEat;
	b->ttl=0;
	
	register float gmt=a->gt.geneMaxSize();
	if (gmt > 0 && a->size > gmt)
		a->size=gmt;
	if (a->size>1) a->size=1;
}

void StandardBehavior::mate(Particle *a, Particle *b, Particle *p) {
	if (p) {
		a->size*=0.5;
		a->age=0;
		*p=*a;
		p->ttl=ttlNew;
		p->flags&=~FLAG_PERSISTENT;
		p->color.set((unsigned char)(((int)a->color.r+(int)b->color.r)*.5+.5),
			     (unsigned char)(((int)a->color.g+(int)b->color.g)*.5+.5),
			     (unsigned char)(((int)a->color.b+(int)b->color.b)*.5+.5),
			     MAX_BYTE);
		p->color2.a=MAX_BYTE;
		p->gt.crossWith(b->gt);
		if (prob(mutationProbability)) {
			LBitMask msk(32,&p->color);
			msk.toggleBit(rand()%24);
			p->gt.mutate(1);
		}
		p->vel.multBy(-1);
	}
}

void BounceBehavior::live(Particle *p) {
	StandardBehavior::live(p);
	float sd=this->sd - p->size*SOLID_RAY;
	float sd2=sd*2;
	float r = this->ray - p->size*SOLID_RAY;
	float r2=r*r;
	if (p->pos.y < -sd && p->vel.y < 0) {
		p->pos.y = -p->pos.y-sd2;
		p->vel.y = -p->vel.y;
		p->vel.multBy(efficiency);
	}
	V3 bounceDir=p->pos;
	bounceDir.y=0;
	float l = bounceDir.norm();
	if (l>r2 && p->vel.dot(bounceDir) > 0) {
		bounceDir.multBy(-1.0/sqrtf(l));
		bounceDir.reflect(p->vel, p->vel);
		if (Particle *P=engine.activateParticle()) {
			*P=*p;
			P->vel.zero();
			P->acc.zero();
			P->ttl=20;
			P->behavior=&BStandard;
		}
		p->pos.moveBy(p->vel,frametime);
	}
}

struct KeyEffectPair {
	int key; 
	int bit;
	int bitShift;
};

static const KeyEffectPair* FindBit(const struct KeyEffectPair *pair, int key) {
	while (pair->bit) {
		if (pair->key == key) 
			return pair;
		pair++;
	}
	return 0;
}

static int KeyHoldBit(int key, int status, int modMask) {
	static const struct KeyEffectPair KeyEffectMap[]=
		{	
			{ GLUT_KEY_F5,   EFFECT_BOUNCE, 0},
			{ GLUT_KEY_F6,   EFFECT_SPRINKLER, 0 },
			{ GLUT_KEY_F7,   EFFECT_FOUNTAIN, 0 },
			{ GLUT_KEY_F8,   EFFECT_SNOW, 0},
			{ GLUT_KEY_F9,   EFFECT_FIREWORKS, 0 },
			{ GLUT_KEY_F10,  EFFECT_FIRE, 0 },
			{ GLUT_KEY_F1,   EFFECT_SWARM_1, 0 },
			{ GLUT_KEY_F2,   EFFECT_SWARM_2, 0 },
			{ GLUT_KEY_F3,   EFFECT_SWARM_3, 0 },
			{ GLUT_KEY_F4,   EFFECT_SWARM_4, 0 },

			{ GLUT_KEY_UP,   DIR_UP, DIR_TOP },
			{ GLUT_KEY_DOWN, DIR_DOWN, DIR_BOTTOM },
			{ GLUT_KEY_LEFT, TURN_LEFT, DIR_LEFT },
			{ GLUT_KEY_RIGHT,TURN_RIGHT, DIR_RIGHT },
			{ GLUT_KEY_PAGE_UP, DIR_TOP, 0 },
			{ GLUT_KEY_PAGE_DOWN, DIR_BOTTOM, 0 },
			{ GLUT_KEY_END,  DIR_RANDOM_2 }, 
			{ GLUT_KEY_INSERT, DIR_RANDOM_1 },
			{ GLUT_KEY_F11,  EFFECT_PERSIST_ADD, 0 },
			{ GLUT_KEY_F12,  EFFECT_PERSIST_SUB, 0 },
			{ 0, 0}
		};
	const struct KeyEffectPair * bit=FindBit(KeyEffectMap, key);
	if (bit) {
		if (status) EffectBits |= ((modMask&GLUT_ACTIVE_SHIFT)?bit->bitShift:bit->bit);
		else EffectBits &= ~(bit->bit | bit->bitShift);
		if (EffectBits & DIR_ALL) {
			camMode=CAM_INTERACTIVE;
			tgtMode=TGT_INTERACTIVE;
		}
	}
	return EffectBits;
}

static void FKeyUp(int key, int x, int y)  {
#ifndef PS2
	KeyHoldBit(key,0, glutGetModifiers());
#else
	KeyHoldBit(key,0, 0);
#endif
	if (EffectBits & DIR_ALL)
		camMode = CAM_INTERACTIVE;
}

static void FKeyDown(int key, int x, int y)  {
	interactiveMode = true;
#ifndef PS2
	KeyHoldBit(key,1, glutGetModifiers());
#else
	KeyHoldBit(key,1, 0);
#endif
}

static void Joy(unsigned int btn, int x, int y, int z) {
	int threshold = 10;
	int lbtn = btn & 1;
	if (x>threshold) EffectBits = (EffectBits & ~DIR_ALL) | (lbtn?DIR_TOP:DIR_UP);
	if (x<-threshold) EffectBits = (EffectBits & ~DIR_ALL) | (lbtn?DIR_BOTTOM:DIR_DOWN);
	if (y>threshold) EffectBits = (EffectBits & ~DIR_ALL) | (lbtn?DIR_LEFT:TURN_LEFT);
	if (y<-threshold) EffectBits = (EffectBits & ~DIR_ALL) | (lbtn?DIR_RIGHT:TURN_RIGHT); 
}
#define ETOGGLE(bit, message) EffectBits ^= bit; FireMessage(message,ONOFF(EffectBits & bit))
static void Key(unsigned char key, int x, int y)
{

	if (!strchr("hfzHFZ",key)) {
		interactiveMode = true;
		FireMessage("interactive mode, Press h for helP","");
	}

	switch (key) {
	case ' ': doLive = !doLive; break;
	case '0': ETOGGLE(EFFECT_FIRE,"fire effect %s"); break;
	case '1': ETOGGLE(EFFECT_SWARM_1, "swarm Predator 1 %s"); break;
	case '2': ETOGGLE(EFFECT_SWARM_2, "swarm Predator 2 %s"); break;
	case '3': ETOGGLE(EFFECT_SWARM_3, "swarm PreY %s"); break;
	case '4': ETOGGLE(EFFECT_SWARM_4, "swarm seeker %s"); break;
	case '5': ETOGGLE(EFFECT_BOUNCE, "bounce %s"); break;
	case '6': ETOGGLE(EFFECT_SPRINKLER, "sPrinkler %s"); break;
	case '7': ETOGGLE(EFFECT_FOUNTAIN, "fountain %s"); break;
	case '8': ETOGGLE(EFFECT_SNOW, "snow %s"); break;
	case '9': ETOGGLE(EFFECT_FIREWORKS, "fireworks %s"); break;
	case '=': glutIdleFunc(Auto); FireMessage("animation started",""); break;
	case '@': glutIdleFunc(NULL); FireMessage ("animation stoPPed",""); break;
	case 'E': ETOGGLE(DIR_RANDOM_2, "escaPe mode 2 %s"); break;
	case 'G': EffectBits = (EffectBits & ~DIR_ALL) | DIR_LEFT; break;
	case 'J': EffectBits = (EffectBits & ~DIR_ALL) | DIR_RIGHT; break;
	case 'N': EffectBits = (EffectBits & ~DIR_ALL) | DIR_BOTTOM; break;
	case 'Q': exit(1); 
	case 'U': EffectBits = (EffectBits & ~DIR_ALL) | DIR_TOP; break;
	case 'a': EffectBits = EFFECT_FIRE;
		FireMessage("camera reset","");
		camMode = CAM_INTERACTIVE; 
		tgtMode = TGT_INTERACTIVE;
		break;
	case 'C': camMode = (camMode +CAM_MODES-1)%CAM_MODES; FireMessage("camera: %s ",camName[camMode]); break;
	case 'c': camMode = (camMode +1)%CAM_MODES; FireMessage("camera: %s",camName[camMode]);break;
	case 'd': glutDisplayFunc(Draw); break;
	case 'e': ETOGGLE(DIR_RANDOM_1, "escape mode 1 %s"); break;
	case 'f': EffectBits ^= DRAW_FPS; break;
	case 'h': EffectBits ^= DRAW_HELP; break;
	case 'g': EffectBits = (EffectBits & ~DIR_ALL) | TURN_LEFT;  break;
	case 'i': interactiveMode = false; FireMessage("automatic mode started",""); break;
	case 'j': EffectBits = (EffectBits & ~DIR_ALL) | TURN_RIGHT;  break;
	case 'n': EffectBits = (EffectBits & ~DIR_ALL) | DIR_DOWN;  break;
	case 'p': EffectBits ^= EFFECT_PERSIST; break;
	case 'T': tgtMode = (tgtMode +TGT_MODES - 1) % TGT_MODES; FireMessage("tarGet: %s",tgtName[tgtMode]); break;
	case 't': tgtMode = (tgtMode + 1) % TGT_MODES; FireMessage("tarGet: %s",tgtName[tgtMode]); break;
	case 'u': EffectBits = (EffectBits & ~DIR_ALL) | DIR_UP;  break;
	case 'x': EffectBits = (EffectBits & ~EFFECT_RAND_MASK); FireMessage("all effects off","");break;
	case 'z': if (zoom<=32) {zoom *= 2; FireMessage("zoom +","");} break;
	case 'Z': if (zoom>1) {zoom /= 2; FireMessage("zoom -","");} break;
	}
	glutPostRedisplay();
}

static void KeyUp(unsigned char key, int x, int y)
{
	switch (key) {
	case 'u': EffectBits &=~DIR_UP;
		break;
	case 'n': EffectBits &=~DIR_DOWN;
		break;
	case 'h': EffectBits &=~TURN_RIGHT;
		break;
	case 'j': EffectBits &=~TURN_LEFT;
		break;
	case 'U': EffectBits &=~DIR_TOP;
		break;
	case 'N': EffectBits &=~DIR_BOTTOM;
		break;
	case 'J': EffectBits &=~DIR_RIGHT;
		break;
	case 'H': EffectBits &=~DIR_LEFT;
		break;
	}
	glutPostRedisplay();
}

#define SPRINKLER_TTL 100
#define SPRINKLER_ROT_PERIOD 2
#define SPRINKLER_OSC_PERIOD 0.5
#define SPRINKLER_ROT_RAY 5
#define SPRINKLER_OSC_EXTENT 4
#define SPRINKLER_VEL_BASE 5

#define FOUNTAIN_TTL 100

#define FIREWORKS_COUNT 20

#define FIREWORKS_TTL_MIN 50

#define BOUNCE_TTL 500
#define PERSIST_UNIT 0.01

#define RANDOM_1_TTL 40
#define RANDOM_2_TTL 1000

static void Effects() {
	Particle *p;
	int i;
	Particle ipar;
	float scalar, th, phi, cth, sth, cphi, sphi;
	
	if (EffectBits & EFFECT_SPRINKLER)
		if (p=engine.activateParticle()) {
			p->reset();
			p->behavior = &BStandard;
			p->pos=light->pos;
			p->ttl=SPRINKLER_TTL;
			p->vel.x=sin(timeSeq*SPRINKLER_ROT_PERIOD)*SPRINKLER_ROT_RAY*odd;
			p->vel.z=cos(timeSeq*SPRINKLER_ROT_PERIOD)*SPRINKLER_ROT_RAY*odd;
			p->vel.y=SPRINKLER_VEL_BASE+sin(timeSeq*SPRINKLER_OSC_PERIOD)*SPRINKLER_OSC_EXTENT;
			p->color.g+=rand()%100;
			p->color.b+=rand()%100;
			p->size=1;
		}
	
	if (EffectBits & EFFECT_FOUNTAIN)
		if (p=engine.activateParticle()) {
			p->reset();
			p->behavior =&BStandard;
			p->pos=light->pos;
			p->vel.x=rand(-2,2);
			p->vel.y=rand(5,10);
			p->vel.z=rand(-2,2);
			p->color.g+=rand()%100;
			p->color.b+=rand()%100;
			p->size=rand(0.5,1.0);
			p->ttl=FOUNTAIN_TTL;
		}

	if (EffectBits & EFFECT_SNOW)
		if (p=engine.activateParticle()) {
			p->behavior = &BStandard;
			p->pos.y=15;
			p->pos.x=rand(-20,20);
			p->pos.z=rand(-20,20);
			p->acc.z=0;
			p->acc.x=0.01;
			p->vel.x=0;
			p->vel.y=rand(-0.3, -0.5);
			p->vel.z=0;
			p->color.r=MAX_BYTE;
			p->color.g=MAX_BYTE;
			p->color.b=MAX_BYTE;
		}
	
	if (EffectBits & EFFECT_FIREWORKS) {
		ipar.reset();
		ipar.pos.x=rand(-20,20);
		ipar.pos.y=rand(-2,10);
		ipar.pos.z=rand(3,-2);
		ipar.vel.x=0;
		ipar.vel.y=rand(3,6);
		ipar.vel.z=0;
		ipar.color.g+=rand()%100;
		ipar.color.b+=rand()%100;
		ipar.ttl = FIREWORKS_TTL_MIN+rand()%20; 
		ipar.behavior = &BStandard;

		scalar = rand(1,1.5);
		ipar.size=0.3;
		phi   = rand(M_PI/10, M_PI/4);
	
		for  (i=1; i<=FIREWORKS_COUNT; i++) {
			if (p=engine.activateParticle()) {
				*p=ipar;
				
				th   = (2*M_PI/FIREWORKS_COUNT)*i + rand(-0.1,0.1);
				phi *= rand(0.9, 1.1);
				
				cth=cos(th);
				sth=sin(th);
				cphi=cos(phi);
				sphi=sin(phi);
				p->vel.x+=scalar * cphi * cth;
				p->vel.y+=scalar * sphi;
				p->vel.z+=scalar * cphi * sth;
			}
		}
	}

	if (EffectBits & EFFECT_BOUNCE)
		if (prob(20) && (p=engine.activateParticle())) {
			p->behavior = &BBounce;
			p->ttl=BOUNCE_TTL;
			th = rand(0,2*M_PI);
			phi = rand(4,8);
			p->vel.x=phi*cos(th);
			p->vel.y=10;
			p->vel.z=phi*sin(th);
			p->pos=generators[iGen%nGen]->pos;
			iGen++;
			p->color.g+=rand()%100;
			p->color.b+=rand()%100;
			p->size=1;
		}

	
	if  (EffectBits & EFFECT_SWARM_1)
		if (p=engine.activateParticle()) {
			p->behavior = &BMoth;
			p->chain = light;
			p->flags |= (FLAG_MOSQUITO | FLAG_WINGS | FLAG_COLLISION | FLAG_MATE | FLAG_PREDATOR);
			p->pos=generators[iGen%nGen]->pos;
			p->vel=generators[iGen%nGen]->vel;
			p->acc.z=0;
			p->acc.x=sin(timeSeq)*5;
			p->acc.y=cos(timeSeq)*4;
			p->vel.x+=rand(-2, 2);
			p->vel.z+=rand(-2, 2);
			p->vel.y+=rand(3,5);
			p->color.set(MAX_BYTE,50,50,MAX_BYTE);
			generators[iGen%nGen]->color=p->color;
			p->color2.set(MAX_BYTE,MAX_BYTE,50,MAX_BYTE);
			LBitMask bm(32,&p->color);
			bm.toggleBit(rand()%24);
			p->size=0.3;
			p->gt.shuffle();
			generators[iGen%nGen]->color2=p->color2;
			iGen++;
		}
	
	
	
	if  (EffectBits & EFFECT_SWARM_2)
		if (p=engine.activateParticle()) {
			p->acc=V3::vZero;
			p->flags |= (FLAG_MOSQUITO | FLAG_WINGS | FLAG_COLLISION | FLAG_MATE | FLAG_PREDATOR);
			p->pos=generators[iGen%nGen]->pos;
			p->vel=generators[iGen%nGen]->vel;
			p->vel.x+=rand(-1,1);
			p->vel.y+=rand(0,2);
			p->vel.z+=rand(-1,1);
			p->acc.z=0;
			p->size=rand(0.1, 0.3);
			p->chain = light;
			p->behavior = &BMoth;
			p->color.set(25,215,MAX_BYTE,MAX_BYTE);
			p->color2.set(215,MAX_BYTE,MAX_BYTE,MAX_BYTE);
			generators[iGen%nGen]->color=p->color;
			generators[iGen%nGen]->color2=p->color2;
			iGen++;
		}
	
	if  (EffectBits & EFFECT_SWARM_3)
		if (p=engine.activateParticle()) {
			p->behavior = &BSpringField;
			p->flags |= (FLAG_COLLISION | FLAG_TAIL | FLAG_MATE | FLAG_PREY);
			p->acc=V3::vZero;
			p->gt.shuffle();
			p->pos=generators[iGen%nGen]->pos;
			p->vel.set(rand(-1,1),rand(-1,1),rand(-1,1),1);
			p->vel.moveBy(generators[iGen%nGen]->vel);
			p->vel.normalize();
			p->acc.z=0;
			p->color.g+=rand()%100;
			p->color.b+=rand()%100;
			p->size=rand(0.2,0.5);
			p->chain = light;
			p->color2.set(215,MAX_BYTE,MAX_BYTE,MAX_BYTE);
			generators[iGen%nGen]->color=p->color;
			generators[iGen%nGen]->color2=p->color2;
			iGen++;
		}

	if  (EffectBits & EFFECT_SWARM_4 )
		if (p=engine.activateParticle()) {
			p->acc=V3::vZero;
			p->flags |= (FLAG_MOSQUITO | FLAG_WINGS | FLAG_COLLISION  | FLAG_MATE | FLAG_PREDATOR);
			p->pos=generators[iGen%nGen]->pos;
			p->vel=generators[iGen%nGen]->vel;
			p->vel.x+=rand(-1,1);
			p->vel.y+=rand( 0,2);
			p->vel.z+=rand(-1,1);
			p->acc.z=0;
			p->size=rand(0.05, 0.15);
			p->behavior = &BQueen;
			p->chain = queen;
			p->color.set(25,MAX_BYTE,25,MAX_BYTE);
			p->color2.set(215,MAX_BYTE,215,MAX_BYTE);
			generators[iGen%nGen]->color=p->color;
			generators[iGen%nGen]->color2=p->color2;
			iGen++;
		}
	
	
	if (EffectBits & EFFECT_PERSIST_ADD)
		accum -= PERSIST_UNIT;
	if (EffectBits & EFFECT_PERSIST_SUB)
		accum += PERSIST_UNIT;
	if (accum<0) accum = 0;
	if (accum>1) accum = 1;

	if (EffectBits & (DIR_RANDOM_1 | DIR_RANDOM_2))
		if (p=engine.activateParticle()) {
			p->behavior = &BStandard;
			p->pos=light->pos;
			p->vel=light->vel;
			p->vel.multBy(-0.2);
			p->acc.y=0;
			p->color.set(MAX_BYTE,50,50,MAX_BYTE);
			p->color2.set(MAX_BYTE,MAX_BYTE,50,MAX_BYTE);
			p->size=rand(0.5,1.5);
			if (EffectBits & DIR_RANDOM_2) {
				p->color.set(MAX_BYTE,150,50,MAX_BYTE);		
				p->ttl=RANDOM_2_TTL;
			}
			else
				p->ttl=RANDOM_1_TTL;
		}
	
	if (EffectBits & EFFECT_FIRE) {
		for (i=0; i<nGen;i++) {
			Particle *templ = generators[i];
			if (p=engine.activateParticle()) {
				p->behavior = &BStandard;
				p->pos=templ->pos;
				p->size=rand(0.5,1);
				p->pos.y-=-rand(0.0,0.1)-(p->size)*SOLID_RAY;
				p->vel.y=rand(0.1,0.8);
				p->pos.x-=rand(-0.1,0.1);
				p->pos.z-=rand(-0.1,0.1);
				p->ttl=40+rand()%20;
				p->acc.y=2;
				p->acc.x=sin(timeSeq)*.5;
				p->acc.z=cos(timeSeq)*.5;
				p->color=templ->color2;
				p->color.a=0;
				p->color2=templ->color;
				p->color2.a=100 + rand() % 70;
				p->flags |= FLAG_INNERFADE;
			}
		}
	}
}

static void Movement() {
	static M44 rotY;
	static M44 rotX;
	static float lnFriction = logf(0.92);
	if (EffectBits & (DIR_LEFT | DIR_RIGHT | 
			  DIR_TOP | DIR_BOTTOM | 
			  DIR_UP | DIR_DOWN |
			  TURN_LEFT | TURN_RIGHT)) {
		V3 cvel;
		if (EffectBits & (TURN_LEFT | TURN_RIGHT)) {
			if (EffectBits & TURN_LEFT)
				rotY.rotY(0.5*frametime);
			else
				rotY.rotY(0.5*-frametime);
			rotY.mult3(cvel, cam->vel);
			cam->vel = cvel;
		}
		else
			cvel = cam->vel;
		V3 cleft; 
		if (EffectBits & (DIR_LEFT | DIR_RIGHT))
			cam->vel.vec(cleft, V4::up);

		cam->acc.set(V4::iW);

		if (EffectBits & DIR_LEFT) cam->acc.moveBy(cleft, -1);
		else if (EffectBits & DIR_RIGHT) cam->acc.moveBy(cleft, 1);
		
		if (EffectBits & DIR_UP) cam->acc.moveBy(cvel, 1);
		else if (EffectBits & DIR_DOWN) cam->acc.moveBy(cvel, -1); 
		
		if (EffectBits & DIR_TOP) cam->acc.moveBy(V4::up, 1);
		else if (EffectBits & DIR_BOTTOM) cam->acc.moveBy(V4::up, -1);
		cam->pos.moveBy(cam->acc, frametime);
	}
	else 
		cam->acc.multBy(pow(0.92, timeunit));
	cam->pos.moveBy(cam->acc, frametime);
	const float camBound =10;
	if (cam->pos.x<-camBound) cam->pos.x=-camBound;
	else if (cam->pos.x>camBound) cam->pos.x=camBound;

	if (cam->pos.z<-camBound) cam->pos.z=-camBound;
	else if (cam->pos.z>camBound) cam->pos.z=camBound;

	if (cam->pos.y<PLANE_FLOOR) cam->pos.y=PLANE_FLOOR;
	else if (cam->pos.y>camBound) cam->pos.y=camBound;

	if (EffectBits & (DIR_RANDOM_1 | DIR_RANDOM_2)) {
		light->vel.y+=rand(-1,1);
		light->vel.x+=rand(-1,1);
		light->vel.z+=rand(-1,1);
	}
}

class Interpolator {
public:
	virtual void pos(FLOAT t, V3 &d) = 0;
	virtual void vel(FLOAT t, V3 &d) = 0;
};

class Cubic: public Interpolator {
protected:
	V4 cx, cy, cz;
public:
	virtual void pos(FLOAT t, V3 &d) {
		register FLOAT t2=t*t;
		register FLOAT t3=t2*t;
		V4 T(t3,t2,t,1);
		d.x=T.dot(cx);
		d.y=T.dot(cy);
		d.z=T.dot(cz);
	}
	virtual void vel(FLOAT t, V3 &d) {
		register FLOAT _2t=2*t;
		register FLOAT _3t2=3*t*t;
		d.x=cx.x*_3t2+cx.y*_2t+cx.z;
		d.y=cy.x*_3t2+cy.y*_2t+cy.z;
		d.z=cz.x*_3t2+cz.y*_2t+cz.z;
	}
};

class HermiteCubic: public Cubic {
public:
	void setCoef(V3 &P1, V3 &P4, V3 &R1, V3 &R4) {
		cx.x = 2 * P1.x -2 * P4.x + R1.x + R4.x;
		cy.x = 2 * P1.y -2 * P4.y + R1.y + R4.y;
		cz.x = 2 * P1.z -2 * P4.z + R1.z + R4.z;
    
		cx.y =-3 * P1.x +3 * P4.x -2 * R1.x - R4.x;
		cy.y =-3 * P1.y +3 * P4.y -2*  R1.y - R4.y;
		cz.y =-3 * P1.z +3 * P4.z -2*  R1.z - R4.z;
    
		cx.z = R1.x;
		cy.z = R1.y;
		cz.z = R1.z;
    
		cx.w = P1.x;
		cy.w = P1.y;
		cz.w = P1.z;
	}
};

static void AutoScene() {
	static HermiteCubic slidingCubic;

	int msec = msecTime();
	if (randSeq < msec) {
		randDelta = 1500 + rand()%4000;
		randSeq = msec + randDelta;

		if (!interactiveMode) {
			unsigned int changeBits=0;
			LBitMask bm(32,&changeBits);
			if (prob(50)) bm.setBit(rand()%16);
			if (prob(50)) bm.setBit(rand()%16);
			EffectBits = (EffectBits & (~EFFECT_RAND_MASK)) | (changeBits & EFFECT_RAND_MASK);
			if (prob(50)) camMode = camRandModes[rand()%sizeof(camRandModes)/sizeof(camRandModes[0])];
			if (prob(50)) tgtMode = tgtRandModes[rand()%sizeof(tgtRandModes)/sizeof(tgtRandModes[0])];
			if (prob(50) && zoom>1) zoom /=2;
			if (prob(50) && zoom<4) zoom *=2;
		}
		V3 P4, R4;
		P4.x = rand(-8,8);
		P4.y = rand(-2,8);
		P4.z = rand(-8,8);
		R4.x = rand(-8,8);
		R4.y = rand(-8,8);
		R4.z = rand(-8,8);
		slidingCubic.setCoef(slidingCam->pos,P4,slidingCam->vel,R4);
		if (prob(10)) slidingCam->flags ^=FLAG_CAMSMOOTH;
		if (prob(30)) slidingCam->flags |=FLAG_CAMSMOOTH;
	}
	const int minCamTTL = 400;
	if (randCamera < msec || (targets[TGT_RANDOM]->flags & FLAG_IDLE)) {
		randCamera = msec + 1500+rand()%1000;
		if ((targets[TGT_RANDOM]->flags & FLAG_IDLE) || prob(20)) 
			targets[TGT_RANDOM] = engine.getParticle(rand()%engine.getNumParticles());
		if (prob(10)) cameras[CAM_RANDOM] = engine.getParticle(rand()%engine.getNumParticles());
		if (!(cameras[CAM_RANDOM]->flags  & (FLAG_MOSQUITO | FLAG_TAIL)))
			cameras[CAM_RANDOM]=slidingCam;
	}
	float u = 1-((float)randSeq-msec)/(float)randDelta;
	slidingCubic.pos(u,slidingCam->pos);
	slidingCubic.vel(u,slidingCam->vel);
}

static void Auto()
{
	AutoScene();
#ifndef PS2
	glutForceJoystickFunc();
#endif	
	glutPostRedisplay();
}

void  TextOut(GLfloat x, GLfloat y, GLfloat z, GLfloat sc, const char *string) {
	glPushMatrix();
	glTranslatef(x, y, z);
	glScalef(sc, sc, sc);
	while (*string)
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *string++);
	glPopMatrix();
}

static void DrawMessage() {
	if (messageTTL>0 && message) {
		glColor4ub(MAX_BYTE,MAX_BYTE,MAX_BYTE,MIN(255,(int)messageTTL*3));
		TextOut(-glutStrokeLength(GLUT_STROKE_ROMAN,(const unsigned char*)message)*msgFontSize,-0.8,-1, msgFontSize*2, message);
		messageTTL-=timeunit;
	}
}

static void DrawHelp() {
	float y=0.8;
	float x0=-0.8;
	glColor4ub((GLubyte)(20+20*cos(timeSeq)),0,128,127);
	glRectf(-0.9,0.9,0.9,-0.9);
	glColor4ub(MAX_BYTE-10,MAX_BYTE-10,MAX_BYTE,250);
	for (const char ** msg = helpmsg; **msg; msg++,y-=0.08)
		TextOut(x0,y,-1,msgFontSize,*msg);
}

static void DrawTextStats(Particle *ccam, Particle *ctarget) {
	char buf[1024];
	glColor4ub(0,0,0,127);
	glRectf(-1  ,1, 1, 0.65);
	glRectf(-1  , -0.65  ,1,-1);
	glColor4ub(127,MAX_BYTE,127,150);
	sprintf(buf,"cam[%s] %.2f,%.2f,%.2f -> %.2f,%.2f,%.2f",camName[camMode],ccam->pos.x, ccam->pos.y, ccam->pos.z,
		ccam->vel.x, ccam->vel.y, ccam->vel.z);
	TextOut(-0.9,0.9,-1, msgFontSize, buf);
	sprintf(buf,"tarGet[%s] %.2f,%.2f,%.2f zoom %.0fx",tgtName[tgtMode],ctarget->pos.x, ctarget->pos.y, ctarget->pos.z, zoom);
	TextOut(-0.9,0.8,-1, msgFontSize, buf);
	sprintf(buf,"Particles %d frame %d fPs %.2f ftime %.2fms",engine.getNumParticles(), nFrames, fps, frametime*1000);
	TextOut(-0.9,0.7,-1, msgFontSize, buf);
	glColor4ub(MAX_BYTE,MAX_BYTE,(GLbyte)(127+127*sin(timeSeq)),150);
	TextOut(-0.9,-0.9,-1, msgFontSize, copyright);
}

static void Timings() {
	static int frameptr = 0;
	static int framems[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int msecNow=msecTime();
	int msecs=msecNow-framems[(nFrames+1) & 0x0f];
	framems[nFrames&0x0f]=msecNow;
	if (msecs) fps = (15.0)/(msecs*0.001);
	nFrames++;
	frametime = (msecs*0.001)/15.0;
	if (frametime > 0.1) frametime = 0.1;
	timeSeq += frametime;
	timeunit = frametime*framebase;
	odd = -odd;
}

static void PaintScene(const Particle * camera, const Particle * lookAt, bool isReflect) {
	float dist, size, sc;
	int j,k,oddtl;
	V3 * pos;
	const V3* tpos;
	V4 vel, lp;
	M44 ptrans,t;
	const Particle **p;
	register const Particle *P;

	/*************************************************************
	 * Solid objects
	 ************************************************************/
	glPushMatrix(); // 1
	float yc=camera->pos.y;
	if (yc<PLANE_FLOOR+EPS_CAM)
		yc=PLANE_FLOOR+EPS_CAM;
	if (!lookAt || lookAt==camera)
		gluLookAt(camera->pos.x,
			  yc,
			  camera->pos.z,
			  camera->pos.x+camera->vel.x,
			  camera->pos.y+camera->vel.y,
			  camera->pos.z+camera->vel.z,
			  0, 1 ,0);
	else
		gluLookAt(camera->pos.x,
			  yc,
			  camera->pos.z,
			  lookAt->pos.x,
			  lookAt->pos.y,
			  lookAt->pos.z,
			  0, 1 ,0);

	glDepthMask(GL_TRUE);
	if (isReflect) {
		glTranslatef(0,2*PLANE_FLOOR,0);
		glScalef(1,-1,1);
	}
	else {
		glDisable(GL_CLIP_PLANE0);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glBegin(GL_TRIANGLE_FAN);
		glColor4ub(55,10,55,200);
		glTexCoord2f(0,0); glVertex3f(-100,PLANE_FLOOR-EPS, 100);
		glTexCoord2f(0,1); glVertex3f(-100,PLANE_FLOOR-EPS ,-100);
		glTexCoord2f(1,1); glVertex3f( 100,PLANE_FLOOR-EPS ,-100);
		glTexCoord2f(1,0); glVertex3f( 100,PLANE_FLOOR-EPS , 100);
		glEnd();

		glBegin(GL_TRIANGLE_FAN);
		
		glColor4ubv((const GLubyte*)&cam->color);
		glTexCoord2f(0,0); glVertex3f(-100, 24, 100);
		glTexCoord2f(1,0); glVertex3f( 100, 24, 100);
		glTexCoord2f(1,1); glVertex3f( 100, 24,-100);
		glTexCoord2f(0,1); glVertex3f(-100, 24,-100);
		glEnd();
		glEnable(GL_DEPTH_TEST);
	}

	GLdouble clip1[]={0,-1/PLANE_FLOOR, 0, 1.0+EPS};
	glClipPlane(GL_CLIP_PLANE0, clip1);
	glEnable(GL_CLIP_PLANE0);

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);

	lp = light->pos;
	glLightfv(GL_LIGHT0,GL_POSITION,lp.GLf());
	V3 dipos=camera->pos;
	if (isReflect) dipos.y=-PLANE_FLOOR-dipos.y;
	engine.sortByDist(dipos);
	engine.scaleDistance(zoom);
	ptrans.identity();
	for (p=engine.getParticles();*p;p++) {
		P=*p;
		if (P!=camera && (P->flags & (FLAG_MOSQUITO | FLAG_TAIL)) && 
		    (dist=P->distFromCamera)<distWings) {
			oddtl= !(P->flags & FLAG_EVEN);
			P->getM44(ptrans);
			ptrans.transpose();
			glPushMatrix();
			glMultMatrixf(ptrans.GLf());
			sc=P->size * 0.33; // Overall size
			glScalef(sc,sc,sc);
			if (P->flags & FLAG_MOSQUITO) {
				glColor3ubv((GLubyte*)&P->color2);
				// Corpo
				if (dist<distBody) glCallList(LIST_BODY);
				
				// Ali
				if (P->flags & FLAG_WINGS) {
					glColor4ubv((GLubyte*)&P->color);
					glCallList(LIST_WING1+oddtl);
				}
			}
			if (P->flags & FLAG_TAIL) {
				glColor3ubv((GLubyte*)&P->color2);
				if (dist<distBody) glCallList(LIST_TAIL);
			}
			glPopMatrix();
		}
		
	}

	/*************************************************************
	 * Shadowlike reflection
	 ************************************************************/
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glDepthMask(GL_FALSE);	

	for (p=engine.getParticles();*p;p++) {
		P=*p;
		if (!isReflect && P->pos.y<PLANE_FLOOR+P->size & P->pos.y>PLANE_FLOOR-P->size) {
			glPushMatrix();
			glTranslatef(P->pos.x, PLANE_FLOOR+EPS, P->pos.z);
			float sf=1-fabs(P->pos.y-PLANE_FLOOR)/P->size;
			sc=P->size*sf*2;
			glScalef(sc,sc,sc);
			glColor4ub(P->color.r,
				   P->color.g,
				   P->color.b,
				   (unsigned char)(P->color.a*sf));
			glCallList(LIST_PARTICLESHADE);
			glPopMatrix();
		}
	}
	// Save camera transform before deletion
	glGetFloatv(GL_MODELVIEW_MATRIX,t.GLf());
	t.transpose();
	glPopMatrix(); // 1

	/*************************************************************
	 * Particles and effects
	 ************************************************************/

	glPushMatrix(); // 2	
	engine.transform(t);
	for (p=engine.getParticles();*p;p++) {
		P=*p;
		if (P!=camera && (dist=P->distFromCamera) < distNothing) {
			tpos = &P->tpos;
			if (dist > distPoints) {		
				glColor4ubv((GLubyte*)&P->color);
				glTexCoord2f(0.25,0.25);
				glBegin(GL_POINTS);
				glVertex3fv(tpos->GLf());
				glEnd();
			}
			else {
				glPushMatrix(); // 3
				glTranslatef(tpos->x, tpos->y, tpos->z);
				sc=P->size;
				glScalef(sc,sc,sc);
				glColor4ubv((GLubyte*)&P->color);
				glCallList(LIST_PARTICLE);
				if (!(P->flags & FLAG_INNERFADE) && dist < distInner) {
					glScalef(0.5, 0.5, 0.5);
					glColor4ubv((GLubyte*)&P->color2);
					glCallList(LIST_PARTICLE);
					
				}
				glPopMatrix(); // 3
			}			
		}
	}
	glPopMatrix(); // 2
}

static void Draw(void) {
	Timings();
	Movement();
	Effects();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glDisable(GL_TEXTURE_2D);
	if (accum>0) {
		glColor4f( 0, 0, 0, 1-accum);
		glRectf(-1,1,1,-1);
		glDepthMask(GL_TRUE);
		glClear(GL_DEPTH_BUFFER_BIT);
	}
	else {
		glDepthMask(GL_TRUE);
		glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
	}

	Particle *ccam,*ctarget;
	int msec = msecTime();
	ccam = cameras[camMode];
	ctarget = targets[tgtMode];

	float fov = 90.0/zoom;
	float fcp = 0.5;
#ifndef PS2
	gluPerspective(fov, (GLfloat)Width/(GLfloat)Height, fcp, 1000);
#else
	gluPerspective(fov, 4.0/3.0, fcp, 1000);
#endif
	glMatrixMode(GL_MODELVIEW);
	PaintScene(ccam,ctarget,1);
	PaintScene(ccam,ctarget,0);
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glTexCoord2f(0.5,0.5);
	glDisable(GL_CLIP_PLANE0);

	if (EffectBits & DRAW_FPS)
		DrawTextStats(ccam, ctarget);
	if (EffectBits & DRAW_HELP)
		DrawHelp();
	DrawMessage();
	glFlush();
	if (doLive) {
		engine.run();
		engine.cleanup();
		detector.detect(&engine);
	}
	glFinish();
	glutSwapBuffers();
}

void Init()
{
	GLfloat rgbat[] = {1,1,1,1};
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	ParticleImage P(64);
//	glBindTexture(GL_TEXTURE_2D,0);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_LUMINANCE8_ALPHA8, P.w, P.h, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, P.la);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, rgbat);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_LIGHT0);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glShadeModel(GL_FLAT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,1);
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.07);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.10);

	GLfloat fspec[4]={1,1,1,1};
	glMaterialfv(GL_FRONT, GL_SPECULAR,fspec);
	glMateriali(GL_FRONT, GL_SHININESS, 20);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glPolygonMode(GL_FRONT, GL_FILL);
	glPolygonMode(GL_BACK, GL_FILL);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearAccum(0.0, 0.0, 0.0, 0.0);
	glClear(GL_ACCUM_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	glNewList(LIST_PARTICLE,GL_COMPILE);
	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(0, 0); glVertex3f(-0.5, -0.5, 0);
	glTexCoord2f(1, 0); glVertex3f(-0.5, 0.5, 0);
	glTexCoord2f(1, 1); glVertex3f( 0.5, 0.5, 0);
	glTexCoord2f(0, 1); glVertex3f( 0.5, -0.5, 0);
	glEnd();
	glEndList();

	glNewList(LIST_PARTICLESHADE,GL_COMPILE);
	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(0,0); glVertex3f(-0.5, 0 ,0.5);
	glTexCoord2f(1,0); glVertex3f(-0.5, 0 ,-0.5);
	glTexCoord2f(1,1); glVertex3f( 0.5, 0 ,-0.5);
	glTexCoord2f(0,1); glVertex3f( 0.5, 0 ,0.5);
	glEnd();
	glEndList();

	V3 vc[26];
	V3 n;
	vc[ 0].set(   0, 0.0, 0.0);
	vc[ 1].set( 0.1, 0.0, 1.0);
	vc[ 2].set(-0.1, 0.0, 1.0);
	vc[ 3].set(   0,-0.1, 1.2);

	vc[ 4].set(-  2, 0.4, 1.3);
	vc[ 5].set(-1.4, 0.4, 1.3);
	vc[ 6].set(   2, 0.4, 1.3);
	vc[ 7].set( 1.4, 0.4, 1.3);

	vc[ 8].set(-1.4,-0.2, 0.7);
	vc[ 9].set(-1.0,-0.2, 0.7);
	vc[10].set( 1.4,-0.2, 0.7);
	vc[11].set( 1.0,-0.2, 0.7);

	vc[12].set(-1.4,-0.4, 1.3);
	vc[13].set(-  2,-0.4, 1.3);
	vc[14].set( 1.4,-0.4, 1.3);
	vc[15].set(   2,-0.4, 1.3);

	vc[16].set(-1.0, 0.2, 0.7);
	vc[17].set(-1.4, 0.2, 0.7);
	vc[18].set( 1.0, 0.2, 0.7);
	vc[19].set( 1.4, 0.2, 0.7);

	vc[20].set( 0.1,-0.2, 1.3);
	vc[21].set(-0.1,-0.2, 1.3);
	vc[22].set( 0.0,-0.3, 1.15);

	vc[23].set( 0.0,   0.2, -1.0);
	vc[24].set( 0.176,-0.1, -1.0);
	vc[25].set(-0.176,-0.1, -1.0);

	glVertexPointer(3,GL_FLOAT,0,vc);
	glEnableClientState(GL_VERTEX_ARRAY);
	// Corpo solido
	glNewList(LIST_BODY, GL_COMPILE);
 	glBegin(GL_TRIANGLE_FAN);
	glArrayElement(0);
	glArrayElement(1);
	glNormal3fv(vc[2].normal(n,vc[1],vc[0]).GLf());
	glArrayElement(2);
	glNormal3fv(vc[3].normal(n,vc[2],vc[0]).GLf());
	glArrayElement(3);
	glNormal3fv(vc[1].normal(n,vc[3],vc[0]).GLf());
	glArrayElement(1);
	glEnd();
	
	glBegin(GL_TRIANGLES);
  	glArrayElement(1); 
	glArrayElement(3);
	glNormal3fv(vc[2].normal(n,vc[3],vc[1]).GLf());
	glArrayElement(2);
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
	glArrayElement(3);
	glArrayElement(20);
	glNormal3fv(vc[21].normal(n,vc[20],vc[3]).GLf());
	glArrayElement(21);
	glNormal3fv(vc[22].normal(n,vc[21],vc[3]).GLf());
	glArrayElement(22);
	glNormal3fv(vc[20].normal(n,vc[22],vc[3]).GLf());
	glArrayElement(20);
	glEnd();

	glBegin(GL_TRIANGLES);
	glArrayElement(20);
	glArrayElement(22);
	glNormal3fv(vc[21].normal(n,vc[22],vc[20]).GLf());
	glArrayElement(21);
	glEnd();

	glEndList();

	// Ali
	for (int k=0; k<2; k++) {
		int ik = k * 8 + 4;
		glNewList(LIST_WING1+k, GL_COMPILE);
		glBegin(GL_TRIANGLE_FAN);
		// Sinistra grande
		glArrayElement(2);
		glArrayElement(ik+0);
		glNormal3fv(vc[ik+1].normal(n,vc[ik+0],vc[2]).GLf());
		glArrayElement(ik+1);
		glNormal3fv(vc[ik+0].normal(n,vc[ik+1],vc[2]).GLf());
		glArrayElement(ik+0);
		glEnd();

		glBegin(GL_TRIANGLE_FAN);
		// Destra grande
		glArrayElement(1);
		glArrayElement(ik+3);
		glNormal3fv(vc[ik+2].normal(n,vc[ik+3],vc[1]).GLf());
		glArrayElement(ik+2);
		glNormal3fv(vc[ik+3].normal(n,vc[ik+2],vc[1]).GLf());
		glArrayElement(ik+3);

		glEnd();

		// Sinistra piccola
		glBegin(GL_TRIANGLE_FAN);		
		glArrayElement(2);
		glArrayElement(ik+5);
		glNormal3fv(vc[ik+4].normal(n,vc[ik+5],vc[2]).GLf());
		glArrayElement(ik+4);
		glNormal3fv(vc[ik+5].normal(n,vc[ik+4],vc[2]).GLf());
		glArrayElement(ik+5);
		glEnd();

		// Destra piccola
		glBegin(GL_TRIANGLE_FAN);
		glArrayElement(1);
		glArrayElement(ik+6);
		glNormal3fv(vc[ik+7].normal(n,vc[ik+6],vc[1]).GLf());
		glArrayElement(ik+7);
		glNormal3fv(vc[ik+6].normal(n,vc[ik+7],vc[1]).GLf());
		glArrayElement(ik+6);
		glEnd();
	
		glEndList();
	}

	// Codina
	glNewList(LIST_TAIL, GL_COMPILE);
 	glBegin(GL_TRIANGLE_FAN);
	glArrayElement(0);
	glArrayElement(23);
	glNormal3fv(vc[24].normal(n,vc[23],vc[0]).GLf());
	glArrayElement(24);
	glNormal3fv(vc[25].normal(n,vc[0],vc[24]).GLf());
	glArrayElement(25);
	glNormal3fv(vc[23].normal(n,vc[25],vc[0]).GLf());
	glArrayElement(23);
	glEnd();
	
	glBegin(GL_TRIANGLES);
  	glArrayElement(25); 
	glArrayElement(24);
	glNormal3fv(vc[23].normal(n,vc[24],vc[25]).GLf());
	glArrayElement(23);
	glEnd();

	glEndList();

	struct timeb tb;
	ftime(&tb);
	srand(tb.millitm);

	cam = new Particle();
	cam->reset();
	cam->color.set(10,20,155,220);
	cam->pos.set(0, -1,  7);
	cam->vel.set(0, 0, -1, 1);
	cam->acc.set(0, 0,  0, 1);

	slidingCam = new Particle();
	slidingCam->reset();
	slidingCam->pos.set(5,5,5);

	center = new Particle();
	center->reset();
	center->pos.set(0,0,0);
	center->vel.set(0,0,-1,1);
	center->acc.set(0,0,0,1);
                                                                    
	birdfly = new Particle();
	birdfly->reset();
	birdfly->pos.set(0,7, EPS);
	birdfly->vel.set(EPS,-1+EPS, EPS, 1);
	birdfly->acc.set(0, 0, 0, 1);

	if (light = engine.activateParticle()) {
		light->reset();
		light->behavior = &BSpringMass;
		light->size=2;
		light->color.set(MAX_BYTE-20,MAX_BYTE-10,MAX_BYTE,MAX_BYTE);	       
		light->acc=V3::vZero;
		light->vel.set(rand(-.1,.1),rand(-1,1),rand(-.1,.1), 1.0);
		light->flags |= FLAG_PERSISTENT | FLAG_COLLISION;
	}
	if (queen = engine.activateParticle()) {
		queen->reset();
		queen->chain=light;
		queen->size=0.5;
		queen->acc=V3::vZero;
		queen->vel.set(0,0,-0.5,1);
		queen->pos.set(-1,1,2);
		queen->color.set(MAX_BYTE,MAX_BYTE,100,MAX_BYTE);
		queen->color2.set(MAX_BYTE,MAX_BYTE,180,MAX_BYTE);
		queen->behavior=&BMoth;
		queen->flags |= (FLAG_PERSISTENT | FLAG_MOSQUITO | FLAG_WINGS | FLAG_COLLISION | FLAG_MATE | FLAG_PREDATOR);
	}
	for (int i=0; i<nGen; i++) {
		if (Particle *p = engine.activateParticle()) {
			p->reset();
			p->flags |= (FLAG_PERSISTENT | FLAG_COLLISION);
			p->ttl=100;
			p->color.set(MAX_BYTE,215,MAX_BYTE,MAX_BYTE);
			p->color2.set(MAX_BYTE,MAX_BYTE,MAX_BYTE,MAX_BYTE);
			p->pos.set(GENERATOR_DIST*sin(2*M_PI/nGen *i),PLANE_FLOOR+GENERATOR_HEIGHT,GENERATOR_DIST*cos(2*M_PI/nGen *i));
			p->vel.set(-sin(2*M_PI/nGen *i)*GENERATOR_VEL,1,-cos(2*M_PI/nGen *i)*GENERATOR_VEL,1);
			p->acc.set(V4::iW);
			p->size=GENERATOR_SIZE;
			p->behavior=&BGen;
			generators[i]=p;
		}
	}


	cameras[CAM_INTERACTIVE]=cam;
	cameras[CAM_LIGHT]=light;
	cameras[CAM_QUEEN]=queen;
	cameras[CAM_RANDOM]=cam;
	cameras[CAM_SLIDER]=slidingCam;
	cameras[CAM_BIRDFLY]=birdfly;
	cameras[CAM_CENTER]=center;
	cameras[CAM_G0]=generators[0];
	cameras[CAM_G1]=generators[1];
	cameras[CAM_G2]=generators[2];
	cameras[CAM_G3]=generators[3];
	cameras[CAM_G4]=generators[4];
	cameras[CAM_G5]=generators[5];

	targets[TGT_INTERACTIVE]=cam;
	targets[TGT_RANDOM]=center;
	targets[TGT_QUEEN]=queen;
	targets[TGT_LIGHT]=light;
	targets[TGT_SLIDER]=slidingCam;
	targets[TGT_CENTER]=center;
	targets[TGT_G0]=generators[0];
	targets[TGT_G1]=generators[1];
	targets[TGT_G2]=generators[2];
	targets[TGT_G3]=generators[3];
	targets[TGT_G4]=generators[4];
	targets[TGT_G5]=generators[5];

	FireMessage("ontheflY by nicola orru'","");
}

static void Reshape(int width, int height) {
	glViewport(0, 0, (GLint)width, (GLint)height);
	Width=width;
	Height=height;
	float H2=(Height*Height*0.25);
	distBody= H2/9.0;
	distWings= H2/7.0;
	distInner= H2/4.0;
	distPoints= H2/3.0;
	distNothing = H2/0.1;
}

int main(int argc, char **argv)
{
	GLenum type;
	bool fullscreen = false;
	glutInit(&argc, argv);
	for (int i=0; i<argc; i++) {
		if (!strcmp(argv[i],"-fullscreen"))
			fullscreen = true;
		else if (!strcmp(argv[i],"-interactive"))
			interactiveMode = true;
		else if (!strcmp(argv[i],"-help"))
			EffectBits |= DRAW_HELP;
	}
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	if (fullscreen) {
		glutGameModeString("800x600:24");
		glutEnterGameMode();
	}
	else {
	glutInitWindowPosition(0,0);
	glutInitWindowSize( 640, 480);
	if (glutCreateWindow("on the flY") == GL_FALSE)
		exit(1);
	}

	Init();
#ifndef PS2
	glutIgnoreKeyRepeat(1);
	glutSpecialUpFunc(FKeyUp);
#ifdef JOYSTICK
	glutJoystickFunc(Joy,-1);
#endif
#endif
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Key);
	glutSpecialFunc(FKeyDown);
	glutDisplayFunc(Draw);
	glutIdleFunc(Auto);
	glutMainLoop();
	if (fullscreen)
		glutLeaveGameMode();
	return 0;
}
