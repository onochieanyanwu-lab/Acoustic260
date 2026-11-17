#include "../Source/Acoustic260Dsp.h"
#include <cstdio>
#include <cmath>
#include <vector>
using namespace a260;

static std::vector<float> guitar(double fs, double secs){
    // plucked chord-ish: E2/B2/E3/G#3 with decay + pick noise, peak ~0.25
    std::vector<float> x((size_t)(fs*secs));
    double f[4]={82.4,123.5,164.8,207.7};
    unsigned s=1;
    for(size_t n=0;n<x.size();++n){
        double t=n/fs, v=0;
        for(int k=0;k<4;++k) for(int h=1;h<=6;++h) v+=std::sin(2*kPi*f[k]*h*t)/(h*1.3)*std::exp(-t*(1.0+0.4*k+0.3*h));
        s=s*1664525u+1013904223u; double nz=((s>>9)&0xffff)/32768.0-1.0;
        v+=nz*0.05*std::exp(-t*40);
        x[n]=(float)(v*0.12);
    }
    return x;
}
struct Stat{float peak=0,rms=0;bool ok=true;};
static Stat run(Params p,double fs){
    const int OS=4; Amp amp; amp.prepare(fs*OS); amp.reset(); amp.setParams(p);
    Cabinet cab; cab.prepare(fs); cab.setHorn(p.horn);
    auto g=guitar(fs,3.0); Stat st; double acc=0;
    for(float s:g){ float y=0;
        for(int k=0;k<OS;++k){ y=amp.process(s); } // crude (no filtering) - just stability check
        if(p.cabOn) y=cab.process(y);
        if(!std::isfinite(y)) st.ok=false;
        st.peak=std::max(st.peak,std::fabs(y)); acc+=y*y;}
    st.rms=(float)std::sqrt(acc/g.size()); return st;
}
static double gainAt(Params p,double f,double fs,float amp_in){
    Amp amp; amp.prepare(fs); amp.reset(); amp.setParams(p);
    double acc=0; int N=(int)(fs*0.6), skip=(int)(fs*0.3), cnt=0;
    for(int n=0;n<N;++n){ float y=amp.process(amp_in*(float)std::sin(2*kPi*f*n/fs)); if(n>=skip){acc+=y*y;cnt++;} }
    return 20*std::log10(std::sqrt(acc/cnt)/(amp_in/std::sqrt(2.0))+1e-12);
}
int main(){
    {   // frequency response, small signal, tone controls
        double fs=192000; Params p; p.volume=0.5f; p.level=0.7f; p.powerDriveDb=0; p.headroom=1.0f;
        double fr[]={60,120,250,500,1000,2000,4000,8000};
        printf("small-signal response (dB rel 1kHz), flat vs bass/treble/variamp:\n");
        auto line=[&](const char*n,Params q){ double ref=gainAt(q,1000,fs,0.001f); printf("%-12s",n); for(double f:fr) printf(" %6.1f",gainAt(q,f,fs,0.001f)-ref); printf("\n"); };
        printf("%-12s","Hz"); for(double f:fr) printf(" %6.0f",f); printf("\n");
        line("flat",p);
        Params b=p; b.bass=1; line("bass max",b);
        Params t=p; t.treble=1; line("treble max",t);
        Params v=p; v.variampRange=2; v.variampEffect=1; line("variamp 599",v);
        Params br=p; br.bright=true; br.volume=0.3f; line("bright v=3",br);
    }
    {   // tremolo depth
        double fs=48000; Params p; p.tremIntensity=1; p.tremSpeed=0.5f; p.powerDriveDb=0; p.headroom=1;
        Amp amp; amp.prepare(fs); amp.reset(); amp.setParams(p);
        float mn=1e9,mx=0; double acc=0; int win=(int)(fs*0.002),cnt=0;
        for(int n=0;n<(int)(fs*3);++n){ float y=amp.process(0.05f*(float)std::sin(2*kPi*440*n/fs)); acc+=y*y; if(++cnt==win){ float r=(float)std::sqrt(acc/cnt); if(n>fs){mn=std::min(mn,r);mx=std::max(mx,r);} acc=0;cnt=0;} }
        printf("tremolo envelope min/max = %.3f (cut %.0f%%)\n",mn/mx,100*(1-mn/mx));
    }
    double fs=48000;
    struct C{const char*n;Params p;} cases[8];
    Params d; cases[0]={"default",d};
    Params a=d; a.volume=0.9f; cases[1]={"vol 9",a};
    Params b=d; b.bright=true; b.variampRange=2; b.variampEffect=1; cases[2]={"bright+variamp",b};
    Params c=d; c.fuzzOn=true; c.fuzzAttack=1; c.fuzzGain=1; cases[3]={"fuzz max",c};
    Params e=d; e.reverb=1; cases[4]={"reverb max",e};
    Params f=d; f.tremIntensity=1; f.tremSpeed=0.5f; cases[5]={"tremolo max",f};
    Params g=d; g.volume=1; g.level=1; g.powerDriveDb=24; g.bass=1; g.treble=1; cases[6]={"everything hot",g};
    Params h=d; h.volume=0; cases[7]={"vol 0",h};
    for(auto&k:cases){ auto s=run(k.p,fs); printf("%-16s peak %.3f rms %.3f %s\n",k.n,s.peak,s.rms,s.ok?"ok":"NaN/Inf!"); }
}
