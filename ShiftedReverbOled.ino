
#include "DaisyDuino.h"
#include <U8g2lib.h>

// Declare a local daisy_petal for hardware access
DaisyHardware hw;


U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI oled(U8G2_R0, /* clock=*/ 8, /* data=*/ 10, /* cs=*/ 7, /* dc=*/ 9, /* reset=*/ 30);

 int    Dtime;
 int    Dsend;
 int    Ddampening;
 int    Dpitchmix;
 int    Dtranspose;
 int    Dlocut;
 
 bool   bypass;
 
Parameter vtime, vfreq, vsend, trans, wet, freqParam;



static PitchShifter ps;
static ATone      flt;
//static ReverbSc  verb;
ReverbSc DSY_SDRAM_BSS verb;



// This runs at a fixed rate, to prepare audio samples
void callback(float *in, float *out, size_t size)
{
    float dryl, dryr, wetl, wetr, sendl, sendr, shiftedl, shiftedr;
    float hfreq, outputl, outputr;

    hw.ProcessDigitalControls();
  
    verb.SetLpFreq(vfreq.Process());
    vsend.Process(); // Process Send to use later

 

    hfreq = freqParam.Process();
    flt.SetFreq(hfreq);    


    //bypass = hw.switches[DaisyPetal::SW_5].Pressed();
    if(hw.buttons[0].RisingEdge())
     {
        bypass = !bypass;
        hw.leds[0].Set(bypass);
     }

    if(hw.buttons[1].Pressed()) // secondary feedback footswitch
     { verb.SetFeedback(0.999f);
     hw.leds[1].Set(false);
       }
    else
    {
      verb.SetFeedback(vtime.Process());
      hw.leds[1].Set(true);
       }

    wet.Process();
   
    trans.Process();


    float transpose = floor((trans.Value() * 100.0f) / 7.0f);
    
    if (hw.buttons[2].Pressed()) {
        ps.SetTransposition(12.0f + transpose);
       
    } else {
        ps.SetTransposition(transpose);
    }


    
    for(size_t i = 0; i < size; i += 2)
    {
        dryl  = in[i];
        dryr  = in[i + 1];
        sendl = dryl * vsend.Value();
        sendr = dryr * vsend.Value();

        verb.Process(sendl, sendr, &wetl, &wetr);

        outputl = flt.Process(wetl);
        outputr = flt.Process(wetr);

        shiftedl = ps.Process(outputl);
        shiftedr = ps.Process(outputr);
        
        if(bypass)
        {
            out[i]     = in[i];     // left
            out[i + 1] = in[i + 1]; // right
        }
        else
        {
            out[i]     = dryl + outputl + shiftedl * wet.Value();
            out[i + 1] = dryr + outputr + shiftedr * wet.Value();
        }
    }
    
}

void setup()
{
    float samplerate;

    hw = DAISY.init(DAISY_PETAL, AUDIO_SR_48K);
    samplerate = DAISY.AudioSampleRate();


  oled.setFont(u8g2_font_ncenB08_tr);
  oled.setFontDirection(0);
  oled.setFontMode(1);
  oled.begin();
    
    bypass = true;

    vtime.Init(hw.controls[0], 0.6f, 0.999f, Parameter::LOGARITHMIC);
    vfreq.Init(hw.controls[1], 500.0f, 20000.0f, Parameter::LOGARITHMIC);
    vsend.Init(hw.controls[2], 0.0f, 1.0f, Parameter::LINEAR);
    verb.Init(samplerate);

    freqParam.Init(hw.controls[5], 80.0f, 20000.0f, Parameter::LOGARITHMIC);
    flt.Init(samplerate);

    wet.Init(hw.controls[3], 0.01f, 0.999f, Parameter::LOGARITHMIC);
    trans.Init(hw.controls[4], 0.0001f, 0.91f, Parameter::LINEAR);
    ps.Init(samplerate);


   DAISY.begin(callback);
}
   void loop() 
    {
        delay(10);

        Dtime = (analogRead(16)) / 10.24;
        delay(15);
        Dsend = (analogRead(18)) / 10.24;
        delay(15);
        Ddampening = (analogRead(20)) / 10.24;
        delay(15);
        Dpitchmix = (analogRead(17)) / 10.24;
        delay(15);
        Dtranspose = (analogRead(19)) / 10.24;
        delay(15);
        Dlocut = (analogRead(21)) / 10.24;
        
        delay(20);
        
        oled.clearBuffer();
        oled.drawStr(0,10,"Lenght");
        oled.setCursor(45, 10);
        oled.print(Dtime);

        oled.drawStr(64,10,"Pitch");
        oled.setCursor(105, 10);
        oled.print(Dpitchmix);

        oled.drawStr(0,30,"Send");
        oled.setCursor(45, 30);
        oled.print(Dsend);

        oled.drawStr(64,30,"Trans");
        oled.setCursor(105, 30);
        oled.print(Dtranspose);

        oled.drawStr(0,50,"Damp");
        oled.setCursor(45, 50);
        oled.print(Ddampening);

        oled.drawStr(64,50,"Lo cut");
        oled.setCursor(105, 50);
        oled.print(Dlocut);

        oled.sendBuffer();
        
        delay(10);
                  
    }
