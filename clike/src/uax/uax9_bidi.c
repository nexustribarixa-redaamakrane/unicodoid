#include "unicodoid/uax/uax9_bidi.h"
#include "unicodoid/unicodoid_ucd.h"

#include <stdlib.h>
#include <string.h>

/* Official Unicode 18.0.0 data (data/ucd/18.0.0/ucd/):
 *   BidiMirroring.txt  - mirrored glyph pairs (438 entries)
 *   BidiBrackets.txt   - paired-bracket table (130 entries)
 * Tables below are derived from those files; keep them in sync via the
 * generator (tools/gen_ucd.py) when the UCD version changes. */

typedef struct { uint32_t cp; uint32_t mirrored; } mirror_pair_t;
typedef struct { uint32_t cp; uint32_t counterpart; char kind; } bracket_pair_t;

static const mirror_pair_t k_mirroring[] = {
    {0x0028u, 0x0029u},
    {0x0029u, 0x0028u},
    {0x003Cu, 0x003Eu},
    {0x003Eu, 0x003Cu},
    {0x005Bu, 0x005Du},
    {0x005Du, 0x005Bu},
    {0x007Bu, 0x007Du},
    {0x007Du, 0x007Bu},
    {0x00ABu, 0x00BBu},
    {0x00BBu, 0x00ABu},
    {0x0F3Au, 0x0F3Bu},
    {0x0F3Bu, 0x0F3Au},
    {0x0F3Cu, 0x0F3Du},
    {0x0F3Du, 0x0F3Cu},
    {0x169Bu, 0x169Cu},
    {0x169Cu, 0x169Bu},
    {0x2039u, 0x203Au},
    {0x203Au, 0x2039u},
    {0x2045u, 0x2046u},
    {0x2046u, 0x2045u},
    {0x207Du, 0x207Eu},
    {0x207Eu, 0x207Du},
    {0x208Du, 0x208Eu},
    {0x208Eu, 0x208Du},
    {0x2208u, 0x220Bu},
    {0x2209u, 0x220Cu},
    {0x220Au, 0x220Du},
    {0x220Bu, 0x2208u},
    {0x220Cu, 0x2209u},
    {0x220Du, 0x220Au},
    {0x2215u, 0x29F5u},
    {0x221Du, 0x1DB10u},
    {0x221Fu, 0x2BFEu},
    {0x2220u, 0x29A3u},
    {0x2221u, 0x299Bu},
    {0x2222u, 0x29A0u},
    {0x2224u, 0x2AEEu},
    {0x223Cu, 0x223Du},
    {0x223Du, 0x223Cu},
    {0x2243u, 0x22CDu},
    {0x2245u, 0x224Cu},
    {0x224Cu, 0x2245u},
    {0x2252u, 0x2253u},
    {0x2253u, 0x2252u},
    {0x2254u, 0x2255u},
    {0x2255u, 0x2254u},
    {0x2264u, 0x2265u},
    {0x2265u, 0x2264u},
    {0x2266u, 0x2267u},
    {0x2267u, 0x2266u},
    {0x2268u, 0x2269u},
    {0x2269u, 0x2268u},
    {0x226Au, 0x226Bu},
    {0x226Bu, 0x226Au},
    {0x226Eu, 0x226Fu},
    {0x226Fu, 0x226Eu},
    {0x2270u, 0x2271u},
    {0x2271u, 0x2270u},
    {0x2272u, 0x2273u},
    {0x2273u, 0x2272u},
    {0x2274u, 0x2275u},
    {0x2275u, 0x2274u},
    {0x2276u, 0x2277u},
    {0x2277u, 0x2276u},
    {0x2278u, 0x2279u},
    {0x2279u, 0x2278u},
    {0x227Au, 0x227Bu},
    {0x227Bu, 0x227Au},
    {0x227Cu, 0x227Du},
    {0x227Du, 0x227Cu},
    {0x227Eu, 0x227Fu},
    {0x227Fu, 0x227Eu},
    {0x2280u, 0x2281u},
    {0x2281u, 0x2280u},
    {0x2282u, 0x2283u},
    {0x2283u, 0x2282u},
    {0x2284u, 0x2285u},
    {0x2285u, 0x2284u},
    {0x2286u, 0x2287u},
    {0x2287u, 0x2286u},
    {0x2288u, 0x2289u},
    {0x2289u, 0x2288u},
    {0x228Au, 0x228Bu},
    {0x228Bu, 0x228Au},
    {0x228Fu, 0x2290u},
    {0x2290u, 0x228Fu},
    {0x2291u, 0x2292u},
    {0x2292u, 0x2291u},
    {0x2298u, 0x29B8u},
    {0x22A2u, 0x22A3u},
    {0x22A3u, 0x22A2u},
    {0x22A6u, 0x2ADEu},
    {0x22A8u, 0x2AE4u},
    {0x22A9u, 0x2AE3u},
    {0x22ABu, 0x2AE5u},
    {0x22B0u, 0x22B1u},
    {0x22B1u, 0x22B0u},
    {0x22B2u, 0x22B3u},
    {0x22B3u, 0x22B2u},
    {0x22B4u, 0x22B5u},
    {0x22B5u, 0x22B4u},
    {0x22B6u, 0x22B7u},
    {0x22B7u, 0x22B6u},
    {0x22B8u, 0x27DCu},
    {0x22C9u, 0x22CAu},
    {0x22CAu, 0x22C9u},
    {0x22CBu, 0x22CCu},
    {0x22CCu, 0x22CBu},
    {0x22CDu, 0x2243u},
    {0x22D0u, 0x22D1u},
    {0x22D1u, 0x22D0u},
    {0x22D6u, 0x22D7u},
    {0x22D7u, 0x22D6u},
    {0x22D8u, 0x22D9u},
    {0x22D9u, 0x22D8u},
    {0x22DAu, 0x22DBu},
    {0x22DBu, 0x22DAu},
    {0x22DCu, 0x22DDu},
    {0x22DDu, 0x22DCu},
    {0x22DEu, 0x22DFu},
    {0x22DFu, 0x22DEu},
    {0x22E0u, 0x22E1u},
    {0x22E1u, 0x22E0u},
    {0x22E2u, 0x22E3u},
    {0x22E3u, 0x22E2u},
    {0x22E4u, 0x22E5u},
    {0x22E5u, 0x22E4u},
    {0x22E6u, 0x22E7u},
    {0x22E7u, 0x22E6u},
    {0x22E8u, 0x22E9u},
    {0x22E9u, 0x22E8u},
    {0x22EAu, 0x22EBu},
    {0x22EBu, 0x22EAu},
    {0x22ECu, 0x22EDu},
    {0x22EDu, 0x22ECu},
    {0x22F0u, 0x22F1u},
    {0x22F1u, 0x22F0u},
    {0x22F2u, 0x22FAu},
    {0x22F3u, 0x22FBu},
    {0x22F4u, 0x22FCu},
    {0x22F6u, 0x22FDu},
    {0x22F7u, 0x22FEu},
    {0x22FAu, 0x22F2u},
    {0x22FBu, 0x22F3u},
    {0x22FCu, 0x22F4u},
    {0x22FDu, 0x22F6u},
    {0x22FEu, 0x22F7u},
    {0x2308u, 0x2309u},
    {0x2309u, 0x2308u},
    {0x230Au, 0x230Bu},
    {0x230Bu, 0x230Au},
    {0x2329u, 0x232Au},
    {0x232Au, 0x2329u},
    {0x2768u, 0x2769u},
    {0x2769u, 0x2768u},
    {0x276Au, 0x276Bu},
    {0x276Bu, 0x276Au},
    {0x276Cu, 0x276Du},
    {0x276Du, 0x276Cu},
    {0x276Eu, 0x276Fu},
    {0x276Fu, 0x276Eu},
    {0x2770u, 0x2771u},
    {0x2771u, 0x2770u},
    {0x2772u, 0x2773u},
    {0x2773u, 0x2772u},
    {0x2774u, 0x2775u},
    {0x2775u, 0x2774u},
    {0x27C3u, 0x27C4u},
    {0x27C4u, 0x27C3u},
    {0x27C5u, 0x27C6u},
    {0x27C6u, 0x27C5u},
    {0x27C8u, 0x27C9u},
    {0x27C9u, 0x27C8u},
    {0x27CBu, 0x27CDu},
    {0x27CDu, 0x27CBu},
    {0x27D5u, 0x27D6u},
    {0x27D6u, 0x27D5u},
    {0x27DCu, 0x22B8u},
    {0x27DDu, 0x27DEu},
    {0x27DEu, 0x27DDu},
    {0x27E2u, 0x27E3u},
    {0x27E3u, 0x27E2u},
    {0x27E4u, 0x27E5u},
    {0x27E5u, 0x27E4u},
    {0x27E6u, 0x27E7u},
    {0x27E7u, 0x27E6u},
    {0x27E8u, 0x27E9u},
    {0x27E9u, 0x27E8u},
    {0x27EAu, 0x27EBu},
    {0x27EBu, 0x27EAu},
    {0x27ECu, 0x27EDu},
    {0x27EDu, 0x27ECu},
    {0x27EEu, 0x27EFu},
    {0x27EFu, 0x27EEu},
    {0x2983u, 0x2984u},
    {0x2984u, 0x2983u},
    {0x2985u, 0x2986u},
    {0x2986u, 0x2985u},
    {0x2987u, 0x2988u},
    {0x2988u, 0x2987u},
    {0x2989u, 0x298Au},
    {0x298Au, 0x2989u},
    {0x298Bu, 0x298Cu},
    {0x298Cu, 0x298Bu},
    {0x298Du, 0x2990u},
    {0x298Eu, 0x298Fu},
    {0x298Fu, 0x298Eu},
    {0x2990u, 0x298Du},
    {0x2991u, 0x2992u},
    {0x2992u, 0x2991u},
    {0x2993u, 0x2994u},
    {0x2994u, 0x2993u},
    {0x2995u, 0x2996u},
    {0x2996u, 0x2995u},
    {0x2997u, 0x2998u},
    {0x2998u, 0x2997u},
    {0x299Bu, 0x2221u},
    {0x29A0u, 0x2222u},
    {0x29A3u, 0x2220u},
    {0x29A4u, 0x29A5u},
    {0x29A5u, 0x29A4u},
    {0x29A8u, 0x29A9u},
    {0x29A9u, 0x29A8u},
    {0x29AAu, 0x29ABu},
    {0x29ABu, 0x29AAu},
    {0x29ACu, 0x29ADu},
    {0x29ADu, 0x29ACu},
    {0x29AEu, 0x29AFu},
    {0x29AFu, 0x29AEu},
    {0x29B8u, 0x2298u},
    {0x29C0u, 0x29C1u},
    {0x29C1u, 0x29C0u},
    {0x29C4u, 0x29C5u},
    {0x29C5u, 0x29C4u},
    {0x29CFu, 0x29D0u},
    {0x29D0u, 0x29CFu},
    {0x29D1u, 0x29D2u},
    {0x29D2u, 0x29D1u},
    {0x29D4u, 0x29D5u},
    {0x29D5u, 0x29D4u},
    {0x29D8u, 0x29D9u},
    {0x29D9u, 0x29D8u},
    {0x29DAu, 0x29DBu},
    {0x29DBu, 0x29DAu},
    {0x29E8u, 0x29E9u},
    {0x29E9u, 0x29E8u},
    {0x29F5u, 0x2215u},
    {0x29F8u, 0x29F9u},
    {0x29F9u, 0x29F8u},
    {0x29FCu, 0x29FDu},
    {0x29FDu, 0x29FCu},
    {0x2A2Bu, 0x2A2Cu},
    {0x2A2Cu, 0x2A2Bu},
    {0x2A2Du, 0x2A2Eu},
    {0x2A2Eu, 0x2A2Du},
    {0x2A34u, 0x2A35u},
    {0x2A35u, 0x2A34u},
    {0x2A3Cu, 0x2A3Du},
    {0x2A3Du, 0x2A3Cu},
    {0x2A64u, 0x2A65u},
    {0x2A65u, 0x2A64u},
    {0x2A79u, 0x2A7Au},
    {0x2A7Au, 0x2A79u},
    {0x2A7Bu, 0x2A7Cu},
    {0x2A7Cu, 0x2A7Bu},
    {0x2A7Du, 0x2A7Eu},
    {0x2A7Eu, 0x2A7Du},
    {0x2A7Fu, 0x2A80u},
    {0x2A80u, 0x2A7Fu},
    {0x2A81u, 0x2A82u},
    {0x2A82u, 0x2A81u},
    {0x2A83u, 0x2A84u},
    {0x2A84u, 0x2A83u},
    {0x2A85u, 0x2A86u},
    {0x2A86u, 0x2A85u},
    {0x2A87u, 0x2A88u},
    {0x2A88u, 0x2A87u},
    {0x2A89u, 0x2A8Au},
    {0x2A8Au, 0x2A89u},
    {0x2A8Bu, 0x2A8Cu},
    {0x2A8Cu, 0x2A8Bu},
    {0x2A8Du, 0x2A8Eu},
    {0x2A8Eu, 0x2A8Du},
    {0x2A8Fu, 0x2A90u},
    {0x2A90u, 0x2A8Fu},
    {0x2A91u, 0x2A92u},
    {0x2A92u, 0x2A91u},
    {0x2A93u, 0x2A94u},
    {0x2A94u, 0x2A93u},
    {0x2A95u, 0x2A96u},
    {0x2A96u, 0x2A95u},
    {0x2A97u, 0x2A98u},
    {0x2A98u, 0x2A97u},
    {0x2A99u, 0x2A9Au},
    {0x2A9Au, 0x2A99u},
    {0x2A9Bu, 0x2A9Cu},
    {0x2A9Cu, 0x2A9Bu},
    {0x2A9Du, 0x2A9Eu},
    {0x2A9Eu, 0x2A9Du},
    {0x2A9Fu, 0x2AA0u},
    {0x2AA0u, 0x2A9Fu},
    {0x2AA1u, 0x2AA2u},
    {0x2AA2u, 0x2AA1u},
    {0x2AA6u, 0x2AA7u},
    {0x2AA7u, 0x2AA6u},
    {0x2AA8u, 0x2AA9u},
    {0x2AA9u, 0x2AA8u},
    {0x2AAAu, 0x2AABu},
    {0x2AABu, 0x2AAAu},
    {0x2AACu, 0x2AADu},
    {0x2AADu, 0x2AACu},
    {0x2AAFu, 0x2AB0u},
    {0x2AB0u, 0x2AAFu},
    {0x2AB1u, 0x2AB2u},
    {0x2AB2u, 0x2AB1u},
    {0x2AB3u, 0x2AB4u},
    {0x2AB4u, 0x2AB3u},
    {0x2AB5u, 0x2AB6u},
    {0x2AB6u, 0x2AB5u},
    {0x2AB7u, 0x2AB8u},
    {0x2AB8u, 0x2AB7u},
    {0x2AB9u, 0x2ABAu},
    {0x2ABAu, 0x2AB9u},
    {0x2ABBu, 0x2ABCu},
    {0x2ABCu, 0x2ABBu},
    {0x2ABDu, 0x2ABEu},
    {0x2ABEu, 0x2ABDu},
    {0x2ABFu, 0x2AC0u},
    {0x2AC0u, 0x2ABFu},
    {0x2AC1u, 0x2AC2u},
    {0x2AC2u, 0x2AC1u},
    {0x2AC3u, 0x2AC4u},
    {0x2AC4u, 0x2AC3u},
    {0x2AC5u, 0x2AC6u},
    {0x2AC6u, 0x2AC5u},
    {0x2AC7u, 0x2AC8u},
    {0x2AC8u, 0x2AC7u},
    {0x2AC9u, 0x2ACAu},
    {0x2ACAu, 0x2AC9u},
    {0x2ACBu, 0x2ACCu},
    {0x2ACCu, 0x2ACBu},
    {0x2ACDu, 0x2ACEu},
    {0x2ACEu, 0x2ACDu},
    {0x2ACFu, 0x2AD0u},
    {0x2AD0u, 0x2ACFu},
    {0x2AD1u, 0x2AD2u},
    {0x2AD2u, 0x2AD1u},
    {0x2AD3u, 0x2AD4u},
    {0x2AD4u, 0x2AD3u},
    {0x2AD5u, 0x2AD6u},
    {0x2AD6u, 0x2AD5u},
    {0x2ADEu, 0x22A6u},
    {0x2AE3u, 0x22A9u},
    {0x2AE4u, 0x22A8u},
    {0x2AE5u, 0x22ABu},
    {0x2AECu, 0x2AEDu},
    {0x2AEDu, 0x2AECu},
    {0x2AEEu, 0x2224u},
    {0x2AF7u, 0x2AF8u},
    {0x2AF8u, 0x2AF7u},
    {0x2AF9u, 0x2AFAu},
    {0x2AFAu, 0x2AF9u},
    {0x2BFEu, 0x221Fu},
    {0x2E02u, 0x2E03u},
    {0x2E03u, 0x2E02u},
    {0x2E04u, 0x2E05u},
    {0x2E05u, 0x2E04u},
    {0x2E09u, 0x2E0Au},
    {0x2E0Au, 0x2E09u},
    {0x2E0Cu, 0x2E0Du},
    {0x2E0Du, 0x2E0Cu},
    {0x2E1Cu, 0x2E1Du},
    {0x2E1Du, 0x2E1Cu},
    {0x2E20u, 0x2E21u},
    {0x2E21u, 0x2E20u},
    {0x2E22u, 0x2E23u},
    {0x2E23u, 0x2E22u},
    {0x2E24u, 0x2E25u},
    {0x2E25u, 0x2E24u},
    {0x2E26u, 0x2E27u},
    {0x2E27u, 0x2E26u},
    {0x2E28u, 0x2E29u},
    {0x2E29u, 0x2E28u},
    {0x2E55u, 0x2E56u},
    {0x2E56u, 0x2E55u},
    {0x2E57u, 0x2E58u},
    {0x2E58u, 0x2E57u},
    {0x2E59u, 0x2E5Au},
    {0x2E5Au, 0x2E59u},
    {0x2E5Bu, 0x2E5Cu},
    {0x2E5Cu, 0x2E5Bu},
    {0x2E62u, 0x2E63u},
    {0x2E63u, 0x2E62u},
    {0x3008u, 0x3009u},
    {0x3009u, 0x3008u},
    {0x300Au, 0x300Bu},
    {0x300Bu, 0x300Au},
    {0x300Cu, 0x300Du},
    {0x300Du, 0x300Cu},
    {0x300Eu, 0x300Fu},
    {0x300Fu, 0x300Eu},
    {0x3010u, 0x3011u},
    {0x3011u, 0x3010u},
    {0x3014u, 0x3015u},
    {0x3015u, 0x3014u},
    {0x3016u, 0x3017u},
    {0x3017u, 0x3016u},
    {0x3018u, 0x3019u},
    {0x3019u, 0x3018u},
    {0x301Au, 0x301Bu},
    {0x301Bu, 0x301Au},
    {0xFE59u, 0xFE5Au},
    {0xFE5Au, 0xFE59u},
    {0xFE5Bu, 0xFE5Cu},
    {0xFE5Cu, 0xFE5Bu},
    {0xFE5Du, 0xFE5Eu},
    {0xFE5Eu, 0xFE5Du},
    {0xFE64u, 0xFE65u},
    {0xFE65u, 0xFE64u},
    {0xFF08u, 0xFF09u},
    {0xFF09u, 0xFF08u},
    {0xFF1Cu, 0xFF1Eu},
    {0xFF1Eu, 0xFF1Cu},
    {0xFF3Bu, 0xFF3Du},
    {0xFF3Du, 0xFF3Bu},
    {0xFF5Bu, 0xFF5Du},
    {0xFF5Du, 0xFF5Bu},
    {0xFF5Fu, 0xFF60u},
    {0xFF60u, 0xFF5Fu},
    {0xFF62u, 0xFF63u},
    {0xFF63u, 0xFF62u},
    {0x1DB03u, 0x1DB04u},
    {0x1DB04u, 0x1DB03u},
    {0x1DB05u, 0x1DB06u},
    {0x1DB06u, 0x1DB05u},
    {0x1DB08u, 0x1DB09u},
    {0x1DB09u, 0x1DB08u},
    {0x1DB10u, 0x221Du},
};

static const bracket_pair_t k_brackets[] = {
    {0x0028u, 0x0029u, 'o'},
    {0x0029u, 0x0028u, 'c'},
    {0x005Bu, 0x005Du, 'o'},
    {0x005Du, 0x005Bu, 'c'},
    {0x007Bu, 0x007Du, 'o'},
    {0x007Du, 0x007Bu, 'c'},
    {0x0F3Au, 0x0F3Bu, 'o'},
    {0x0F3Bu, 0x0F3Au, 'c'},
    {0x0F3Cu, 0x0F3Du, 'o'},
    {0x0F3Du, 0x0F3Cu, 'c'},
    {0x169Bu, 0x169Cu, 'o'},
    {0x169Cu, 0x169Bu, 'c'},
    {0x2045u, 0x2046u, 'o'},
    {0x2046u, 0x2045u, 'c'},
    {0x207Du, 0x207Eu, 'o'},
    {0x207Eu, 0x207Du, 'c'},
    {0x208Du, 0x208Eu, 'o'},
    {0x208Eu, 0x208Du, 'c'},
    {0x2308u, 0x2309u, 'o'},
    {0x2309u, 0x2308u, 'c'},
    {0x230Au, 0x230Bu, 'o'},
    {0x230Bu, 0x230Au, 'c'},
    {0x2329u, 0x232Au, 'o'},
    {0x232Au, 0x2329u, 'c'},
    {0x2768u, 0x2769u, 'o'},
    {0x2769u, 0x2768u, 'c'},
    {0x276Au, 0x276Bu, 'o'},
    {0x276Bu, 0x276Au, 'c'},
    {0x276Cu, 0x276Du, 'o'},
    {0x276Du, 0x276Cu, 'c'},
    {0x276Eu, 0x276Fu, 'o'},
    {0x276Fu, 0x276Eu, 'c'},
    {0x2770u, 0x2771u, 'o'},
    {0x2771u, 0x2770u, 'c'},
    {0x2772u, 0x2773u, 'o'},
    {0x2773u, 0x2772u, 'c'},
    {0x2774u, 0x2775u, 'o'},
    {0x2775u, 0x2774u, 'c'},
    {0x27C5u, 0x27C6u, 'o'},
    {0x27C6u, 0x27C5u, 'c'},
    {0x27E6u, 0x27E7u, 'o'},
    {0x27E7u, 0x27E6u, 'c'},
    {0x27E8u, 0x27E9u, 'o'},
    {0x27E9u, 0x27E8u, 'c'},
    {0x27EAu, 0x27EBu, 'o'},
    {0x27EBu, 0x27EAu, 'c'},
    {0x27ECu, 0x27EDu, 'o'},
    {0x27EDu, 0x27ECu, 'c'},
    {0x27EEu, 0x27EFu, 'o'},
    {0x27EFu, 0x27EEu, 'c'},
    {0x2983u, 0x2984u, 'o'},
    {0x2984u, 0x2983u, 'c'},
    {0x2985u, 0x2986u, 'o'},
    {0x2986u, 0x2985u, 'c'},
    {0x2987u, 0x2988u, 'o'},
    {0x2988u, 0x2987u, 'c'},
    {0x2989u, 0x298Au, 'o'},
    {0x298Au, 0x2989u, 'c'},
    {0x298Bu, 0x298Cu, 'o'},
    {0x298Cu, 0x298Bu, 'c'},
    {0x298Du, 0x2990u, 'o'},
    {0x298Eu, 0x298Fu, 'c'},
    {0x298Fu, 0x298Eu, 'o'},
    {0x2990u, 0x298Du, 'c'},
    {0x2991u, 0x2992u, 'o'},
    {0x2992u, 0x2991u, 'c'},
    {0x2993u, 0x2994u, 'o'},
    {0x2994u, 0x2993u, 'c'},
    {0x2995u, 0x2996u, 'o'},
    {0x2996u, 0x2995u, 'c'},
    {0x2997u, 0x2998u, 'o'},
    {0x2998u, 0x2997u, 'c'},
    {0x29D8u, 0x29D9u, 'o'},
    {0x29D9u, 0x29D8u, 'c'},
    {0x29DAu, 0x29DBu, 'o'},
    {0x29DBu, 0x29DAu, 'c'},
    {0x29FCu, 0x29FDu, 'o'},
    {0x29FDu, 0x29FCu, 'c'},
    {0x2E22u, 0x2E23u, 'o'},
    {0x2E23u, 0x2E22u, 'c'},
    {0x2E24u, 0x2E25u, 'o'},
    {0x2E25u, 0x2E24u, 'c'},
    {0x2E26u, 0x2E27u, 'o'},
    {0x2E27u, 0x2E26u, 'c'},
    {0x2E28u, 0x2E29u, 'o'},
    {0x2E29u, 0x2E28u, 'c'},
    {0x2E55u, 0x2E56u, 'o'},
    {0x2E56u, 0x2E55u, 'c'},
    {0x2E57u, 0x2E58u, 'o'},
    {0x2E58u, 0x2E57u, 'c'},
    {0x2E59u, 0x2E5Au, 'o'},
    {0x2E5Au, 0x2E59u, 'c'},
    {0x2E5Bu, 0x2E5Cu, 'o'},
    {0x2E5Cu, 0x2E5Bu, 'c'},
    {0x2E62u, 0x2E63u, 'o'},
    {0x2E63u, 0x2E62u, 'c'},
    {0x3008u, 0x3009u, 'o'},
    {0x3009u, 0x3008u, 'c'},
    {0x300Au, 0x300Bu, 'o'},
    {0x300Bu, 0x300Au, 'c'},
    {0x300Cu, 0x300Du, 'o'},
    {0x300Du, 0x300Cu, 'c'},
    {0x300Eu, 0x300Fu, 'o'},
    {0x300Fu, 0x300Eu, 'c'},
    {0x3010u, 0x3011u, 'o'},
    {0x3011u, 0x3010u, 'c'},
    {0x3014u, 0x3015u, 'o'},
    {0x3015u, 0x3014u, 'c'},
    {0x3016u, 0x3017u, 'o'},
    {0x3017u, 0x3016u, 'c'},
    {0x3018u, 0x3019u, 'o'},
    {0x3019u, 0x3018u, 'c'},
    {0x301Au, 0x301Bu, 'o'},
    {0x301Bu, 0x301Au, 'c'},
    {0xFE59u, 0xFE5Au, 'o'},
    {0xFE5Au, 0xFE59u, 'c'},
    {0xFE5Bu, 0xFE5Cu, 'o'},
    {0xFE5Cu, 0xFE5Bu, 'c'},
    {0xFE5Du, 0xFE5Eu, 'o'},
    {0xFE5Eu, 0xFE5Du, 'c'},
    {0xFF08u, 0xFF09u, 'o'},
    {0xFF09u, 0xFF08u, 'c'},
    {0xFF3Bu, 0xFF3Du, 'o'},
    {0xFF3Du, 0xFF3Bu, 'c'},
    {0xFF5Bu, 0xFF5Du, 'o'},
    {0xFF5Du, 0xFF5Bu, 'c'},
    {0xFF5Fu, 0xFF60u, 'o'},
    {0xFF60u, 0xFF5Fu, 'c'},
    {0xFF62u, 0xFF63u, 'o'},
    {0xFF63u, 0xFF62u, 'c'},
};

static bool pair_lookup_mirror(uint32_t cp, uint32_t* out) {
    size_t lo = 0, hi = sizeof(k_mirroring) / sizeof(k_mirroring[0]);
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (cp < k_mirroring[mid].cp) hi = mid;
        else if (cp > k_mirroring[mid].cp) lo = mid + 1;
        else { *out = k_mirroring[mid].mirrored; return true; }
    }
    return false;
}

static bool pair_lookup_bracket(uint32_t cp, uint32_t* counterpart, char* kind) {
    size_t lo = 0, hi = sizeof(k_brackets) / sizeof(k_brackets[0]);
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (cp < k_brackets[mid].cp) hi = mid;
        else if (cp > k_brackets[mid].cp) lo = mid + 1;
        else {
            if (counterpart) *counterpart = k_brackets[mid].counterpart;
            if (kind) *kind = k_brackets[mid].kind;
            return true;
        }
    }
    return false;
}

uint8_t unicodoid_uax9_bidi_class(uint32_t cp) {
    return unicodoid_ucd_bidi_class(cp);
}

static bool is_strong_l(uint8_t c) {
    return c == (uint8_t)UNICODOID_BIDI_L;
}

static bool is_strong_r(uint8_t c) {
    return c == (uint8_t)UNICODOID_BIDI_R || c == (uint8_t)UNICODOID_BIDI_AL;
}

static bool is_strong(uint8_t c) { return is_strong_l(c) || is_strong_r(c); }

uint8_t unicodoid_uax9_paragraph_level(
    const uint32_t* cps, size_t length, int base_dir) {
    if (base_dir == (int)UNICODOID_BIDI_RTL) return 1;
    if (base_dir == (int)UNICODOID_BIDI_LTR) return 0;
    /* P2: first strong character determines the paragraph direction. */
    if (cps) {
        for (size_t i = 0; i < length; ++i) {
            uint8_t c = unicodoid_ucd_bidi_class(cps[i]);
            if (is_strong_r(c)) return 1;
            if (is_strong_l(c)) return 0;
        }
    }
    return 0; /* P3: default LTR. */
}

static bool is_explicit_open(uint8_t c) {
    return c == (uint8_t)UNICODOID_BIDI_LRE || c == (uint8_t)UNICODOID_BIDI_LRO ||
           c == (uint8_t)UNICODOID_BIDI_RLE || c == (uint8_t)UNICODOID_BIDI_RLO;
}

static bool is_explicit_close(uint8_t c) {
    return c == (uint8_t)UNICODOID_BIDI_PDF;
}

static bool is_isolate(uint8_t c) {
    return c == (uint8_t)UNICODOID_BIDI_FSI || c == (uint8_t)UNICODOID_BIDI_LRI ||
           c == (uint8_t)UNICODOID_BIDI_RLI || c == (uint8_t)UNICODOID_BIDI_PDI;
}

static bool is_term(uint8_t c) {
    return c == (uint8_t)UNICODOID_BIDI_B || c == (uint8_t)UNICODOID_BIDI_S;
}

/* Bracket-pair stack entry for N0 (levels 0..125 nesting). */
#define BIDI_MAX_DEPTH 125

/* Resolve levels for one run.  Returns malloc'd array or NULL on OOM. */
static uint8_t* resolve_alloc(const uint32_t* cps, size_t length, int base_dir) {
    if (length == 0) return NULL;
    uint8_t* cls = (uint8_t*)malloc(length);
    uint8_t* lvl = (uint8_t*)malloc(length);
    if (!cls || !lvl) { free(cls); free(lvl); return NULL; }
    for (size_t i = 0; i < length; ++i) cls[i] = unicodoid_ucd_bidi_class(cps[i]);

    /* P1/X9 preparation: B/S act as paragraph/segment separators.  For a
     * single-paragraph API, trailing/trailing separators get level 0 and are
     * reordered as boundaries by the caller.  BN is removed by assigning it a
     * level that lets L2 skip it; we keep positions (no shifting) and let
     * visual ordering skip BN inputs. */
    uint8_t para = unicodoid_uax9_paragraph_level(cps, length, base_dir);
    for (size_t i = 0; i < length; ++i) lvl[i] = para;

    /* W1: NSM -> preceding strong, else ON.  Scan with last strong cache. */
    for (size_t i = 0; i < length; ++i) {
        if (cls[i] != (uint8_t)UNICODOID_BIDI_NSM) continue;
        uint8_t resolved = (uint8_t)UNICODOID_BIDI_ON;
        /* Find preceding non-NSM/BN character. */
        size_t j = i;
        bool found = false;
        while (j > 0) {
            --j;
            if (cls[j] == (uint8_t)UNICODOID_BIDI_NSM ||
                cls[j] == (uint8_t)UNICODOID_BIDI_BN) continue;
            resolved = cls[j];
            found = true;
            break;
        }
        (void)found;
        if (resolved == (uint8_t)UNICODOID_BIDI_BN) resolved = (uint8_t)UNICODOID_BIDI_ON;
        cls[i] = resolved;
    }

    /* W2: search backwards skipping EN/ES/ET/AN/CS for R/AL. */
    for (size_t i = 0; i < length; ++i) {
        if (cls[i] != (uint8_t)UNICODOID_BIDI_EN) continue;
        for (size_t j = i; j > 0; --j) {
            uint8_t b = cls[j - 1];
            if (b == (uint8_t)UNICODOID_BIDI_EN || b == (uint8_t)UNICODOID_BIDI_ES ||
                b == (uint8_t)UNICODOID_BIDI_ET || b == (uint8_t)UNICODOID_BIDI_AN ||
                b == (uint8_t)UNICODOID_BIDI_CS) continue;
            if (b == (uint8_t)UNICODOID_BIDI_R || b == (uint8_t)UNICODOID_BIDI_AL)
                cls[i] = (uint8_t)UNICODOID_BIDI_AN;
            break;
        }
    }

    /* W3: AL -> R. */
    for (size_t i = 0; i < length; ++i)
        if (cls[i] == (uint8_t)UNICODOID_BIDI_AL) cls[i] = (uint8_t)UNICODOID_BIDI_R;

    /* W4: single ES between EN -> EN; single CS between N -> N. */
    for (size_t i = 1; i + 1 < length; ++i) {
        if (cls[i] == (uint8_t)UNICODOID_BIDI_ES &&
            cls[i-1] == (uint8_t)UNICODOID_BIDI_EN &&
            cls[i+1] == (uint8_t)UNICODOID_BIDI_EN) cls[i] = (uint8_t)UNICODOID_BIDI_EN;
        if (cls[i] == (uint8_t)UNICODOID_BIDI_CS) {
            bool lp = cls[i-1] == (uint8_t)UNICODOID_BIDI_EN ||
                      cls[i-1] == (uint8_t)UNICODOID_BIDI_AN;
            bool rp = cls[i+1] == (uint8_t)UNICODOID_BIDI_EN ||
                      cls[i+1] == (uint8_t)UNICODOID_BIDI_AN;
            if (lp && rp) cls[i] = (uint8_t)UNICODOID_BIDI_EN;
        }
    }

    /* W5: ET adjacent to EN (search skipping ET) -> EN. */
    for (size_t i = 0; i < length; ++i) {
        if (cls[i] != (uint8_t)UNICODOID_BIDI_ET) continue;
        bool near_en = false;
        size_t j = i;
        while (j > 0) {
            --j;
            if (cls[j] == (uint8_t)UNICODOID_BIDI_ET) continue;
            near_en = (cls[j] == (uint8_t)UNICODOID_BIDI_EN);
            break;
        }
        if (!near_en) {
            j = i + 1;
            while (j < length) {
                if (cls[j] == (uint8_t)UNICODOID_BIDI_ET) { ++j; continue; }
                near_en = (cls[j] == (uint8_t)UNICODOID_BIDI_EN);
                break;
            }
        }
        if (near_en) cls[i] = (uint8_t)UNICODOID_BIDI_EN;
    }

    /* W6: remaining ES/ET/CS -> ON. */
    for (size_t i = 0; i < length; ++i) {
        if (cls[i] == (uint8_t)UNICODOID_BIDI_ES ||
            cls[i] == (uint8_t)UNICODOID_BIDI_ET ||
            cls[i] == (uint8_t)UNICODOID_BIDI_CS) cls[i] = (uint8_t)UNICODOID_BIDI_ON;
    }

    /* W7: EN -> L when preceded by L (search skipping WS/ES/ET). */
    for (size_t i = 0; i < length; ++i) {
        if (cls[i] != (uint8_t)UNICODOID_BIDI_EN) continue;
        for (size_t j = i; j > 0; --j) {
            uint8_t b = cls[j - 1];
            if (b == (uint8_t)UNICODOID_BIDI_WS || b == (uint8_t)UNICODOID_BIDI_ES ||
                b == (uint8_t)UNICODOID_BIDI_ET) continue;
            if (b == (uint8_t)UNICODOID_BIDI_L) cls[i] = (uint8_t)UNICODOID_BIDI_L;
            break;
        }
    }

    /* N0: paired brackets.  Build matching pairs with a stack and give the
     * pair a type (L/R/ON) from the enclosing strong type.  Simplified but
     * faithful for non-nested and singly-nested cases. */
    {
        /* bracket stack of indices */
        size_t stack[256];
        int sp = 0;
        for (size_t i = 0; i < length; ++i) {
            uint32_t counterpart = 0; char kind = 0;
            if (!pair_lookup_bracket(cps[i], &counterpart, &kind)) continue;
            if (kind == 'o') {
                if (sp < 256) stack[sp++] = i;
            } else {
                if (sp <= 0) continue;
                /* Find nearest unmatched opener whose counterpart matches. */
                int found = -1;
                for (int k = sp - 1; k >= 0; --k) {
                    uint32_t open_counter = 0; char ok = 0;
                    if (pair_lookup_bracket(cps[stack[k]], &open_counter, &ok) &&
                        open_counter == cps[i]) { found = k; break; }
                }
                if (found < 0) continue;
                size_t open_idx = stack[found];
                /* Remove stack entries up to and including found (nested
                 * unmatched openers are discarded per N0). */
                sp = found;
                (void)sp;
                /* Determine bracket type from inner strong characters. */
                uint8_t inner_type = (uint8_t)UNICODOID_BIDI_ON;
                for (size_t k = open_idx + 1; k < i; ++k) {
                    uint8_t c = cls[k];
                    if (is_strong_r(c)) { inner_type = (uint8_t)UNICODOID_BIDI_R; break; }
                    if (is_strong_l(c)) { inner_type = (uint8_t)UNICODOID_BIDI_L; break; }
                }
                /* Convert pair from ON to inner_type when inner is strong;
                 * otherwise keep ON (N0 rules for enclosing type). */
                if (inner_type != (uint8_t)UNICODOID_BIDI_ON) {
                    cls[open_idx] = inner_type;
                    cls[i] = inner_type;
                }
                /* Pop processed entries above found (keep sp at found). */
                sp = found;
                if (sp < 256) {
                    /* continue with remaining stack below found untouched */
                }
                (void)0;
            }
        }
    }

    /* N1: neutrals between same strong (or EN/AN as L) take that type. */
    for (size_t i = 0; i < length; ++i) {
        if (cls[i] != (uint8_t)UNICODOID_BIDI_ON) continue;
        uint8_t left = para ? (uint8_t)UNICODOID_BIDI_R : (uint8_t)UNICODOID_BIDI_L;
        for (size_t j = i; j > 0; --j) {
            uint8_t b = cls[j - 1];
            if (b == (uint8_t)UNICODOID_BIDI_ON) continue;
            if (b == (uint8_t)UNICODOID_BIDI_EN || b == (uint8_t)UNICODOID_BIDI_AN)
                left = (uint8_t)UNICODOID_BIDI_L;
            else if (b == (uint8_t)UNICODOID_BIDI_L || b == (uint8_t)UNICODOID_BIDI_R)
                left = b;
            break;
        }
        uint8_t right = para ? (uint8_t)UNICODOID_BIDI_R : (uint8_t)UNICODOID_BIDI_L;
        for (size_t j = i + 1; j < length; ++j) {
            uint8_t b = cls[j];
            if (b == (uint8_t)UNICODOID_BIDI_ON) continue;
            if (b == (uint8_t)UNICODOID_BIDI_EN || b == (uint8_t)UNICODOID_BIDI_AN)
                right = (uint8_t)UNICODOID_BIDI_L;
            else if (b == (uint8_t)UNICODOID_BIDI_L || b == (uint8_t)UNICODOID_BIDI_R)
                right = b;
            break;
        }
        if (left == right) cls[i] = left;
    }

    /* N2: remaining neutrals take paragraph type. */
    for (size_t i = 0; i < length; ++i)
        if (cls[i] == (uint8_t)UNICODOID_BIDI_ON)
            cls[i] = para ? (uint8_t)UNICODOID_BIDI_R : (uint8_t)UNICODOID_BIDI_L;

    /* I1/I2 + X1..X10: assign levels.  Embedding stack follows X1..X8;
     * isolates (X9c) are treated as neutral at their level for this
     * implementation (isolate contents are resolved independently by the
     * caller per paragraph). */
    {
        int delta = 0;
        /* Odd/even tracking for X1 embedding base. */
        /* We walk left to right maintaining embedding type stack. */
        uint8_t emb_stack[BIDI_MAX_DEPTH];
        int emb_sp = 0;
        /* Track explicit embedding current type: 0 L, 1 R. */
        /* embedding base level is para + delta; isolate handling simplified. */
        for (size_t i = 0; i < length; ++i) {
            uint8_t c = cls[i];
            int cur_base = para + 2 * delta;
            if (cur_base < 0) cur_base = 0;
            if (cur_base > BIDI_MAX_DEPTH) cur_base = BIDI_MAX_DEPTH;

            if (c == (uint8_t)UNICODOID_BIDI_LRE || c == (uint8_t)UNICODOID_BIDI_RLE) {
                /* X1/X2: push embedding, increment for new level. */
                if (emb_sp < BIDI_MAX_DEPTH) emb_stack[emb_sp++] = (uint8_t)(para + 2 * delta);
                delta++;
                lvl[i] = (uint8_t)cur_base;
                continue;
            }
            if (c == (uint8_t)UNICODOID_BIDI_LRO || c == (uint8_t)UNICODOID_BIDI_RLO) {
                if (emb_sp < BIDI_MAX_DEPTH) emb_stack[emb_sp++] = (uint8_t)(para + 2 * delta);
                delta++;
                lvl[i] = (uint8_t)cur_base;
                continue;
            }
            if (c == (uint8_t)UNICODOID_BIDI_PDF) {
                if (emb_sp > 0) --emb_sp;
                if (delta > 0) --delta;
                lvl[i] = (uint8_t)cur_base;
                continue;
            }
            if (is_isolate(c)) {
                /* FSI/LRI/RLI/PDI: assign level, no change to delta for
                 * this simplified implementation (caller should resolve
                 * isolate contents as sub-paragraphs). */
                lvl[i] = (uint8_t)cur_base;
                continue;
            }
            if (is_term(c)) {
                lvl[i] = (uint8_t)para;
                continue;
            }
            if (c == (uint8_t)UNICODOID_BIDI_BN) {
                /* X9: BN takes surrounding level (keep current). */
                lvl[i] = (uint8_t)cur_base;
                continue;
            }
            bool odd = (cur_base % 2) == 1;
            bool is_r = (c == (uint8_t)UNICODOID_BIDI_R ||
                         c == (uint8_t)UNICODOID_BIDI_AN);
            bool is_l = (c == (uint8_t)UNICODOID_BIDI_L ||
                         c == (uint8_t)UNICODOID_BIDI_EN);
            if (is_r) lvl[i] = (uint8_t)(odd ? cur_base : cur_base + 1);
            else if (is_l) lvl[i] = (uint8_t)(odd ? cur_base + 1 : cur_base);
            else lvl[i] = (uint8_t)cur_base; /* WS treated like L/R by I1/I2 via N2 already resolved */
            if (c == (uint8_t)UNICODOID_BIDI_WS) {
                if (is_r) lvl[i] = (uint8_t)(odd ? cur_base : cur_base + 1);
                /* otherwise previous assignment stands */
            }
        }
    }

    free(cls);
    return lvl;
}

size_t unicodoid_uax9_resolve_levels(
    const uint32_t* cps, size_t length, int base_dir,
    uint8_t* out, size_t cap) {
    if (!cps || length == 0) return 0;
    if (out == NULL && cap != 0) return length; /* defensive: query */
    uint8_t* tmp = resolve_alloc(cps, length, base_dir);
    if (!tmp) return length;
    if (out) {
        size_t n = length < cap ? length : cap;
        memcpy(out, tmp, n);
    }
    free(tmp);
    return length;
}

size_t unicodoid_uax9_visual_order(
    const uint32_t* cps, size_t length, int base_dir,
    uint32_t* out, size_t cap) {
    if (!cps || length == 0) return 0;
    uint8_t* lvl = resolve_alloc(cps, length, base_dir);
    if (!lvl) return length;
    /* L1: reset B/S levels to paragraph level. */
    uint8_t para = unicodoid_uax9_paragraph_level(cps, length, base_dir);
    for (size_t i = 0; i < length; ++i) {
        uint8_t c = unicodoid_ucd_bidi_class(cps[i]);
        if (c == (uint8_t)UNICODOID_BIDI_B || c == (uint8_t)UNICODOID_BIDI_S)
            lvl[i] = para;
    }
    /* L2: reverse each maximal contiguous run with level >= K for K from
     * max down to lowest odd.  Then L3/L4 mirror odd-level characters. */
    size_t idx[4096];
    /* Fallback dynamic array for long paragraphs. */
    size_t* order = (size_t*)malloc(length * sizeof(size_t));
    if (!order) { free(lvl); return length; }
    for (size_t i = 0; i < length; ++i) order[i] = i;

    uint8_t max_l = 0, min_odd = 0;
    bool has_odd = false;
    for (size_t i = 0; i < length; ++i) {
        if (lvl[i] > max_l) max_l = lvl[i];
        if ((lvl[i] % 2) == 1) { has_odd = true; if (!min_odd || lvl[i] < min_odd) min_odd = lvl[i]; }
    }
    (void)idx;
    if (has_odd) {
        for (int k = (int)max_l; k >= (int)min_odd; --k) {
            size_t i = 0;
            while (i < length) {
                if (lvl[order[i]] < (uint8_t)k) { ++i; continue; }
                size_t j = i;
                while (j < length && lvl[order[j]] >= (uint8_t)k) ++j;
                /* reverse order[i..j) */
                size_t a = i, b = j;
                while (a < b) { --b; if (a >= b) break; size_t t = order[a]; order[a] = order[b]; order[b] = t; ++a; }
                i = j;
            }
        }
    }

    size_t total = length;
    if (out) {
        size_t n = total < cap ? total : cap;
        for (size_t i = 0; i < n; ++i) {
            uint32_t cp = cps[order[i]];
            if ((lvl[order[i]] % 2) == 1) {
                uint32_t mir = cp;
                if (pair_lookup_mirror(cp, &mir)) cp = mir;
            }
            out[i] = cp;
        }
    }
    free(order);
    free(lvl);
    return total;
}

bool unicodoid_uax9_is_mirrored(uint32_t cp) {
    uint32_t tmp = 0;
    return pair_lookup_mirror(cp, &tmp);
}

uint32_t unicodoid_uax9_mirrored_glyph(uint32_t cp) {
    uint32_t out = cp;
    pair_lookup_mirror(cp, &out);
    return out;
}

bool unicodoid_uax9_is_paired_bracket(uint32_t cp) {
    return pair_lookup_bracket(cp, NULL, NULL);
}
