/*							j1l.c
 *
 *	Bessel function of order one
 *
 *
 *
 * SYNOPSIS:
 *
 * long double x, y, j1l();
 *
 * y = j1l( x );
 *
 *
 *
 * DESCRIPTION:
 *
 * Returns Bessel function of first kind, order one of the argument.
 *
 * The domain is divided into two major intervals [0, 2] and
 * (2, infinity). In the first interval the rational approximation is
 * J1(x) = .5x + x x^2 R(x^2)
 *
 * The second interval is further partitioned into eight equal segments
 * of 1/x.
 * J1(x) = sqrt(2/(pi x)) (P1(x) cos(X) - Q1(x) sin(X)),
 * X = x - 3 pi / 4,
 *
 * and the auxiliary functions are given by
 *
 * J1(x)cos(X) + Y1(x)sin(X) = sqrt( 2/(pi x)) P1(x),
 * P1(x) = 1 + 1/x^2 R(1/x^2)
 *
 * Y1(x)cos(X) - J1(x)sin(X) = sqrt( 2/(pi x)) Q1(x),
 * Q1(x) = 1/x (.375 + 1/x^2 R(1/x^2)).
 *
 *
 *
 * ACCURACY:
 *
 *                      Absolute error:
 * arithmetic   domain      # trials      peak         rms
 *    IEEE      0, 30       100000      2.8e-34      2.7e-35
 *
 *
 */

/*							y1l.c
 *
 *	Bessel function of the second kind, order one
 *
 *
 *
 * SYNOPSIS:
 *
 * double x, y, y1l();
 *
 * y = y1l( x );
 *
 *
 *
 * DESCRIPTION:
 *
 * Returns Bessel function of the second kind, of order
 * one, of the argument.
 *
 * The domain is divided into two major intervals [0, 2] and
 * (2, infinity). In the first interval the rational approximation is
 * Y1(x) = 2/pi * (log(x) * J1(x) - 1/x) + x R(x^2) .
 * In the second interval the approximation is the same as for J1(x), and
 * Y1(x) = sqrt(2/(pi x)) (P1(x) sin(X) + Q1(x) cos(X)),
 * X = x - 3 pi / 4.
 *
 * ACCURACY:
 *
 *  Absolute error, when y0(x) < 1; else relative error:
 *
 * arithmetic   domain     # trials      peak         rms
 *    IEEE      0, 30       100000      2.7e-34     2.9e-35
 *
 */

/* Copyright 2001 by Stephen L. Moshier (moshier@na-net.onrl.gov).

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, see
    <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <math.h>
#include <math_private.h>
#include <float.h>

/* 1 / sqrt(pi) */
static const long double ONEOSQPI = 5.6418958354775628694807945156077258584405E-1L;
/* 2 / pi */
static const long double TWOOPI = 6.3661977236758134307553505349005744813784E-1L;
static const long double zero = 0.0L;

/* J1(x) = .5x + x x^2 R(x^2)
   Peak relative error 1.9e-35
   0 <= x <= 2  */
#define NJ0_2N 6
static const long double J0_2N[NJ0_2N + 1] = {
 -5.943799577386942855938508697619735179660E16L,
  1.812087021305009192259946997014044074711E15L,
 -2.761698314264509665075127515729146460895E13L,
  2.091089497823600978949389109350658815972E11L,
 -8.546413231387036372945453565654130054307E8L,
  1.797229225249742247475464052741320612261E6L,
 -1.559552840946694171346552770008812083969E3L
};
#define NJ0_2D 6
static const long double J0_2D[NJ0_2D + 1] = {
  9.510079323819108569501613916191477479397E17L,
  1.063193817503280529676423936545854693915E16L,
  5.934143516050192600795972192791775226920E13L,
  2.168000911950620999091479265214368352883E11L,
  5.673775894803172808323058205986256928794E8L,
  1.080329960080981204840966206372671147224E6L,
  1.411951256636576283942477881535283304912E3L,
 /* 1.000000000000000000000000000000000000000E0L */
};

/* J1(x)cosX + Y1(x)sinX = sqrt( 2/(pi x)) P1(x), P1(x) = 1 + 1/x^2 R(1/x^2),
   0 <= 1/x <= .0625
   Peak relative error 3.6e-36  */
#define NP16_IN 9
static const long double P16_IN[NP16_IN + 1] = {
  5.143674369359646114999545149085139822905E-16L,
  4.836645664124562546056389268546233577376E-13L,
  1.730945562285804805325011561498453013673E-10L,
  3.047976856147077889834905908605310585810E-8L,
  2.855227609107969710407464739188141162386E-6L,
  1.439362407936705484122143713643023998457E-4L,
  3.774489768532936551500999699815873422073E-3L,
  4.723962172984642566142399678920790598426E-2L,
  2.359289678988743939925017240478818248735E-1L,
  3.032580002220628812728954785118117124520E-1L,
};
#define NP16_ID 9
static const long double P16_ID[NP16_ID + 1] = {
  4.389268795186898018132945193912677177553E-15L,
  4.132671824807454334388868363256830961655E-12L,
  1.482133328179508835835963635130894413136E-9L,
  2.618941412861122118906353737117067376236E-7L,
  2.467854246740858470815714426201888034270E-5L,
  1.257192927368839847825938545925340230490E-3L,
  3.362739031941574274949719324644120720341E-2L,
  4.384458231338934105875343439265370178858E-1L,
  2.412830809841095249170909628197264854651E0L,
  4.176078204111348059102962617368214856874E0L,
 /* 1.000000000000000000000000000000000000000E0 */
};

/* J1(x)cosX + Y1(x)sinX = sqrt( 2/(pi x)) P1(x), P1(x) = 1 + 1/x^2 R(1/x^2),
    0.0625 <= 1/x <= 0.125
    Peak relative error 1.9e-36  */
#define NP8_16N 11
static const long double P8_16N[NP8_16N + 1] = {
  2.984612480763362345647303274082071598135E-16L,
  1.923651877544126103941232173085475682334E-13L,
  4.881258879388869396043760693256024307743E-11L,
  6.368866572475045408480898921866869811889E-9L,
  4.684818344104910450523906967821090796737E-7L,
  2.005177298271593587095982211091300382796E-5L,
  4.979808067163957634120681477207147536182E-4L,
  6.946005761642579085284689047091173581127E-3L,
  5.074601112955765012750207555985299026204E-2L,
  1.698599455896180893191766195194231825379E-1L,
  1.957536905259237627737222775573623779638E-1L,
  2.991314703282528370270179989044994319374E-2L,
};
#define NP8_16D 10
static const long double P8_16D[NP8_16D + 1] = {
  2.546869316918069202079580939942463010937E-15L,
  1.644650111942455804019788382157745229955E-12L,
  4.185430770291694079925607420808011147173E-10L,
  5.485331966975218025368698195861074143153E-8L,
  4.062884421686912042335466327098932678905E-6L,
  1.758139661060905948870523641319556816772E-4L,
  4.445143889306356207566032244985607493096E-3L,
  6.391901016293512632765621532571159071158E-2L,
  4.933040207519900471177016015718145795434E-1L,
  1.839144086168947712971630337250761842976E0L,
  2.715120873995490920415616716916149586579E0L,
 /* 1.000000000000000000000000000000000000000E0 */
};

/* J1(x)cosX + Y1(x)sinX = sqrt( 2/(pi x)) P1(x), P1(x) = 1 + 1/x^2 R(1/x^2),
  0.125 <= 1/x <= 0.1875
  Peak relative error 1.3e-36  */
#define NP5_8N 10
static const long double P5_8N[NP5_8N + 1] = {
  2.837678373978003452653763806968237227234E-12L,
  9.726641165590364928442128579282742354806E-10L,
  1.284408003604131382028112171490633956539E-7L,
  8.524624695868291291250573339272194285008E-6L,
  3.111516908953172249853673787748841282846E-4L,
  6.423175156126364104172801983096596409176E-3L,
  7.430220589989104581004416356260692450652E-2L,
  4.608315409833682489016656279567605536619E-1L,
  1.396870223510964882676225042258855977512E0L,
  1.718500293904122365894630460672081526236E0L,
  5.465927698800862172307352821870223855365E-1L
};
#define NP5_8D 10
static const long double P5_8D[NP5_8D + 1] = {
  2.421485545794616609951168511612060482715E-11L,
  8.329862750896452929030058039752327232310E-9L,
  1.106137992233383429630592081375289010720E-6L,
  7.405786153760681090127497796448503306939E-5L,
  2.740364785433195322492093333127633465227E-3L,
  5.781246470403095224872243564165254652198E-2L,
  6.927711353039742469918754111511109983546E-1L,
  4.558679283460430281188304515922826156690E0L,
  1.534468499844879487013168065728837900009E1L,
  2.313927430889218597919624843161569422745E1L,
  1.194506341319498844336768473218382828637E1L,
 /* 1.000000000000000000000000000000000000000E0 */
};

/* J1(x)cosX + Y1(x)sinX = sqrt( 2/(pi x)) P1(x), P1(x) = 1 + 1/x^2 R(1/x^2),
   Peak relative error 1.4e-36
   0.1875 <= 1/x <= 0.25  */
#define NP4_5N 10
static const long double P4_5N[NP4_5N + 1] = {
  1.846029078268368685834261260420933914621E-10L,
  3.916295939611376119377869680335444207768E-8L,
  3.122158792018920627984597530935323997312E-6L,
  1.218073444893078303994045653603392272450E-4L,
  2.536420827983485448140477159977981844883E-3L,
  2.883011322006690823959367922241169171315E-2L,
  1.755255190734902907438042414495469810830E-1L,
  5.379317079922628599870898285488723736599E-1L,
  7.284904050194300773890303361501726561938E-1L,
  3.270110346613085348094396323925000362813E-1L,
  1.804473805689725610052078464951722064757E-2L,
};
#define NP4_5D 9
static const long double P4_5D[NP4_5D + 1] = {
  1.575278146806816970152174364308980863569E-9L,
  3.361289173657099516191331123405675054321E-7L,
  2.704692281550877810424745289838790693708E-5L,
  1.070854930483999749316546199273521063543E-3L,
  2.282373093495295842598097265627962125411E-2L,
  2.692025460665354148328762368240343249830E-1L,
  1.739892942593664447220951225734811133759E0L,
  5.890727576752230385342377570386657229324E0L,
  9.517442287057841500750256954117735128153E0L,
  6.100616353935338240775363403030137736013E0L,
 /* 1.000000000000000000000000000000000000000E0 */
};

/* J1(x)cosX + Y1(x)sinX = sqrt( 2/(pi x)) P1(x), P1(x) = 1 + 1/x^2 R(1/x^2),
   Peak relative error 3.0e-36
   0.25 <= 1/x <= 0.3125  */
#define NP3r2_4N 9
static const long double P3r2_4N[NP3r2_4N + 1] = {
  8.240803130988044478595580300846665863782E-8L,
  1.179418958381961224222969866406483744580E-5L,
  6.179787320956386624336959112503824397755E-4L,
  1.540270833608687596420595830747166658383E-2L,
  1.983904219491512618376375619598837355076E-1L,
  1.341465722692038870390470651608301155565E0L,
  4.617865326696612898792238245990854646057E0L,
  7.435574801812346424460233180412308000587E0L,
  4.671327027414635292514599201278557680420E0L,
  7.299530852495776936690976966995187714739E-1L,
};
#define NP3r2_4D 9
static const long double P3r2_4D[NP3r2_4D + 1] = {
  7.032152009675729604487575753279187576521E-7L,
  1.015090352324577615777511269928856742848E-4L,
  5.394262184808448484302067955186308730620E-3L,
  1.375291438480256110455809354836988584325E-1L,
  1.836247144461106304788160919310404376670E0L,
  1.314378564254376655001094503090935880349E1L,
  4.957184590465712006934452500894672343488E1L,
  9.287394244300647738855415178790263465398E1L,
  7.652563275535900609085229286020552768399E1L,
  2.147042473003074533150718117770093209096E1L,
 /* 1.000000000000000000000000000000000000000E0 */
};

/* J1(x)cosX + Y1(x)sinX = sqrt( 2/(pi x)) P1(x), P1(x) = 1 + 1/x^2 R(1/x^2),
   Peak relative error 1.0e-35
   0.3125 <= 1/x <= 0.375  */
#define NP2r7_3r2N 9
static const long double P2r7_3r2N[NP2r7_3r2N + 1] = {
  4.599033469240421554219816935160627085991E-7L,
  4.665724440345003914596647144630893997284E-5L,
  1.684348845667764271596142716944374892756E-3L,
  2.802446446884455707845985913454440176223E-2L,
  2.321937586453963310008279956042545173930E-1L,
  9.640277413988055668692438709376437553804E-1L,
  1.911021064710270904508663334033003246028E0L,
  1.600811610164341450262992138893970224971E0L,
  4.266299218652587901171386591543457861138E-1L,
  1.316470424456061252962568223251247207325E-2L,
};
#define NP2r7_3r2D 8
static const long double P2r7_3r2D[NP2r7_3r2D + 1] = {
  3.924508608545520758883457108453520099610E-6L,
  4.029707889408829273226495756222078039823E-4L,
  1.484629715787703260797886463307469600219E-2L,
  2.553136379967180865331706538897231588685E-1L,
  2.229457223891676394409880026887106228740E0L,
  1.005708903856384091956550845198392117318E1L,
  2.277082659664386953166629360352385889558E1L,
  2.384726835193630788249826630376533988245E1L,
  9.700989749041320895890113781610939632410E0L,
 /* 1.000000000000000000000000000000000000000E0 */
};

/* J1(x)cosX + Y1(x)sinX = sqrt( 2/(pi x)) P1(x), P1(x) = 1 + 1/x^2 R(1/x^2),
   Peak relative error 1.7e-36
   0.3125 <= 1/x <= 0.4375  */
#define NP2r3_2r7N 9
static const long double P2r3_2r7N[NP2r3_2r7N + 1] = {
  3.916766777108274628543759603786857387402E-6L,
  3.212176636756546217390661984304645137013E-4L,
  9.255768488524816445220126081207248947118E-3L,
  1.214853146369078277453080641911700735354E-1L,
  7.855163309847214136198449861311404633665E-1L,
  2.520058073282978403655488662066019816540E0L,
  3.825136484837545257209234285382183711466E0L,
  2.432569427554248006229715163865569506873E0L,
  4.877934835018231178495030117729800489743E-1L,
  1.109902737860249670981355149101343427885E-2L,
};
#define NP2r3_2r7D 8
static const long double P2r3_2r7D[NP2r3_2r7D + 1] = {
  3.342307880794065640312646341190547184461E-5L,
  2.782182891138893201544978009012096558265E-3L,
  8.221304931614200702142049236141249929207E-2L,
  1.123728246291165812392918571987858010949E0L,
  7.740482453652715577233858317133423434590E0L,
  2.737624677567945952953322566311201919139E1L,
  4.837181477096062403118304137851260715475E1L,
  3.941098643468580791437772701093795299274E1L,
  1.245821247166544627558323920382547533630E1L,
 /* 1.000000000000000000000000000000000000000E0 */
};

/* J1(x)cosX + Y1(x)sinX = sqrt( 2/(pi x)) P1(x), P1(x) = 1 + 1/x^2 R(1/x^2),
   Peak relative error 1.7e-35
   0.4375 <= 1/x <= 0.5  */
#define NP2_2r3N 8
static const long double P2_2r3N[NP2_2r3N + 1] = {
  3.397930802851248553545191160608731940751E-4L,
  2.104020902735482418784312825637833698217E-2L,
  4.442291771608095963935342749477836181939E-1L,
  4.131797328716583282869183304291833754967E0L,
  1.819920169779026500146134832455189917589E1L,
  3.781779616522937565300309684282401791291E1L,
  3.459605449728864218972931220783543410347E1L,
  1.173594248397603882049066603238568316561E1L,
  9.455702270242780642835086549285560316461E-1L,
};
#define NP2_2r3D 8
static const long double P2_2r3D[NP2_2r3D + 1] = {
  2.899568897241432883079888249845707400614E-3L,
  1.831107138190848460767699919531132426356E-1L,
  3.999350044057883839080258832758908825165E0L,
  3.929041535867957938340569419874195303712E1L,
  1.884245613422523323068802689915538908291E2L,
  4.461469948819229734353852978424629815929E2L,
  5.004998753999796821224085972610636347903E2L,
  2.386342520092608513170837883757163414100E2L,
  3.791322528149347975999851588922424189957E1L,
 /* 1.000000000000000000000000000000000000000E0 */
};

/* Y1(x)cosX - J1(x)sinX = sqrt( 2/(pi x)) Q1(x),
   Q1(x) = 1/x (.375 + 1/x^2 R(1/x^2)),
   Peak relative error 8.0e-36
   0 <= 1/x <= .0625  */
#define NQ16_IN 10
static const long double Q16_IN[NQ16_IN + 1] = {
  -3.917420835712508001321875734030357393421E-18L,
  -4.440311387483014485304387406538069930457E-15L,
  -1.951635424076926487780929645954007139616E-12L,
  -4.318256438421012555040546775651612810513E-10L,
  -5.231244131926180765270446557146989238020E-8L,
  -3.540072702902043752460711989234732357653E-6L,
  -1.311017536555269966928228052917534882984E-4L,
  -2.495184669674631806622008769674827575088E-3L,
  -2.141868222987209028118086708697998506716E-2L,
  -6.184031415202148901863605871197272650090E-2L,
  -1.922298704033332356899546792898156493887E-2L,
};
#define NQ16_ID 9
static const long double Q16_ID[NQ16_ID + 1] = {
  3.820418034066293517479619763498400162314E-17L,
  4.340702810799239909648911373329149354911E-14L,
  1.914985356383416140706179933075303538524E-11L,
  4.262333682610888819476498617261895474330E-9L,
  5.213481314722233980346462747902942182792E-7L,
  3.585741697694069399299005316809954590558E-5L,
  1.366513429642842006385029778105539457546E-3L,
  2.745282599850704662726337474371355160594E-2L,
  2.637644521611867647651200098449903330074E-1L,
  1.006953426110765984590782655598680488746E0L,
 /* 1.000000000000000000000000000000000000000E0 */
 };

/* Y1(x)cosX - J1(x)sinX = sqrt( 2/(pi x)) Q1(x),
   Q1(x) = 1/x (.375 + 1/x^2 R(1/x^2)),
   Peak relative error 1.9e-36
   0.0625 <= 1/x <= 0.125  */
#define NQ8_16N 11
static const long double Q8_16N[NQ8_16N + 1] = {
  -2.028630366670228670781362543615221542291E-17L,
  -1.519634620380959966438130374006858864624E-14L,
  -4.540596528116104986388796594639405114524E-12L,
  -7.085151756671466559280490913558388648274E-10L,
  -6.351062671323970823761883833531546885452E-8L,
  -3.390817171111032905297982523519503522491E-6L,
  -1.082340897018886970282138836861233213972E-4L,
  -2.020120801187226444822977006648252379508E-3L,
  -2.093169910981725694937457070649605557555E-2L,
  -1.092176538874275712359269481414448063393E-1L,
  -2.374790947854765809203590474789108718733E-1L,
  -1.365364204556573800719985118029601401323E-1L,
};
#define NQ8_16D 11
static const long double Q8_16D[NQ8_16D + 1] = {
  1.978397614733632533581207058069628242280E-16L,
  1.487361156806202736877009608336766720560E-13L,
  4.468041406888412086042576067133365913456E-11L,
  7.027822074821007443672290507210594648877E-9L,
  6.375740580686101224127290062867976007374E-7L,
  3.466887658320002225888644977076410421940E-5L,
  1.138625640905289601186353909213719596986E-3L,
  2.224470799470414663443449818235008486439E-2L,
  2.487052928527244907490589787691478482358E-1L,
  1.483927406564349124649083853892380899217E0L,
  4.182773513276056975777258788903489507705E0L,
  4.419665392573449746043880892524360870944E0L,
 /* 1.000000000000000000000000000000000000000E0 */
};

/* Y1(x)cosX - J1(x)sinX = sqrt( 2/(pi x)) Q1(x),
   Q1(x) = 1/x (.375 + 1/x^2 R(1/x^2)),
   Peak relative error 1.5e-35
   0.125 <= 1/x <= 0.1875  */
#define NQ5_8N 10
static const long double Q5_8N[NQ5_8N + 1] = {
  -3.656082407740970534915918390488336879763E-13L,
  -1.344660308497244804752334556734121771023E-10L,
  -1.909765035234071738548629788698150760791E-8L,
  -1.366668038160120210269389551283666716453E-6L,
  -5.392327355984269366895210704976314135683E-5L,
  -1.206268245713024564674432357634540343884E-3L,
  -1.515456784370354374066417703736088291287E-2L,
  -1.022454301137286306933217746545237098518E-1L,
  -3.373438906472495080504907858424251082240E-1L,
  -4.510782522110845697262323973549178453405E-1L,
  -1.549000892545288676809660828213589804884E-1L,
};
#define NQ5_8D 10
static const long double Q5_8D[NQ5_8D + 1] = {
  3.565550843359501079050699598913828460036E-12L,
  1.321016015556560621591847454285330528045E-9L,
  1.897542728662346479999969679234270605975E-7L,
  1.381720283068706710298734234287456219474E-5L,
  5.599248147286524662305325795203422873725E-4L,
  1.305442352653121436697064782499122164843E-2L,
  1.750234079626943298160445750078631894985E-1L,
  1.311420542073436520965439883806946678491E0L,
  5.162757689856842406744504211089724926650E0L,
  9.527760296384704425618556332087850581308E0L,
  6.604648207463236667912921642545100248584E0L,
 /* 1.000000000000000000000000000000000000000E0 */
};

/* Y1(x)cosX - J1(x)sinX = sqrt( 2/(pi x)) Q1(x),
   Q1(x) = 1/x (.375 + 1/x^2 R(1/x^2)),
   Peak relative error 1.3e-35
   0.1875 <= 1/x <= 0.25  */
#define NQ4_5N 10
static const long double Q4_5N[NQ4_5N + 1] = {
  -4.079513568708891749424783046520200903755E-11L,
  -9.326548104106791766891812583019664893311E-9L,
  -8.016795121318423066292906123815687003356E-7L,
  -3.372350544043594415609295225664186750995E-5L,
  -7.566238665947967882207277686375417983917E-4L,
  -9.248861580055565402130441618521591282617E-3L,
  -6.033106131055851432267702948850231270338E-2L,
  -1.966908754799996793730369265431584303447E-1L,
  -2.791062741179964150755788226623462207560E-1L,
  -1.255478605849190549914610121863534191666E-1L,
  -4.320429862021265463213168186061696944062E-3L,
};
#define NQ4_5D 9
static const long double Q4_5D[NQ4_5D + 1] = {
  3.978497042580921479003851216297330701056E-10L,
  9.203304163828145809278568906420772246666E-8L,
  8.059685467088175644915010485174545743798E-6L,
  3.490187375993956409171098277561669167446E-4L,
  8.189109654456872150100501732073810028829E-3L,
  1.072572867311023640958725265762483033769E-1L,
  7.790606862409960053675717185714576937994E-1L,
  3.016049768232011196434185423512777656328E0L,
  5.722963851442769787733717162314477949360E0L,
  4.510527838428473279647251350931380867663E0L,
 /* 1.000000000000000000000000000000000000000E0 */
};

/* Y1(x)cosX - J1(x)sinX = sqrt( 2/(pi x)) Q1(x),
   Q1(x) = 1/x (.375 + 1/x^2 R(1/x^2)),
   Peak relative error 2.1e-35
   0.25 <= 1/x <= 0.3125  */
#define NQ3r2_4N 9
static const long double Q3r2_4N[NQ3r2_4N + 1] = {
  -1.087480809271383885936921889040388133627E-8L,
  -1.690067828697463740906962973479310170932E-6L,
  -9.608064416995105532790745641974762550982E-5L,
  -2.594198839156517191858208513873961837410E-3L,
  -3.610954144421543968160459863048062977822E-2L,
  -2.629866798251843212210482269563961685666E-1L,
  -9.709186825881775885917984975685752956660E-1L,
  -1.667521829918185121727268867619982417317E0L,
  -1.109255082925540057138766105229900943501E0L,
  -1.812932453006641348145049323713469043328E-1L,
};
#define NQ3r2_4D 9
static const long double Q3r2_4D[NQ3r2_4D + 1] = {
  1.060552717496912381388763753841473407026E-7L,
  1.676928002024920520786883649102388708024E-5L,
  9.803481712245420839301400601140812255737E-4L,
  2.765559874262309494758505158089249012930E-2L,
  4.117921827792571791298862613287549140706E-1L,
  3.323769515244751267093378361930279161413E0L,
  1.436602494405814164724810151689705353670E1L,
  3.163087869617098638064881410646782408297E1L,
  3.198181264977021649489103980298349589419E1L,
  1.203649258862068431199471076202897823272E1L,
 /* 1.000000000000000000000000000000000000000E0  */
};

/* Y1(x)cosX - J1(x)sinX = sqrt( 2/(pi x)) Q1(x),
   Q1(x) = 1/x (.375 + 1/x^2 R(1/x^2)),
   Peak relative error 1.6e-36
   0.3125 <= 1/x <= 0.375  */
#define NQ2r7_3r2N 9
static const long double Q2r7_3r2N[NQ2r7_3r2N + 1] = {
  -1.723405393982209853244278760171643219530E-7L,
  -2.090508758514655456365709712333460087442E-5L,
  -9.140104013370974823232873472192719263019E-4L,
  -1.871349499990714843332742160292474780128E-2L,
  -1.948930738119938669637865956162512983416E-1L,
  -1.048764684978978127908439526343174139788E0L,
  -2.827714929925679500237476105843643064698E0L,
  -3.508761569156476114276988181329773987314E0L,
  -1.669332202790211090973255098624488308989E0L,
  -1.930796319299022954013840684651016077770E-1L,
};
#define NQ2r7_3r2D 9
static const long double Q2r7_3r2D[NQ2r7_3r2D + 1] = {
  1.680730662300831976234547482334347983474E-6L,
  2.084241442440551016475972218719621841120E-4L,
  9.445316642108367479043541702688736295579E-3L,
  2.044637889456631896650179477133252184672E-1L,
  2.316091982244297350829522534435350078205E0L,
  1.412031891783015085196708811890448488865E1L,
  4.583830154673223384837091077279595496149E1L,
  7.549520609270909439885998474045974122261E1L,
  5.697605832808113367197494052388203310638E1L,
  1.601496240876192444526383314589371686234E1L,
  /* 1.000000000000000000000000000000000000000E0 */
};

/* Y1(x)cosX - J1(x)sinX = sqrt( 2/(pi x)) Q1(x),
   Q1(x) = 1/x (.375 + 1/x^2 R(1/x^2)),
   Peak relative error 9.5e-36
   0.375 <= 1/x <= 0.4375  */
#define NQ2r3_2r7N 9
static const long double Q2r3_2r7N[NQ2r3_2r7N + 1] = {
  -8.603042076329122085722385914954878953775E-7L,
  -7.701746260451647874214968882605186675720E-5L,
  -2.407932004380727587382493696877569654271E-3L,
  -3.403434217607634279028110636919987224188E-2L,
  -2.348707332185238159192422084985713102877E-1L,
  -7.957498841538254916147095255700637463207E-1L,
  -1.258469078442635106431098063707934348577E0L,
  -8.162415474676345812459353639449971369890E-1L,
  -1.581783890269379690141513949609572806898E-1L,
  -1.890595651683552228232308756569450822905E-3L,
};
#define NQ2r3_2r7D 8
static const long double Q2r3_2r7D[NQ2r3_2r7D + 1] = {
  8.390017524798316921170710533381568175665E-6L,
  7.738148683730826286477254659973968763659E-4L,
  2.541480810958665794368759558791634341779E-2L,
  3.878879789711276799058486068562386244873E-1L,
  3.003783779325811292142957336802456109333E0L,
  1.206480374773322029883039064575464497400E1L,
  2.458414064785315978408974662900438351782E1L,
  2.367237826273668567199042088835448715228E1L,
  9.231451197519171090875569102116321676763E0L,
 /* 1.000000000000000000000000000000000000000E0 */
};

/* Y1(x)cosX - J1(x)sinX = sqrt( 2/(pi x)) Q1(x),
   Q1(x) = 1/x (.375 + 1/x^2 R(1/x^2)),
   Peak relative error 1.4e-36
   0.4375 <= 1/x <= 0.5  */
#define NQ2_2r3N 9
static const long double Q2_2r3N[NQ2_2r3N + 1] = {
  -5.552507516089087822166822364590806076174E-6L,
  -4.135067659799500521040944087433752970297E-4L,
  -1.059928728869218962607068840646564457980E-2L,
  -1.212070036005832342565792241385459023801E-1L,
  -6.688350110633603958684302153362735625156E-1L,
  -1.793587878197360221340277951304429821582E0L,
  -2.225407682237197485644647380483725045326E0L,
  -1.123402135458940189438898496348239744403E0L,
  -1.679187241566347077204805190763597299805E-1L,
  -1.458550613639093752909985189067233504148E-3L,
};
#define NQ2_2r3D 8
static const long double Q2_2r3D[NQ2_2r3D + 1] = {
  5.415024336507980465169023996403597916115E-5L,
  4.179246497380453022046357404266022870788E-3L,
  1.136306384261959483095442402929502368598E-1L,
  1.422640343719842213484515445393284072830E0L,
  8.968786703393158374728850922289204805764E0L,
  2.914542473339246127533384118781216495934E1L,
  4.781605421020380669870197378210457054685E1L,
  3.693865837171883152382820584714795072937E1L,
  1.153220502744204904763115556224395893076E1L,
  /* 1.000000000000000000000000000000000000000E0 */
};


/* Evaluate P[n] x^n  +  P[n-1] x^(n-1)  +  ...  +  P[0] */

static long double
neval (long double x, const long double *p, int n)
{
  long double y;

  p += n;
  y = *p--;
  do
    {
      y = y * x + *p--;
    }
  while (--n > 0);
  return y;
}


/* Evaluate x^n+1  +  P[n] x^(n)  +  P[n-1] x^(n-1)  +  ...  +  P[0] */

static long double
deval (long double x, const long double *p, int n)
{
  long double y;

  p += n;
  y = x + *p--;
  do
    {
      y = y * x + *p--;
    }
  while (--n > 0);
  return y;
}


/* Bessel function of the first kind, order one.  */

long double
__ieee754_j1l (long double x)
{
  long double xx, xinv, z, p, q, c, s, cc, ss;

  if (! isfinite (x))
    {
      if (x != x)
	return x;
      else
	return 0.0L;
    }
  if (x == 0.0L)
    return x;
  xx = fabsl (x);
  if (xx <= 0x1p-58L)
    {
      long double ret = x * 0.5L;
      math_check_force_underflow (ret);
      if (ret == 0)
	__set_errno (ERANGE);
      return ret;
    }
  if (xx <= 2.0L)
    {
      /* 0 <= x <= 2 */
      z = xx * xx;
      p = xx * z * neval (z, J0_2N, NJ0_2N) / deval (z, J0_2D, NJ0_2D);
      p += 0.5L * xx;
      if (x < 0)
	p = -p;
      return p;
    }

  /* X = x - 3 pi/4
     cos(X) = cos(x) cos(3 pi/4) + sin(x) sin(3 pi/4)
     = 1/sqrt(2) * (-cos(x) + sin(x))
     sin(X) = sin(x) cos(3 pi/4) - cos(x) sin(3 pi/4)
     = -1/sqrt(2) * (sin(x) + cos(x))
     cf. Fdlibm.  */
  __sincosl (xx, &s, &c);
  ss = -s - c;
  cc = s - c;
  if (xx <= LDBL_MAX / 2.0L)
    {
      z = __cosl (xx + xx);
      if ((s * c) > 0)
	cc = z / ss;
      else
	ss = z / cc;
    }

  if (xx > 0x1p256L)
    {
      z = ONEOSQPI * cc / __ieee754_sqrtl (xx);
      if (x < 0)
	z = -z;
      return z;
    }

  xinv = 1.0L / xx;
  z = xinv * xinv;
  if (xinv <= 0.25)
    {
      if (xinv <= 0.125)
	{
	  if (xinv <= 0.0625)
	    {
	      p = neval (z, P16_IN, NP16_IN) / deval (z, P16_ID, NP16_ID);
	      q = neval (z, Q16_IN, NQ16_IN) / deval (z, Q16_ID, NQ16_ID);
	    }
	  else
	    {
	      p = neval (z, P8_16N, NP8_16N) / deval (z, P8_16D, NP8_16D);
	      q = neval (z, Q8_16N, NQ8_16N) / deval (z, Q8_16D, NQ8_16D);
	    }
	}
      else if (xinv <= 0.1875)
	{
	  p = neval (z, P5_8N, NP5_8N) / deval (z, P5_8D, NP5_8D);
	  q = neval (z, Q5_8N, NQ5_8N) / deval (z, Q5_8D, NQ5_8D);
	}
      else
	{
	  p = neval (z, P4_5N, NP4_5N) / deval (z, P4_5D, NP4_5D);
	  q = neval (z, Q4_5N, NQ4_5N) / deval (z, Q4_5D, NQ4_5D);
	}
    }				/* .25 */
  else /* if (xinv <= 0.5) */
    {
      if (xinv <= 0.375)
	{
	  if (xinv <= 0.3125)
	    {
	      p = neval (z, P3r2_4N, NP3r2_4N) / deval (z, P3r2_4D, NP3r2_4D);
	      q = neval (z, Q3r2_4N, NQ3r2_4N) / deval (z, Q3r2_4D, NQ3r2_4D);
	    }
	  else
	    {
	      p = neval (z, P2r7_3r2N, NP2r7_3r2N)
		  / deval (z, P2r7_3r2D, NP2r7_3r2D);
	      q = neval (z, Q2r7_3r2N, NQ2r7_3r2N)
		  / deval (z, Q2r7_3r2D, NQ2r7_3r2D);
	    }
	}
      else if (xinv <= 0.4375)
	{
	  p = neval (z, P2r3_2r7N, NP2r3_2r7N)
	      / deval (z, P2r3_2r7D, NP2r3_2r7D);
	  q = neval (z, Q2r3_2r7N, NQ2r3_2r7N)
	      / deval (z, Q2r3_2r7D, NQ2r3_2r7D);
	}
      else
	{
	  p = neval (z, P2_2r3N, NP2_2r3N) / deval (z, P2_2r3D, NP2_2r3D);
	  q = neval (z, Q2_2r3N, NQ2_2r3N) / deval (z, Q2_2r3D, NQ2_2r3D);
	}
    }
  p = 1.0L + z * p;
  q = z * q;
  q = q * xinv + 0.375L * xinv;
  z = ONEOSQPI * (p * cc - q * ss) / __ieee754_sqrtl (xx);
  if (x < 0)
    z = -z;
  return z;
}
strong_alias (__ieee754_j1l, __j1l_finite)


/* Y1(x) = 2/pi * (log(x) * J1(x) - 1/x) + x R(x^2)
   Peak relative error 6.2e-38
   0 <= x <= 2   */
#define NY0_2N 7
static long double Y0_2N[NY0_2N + 1] = {
  -6.804415404830253804408698161694720833249E19L,
  1.805450517967019908027153056150465849237E19L,
  -8.065747497063694098810419456383006737312E17L,
  1.401336667383028259295830955439028236299E16L,
  -1.171654432898137585000399489686629680230E14L,
  5.061267920943853732895341125243428129150E11L,
  -1.096677850566094204586208610960870217970E9L,
  9.541172044989995856117187515882879304461E5L,
};
#define NY0_2D 7
static long double Y0_2D[NY0_2D + 1] = {
  3.470629591820267059538637461549677594549E20L,
  4.120796439009916326855848107545425217219E18L,
  2.477653371652018249749350657387030814542E16L,
  9.954678543353888958177169349272167762797E13L,
  2.957927997613630118216218290262851197754E11L,
  6.748421382188864486018861197614025972118E8L,
  1.173453425218010888004562071020305709319E6L,
  1.450335662961034949894009554536003377187E3L,
  /* 1.000000000000000000000000000000000000000E0 */
};


/* Bessel function of the second kind, order one.  */

long double
__ieee754_y1l (long double x)
{
  long double xx, xinv, z, p, q, c, s, cc, ss;

  if (! isfinite (x))
    {
      if (x != x)
	return x;
      else
	return 0.0L;
    }
  if (x <= 0.0L)
    {
      if (x < 0.0L)
	return (zero / (zero * x));
      return -HUGE_VALL + x;
    }
  xx = fabsl (x);
  if (xx <= 0x1p-114)
    {
      z = -TWOOPI / x;
      if (isinf (z))
	__set_errno (ERANGE);
      return z;
    }
  if (xx <= 2.0L)
    {
      /* 0 <= x <= 2 */
      SET_RESTORE_ROUNDL (FE_TONEAREST);
      z = xx * xx;
      p = xx * neval (z, Y0_2N, NY0_2N) / deval (z, Y0_2D, NY0_2D);
      p = -TWOOPI / xx + p;
      p = TWOOPI * __ieee754_logl (x) * __ieee754_j1l (x) + p;
      return p;
    }

  /* X = x - 3 pi/4
     cos(X) = cos(x) cos(3 pi/4) + sin(x) sin(3 pi/4)
     = 1/sqrt(2) * (-cos(x) + sin(x))
     sin(X) = sin(x) cos(3 pi/4) - cos(x) sin(3 pi/4)
     = -1/sqrt(2) * (sin(x) + cos(x))
     cf. Fdlibm.  */
  __sincosl (xx, &s, &c);
  ss = -s - c;
  cc = s - c;
  if (xx <= LDBL_MAX / 2.0L)
    {
      z = __cosl (xx + xx);
      if ((s * c) > 0)
	cc = z / ss;
      else
	ss = z / cc;
    }

  if (xx > 0x1p256L)
    return ONEOSQPI * ss / __ieee754_sqrtl (xx);

  xinv = 1.0L / xx;
  z = xinv * xinv;
  if (xinv <= 0.25)
    {
      if (xinv <= 0.125)
	{
	  if (xinv <= 0.0625)
	    {
	      p = neval (z, P16_IN, NP16_IN) / deval (z, P16_ID, NP16_ID);
	      q = neval (z, Q16_IN, NQ16_IN) / deval (z, Q16_ID, NQ16_ID);
	    }
	  else
	    {
	      p = neval (z, P8_16N, NP8_16N) / deval (z, P8_16D, NP8_16D);
	      q = neval (z, Q8_16N, NQ8_16N) / deval (z, Q8_16D, NQ8_16D);
	    }
	}
      else if (xinv <= 0.1875)
	{
	  p = neval (z, P5_8N, NP5_8N) / deval (z, P5_8D, NP5_8D);
	  q = neval (z, Q5_8N, NQ5_8N) / deval (z, Q5_8D, NQ5_8D);
	}
      else
	{
	  p = neval (z, P4_5N, NP4_5N) / deval (z, P4_5D, NP4_5D);
	  q = neval (z, Q4_5N, NQ4_5N) / deval (z, Q4_5D, NQ4_5D);
	}
    }				/* .25 */
  else /* if (xinv <= 0.5) */
    {
      if (xinv <= 0.375)
	{
	  if (xinv <= 0.3125)
	    {
	      p = neval (z, P3r2_4N, NP3r2_4N) / deval (z, P3r2_4D, NP3r2_4D);
	      q = neval (z, Q3r2_4N, NQ3r2_4N) / deval (z, Q3r2_4D, NQ3r2_4D);
	    }
	  else
	    {
	      p = neval (z, P2r7_3r2N, NP2r7_3r2N)
		  / deval (z, P2r7_3r2D, NP2r7_3r2D);
	      q = neval (z, Q2r7_3r2N, NQ2r7_3r2N)
		  / deval (z, Q2r7_3r2D, NQ2r7_3r2D);
	    }
	}
      else if (xinv <= 0.4375)
	{
	  p = neval (z, P2r3_2r7N, NP2r3_2r7N)
	      / deval (z, P2r3_2r7D, NP2r3_2r7D);
	  q = neval (z, Q2r3_2r7N, NQ2r3_2r7N)
	      / deval (z, Q2r3_2r7D, NQ2r3_2r7D);
	}
      else
	{
	  p = neval (z, P2_2r3N, NP2_2r3N) / deval (z, P2_2r3D, NP2_2r3D);
	  q = neval (z, Q2_2r3N, NQ2_2r3N) / deval (z, Q2_2r3D, NQ2_2r3D);
	}
    }
  p = 1.0L + z * p;
  q = z * q;
  q = q * xinv + 0.375L * xinv;
  z = ONEOSQPI * (p * ss + q * cc) / __ieee754_sqrtl (xx);
  return z;
}
strong_alias (__ieee754_y1l, __y1l_finite)
