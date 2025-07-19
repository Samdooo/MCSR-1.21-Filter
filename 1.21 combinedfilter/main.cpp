#include <stdio.h>
#include <omp.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include <iterator>
#include <climits>

#include "cubiomes.h"
#include "bastion.h"
#include "ruined_portal.h"
#include "stronghold_generator/StrongholdGenerator.h"
#include "MiLTSU.h"

#define XRSR_MIX1 0xbf58476d1ce4e5b9ULL
#define XRSR_MIX2 0x94d049bb133111ebULL
#define XRSR_MIX1_INVERSE 0x96de1b173f119089ULL
#define XRSR_MIX2_INVERSE 0x319642b2d24d8ec3ULL
#define XRSR_SILVER_RATIO 0x6a09e667f3bcc909ULL
#define XRSR_GOLDEN_RATIO 0x9e3779b97f4a7c15ULL

typedef struct xrsr128 XRSR128;
typedef struct xrsrmat XRSRMAT;

struct xrsr128 {
    uint64_t hi;
    uint64_t lo;
};

struct xrsrmat {
    XRSR128 elem[128];
};

static XRSRMAT skips[128];

 static uint64_t mix64(uint64_t u64)
{
    u64 = (u64 ^ u64 >> 30) * XRSR_MIX1;
    u64 = (u64 ^ u64 >> 27) * XRSR_MIX2;
    return u64 ^ u64 >> 31;
}

 static uint64_t fix64(uint64_t u64)
{
    u64 = (u64 ^ u64 >> 31 ^ u64 >> 62) * XRSR_MIX2_INVERSE;
    u64 = (u64 ^ u64 >> 27 ^ u64 >> 54) * XRSR_MIX1_INVERSE;
    return u64 ^ u64 >> 30 ^ u64 >> 60;
}

 static uint64_t rol64(uint64_t u64, int n)
{
    return u64 << n | u64 >> (64 - n);
}

 void xrsr128_init(XRSR128 *rng, uint64_t lo, uint64_t hi)
{
    rng->lo = lo;
    rng->hi = hi;
}

 void xrsr128_next(XRSR128 *rng)
{
    rng->hi = rng->hi ^ rng->lo;
    rng->lo = rol64(rng->lo, 49) ^ rng->hi ^ (rng->hi << 21);
    rng->hi = rol64(rng->hi, 28);
}

 void xrsr128_prev(XRSR128 *rng)
{
    rng->hi = rol64(rng->hi, 36);
    rng->lo = rol64(rng->lo ^ rng->hi ^ (rng->hi << 21), 15);
    rng->hi = rng->hi ^ rng->lo;
}

 void xrsr128_comb(XRSR128 *rng, XRSRMAT *other)
{
    XRSR128 tmp = *rng;
    xrsr128_init(rng, 0, 0);
    for (int i = 0; i < 64; i++) {
        if (tmp.lo & (uint64_t) 1 << i) {
            rng->hi ^= other->elem[i].hi;
            rng->lo ^= other->elem[i].lo;
        }
    }
    for (int j = 0; j < 64; j++) {
        if (tmp.hi & (uint64_t) 1 << j) {
            rng->hi ^= other->elem[j + 64].hi;
            rng->lo ^= other->elem[j + 64].lo;
        }
    }
}

void xrsr128_skip(XRSR128 *rng, XRSR128 *other)
{
    for (int i = 0; i < 64; i++)
        if (other->lo & (uint64_t) 1 << i)
            xrsr128_comb(rng, &skips[i]);
    for (int j = 0; j < 64; j++)
        if (other->hi & (uint64_t) 1 << j)
            xrsr128_comb(rng, &skips[j + 64]);
}

 void xrsrmat_init(XRSRMAT *mat)
{
    for (int i = 0; i < 64; i++)
        xrsr128_init(&mat->elem[i], (uint64_t) 1 << i, 0);
    for (int j = 0; j < 64; j++)
        xrsr128_init(&mat->elem[j + 64], 0, (uint64_t) 1 << j);
}

 void xrsrmat_next(XRSRMAT *mat)
{
    for (int i = 0; i < 128; i++)
        xrsr128_next(&mat->elem[i]);
}

 void xrsrmat_prev(XRSRMAT *mat)
{
    for (int i = 0; i < 128; i++)
        xrsr128_prev(&mat->elem[i]);
}

 void xrsrmat_comb(XRSRMAT *mat, XRSRMAT *other)
{
    for (int i = 0; i < 128; i++)
        xrsr128_comb(&mat->elem[i], other);
}

void xrsrmat_skip(XRSRMAT *mat, XRSR128 *other)
{
    for (int i = 0; i < 128; i++)
        xrsr128_skip(&mat->elem[i], other);
}

void xrsr_init(void)
{
    xrsrmat_init(&skips[0]);
    xrsrmat_next(&skips[0]);
    for (int i = 1; i < 128; i++) {
        skips[i] = skips[i - 1];
        xrsrmat_comb(&skips[i], &skips[i - 1]);
    }
}

 void xrsr_seed(XRSR128 *rng, uint64_t seed)
{
    seed ^= XRSR_SILVER_RATIO;
    rng->lo = mix64(seed);
    rng->hi = mix64(seed + XRSR_GOLDEN_RATIO);
}

 uint64_t xrsr_long(XRSR128 *rng)
{
    uint64_t res = rol64(rng->lo + rng->hi, 17) + rng->lo;
    xrsr128_next(rng);
    return res;
}

 uint64_t xrsr_lo2s(uint64_t lo)
{
    return XRSR_SILVER_RATIO ^ fix64(lo);
}

 uint64_t xrsr_hi2s(uint64_t hi)
{
    return XRSR_SILVER_RATIO ^ (fix64(hi) - XRSR_GOLDEN_RATIO);
}

#define XOR(reg, idx, him, lom) \
    if (reg & (uint64_t) 1 << idx) { \
        rng->hi ^= him; \
        rng->lo ^= lom; \
    }

void skip(XRSR128 *rng) {
    XRSR128 tmp = *rng;
    xrsr128_init(rng, 0, 0);
    XOR(tmp.lo, 0, 0xbea1cb0e235828b3ULL, 0x9729b1f34956cf87ULL)
    XOR(tmp.lo, 1, 0x30d9b3a637b8cc30ULL, 0x69438152e6e4137bULL)
    XOR(tmp.lo, 2, 0xef9ba2a91df9826aULL, 0x54d7aa11544d7f0dULL)
    XOR(tmp.lo, 3, 0x6b77a2a23f77b567ULL, 0x184dd2690f183868ULL)
    XOR(tmp.lo, 4, 0x8ee1a0478680083aULL, 0x66cb946dcd298822ULL)
    XOR(tmp.lo, 5, 0x7396c0e3bb0fca9aULL, 0x715a6f94782b3af2ULL)
    XOR(tmp.lo, 6, 0x9386458f6709c14aULL, 0xa57b30c71d13dd35ULL)
    XOR(tmp.lo, 7, 0x77b6ca6dbf1381b0ULL, 0x860757166f353f18ULL)
    XOR(tmp.lo, 8, 0xe98f1e3a0e68ee6bULL, 0x2eb84f434773c0f0ULL)
    XOR(tmp.lo, 9, 0x48a8feb481412058ULL, 0x6ec4a811d3f5e13aULL)
    XOR(tmp.lo, 10, 0x2acb074fd1528435ULL, 0x310cb9f1bb763692ULL)
    XOR(tmp.lo, 11, 0x07cfecc893741e1eULL, 0x54445aa20b269e02ULL)
    XOR(tmp.lo, 12, 0x175f9549f77a39adULL, 0xe9066c93af9dfdb0ULL)
    XOR(tmp.lo, 13, 0x4426c48954682e38ULL, 0x62c2b5e8e0d69c9bULL)
    XOR(tmp.lo, 14, 0x06eeb208c90ce28cULL, 0x5e5ef7e00ed35ea9ULL)
    XOR(tmp.lo, 15, 0xb20ad12841018041ULL, 0xb1b3fcfb522ae01bULL)
    XOR(tmp.lo, 16, 0xf94466f60cc9cfd9ULL, 0xfc7cf254b6da0494ULL)
    XOR(tmp.lo, 17, 0x7420dd36650b4e03ULL, 0xe6ee807dd4f693c4ULL)
    XOR(tmp.lo, 18, 0xa4d5e84842496c56ULL, 0x7ae8e208f7190436ULL)
    XOR(tmp.lo, 19, 0x0a1651d2513089caULL, 0xd1ea97de107b32c9ULL)
    XOR(tmp.lo, 20, 0xfa7bfb681850a9a3ULL, 0x5f5291a6283d5e3bULL)
    XOR(tmp.lo, 21, 0xef063377f32c1b27ULL, 0xe075e04ebb0cd1a4ULL)
    XOR(tmp.lo, 22, 0x8ee415201f9ac0d7ULL, 0x4a94290c864c26abULL)
    XOR(tmp.lo, 23, 0x59d29bb6ecd74a35ULL, 0x5e5ddf9a3ab76eccULL)
    XOR(tmp.lo, 24, 0x70d37d8062ede900ULL, 0x80b8f5bd4f4296e2ULL)
    XOR(tmp.lo, 25, 0xc325d770d103a272ULL, 0x14babf1d5d7b1bf1ULL)
    XOR(tmp.lo, 26, 0x1386ef197be907e6ULL, 0xfe7068c56f7362b3ULL)
    XOR(tmp.lo, 27, 0x378500ecc90a9cf1ULL, 0xd22c04459a7949c0ULL)
    XOR(tmp.lo, 28, 0xfe49609deaa81ae4ULL, 0x45f33f678cfa46dfULL)
    XOR(tmp.lo, 29, 0xc8df216f835c4cdcULL, 0x0cbf3a8b7a964ff5ULL)
    XOR(tmp.lo, 30, 0xadf96f98af5a1ec9ULL, 0xb0c0a793d68846d9ULL)
    XOR(tmp.lo, 31, 0x814f25287c709c9bULL, 0x148a61faa04a5b72ULL)
    XOR(tmp.lo, 32, 0xe0165b6fcb8e09f0ULL, 0xa500b82a5dd0442fULL)
    XOR(tmp.lo, 33, 0xa559a881ba91e661ULL, 0x4ab3bd9ea43a43fcULL)
    XOR(tmp.lo, 34, 0x828686ae4bf3b1e7ULL, 0x10a26e2069b47539ULL)
    XOR(tmp.lo, 35, 0x354a4970c3798857ULL, 0x6617a2470d9126f2ULL)
    XOR(tmp.lo, 36, 0x627399b8aeff223bULL, 0xc8cb1f7ff71d7799ULL)
    XOR(tmp.lo, 37, 0xf5dc48949583aab8ULL, 0xf5da697d4650b5b4ULL)
    XOR(tmp.lo, 38, 0x20b68ce2799464a4ULL, 0xaed50dfa8e1f2c0aULL)
    XOR(tmp.lo, 39, 0x6ce16fa557508398ULL, 0x00621d3359224c5dULL)
    XOR(tmp.lo, 40, 0xf5437e0d460fb080ULL, 0xdd0d3cab187ad8cdULL)
    XOR(tmp.lo, 41, 0x5a6784625179eb67ULL, 0x0a7dff3ceca8340aULL)
    XOR(tmp.lo, 42, 0x5bada798d37bf1daULL, 0x66c0ea498ae30744ULL)
    XOR(tmp.lo, 43, 0xf49f3202af8b828dULL, 0x9649d0792a7c0f4dULL)
    XOR(tmp.lo, 44, 0x06a6f88edfd5f696ULL, 0xdc21a06b0df2c4d7ULL)
    XOR(tmp.lo, 45, 0xa958de14f8755f8eULL, 0x5c36949458eb5ff4ULL)
    XOR(tmp.lo, 46, 0xf41b985d37852acaULL, 0x67840ae2d8980411ULL)
    XOR(tmp.lo, 47, 0x0889eabc540cb5fcULL, 0x1002322c32d607c3ULL)
    XOR(tmp.lo, 48, 0x2df8ffea481d069aULL, 0xd08621bb396c6752ULL)
    XOR(tmp.lo, 49, 0x323e4d1e502fcc2eULL, 0xd677a3a5fbcf0701ULL)
    XOR(tmp.lo, 50, 0x2a1b5a014f1e28d6ULL, 0x8fbc61e11fb583e8ULL)
    XOR(tmp.lo, 51, 0x79920c49fc2295feULL, 0x1eb45493f04b9382ULL)
    XOR(tmp.lo, 52, 0xa6f2980b77654919ULL, 0xd5706cc1e0b7274eULL)
    XOR(tmp.lo, 53, 0x49242973f0d38bc5ULL, 0xbaae62ca0da195eeULL)
    XOR(tmp.lo, 54, 0x5bb41874775c1cf5ULL, 0x9a1086f162376218ULL)
    XOR(tmp.lo, 55, 0xa1c27475a00c0d56ULL, 0x5182ac806d5cecefULL)
    XOR(tmp.lo, 56, 0xfa8abd84ca3bf283ULL, 0xfb2ef04531097121ULL)
    XOR(tmp.lo, 57, 0xca58f43d9cb60c51ULL, 0xac19179c3d689655ULL)
    XOR(tmp.lo, 58, 0x4a258cc0cdea0d50ULL, 0xed43af5dd7af7accULL)
    XOR(tmp.lo, 59, 0xcc1823d552f2c042ULL, 0x468128efb2d5f96cULL)
    XOR(tmp.lo, 60, 0xedc3c15e60d7a055ULL, 0xb25a4a5386305762ULL)
    XOR(tmp.lo, 61, 0x9b3edfebd022f75fULL, 0x29041dcb27c38d24ULL)
    XOR(tmp.lo, 62, 0xb4a161f08d70845aULL, 0xc64435382757e8c5ULL)
    XOR(tmp.lo, 63, 0x1476f5e55753c1e6ULL, 0x66d8209886988ee9ULL)
    XOR(tmp.hi, 0, 0x32b37ba8ebcebd21ULL, 0x19c7c3b6a2248bf0ULL)
    XOR(tmp.hi, 1, 0xac20e08487f30521ULL, 0x04585f12f71a7a7eULL)
    XOR(tmp.hi, 2, 0x3a04800b2f57d6c2ULL, 0x4361d754ffc3ef3aULL)
    XOR(tmp.hi, 3, 0x9116ff8703a2fde4ULL, 0x94781ed65ce4f05cULL)
    XOR(tmp.hi, 4, 0xc8bf201295aa0187ULL, 0x2dada86c4c4116a0ULL)
    XOR(tmp.hi, 5, 0x169d177d72e1cf6eULL, 0x6190775f0a84b841ULL)
    XOR(tmp.hi, 6, 0x6dc7ddf8aef47810ULL, 0x5bfa7ddac8c92335ULL)
    XOR(tmp.hi, 7, 0xb2892a43430ef64eULL, 0xe0879f073b8390aaULL)
    XOR(tmp.hi, 8, 0x935baac54121cfc4ULL, 0x9f0c3ea0adb5a286ULL)
    XOR(tmp.hi, 9, 0x0ea8518bf879a5efULL, 0xf664d22a0895a881ULL)
    XOR(tmp.hi, 10, 0x8ba56f98f2cc4f7aULL, 0xbf4ed50a596ac088ULL)
    XOR(tmp.hi, 11, 0xeef21af06e0a0185ULL, 0xb003a9c42a64947fULL)
    XOR(tmp.hi, 12, 0x9fb641777d5b0d1eULL, 0x63c0cd54965137beULL)
    XOR(tmp.hi, 13, 0x0a06c702e598e322ULL, 0xfbeaf3c81b304e89ULL)
    XOR(tmp.hi, 14, 0x1a5ba9c2663dd33fULL, 0x37d7f24be68d314fULL)
    XOR(tmp.hi, 15, 0xbb27059694afd248ULL, 0x8d1ca37e6d9802f0ULL)
    XOR(tmp.hi, 16, 0xfa53f67a95deccedULL, 0x84e7a1db22563dc3ULL)
    XOR(tmp.hi, 17, 0xf6458473367f4bdaULL, 0xbb13ff967e38bc8cULL)
    XOR(tmp.hi, 18, 0x6603c03ffc16d685ULL, 0x4f459c09fe3c94c6ULL)
    XOR(tmp.hi, 19, 0x085404637907b059ULL, 0xbba841c7d6560f22ULL)
    XOR(tmp.hi, 20, 0xbf5e11dcbb585d38ULL, 0xaa8bd2b5b259a49bULL)
    XOR(tmp.hi, 21, 0x8be8ba0509a9e3b1ULL, 0xf4077d9579ed71c4ULL)
    XOR(tmp.hi, 22, 0x6f1ee9bba817ede4ULL, 0x5a56ce904aadad43ULL)
    XOR(tmp.hi, 23, 0x64bca8c9658ea667ULL, 0x85bd0a5b2b1f613eULL)
    XOR(tmp.hi, 24, 0x9af56039ed5b676bULL, 0x7e8ced804a7bf3c1ULL)
    XOR(tmp.hi, 25, 0x605b50115c8471b7ULL, 0xa2ec7b2e7c4cbcc9ULL)
    XOR(tmp.hi, 26, 0x368afd852e5264eaULL, 0xec3d9bb94636cd36ULL)
    XOR(tmp.hi, 27, 0xe492beffe8642a2bULL, 0x76982087ceeeab85ULL)
    XOR(tmp.hi, 28, 0xb5395b14c2c37815ULL, 0x4801e76ae6fd6cbbULL)
    XOR(tmp.hi, 29, 0xd9333173d2a5871eULL, 0x8d9d02574584a4bbULL)
    XOR(tmp.hi, 30, 0x4b407a8076b89d2aULL, 0x4661b6902fc7d7beULL)
    XOR(tmp.hi, 31, 0x0e50bff9ddccb272ULL, 0x570906b495b01afbULL)
    XOR(tmp.hi, 32, 0xa05ac64529d282cfULL, 0xb5f8c537fd811c76ULL)
    XOR(tmp.hi, 33, 0x4796788837961b7eULL, 0x8ceaa8fd64633a67ULL)
    XOR(tmp.hi, 34, 0x826090d1fd6087b9ULL, 0x034ce4ff7e425e42ULL)
    XOR(tmp.hi, 35, 0xce777c9e109b7000ULL, 0x18200d9679cc6824ULL)
    XOR(tmp.hi, 36, 0x7f997981adf1dbf1ULL, 0xe7e04d380470b92cULL)
    XOR(tmp.hi, 37, 0xfa5f51fac197f6abULL, 0x03f57d6bfc05bba6ULL)
    XOR(tmp.hi, 38, 0x813e4a988e6ca307ULL, 0xc22968d5e4ce50c7ULL)
    XOR(tmp.hi, 39, 0xa39f5ef8fd150cddULL, 0xe1c2bc69fe501b1eULL)
    XOR(tmp.hi, 40, 0x8cb69e62422cb70eULL, 0xb4c2fbe82e04a84eULL)
    XOR(tmp.hi, 41, 0x694ac8fda0855c93ULL, 0x87c0cdc2253a7269ULL)
    XOR(tmp.hi, 42, 0xabaff55f08adaf63ULL, 0x066b779d29655731ULL)
    XOR(tmp.hi, 43, 0x5fea359b6c9d1814ULL, 0x97bd79af0dfc9c0aULL)
    XOR(tmp.hi, 44, 0x17e0a1707ea1201bULL, 0xf8a33459217404a2ULL)
    XOR(tmp.hi, 45, 0x00122ea1784a4d19ULL, 0x00dd88bb0742b4e9ULL)
    XOR(tmp.hi, 46, 0xa81623c9278d8a0dULL, 0xfc8078cedc62f651ULL)
    XOR(tmp.hi, 47, 0xb5a03428ac00fd10ULL, 0x15cb73b6cbbff9d3ULL)
    XOR(tmp.hi, 48, 0x308c609078741ef2ULL, 0x64ecd8db5c9ff721ULL)
    XOR(tmp.hi, 49, 0xb3afddc877a91684ULL, 0xe8dd2d0bf421b74fULL)
    XOR(tmp.hi, 50, 0xc4508328c345c028ULL, 0x566f3dbbc74afc1aULL)
    XOR(tmp.hi, 51, 0x7b9d7f4dad12a68aULL, 0xf349042186423b7cULL)
    XOR(tmp.hi, 52, 0x43f9aade34df2e55ULL, 0x6841f59bfec9c94bULL)
    XOR(tmp.hi, 53, 0xc532c3fa0aadfa0bULL, 0xaffec201fcb72a07ULL)
    XOR(tmp.hi, 54, 0xdd894f38e24ec479ULL, 0x6925e83b393340c1ULL)
    XOR(tmp.hi, 55, 0x9815571c6837338cULL, 0x08ae87e6f15032e1ULL)
    XOR(tmp.hi, 56, 0x2e34ccc955545d0dULL, 0x380e377d081e988aULL)
    XOR(tmp.hi, 57, 0xabb6695499088738ULL, 0xfa489750820c64b1ULL)
    XOR(tmp.hi, 58, 0xd1fc977813aa97f6ULL, 0x8e229c3a401cc040ULL)
    XOR(tmp.hi, 59, 0xf33a77ab9e804e16ULL, 0x84fc15f114c8ffd5ULL)
    XOR(tmp.hi, 60, 0x84774bf7aa07a202ULL, 0x5cbb3810d96db47aULL)
    XOR(tmp.hi, 61, 0xabd2e2e4517a3793ULL, 0x61ba8c6be722c83bULL)
    XOR(tmp.hi, 62, 0xcb76df4453ed7bf3ULL, 0xf958d44c7154bca9ULL)
    XOR(tmp.hi, 63, 0x4955857644270dc4ULL, 0xba2b39a94a63bc6fULL)
}

// #define INT_MIN  -2147483648
// #define INT_MAX  2147483647

uint64_t set_feature_seed(uint64_t world_seed, int32_t x, int32_t z, int32_t index, int32_t step) {
    // xrsr_seed(rng, world_seed);
    Xoroshiro rng;
    xSetSeed(&rng, world_seed);
    int64_t a = xNextLongJ(&rng) | 1LL;
    int64_t b = xNextLongJ(&rng) | 1LL;
    int64_t decorationSeed = ((int64_t)x * a + (int64_t)z * b) ^ world_seed;
    int64_t featureSeed = decorationSeed + (int64_t)index + (int64_t)(10000 * step);
    return featureSeed;
    // xSetSeed(rng, featureSeed);
}

void get_eyes(int chunk_x, int chunk_z, uint64_t world_seed, Pos *frame_positions, int *eyes) {
    uint64_t feature_seed = set_feature_seed(world_seed, chunk_x << 4, chunk_z << 4, 19, 4);

    XRSR128 rng;
    xrsr_seed(&rng, feature_seed);
    skip(&rng);

    for (int i = 0; i < 12; ++i) {
        if (xrsr_long(&rng) >= 16602070326045573120ULL) {
            if (frame_positions[i].x >> 4 == chunk_x && frame_positions[i].z >> 4 == chunk_z) {
                eyes[i] = 1;  
            }
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Invalid arguments.\n");
        return 1;
    }

    int part = strtoull(argv[1], NULL, 10);
    if (part < 1 || part > 16) {
        printf("Invalid arguments.\n");
        return 1;
    }

	#pragma omp parallel
	{
		FILE* input = fopen("input", "r");
		if (input == NULL) {
		    printf("Couldn't open the input file.\n");
		    exit(1);
		}

		StrongholdIter sh;
	    stronghold_generator::StrongholdGenerator strongholdGenerator;
	    int startChunkX, startChunkZ, tmp, required_eyes;
	    Pos frame_positions[12];
	    int eyes[12];
	    int eyes_count;
	    int chunkX, chunkZ, minChunkX, maxChunkX, minChunkZ, maxChunkZ, portal_start_x, portal_end_x, portal_start_z, portal_end_z;
	    int stronghold_index;
	    int first_blaze_no_looting;

		Generator g;
		setupGenerator(&g, VERSION, 0);

		uint64_t structure_seed, world_seed, j, rnd;
		Pos bastion_pos, portal_pos;

		int rotation;
		int loot[1024];
		int id, count, enchantment_id, enchantment_level;
		int obsidian;
		RuinedPortal portal;
		int portal_obsidian, charges, nether_charges, flint_and_steel, flint, sword, axe;
		Pos3 portal_chest_pos;
		int entries;
		Xoroshiro xoro;
		int string;
		int pearls, gold_ingots, biome_id;
		StructureVariant portal_variant;
		Pos tmp_pos1, tmp_pos2, tmp_pos3;

		int five_chests_x, five_chests_z, three_chests_x, three_chests_z;
		int iron_nuggets, iron_ingots, salt;
		char blaze_str[] = "minecraft:entities/blaze";
		char bartering_str[] = "minecraft:gameplay/piglin_bartering";
		int rods, i;
		int MAX_INGOTS = 9;
		const int required_pearls = 6;
		const int required_rods = 2;
		Pos spawn;

		char chunk[128];
		while(fgets(chunk, sizeof(chunk), input) != NULL) {
			structure_seed = strtoull(chunk, NULL, 10);

			getStructurePos(Ruined_Portal, MC_1_21, structure_seed, 0, 0, &portal_pos);
			getStructurePos(Bastion, MC_1_21, structure_seed, 0, 0, &bastion_pos);

			setSeed(&rnd, structure_seed);
			rnd = (nextLong(&rnd) * (bastion_pos.x >> 4)) ^ (nextLong(&rnd) * (bastion_pos.z >> 4)) ^ structure_seed;

			setSeed(&rnd, rnd);
			rotation = nextInt(&rnd, 4);

			switch (rotation) {
		        case 0: { // -> dodatnie z
		            five_chests_x = bastion_pos.x - 16;
		            five_chests_z = bastion_pos.z + 16;

					three_chests_x = five_chests_x;
		            three_chests_z = five_chests_z + 16;
		        } break;

		        case 1: { // -> ujemny x
		            five_chests_x = bastion_pos.x - 32;
		            five_chests_z = bastion_pos.z - 16;

					three_chests_x = five_chests_x - 16;
		            three_chests_z = five_chests_z;
		        } break;
		        
		        case 2: { // -> ujemne z
		            five_chests_x = bastion_pos.x;
		            five_chests_z = bastion_pos.z - 32;

					three_chests_x = five_chests_x;
		            three_chests_z = five_chests_z - 16;
		        } break;
		        
		        case 3: { // -> dodatni x 
		            five_chests_x = bastion_pos.x + 16;
		            five_chests_z = bastion_pos.z;

					three_chests_x = five_chests_x + 16;
		            three_chests_z = five_chests_z;
		        } break;
		        
		        default:
		            break;
		    }

			get_ruined_portal(&portal, structure_seed, portal_pos.x >> 4, portal_pos.z >> 4);
			get_ruined_portal_chest_pos(&portal, &portal_chest_pos);

			#pragma omp for
			for (j = 4096 * (part - 1); j < (uint64_t)4096 * (part); ++j) {
				iron_nuggets = 0;
			    iron_ingots = 0;
			    gold_ingots = 0;
			    obsidian = 0;
			    string = 0;
				charges = 0;
				nether_charges = 0;
				pearls = 0;
				portal_obsidian = 0;
				flint = 0;
				flint_and_steel = 0;
				sword = 0;
				rods = 0;
				axe = 0;
				first_blaze_no_looting = 0;

				world_seed = structure_seed | (j << 48);

				xoro = getRandomSequenceXoro(world_seed, bartering_str);
		   		for (i = 0; i < MAX_INGOTS; ++i) {    	
				    BarteringOut out = nextBarteringLoot(&xoro);

				    if (out.id == 6) {
				    	pearls += out.amount;
				    }
			    }	

			    if (pearls < required_pearls) {
			    	continue;
			    }
			    pearls = 0;

			    tmp_pos1 = { five_chests_x, five_chests_z };
			    tmp_pos2 = { three_chests_x, three_chests_z };
				entries = get_bastion_ramparts_loot(world_seed, &tmp_pos1, &tmp_pos2, loot);
				// entries = get_stables_loot(world_seed, &(Pos){ five_chests_x, five_chests_z }, &(Pos){ three_chests_x, three_chests_z }, &(Pos){ 128, 128 }, loot);
				// entries = get_bastion_triple_loot(world_seed, &(Pos){ five_chests_x, five_chests_z }, loot);
				for (i =0 ; i < entries; ++i) {
					id = loot[4*i];
					count = loot[4*i+1];
				    enchantment_id = loot[4 * i + 2];
				    enchantment_level = loot[4 * i + 3];

					if (id == 30) {
						obsidian += count;
					}

					if (id == 32) {
						string += count;
					}

					if (id == 11 && enchantment_id == 16) {
						sword = max(sword, enchantment_level);
					}

					if (id == 17) gold_ingots += count;
					if (id == 15) gold_ingots += count*9;

					if (id == 12) {
						iron_ingots += count*9;
					}

					if (id == 18) {
						iron_ingots += count;
					}

					if (id == 29) {
						iron_nuggets += count;
					}

					if (id == 14) {
						axe = 1;
					}
				}

				// if (iron_ingots < 16) {
				// 	continue;
				// }

				applySeed(&g, DIM_OVERWORLD, world_seed);
				biome_id = getBiomeAt(&g, 4, portal_pos.x >> 2, 320 >> 2, portal_pos.z >> 2);
				// biome_id = getBiomeAt(&g, 4, portal_pos.x, 320 >> 2, portal_pos.z);

				getVariant(&portal_variant, Ruined_Portal, VERSION, world_seed, portal_pos.x, portal_pos.z, biome_id);

		        // int X = (portal_pos.x + portal_variant.x + portal_variant.sx / 2) & ~15;
        		// int Z = (portal_pos.z + portal_variant.z + portal_variant.sz / 2) & ~15;
        		// printf("(%d, %d)\n", X, Z);

				if (portal_variant.biome == plains) {
					salt = 40010;
				} else if (portal_variant.biome == mountains || biome_id == cherry_grove) {
					salt = 40013;
				} else {
					continue;
				}

				// printf("%d\n", salt);

				tmp_pos3 = { portal_chest_pos.x >> 4 << 4, portal_chest_pos.z >> 4 << 4 };
				entries = get_ruined_portal_loot(world_seed, &tmp_pos3, loot, salt);
				for (i = 0; i < entries; ++i) {
				    id = loot[4 * i];
				    count = loot[4 * i + 1];
				    enchantment_id = loot[4 * i + 2];
				    enchantment_level = loot[4 * i + 3];

				    if (id == 0) portal_obsidian += count;
				    if (id == 1) flint += count;
				    if (id == 3) flint_and_steel = 1;
				    if (id == 4) charges += count;
				    if (id == 7 && enchantment_id == 16) sword = max(sword, enchantment_level);

				    if (id == 21) gold_ingots += count;
				    if (id == 24) gold_ingots += count*9;

				     if (id == 2) iron_nuggets += count;
				     if (id == 8) axe = 1;
				    //  int amount = count;

	                //  switch (id) {
		            //     case 0: printf("obsidian; %d\n", amount); break;
		            //     case 1: printf("flint; %d\n", amount); break;
		            //     case 2: printf("iron_nugget; %d\n", amount); break;
		            //     case 3: printf("flint_and_steel; %d\n", amount); break;
		            //     case 4: printf("fire_charge; %d\n", amount); break;
		            //     case 5: printf("golden_apple; %d\n", amount); break;
		            //     case 6: printf("gold_nugget; %d\n", amount); break;
		            //     case 7: printf("golden_sword; %d; ", amount); print_enchantment(enchantment_id, enchantment_level, 1); break; break;
		            //     case 8: printf("golden_axe; %d; ", amount); print_enchantment(enchantment_id, enchantment_level, 1); break; break;
		            //     case 9: printf("golden_hoe; %d; ", amount); print_enchantment(enchantment_id, enchantment_level, 1); break; break;
		            //     case 10: printf("golden_shovel; %d; ", amount); print_enchantment(enchantment_id, enchantment_level, 1); break; break;
		            //     case 11: printf("golden_pickaxe; %d; ", amount); print_enchantment(enchantment_id, enchantment_level, 1); break; break;
		            //     case 12: printf("golden_boots; %d; ", amount); print_enchantment(enchantment_id, enchantment_level, 1); break; break;
		            //     case 13: printf("golden_chestplate; %d; ", amount); print_enchantment(enchantment_id, enchantment_level, 1); break; break;
		            //     case 14: printf("golden_helmet; %d; ", amount); print_enchantment(enchantment_id, enchantment_level, 1); break; break;
		            //     case 15: printf("golden_leggings; %d; ", amount); print_enchantment(enchantment_id, enchantment_level, 1); break; break;
		            //     case 16: printf("glistering_melon_slice; %d\n", amount); break;
		            //     case 17: printf("golden_horse_armour; %d\n", amount); break;
		            //     case 18: printf("light_weighted_pressure_plate; %d\n", amount); break;
		            //     case 19: printf("golden_carrot; %d\n", amount); break;
		            //     case 20: printf("clock; %d\n", amount); break;
		            //     case 21: printf("gold_ingot; %d\n", amount); break;
		            //     case 22: printf("bell; %d\n", amount); break;
		            //     case 23: printf("enchanted_golden_apple; %d\n", amount); break;
		            //     case 24: printf("gold_block; %d\n", amount); break;
		            //     default: break;
		            // }
				}

				if (portal_obsidian < portal.required_obsidian) {
					continue;
				}

				if (axe != 1) {
					continue;
				}

				xoro = getRandomSequenceXoro(world_seed, blaze_str);
				if (sword != 0) {
					for (i = 0; i < 2; ++i) {
						rods += xNextInt(&xoro, 2) + round(xNextFloat(&xoro) * (float)sword);
					}
				} else {
					for (i = 0; i < 2; ++i) {
						rods += xNextInt(&xoro, 2);
					}
				}

				if (rods < required_rods) {
					if (sword == 0) {
						continue;
					}

					rods = 0;
					xoro = getRandomSequenceXoro(world_seed, blaze_str);
					rods += xNextInt(&xoro, 2);
					rods += xNextInt(&xoro, 2) + round(xNextFloat(&xoro) * (float)sword);

					if (rods < required_rods) {
						continue;
					}

					first_blaze_no_looting = 1;
				}


				xoro = getRandomSequenceXoro(world_seed, bartering_str);
				for (i = 0; i < min(MAX_INGOTS, gold_ingots); ++i) {
				    BarteringOut out = nextBarteringLoot(&xoro);

				    if (out.id == 6) {
				    	pearls += out.amount;
				    }

				    if (out.id == 7) {
				    	string += out.amount;
				    }

				    if (out.id == 11) {
				    	nether_charges += out.amount;
				    }

				    if (out.id == 9) {
				    	obsidian += out.amount;
				    }

				    if (out.id == 5) {
				    	iron_nuggets += out.amount;
				    }
			    }

				if (!(
					(flint_and_steel || (charges && flint) || (charges && nether_charges) || (charges >= 2))
				)) {
					continue;
				}

				if (iron_ingots + iron_nuggets / 9 < 26) {
					continue;
				}

				if ((obsidian + portal_obsidian - portal.required_obsidian) < 10) {
					continue;
				}

			    if (pearls < required_pearls) {
			    	continue;
			    }

				if (string < 3) {
					continue;
				}

				spawn = getSpawn(&g);
				if (abs(spawn.x - portal_pos.x) > 48 || abs(spawn.z - portal_pos.z) > 48) {
					continue;
				}

				required_eyes = std::max(12 - rods * 2, 12 - (pearls - 2));
				if (required_eyes <= 0) {
		            printf("%ld ; 14+ pearls, 6+ rods\n", world_seed);
		            continue;
		        }

		        initFirstStronghold(&sh, MC_1_21_1, world_seed);
		        for (stronghold_index = 0; stronghold_index < 3; ++stronghold_index) {
		            std::fill(std::begin(eyes), std::end(eyes), 0);
		        	nextStronghold(&sh, &g);

		            startChunkX = sh.pos.x >> 4;
		            startChunkZ = sh.pos.z >> 4;

		            strongholdGenerator.generate(world_seed, startChunkX, startChunkZ);
		            stronghold_generator::Piece *portalRoomPiece = strongholdGenerator.portalRoomPiece;
		            minChunkX = portalRoomPiece->getWorldX(4,  9) >> 4;
		            maxChunkX = portalRoomPiece->getWorldX(6, 11) >> 4;
		            if (minChunkX > maxChunkX) {
		                // std::swap(minChunkX, maxChunkX);
		                tmp = maxChunkX;
		                maxChunkX = minChunkX;
		                minChunkX = tmp;
		            }
		            minChunkZ = portalRoomPiece->getWorldZ(4,  9) >> 4;
		            maxChunkZ = portalRoomPiece->getWorldZ(6, 11) >> 4;
		            if (minChunkZ > maxChunkZ) {
		                tmp = maxChunkZ;
		                maxChunkZ = minChunkZ;
		                minChunkZ = tmp;
		            }

		            portal_start_x = portalRoomPiece->getWorldX(3,  8);
		            portal_end_x = portalRoomPiece->getWorldX(7,  12);
		            if (portal_end_x > portal_start_x) {
		                tmp = portal_end_x;
		                portal_end_x = portal_start_x;
		                portal_start_x = tmp;
		            }

		            portal_start_z = portalRoomPiece->getWorldZ(3,  8); 
		            portal_end_z = portalRoomPiece->getWorldZ(7,  12);
		            if (portal_end_z > portal_start_z) {
		                tmp = portal_end_z;
		                portal_end_z = portal_start_z;
		                portal_start_z = tmp;
		            }

		            switch (portalRoomPiece->orientation) {
		                case stronghold_generator::NORTH: {
		                    frame_positions[0]   = { .x = portal_start_x - 3, .z = portal_start_z };
		                    frame_positions[1]   = { .x = portal_start_x - 2, .z = portal_start_z };
		                    frame_positions[2]   = { .x = portal_start_x - 1, .z = portal_start_z };

		                    frame_positions[3]   = { .x = portal_start_x - 3, .z = portal_start_z - 4 };
		                    frame_positions[4]   = { .x = portal_start_x - 2, .z = portal_start_z - 4 };
		                    frame_positions[5]   = { .x = portal_start_x - 1, .z = portal_start_z - 4 };

		                    frame_positions[6]   = { .x = portal_start_x - 4, .z = portal_start_z - 1 };
		                    frame_positions[7]   = { .x = portal_start_x - 4, .z = portal_start_z - 2 };
		                    frame_positions[8]   = { .x = portal_start_x - 4, .z = portal_start_z - 3 };

		                    frame_positions[9]   = { .x = portal_start_x, .z = portal_start_z - 1 };
		                    frame_positions[10] = { .x = portal_start_x, .z = portal_start_z - 2 };
		                    frame_positions[11] = { .x = portal_start_x, .z = portal_start_z - 3 };
		                } break;

		                case stronghold_generator::WEST: {
		                    frame_positions[0]   = { .x = portal_start_x, .z = portal_start_z - 3 };
		                    frame_positions[1]   = { .x = portal_start_x, .z = portal_start_z - 2 };
		                    frame_positions[2]   = { .x = portal_start_x, .z = portal_start_z - 1 };

		                    frame_positions[3]   = { .x = portal_start_x - 4, .z = portal_start_z - 3 };
		                    frame_positions[4]   = { .x = portal_start_x - 4, .z = portal_start_z - 2 };
		                    frame_positions[5]   = { .x = portal_start_x - 4, .z = portal_start_z - 1 };

		                    frame_positions[6]   = { .x = portal_start_x - 1, .z = portal_start_z - 4 };
		                    frame_positions[7]   = { .x = portal_start_x - 2, .z = portal_start_z - 4 };
		                    frame_positions[8]   = { .x = portal_start_x - 3, .z = portal_start_z - 4 };

		                    frame_positions[9]   = { .x = portal_start_x - 1, .z = portal_start_z };
		                    frame_positions[10] = { .x = portal_start_x - 2, .z = portal_start_z };
		                    frame_positions[11] = { .x = portal_start_x - 3, .z = portal_start_z };
		                } break;

		                case stronghold_generator::SOUTH: {
		                    frame_positions[0]   = { .x = portal_start_x - 3, .z = portal_start_z - 4 };
		                    frame_positions[1]   = { .x = portal_start_x - 2, .z = portal_start_z - 4 };
		                    frame_positions[2]   = { .x = portal_start_x - 1, .z = portal_start_z - 4 };

		                    frame_positions[3]   = { .x = portal_start_x - 3, .z = portal_start_z };
		                    frame_positions[4]   = { .x = portal_start_x - 2, .z = portal_start_z };
		                    frame_positions[5]   = { .x = portal_start_x - 1, .z = portal_start_z };

		                    frame_positions[6]   = { .x = portal_start_x - 4, .z = portal_start_z - 3 };
		                    frame_positions[7]   = { .x = portal_start_x - 4, .z = portal_start_z - 2 };
		                    frame_positions[8]   = { .x = portal_start_x - 4, .z = portal_start_z - 1 };

		                    frame_positions[9]   = { .x = portal_start_x, .z = portal_start_z - 3 };
		                    frame_positions[10] = { .x = portal_start_x, .z = portal_start_z - 2 };
		                    frame_positions[11] = { .x = portal_start_x, .z = portal_start_z - 1 };
		                } break;

		                case stronghold_generator::EAST: {
		                    frame_positions[0]   = { .x = portal_start_x - 4, .z = portal_start_z - 3 };
		                    frame_positions[1]   = { .x = portal_start_x - 4, .z = portal_start_z - 2 };
		                    frame_positions[2]   = { .x = portal_start_x - 4, .z = portal_start_z - 1 };

		                    frame_positions[3]   = { .x = portal_start_x, .z = portal_start_z - 3 };
		                    frame_positions[4]   = { .x = portal_start_x, .z = portal_start_z - 2 };
		                    frame_positions[5]   = { .x = portal_start_x, .z = portal_start_z - 1 };

		                    frame_positions[6]   = { .x = portal_start_x - 3, .z = portal_start_z - 4 };
		                    frame_positions[7]   = { .x = portal_start_x - 2, .z = portal_start_z - 4 };
		                    frame_positions[8]   = { .x = portal_start_x - 1, .z = portal_start_z - 4 };

		                    frame_positions[9]   = { .x = portal_start_x - 3, .z = portal_start_z };
		                    frame_positions[10] = { .x = portal_start_x - 2, .z = portal_start_z };
		                    frame_positions[11] = { .x = portal_start_x - 1, .z = portal_start_z };
		                } break;

		                default: break;  
		            }

		            for (chunkX = minChunkX; chunkX <= maxChunkX; chunkX++) {
		                for (chunkZ = minChunkZ; chunkZ <= maxChunkZ; chunkZ++) {
		                    stronghold_generator::BoundingBox chunkBox(chunkX << 4, INT_MIN, chunkZ << 4, (chunkX << 4) + 15, INT_MAX, (chunkZ << 4) + 15);
		                    
		                    bool is_first = true;
		                    for (i = 0; i < strongholdGenerator.piecesSize - 1; i++) {
		                        if (chunkBox.intersects(strongholdGenerator.pieces[i].boundingBox)) {
		                            is_first = false;
		                            break;
		                        }
		                    }
		                    if (!is_first) continue;

		                    get_eyes(chunkX, chunkZ, world_seed, frame_positions, eyes);
		                }
		            }

		            eyes_count = 0;
		            for (i = 0; i < 12; ++i) {
		                eyes_count += eyes[i];
		            }

		            if (eyes_count >= required_eyes) {
		                printf("%" PRIi64 " ; %d ; %d ; /teleport @a %d ~ %d\n", world_seed, first_blaze_no_looting, eyes_count, portal_start_x, portal_start_z);
		            }
		        }
			}
		} 
	}

	return 0;
}