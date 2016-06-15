// Copyright (c) 2011-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "arith_uint256.h"
#include "consensus/validation.h"
#include "main.h"
#include "miner.h"
#include "pubkey.h"
#include "uint256.h"
#include "util.h"
#include "crypto/equihash.h"

#include "test/test_bitcoin.h"

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(miner_tests, TestingSetup)

#define NUM_EQUIHASH_SOLUTIONS 8 // k = 3

static
struct {
    const char *nonce_hex;
    uint32_t vSolutions[NUM_EQUIHASH_SOLUTIONS];
} blockinfo[] = {
    {"0000000000000000000000000000000000000000000000000000000000000000", {6460681,13237162,7709584,31045419,10285519,17375614,15570856,18802679,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {4561870,17803565,5142864,27252551,4878460,12303443,7865598,11300160,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {1089651,25039626,8810095,9296274,3249918,29008340,24645847,26512131,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {7728372,28861696,9545832,19144166,17556589,24119335,21961452,31035364,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {834238,9184343,13243668,23166288,7981278,26794117,14325463,14669701,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {734172,13569750,3562648,29145860,10283866,23063716,13248910,16359387,}},
    {"0000000000000000000000000000000000000000000000000000000000000002", {1587871,17811157,12023427,21207538,5720315,7856396,7103244,24229710,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {1888147,27449466,13224507,14518313,15963469,22950485,17952156,24720854,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {4302677,18772320,9863528,19410554,12176883,14140896,25924630,26251701,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {197279,32404497,11172719,18080543,15487758,28660859,31764039,32249219,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {4024821,20999820,8894371,30988205,7275560,30827261,9405363,21175857,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {1976935,18657882,12319377,33437938,2895318,8491699,8562465,9766831,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {1205583,22738706,10435172,22796265,16651295,26452611,17851968,21396176,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {4703343,12058656,8341840,9642250,12545365,23735257,29717152,30858347,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {692337,24190308,21647683,23770044,6914318,7096840,20965362,26417921,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {474315,31656164,6219098,28125716,5816881,15195781,13568953,29055883,}},
    {"0000000000000000000000000000000000000000000000000000000000000002", {31517,29912071,8389728,8760739,8901626,26743223,10511308,31784346,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {2869946,23991756,8426881,20564925,5642907,26127833,12441891,22131781,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {5580020,20830652,29410091,32809082,15349511,19331016,18657410,26534999,}},
    {"0000000000000000000000000000000000000000000000000000000000000002", {705535,33235971,1184462,26966580,1849828,26167591,18261528,19642580,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {360784,3620708,15346909,27811152,3736810,12736765,9492651,13418606,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {1229600,7684048,20079597,24040691,10128962,30769642,15215479,21182096,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {1684299,12541147,11247589,28636814,5760908,24056463,13771233,20156600,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {366615,1724571,10566227,11511903,17264833,25873819,22389268,27114210,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {2536004,10826889,5554022,25567653,17117623,27087905,24552584,25213783,}},
    {"0000000000000000000000000000000000000000000000000000000000000002", {1477713,7756450,10956188,18843116,5280035,23926642,21485984,25409801,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {7944879,30703371,18593785,31681620,17003578,23749264,23251318,24362277,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {3151970,16283037,12190544,27031633,16929809,21092626,19126308,20966999,}},
    {"0000000000000000000000000000000000000000000000000000000000000002", {11742805,26242944,23697920,25996586,21500488,31327474,26510314,29943352,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {3474994,10457317,7857597,9399886,3867550,30337263,7069425,9680763,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {4070482,23916700,15699207,32888441,13650427,16462993,13669068,32222979,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {1627610,2703221,25539467,26038173,20347871,22002889,27516372,33238984,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {5468299,20890642,15915944,28896368,5606254,30055314,20185515,26677498,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {4462855,25840162,11275769,25729029,4826227,8562549,17814144,29614168,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {4650460,13427503,11884796,27041814,8349794,29591502,8987422,28994357,}},
    {"0000000000000000000000000000000000000000000000000000000000000002", {2166037,24526315,16259633,18008356,12870736,27719518,18085823,19116447,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {3052277,18769667,10791737,29179500,6365222,25640257,12119977,23834845,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {931226,6222676,1981516,29528857,5526494,6437446,10216785,28915484,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {402336,20565306,1583379,17377951,2444738,20364696,20541200,28451640,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {481364,23732599,3623136,30032932,14778581,29656013,16319804,26087720,}},
    {"0000000000000000000000000000000000000000000000000000000000000004", {673463,14487660,3861111,12772690,12841615,19331262,18127729,25891349,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {2688675,19459330,21829421,33160296,2824435,7396953,21175704,24949453,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {421892,18602015,9591135,12373184,7173347,17694169,7290439,33340580,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {3679946,29187321,20720370,29454078,23815548,28025307,24512609,27027920,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {5915260,18635781,13270767,30532080,12087680,29287028,19295798,24363799,}},
    {"0000000000000000000000000000000000000000000000000000000000000002", {1379380,7806321,2502697,6908506,4744762,14959880,13980259,22137966,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {803408,20975966,1413482,13231173,3839363,23141291,13060405,18560250,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {1947212,33079753,4063708,21246355,11314469,30018165,21961696,31328001,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {6801506,7820458,13124300,19286633,13786873,26395883,24579925,27092845,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {4620414,21545333,24603466,27122394,8013117,19976859,26103775,33462356,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {1370285,27998389,15658264,32386855,13657425,28489219,21379429,27671710,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {1046241,9720078,22270576,31073443,2249489,18541008,3403135,25347615,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {6628384,15347228,7109798,12299310,13621159,18901367,17361405,24566552,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {485899,7175305,13204288,20383108,4291864,25276227,15428198,22075223,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {4576157,29654495,26010900,27823742,10508010,28828381,18751049,19433359,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {334007,24126201,14043036,28663573,12796738,30384031,23762812,32543448,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {1564387,20284844,7145783,29120878,16382945,23659265,24017950,25818664,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {1484737,27983306,17749884,21444188,20800690,30784231,28120163,30053577,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {2865891,27768876,4204802,17130915,11616277,31257628,32536380,33369091,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {1725874,5215139,12692907,19278036,15366567,15828025,21793226,31816514,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {734414,21482967,11953109,25118212,20766944,32456333,25492301,31547478,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {362428,14414265,5075444,27902624,9831174,31212666,13737393,32385081,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {145591,8161454,3423499,6631632,10102448,33473585,23115896,28527903,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {5444772,12807948,22417503,25113898,5895721,31533619,9369957,32708724,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {191145,14692308,10562753,23643852,1755413,2466351,12169279,32286664,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {4217796,23246407,8887845,31695154,10533326,30025673,22206874,32714380,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {1753102,14244174,8640641,9405374,2166463,9756880,20869865,32394445,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {8201850,31499684,11129001,16748109,14515976,24003883,17165759,32317200,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {6917819,32404441,19271143,21516446,15418960,20868981,24923177,28394671,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {321206,26913076,14302151,33448705,7685964,24523233,22925251,29572618,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {2903820,23498516,15828657,20209266,9829797,25536341,28768552,32421544,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {9083532,16515943,25935474,33429629,12792895,14550317,28467194,32688447,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {4996695,29308066,7380272,28823119,14269104,28218529,21486846,26875541,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {228029,9456103,5531810,9109588,3885858,13450710,11493960,23832083,}},
    {"0000000000000000000000000000000000000000000000000000000000000002", {958915,15090644,10493937,26335262,2070393,5579592,18459958,26779385,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {3493559,20210149,7605993,29565780,5511822,15350620,12521491,16673167,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {710976,1160173,1850466,12141940,13492316,15225365,27352064,29101085,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {833211,21378874,6103635,22908955,14126213,17853331,21011902,23341861,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {181919,31623201,16582055,19621754,3411910,25415514,12417425,18284193,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {5204608,22166236,7632269,8411635,10740217,26043175,12647988,26574735,}},
    {"0000000000000000000000000000000000000000000000000000000000000005", {4664139,29955587,14704709,25453965,4905764,29351642,16659156,23751368,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {5074419,30890716,6339326,28953313,9291001,24375735,15257160,21165865,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {190613,10145278,13714746,18808813,1423680,9382947,4269988,10838567,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {504650,14462835,10620156,24205954,15972669,20658502,16300078,23707181,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {7086481,22486367,28908448,28999017,18308207,19251588,25979812,32255764,}},
    {"0000000000000000000000000000000000000000000000000000000000000003", {1172269,3423547,9911856,12439428,8953916,22659976,21037751,30753942,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {3184078,22469134,7580931,10118630,9045686,13673962,11524613,33303343,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {342541,746191,25924366,26107112,9690659,21648096,15916528,18201749,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {582456,27300325,15677298,25818305,2923523,31393258,14404095,22050324,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {10098349,15404997,29705933,31559548,11692299,14511261,15242769,23172215,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {6647678,19913634,25164906,26855602,14666922,20168128,24127251,32531463,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {843691,21167176,4272064,8283965,12764152,21618083,24528474,27375014,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {978668,10447726,18233823,25605427,2261375,2637831,2614534,9252646,}},
    {"0000000000000000000000000000000000000000000000000000000000000002", {1523033,15567854,31485034,31653930,3636244,29609827,9083361,13896638,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {14773161,24406294,20062169,23703412,19849451,25344930,21297766,21606960,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {10100496,12477457,16853659,28087974,18171731,31136212,22832204,25990125,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {531788,29206121,8960854,25889718,3126134,22174191,8498009,30497485,}},
    {"0000000000000000000000000000000000000000000000000000000000000002", {1096424,23770446,8053481,14993175,3764227,10960948,6565175,22844110,}},
    {"0000000000000000000000000000000000000000000000000000000000000002", {6429009,13512960,16148076,24871100,8228201,22446947,25288067,28595618,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {1434391,25515128,14612732,17121094,12948147,15232470,18432774,21355020,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {3262210,13836997,4066552,27432736,10722950,27475484,19851879,33265322,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {2137233,17090519,21817897,23680576,4999337,31859639,19266661,21935793,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {11111617,28095860,19042244,26086616,11348636,11733615,12672763,22722616,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {3028356,33444135,24052719,33236905,5745448,17228696,12238401,17713606,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {3682992,24667486,8936251,20577232,21868711,24392760,25828087,25912226,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {7856263,26873641,8940072,26662014,18323682,23361977,27035850,31048059,}},
    {"0000000000000000000000000000000000000000000000000000000000000001", {4788899,18223624,10534029,17985114,14007906,14374373,18385178,32483622,}},
    {"0000000000000000000000000000000000000000000000000000000000000002", {7610807,27213835,11937691,19779229,8510123,27229098,12664187,21714276,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {1108792,32718813,27176467,32141527,9222059,17822217,9907758,25989472,}},
    {"0000000000000000000000000000000000000000000000000000000000000000", {1797437,9489520,25931087,27014881,9885952,31932732,10063945,20983975,}},
};

// NOTE: These tests rely on CreateNewBlock doing its own self-validation!
BOOST_AUTO_TEST_CASE(CreateNewBlock_validity)
{
    CScript scriptPubKey = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
    CBlockTemplate *pblocktemplate;
    CMutableTransaction tx,tx2;
    CScript script;
    uint256 hash;

    LOCK(cs_main);
    fCheckpointsEnabled = false;

    // We can't make transactions until we have inputs
    // Therefore, load 100 blocks :)
    std::vector<CTransaction*>txFirst;
    for (unsigned int i = 0; i < sizeof(blockinfo)/sizeof(*blockinfo); ++i)
    {
        // Simple block creation, nothing special yet:
        BOOST_CHECK(pblocktemplate = CreateNewBlock(scriptPubKey));

        CBlock *pblock = &pblocktemplate->block; // pointer for convenience
        pblock->nVersion = 4;
        // Fake the blocks taking at least nPowTargetSpacing to be mined.
        // GetMedianTimePast() returns the median of 11 blocks, so the timestamp
        // of the next block must be six spacings ahead of that to be at least
        // one spacing ahead of the tip. Within 11 blocks of genesis, the median
        // will be closer to the tip, and blocks will appear slower.
        pblock->nTime = chainActive.Tip()->GetMedianTimePast()+6*Params().GetConsensus().nPowTargetSpacing;
        CMutableTransaction txCoinbase(pblock->vtx[0]);
        txCoinbase.vin[0].scriptSig = CScript() << (chainActive.Height()+1) << OP_0;
        txCoinbase.vout[0].scriptPubKey = CScript();
        pblock->vtx[0] = CTransaction(txCoinbase);
        if (txFirst.size() < 2)
            txFirst.push_back(new CTransaction(pblock->vtx[0]));
        pblock->hashMerkleRoot = pblock->BuildMerkleTree();
        pblock->nNonce = uint256S(blockinfo[i].nonce_hex);
        pblock->nSolution = std::vector<uint32_t>(blockinfo[i].vSolutions,
                                                  blockinfo[i].vSolutions + NUM_EQUIHASH_SOLUTIONS);

/*
        {
        arith_uint256 try_nonce(0);
        unsigned int n = 96;
        unsigned int k = 3;

        // Hash state
        crypto_generichash_blake2b_state eh_state;
        EhInitialiseState(n, k, eh_state);

        // I = the block header minus nonce and solution.
        CEquihashInput I{*pblock};
        CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
        ss << I;

        // H(I||...
        crypto_generichash_blake2b_update(&eh_state, (unsigned char*)&ss[0], ss.size());

        while (true) {
            pblock->nNonce = ArithToUint256(try_nonce);

            // H(I||V||...
            crypto_generichash_blake2b_state curr_state;
            curr_state = eh_state;
            crypto_generichash_blake2b_update(&curr_state,
                                              pblock->nNonce.begin(),
                                              pblock->nNonce.size());

            // (x_1, x_2, ...) = A(I, V, n, k)
            std::set<std::vector<unsigned int>> solns;
            EhOptimisedSolve(n, k, curr_state, solns);

            bool ret;
            for (auto soln : solns) {
                EhIsValidSolution(n, k, curr_state, soln, ret);
                assert(ret);
                pblock->nSolution = soln;

                CValidationState state;
                
                if (ProcessNewBlock(state, NULL, pblock, true, NULL) && state.IsValid()) {
                    goto foundit;
                }

                //std::cout << state.GetRejectReason() << std::endl;
            }

            try_nonce += 1;
        }
        foundit:

            std::cout << "    {\"" << pblock->nNonce.GetHex() << "\", {";
            BOOST_FOREACH(auto num, pblock->nSolution) {
                std::cout << num << ",";
            }
            std::cout << "}}," << std::endl;

        }
*/

        CValidationState state;
        BOOST_CHECK(ProcessNewBlock(state, NULL, pblock, true, NULL));
        BOOST_CHECK_MESSAGE(state.IsValid(), state.GetRejectReason());
        pblock->hashPrevBlock = pblock->GetHash();

        // Need to recreate the template each round because of mining slow start
        delete pblocktemplate;
    }

    // Just to make sure we can still make simple blocks
    BOOST_CHECK(pblocktemplate = CreateNewBlock(scriptPubKey));
    delete pblocktemplate;

    // block sigops > limit: 1000 CHECKMULTISIG + 1
    tx.vin.resize(1);
    // NOTE: OP_NOP is used to force 20 SigOps for the CHECKMULTISIG
    tx.vin[0].scriptSig = CScript() << OP_0 << OP_0 << OP_0 << OP_NOP << OP_CHECKMULTISIG << OP_1;
    tx.vin[0].prevout.hash = txFirst[0]->GetHash();
    tx.vin[0].prevout.n = 0;
    tx.vout.resize(1);
    tx.vout[0].nValue = 50000LL;
    for (unsigned int i = 0; i < 1001; ++i)
    {
        tx.vout[0].nValue -= 10;
        hash = tx.GetHash();
        mempool.addUnchecked(hash, CTxMemPoolEntry(tx, 11, GetTime(), 111.0, 11));
        tx.vin[0].prevout.hash = hash;
    }
    BOOST_CHECK(pblocktemplate = CreateNewBlock(scriptPubKey));
    delete pblocktemplate;
    mempool.clear();

    // block size > limit
    tx.vin[0].scriptSig = CScript();
    // 18 * (520char + DROP) + OP_1 = 9433 bytes
    std::vector<unsigned char> vchData(520);
    for (unsigned int i = 0; i < 18; ++i)
        tx.vin[0].scriptSig << vchData << OP_DROP;
    tx.vin[0].scriptSig << OP_1;
    tx.vin[0].prevout.hash = txFirst[0]->GetHash();
    tx.vout[0].nValue = 50000LL;
    for (unsigned int i = 0; i < 128; ++i)
    {
        tx.vout[0].nValue -= 350;
        hash = tx.GetHash();
        mempool.addUnchecked(hash, CTxMemPoolEntry(tx, 11, GetTime(), 111.0, 11));
        tx.vin[0].prevout.hash = hash;
    }
    BOOST_CHECK(pblocktemplate = CreateNewBlock(scriptPubKey));
    delete pblocktemplate;
    mempool.clear();

    // orphan in mempool
    hash = tx.GetHash();
    mempool.addUnchecked(hash, CTxMemPoolEntry(tx, 11, GetTime(), 111.0, 11));
    BOOST_CHECK(pblocktemplate = CreateNewBlock(scriptPubKey));
    delete pblocktemplate;
    mempool.clear();

    // child with higher priority than parent
    tx.vin[0].scriptSig = CScript() << OP_1;
    tx.vin[0].prevout.hash = txFirst[1]->GetHash();
    tx.vout[0].nValue = 39000LL;
    hash = tx.GetHash();
    mempool.addUnchecked(hash, CTxMemPoolEntry(tx, 11, GetTime(), 111.0, 11));
    tx.vin[0].prevout.hash = hash;
    tx.vin.resize(2);
    tx.vin[1].scriptSig = CScript() << OP_1;
    tx.vin[1].prevout.hash = txFirst[0]->GetHash();
    tx.vin[1].prevout.n = 0;
    tx.vout[0].nValue = 49000LL;
    hash = tx.GetHash();
    mempool.addUnchecked(hash, CTxMemPoolEntry(tx, 11, GetTime(), 111.0, 11));
    BOOST_CHECK(pblocktemplate = CreateNewBlock(scriptPubKey));
    delete pblocktemplate;
    mempool.clear();

    // coinbase in mempool
    tx.vin.resize(1);
    tx.vin[0].prevout.SetNull();
    tx.vin[0].scriptSig = CScript() << OP_0 << OP_1;
    tx.vout[0].nValue = 0;
    hash = tx.GetHash();
    mempool.addUnchecked(hash, CTxMemPoolEntry(tx, 11, GetTime(), 111.0, 11));
    BOOST_CHECK(pblocktemplate = CreateNewBlock(scriptPubKey));
    delete pblocktemplate;
    mempool.clear();

    // invalid (pre-p2sh) txn in mempool
    tx.vin[0].prevout.hash = txFirst[0]->GetHash();
    tx.vin[0].prevout.n = 0;
    tx.vin[0].scriptSig = CScript() << OP_1;
    tx.vout[0].nValue = 49000LL;
    script = CScript() << OP_0;
    tx.vout[0].scriptPubKey = GetScriptForDestination(CScriptID(script));
    hash = tx.GetHash();
    mempool.addUnchecked(hash, CTxMemPoolEntry(tx, 11, GetTime(), 111.0, 11));
    tx.vin[0].prevout.hash = hash;
    tx.vin[0].scriptSig = CScript() << (std::vector<unsigned char>)script;
    tx.vout[0].nValue -= 10000;
    hash = tx.GetHash();
    mempool.addUnchecked(hash, CTxMemPoolEntry(tx, 11, GetTime(), 111.0, 11));
    BOOST_CHECK(pblocktemplate = CreateNewBlock(scriptPubKey));
    delete pblocktemplate;
    mempool.clear();

    // double spend txn pair in mempool
    tx.vin[0].prevout.hash = txFirst[0]->GetHash();
    tx.vin[0].scriptSig = CScript() << OP_1;
    tx.vout[0].nValue = 49000LL;
    tx.vout[0].scriptPubKey = CScript() << OP_1;
    hash = tx.GetHash();
    mempool.addUnchecked(hash, CTxMemPoolEntry(tx, 11, GetTime(), 111.0, 11));
    tx.vout[0].scriptPubKey = CScript() << OP_2;
    hash = tx.GetHash();
    mempool.addUnchecked(hash, CTxMemPoolEntry(tx, 11, GetTime(), 111.0, 11));
    BOOST_CHECK(pblocktemplate = CreateNewBlock(scriptPubKey));
    delete pblocktemplate;
    mempool.clear();

    // subsidy changing
    int nHeight = chainActive.Height();
    chainActive.Tip()->nHeight = 209999;
    BOOST_CHECK(pblocktemplate = CreateNewBlock(scriptPubKey));
    delete pblocktemplate;
    chainActive.Tip()->nHeight = 210000;
    BOOST_CHECK(pblocktemplate = CreateNewBlock(scriptPubKey));
    delete pblocktemplate;
    chainActive.Tip()->nHeight = nHeight;

    // non-final txs in mempool
    SetMockTime(chainActive.Tip()->GetMedianTimePast()+1);

    // height locked
    tx.vin[0].prevout.hash = txFirst[0]->GetHash();
    tx.vin[0].scriptSig = CScript() << OP_1;
    tx.vin[0].nSequence = 0;
    tx.vout[0].nValue = 49000LL;
    tx.vout[0].scriptPubKey = CScript() << OP_1;
    tx.nLockTime = chainActive.Tip()->nHeight+1;
    hash = tx.GetHash();
    mempool.addUnchecked(hash, CTxMemPoolEntry(tx, 11, GetTime(), 111.0, 11));
    BOOST_CHECK(!CheckFinalTx(tx, LOCKTIME_MEDIAN_TIME_PAST));

    // time locked
    tx2.vin.resize(1);
    tx2.vin[0].prevout.hash = txFirst[1]->GetHash();
    tx2.vin[0].prevout.n = 0;
    tx2.vin[0].scriptSig = CScript() << OP_1;
    tx2.vin[0].nSequence = 0;
    tx2.vout.resize(1);
    tx2.vout[0].nValue = 79000LL;
    tx2.vout[0].scriptPubKey = CScript() << OP_1;
    tx2.nLockTime = chainActive.Tip()->GetMedianTimePast()+1;
    hash = tx2.GetHash();
    mempool.addUnchecked(hash, CTxMemPoolEntry(tx2, 11, GetTime(), 111.0, 11));
    BOOST_CHECK(!CheckFinalTx(tx2, LOCKTIME_MEDIAN_TIME_PAST));

    BOOST_CHECK(pblocktemplate = CreateNewBlock(scriptPubKey));

    // Neither tx should have make it into the template.
    BOOST_CHECK_EQUAL(pblocktemplate->block.vtx.size(), 1);
    delete pblocktemplate;

    // However if we advance height and time by one, both will.
    chainActive.Tip()->nHeight++;
    SetMockTime(chainActive.Tip()->GetMedianTimePast()+2);

    // FIXME: we should *actually* create a new block so the following test
    //        works; CheckFinalTx() isn't fooled by monkey-patching nHeight.
    //BOOST_CHECK(CheckFinalTx(tx));
    //BOOST_CHECK(CheckFinalTx(tx2));

    BOOST_CHECK(pblocktemplate = CreateNewBlock(scriptPubKey));
    BOOST_CHECK_EQUAL(pblocktemplate->block.vtx.size(), 2);
    delete pblocktemplate;

    chainActive.Tip()->nHeight--;
    SetMockTime(0);
    mempool.clear();

    BOOST_FOREACH(CTransaction *tx, txFirst)
        delete tx;

    fCheckpointsEnabled = true;
}

BOOST_AUTO_TEST_SUITE_END()
