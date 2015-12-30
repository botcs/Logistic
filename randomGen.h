#ifndef RANDOMGEN_H_INCLUDED
#define RANDOMGEN_H_INCLUDED

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
using namespace std;

const string maganhangzo = "aeuio";
const string massalhangzo = "wrtpsdfghjklzcvbnm";

void generate(const unsigned cityNum, const unsigned shipNum) {
    //mÃ³dosÃ­thatÃ³ paramÃ©terek
    unsigned int varosszam = cityNum;
    unsigned int jaratszam = shipNum;
    unsigned int rakomanyfeleszam = 3;
    int tavolsagmin = 2;
    int tavolsagmax = 20;
    int mennyisegmin = 1;
    int mennyisegmax = 50;
    int kapacitasmin = 10;
    int kapacitasmax = 80;


    int randomseed = time(0);
    srand(randomseed);
    stringstream fejlec;
    fejlec << "# random input file, varosszam: " << varosszam
            << "  jaratszam: " << jaratszam
            << "  rakomanyfeleszam: " << rakomanyfeleszam
            << "  tavolsagmin: " << tavolsagmin
            << "  tavolsagmax: " << tavolsagmax
            << "  mennyisegmin: " << mennyisegmin
            << "  mennyisegmax: " << mennyisegmax
            << "  kapacitasmin: " << kapacitasmin
            << "  kapacitasmax: " << kapacitasmax
            << "  randomseed: " << randomseed;

    set<string> varosnev;
    //esetleg Ã¼tkÃ¶zÅ‘ nevek miatt set
    while(varosnev.size()<varosszam) {
        string s;
        s+='A'+rand()%26;
        for (int i=0;i<3;i++) {
            s+=maganhangzo[rand()%maganhangzo.length()];
            s+=massalhangzo[rand()%massalhangzo.length()];
            s+=massalhangzo[rand()%massalhangzo.length()];
        }
        varosnev.insert(s);
    }

    set<string> jaratkod;
    while(jaratkod.size()<jaratszam) {
        int x = rand()%100000;
        stringstream ss;
        ss << x;
        string s = ss.str();
        while(s.length()<5) s = '0'+s;
        jaratkod.insert(s);
    }

    set<string> rakomanyfele;
    while(rakomanyfele.size()<rakomanyfeleszam) {
        string s;
        s+='A'+rand()%26;
        for (int i=0;i<2+rand()%3;i++) {
            s+=maganhangzo[rand()%maganhangzo.length()];
            s+=massalhangzo[rand()%massalhangzo.length()];
        }
        rakomanyfele.insert(s);
    }

    //indexelhetÅ‘sÃ©g kedvÃ©Ã©rt vectorba tesszÃ¼k ezeket
    vector<string> v_varosnev;
    for (string s: varosnev) {
        v_varosnev.push_back(s);
    }
    vector<string> v_jaratkod;
    for (string s: jaratkod) {
        v_jaratkod.push_back(s);
    }
    vector<string> v_rakomanyfele;
    for (string s: rakomanyfele) {
        v_rakomanyfele.push_back(s);
    }

    //kiirjuk a rakomanyokat
    ofstream frak("rakomany.txt");
    frak << fejlec.str() << endl;
    int osszrakomanyszam = rand()%(varosszam*rakomanyfeleszam)+varosszam*rakomanyfeleszam;
    for (int i=0;i<osszrakomanyszam;i++) {
        string nev = v_rakomanyfele[rand()%rakomanyfele.size()];
        int mennyi = rand()%(mennyisegmax-mennyisegmin)+mennyisegmin;
        string hely = v_varosnev[rand()%v_varosnev.size()];
        string celhely;
        do {
            celhely = v_varosnev[rand()%v_varosnev.size()];
        } while (celhely == hely);
        int bonuszido = (rand()%(tavolsagmax-tavolsagmin)+tavolsagmin)*varosszam/3;

        frak << nev << " " << mennyi << " " << hely << " " << celhely << " " << bonuszido << endl;
    }
    frak.close();

    //kiirjuk a menetrendet
    ofstream fmenet("menetrend.txt");
    fmenet << fejlec.str() << endl;
    for (unsigned int i=0;i<jaratszam;i++) {
        string kod = v_jaratkod[i]; // itt nincs random, minden jÃ¡ratnak pontosan egyszer kell szerepelnie
        int kapacitas = rand()%(kapacitasmax-kapacitasmin)+kapacitasmin;
        string hely1 = v_varosnev[rand()%v_varosnev.size()];
        string hely2;
        do {
            hely2 = v_varosnev[rand()%v_varosnev.size()];
        } while (hely2 == hely1);
        int uthossz = rand()%(tavolsagmax-tavolsagmin)+tavolsagmin;
        int odautnap;
        do {
            odautnap = uthossz + rand()%5-2;
        } while (odautnap<tavolsagmin || odautnap > tavolsagmax);
        int visszautnap;
        do {
            visszautnap = uthossz + rand()%5-2;
        } while (visszautnap<tavolsagmin || visszautnap > tavolsagmax);

        int fazis = rand()%odautnap;

        fmenet << kod << " " << kapacitas << " " << hely1 << " " << hely2 << " " << odautnap<< " " << visszautnap << " " << fazis << endl;
    }
    fmenet.close();
}

#endif // RANDOMGEN_H_INCLUDED
