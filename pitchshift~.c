#include "m_pd.h"
#include <stdlib.h>
#include <math.h>
#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* ------------------------ pitchshift~ ----------------------------- */

#define WAVETABLESIZE 1024

static t_class *pitchshift_class;

typedef struct _pitchshift
{
    t_object x_obj; 	/* obligatory header */
    t_float x_f;    	/* place to hold inlet's value if it's set by message */
	t_float *delayline;
	t_float delay_time;
	t_float delay_samples;
	t_float delay_samples_current;
	t_int head_pos;
	t_float pi;
	t_int delay_size; 
	t_float phase;		// a float for our phase..
	t_float sample_rate;	// a float for our sample_rate
	t_float pitch;		// a float which we will use as our basis for pitch (1 being standard pitch of the sample)
	t_float phasor;
	t_float cos1;
	t_float cos2;


} t_pitchshift;





    /* this is the actual performance routine which acts on the samples.
    It's called with a single pointer "w" which is our location in the
    DSP call list.  We return a new "w" which will point to the next item
    after us.  Meanwhile, w[0] is just a pointer to dsp-perform itself
    (no use to us), w[1] and w[2] are the input and output vector locations,
    and w[3] is the number of points to calculate. */

	/// pitchshift perform function

	// the goal is to take solely pitch as an inlet. perform the phasor math, inputted into 4 objects within this external
	// 1) phasor with -0.5 *0.5 into cos object
	// 2) phasor with * 50 into variable delay object (our delayline tap 1)
	// 3) phasor with *0.5 into cos object
	// 4) phasor with *50 into variable delay object (delayline tap2)
	//
	// from here, we multiply signals 1*2 and 3*4 and sum to get final output of desired real-time pitch shift

	// note this is real-time pitch shifting vs rotating tape head pitch shift (like G09 miller's example)...I thought this would be simpler...
	// the working abstraction tree in the help patch...works. trying to put everything in an external blackbox is tough... a LOT to do still

	//TODO:

	// fix 2) & 4) delay time inputs so that they properly work with phasor
	// implement a wavetable for cos such to complete 1) & 3)
	// from there it becomes straight forward, i believe



static t_int *pitchshift_perform(t_int *w)
{
	t_pitchshift *x = (t_pitchshift *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);


	int blocksize = 2097152;
	long i, sample,sample_phasor = 0;
	float playposfloat, playposfrac;
	long playpos, playposp1;
	float insample,outsample,outsample2,delayinput;

	float delay_increment, feedback_increment;
	float oneoverblocksize = 1.0f/(float)blocksize;


	float samplerate = x->sample_rate;

	//convert pitch to frequency for phasor
	x->pitch = (x->pitch - 1) * -20;
	float freq_phasor = x->pitch;

	float * phasor_out;

	float phasePerSample = 0;
	float phase = 0;

	
		
	
	
    
	// idea
	// TOO DO*
	// we need to do something with the phase & delay time, but keep only a single delayline...how to do this...not sure
	 x->delay_samples = x->delay_time * x->sample_rate;

	delay_increment = (x->delay_samples - x->delay_samples_current) * oneoverblocksize;

    while (n--)
    {
		
		x->delay_samples_current += delay_increment;
		// update the delay_samples
        insample = *(in+sample);



		// calculate the new position in the delayline to take the output
		playposfloat = (float)(x->head_pos) + x->delay_samples_current;

        
		// make sure we don't run over the ends of the table
        
        playpos = (long)playposfloat;
        playposfrac = playposfloat - (float)playpos;
        playposp1 = playpos + 1;

		while(playpos < 0)
			playpos += x->delay_size;
		while(playpos >= x->delay_size)
			playpos -= x->delay_size;
        
		while(playposp1 < 0)
			playposp1 += x->delay_size;
		while(playposp1 >= x->delay_size)
			playposp1 -= x->delay_size;
        
		// look into the delayline and grab the output
		 outsample = *(x->delayline+playpos)* (1.f - playposfrac)
                        + *(x->delayline+playposp1)*playposfrac;


		// second tap for our delayline
		//outsample2 = outsample;

		//phasor stuff....

		// do stuff for this iteration
		//phasePerSample = freq_phasor/samplerate;
		//*(phasor_out+sample_phasor) = phase;
		//phase = phase + phasePerSample

		//float phase_into1_cos = (phase - 0.5) * 0.5;
		//float phase_into1_delay = phase * 50;


		//float phase_into2_cos = phase + 0.5;
		//float phase_into2_delay = phase_into2_cos;
		//phase_into2_cos = (phase_into2_cos - 0.5)*0.5;
		//phase_into2_delay = phase_into2_delay * 50;

		// stuff into cos objects...

		// cos1_out = cos @ phase_into1_cos
		// cos2_out = cos @ phase_into2_cos

		// finishing touches



		// pop it back in the delayline.. how do i choose which input ? or keep it a single input?



		// outsample = cos1_out * outsample
		// outsample2 = cos2_out * outsample2

		//????? how to choose which to put pop back in
		delayinput = outsample + insample;
        
        if(delayinput > 1.0f)
            delayinput = 1.0f;
        if(delayinput < -1.0f)
            delayinput = -1.0f;
        
        delayinput = 1.5f * delayinput - (delayinput * delayinput * delayinput * .5f);
        
		*(x->delayline+x->head_pos) = delayinput;
        
		// mix the signals back together

		// *(out+sample) = outsample + insample + outsample2
        *(out+sample) = outsample+insample;
        
		sample++;
		//sample_phasor++;
		x->head_pos--;
		if(x->head_pos < 0)
			x->head_pos = x->delay_size - 1;


    }

    return (w+5);
}

    /* called to start DSP.  Here we call Pd back to add our perform
    routine to a linear callback list which Pd in turn calls to grind
    out the samples. */
static void pitchshift_dsp(t_pitchshift *x, t_signal **sp)
{
	// we'll initialize sample_rate when starting up
	x->sample_rate = sp[0]->s_sr;
    dsp_add(pitchshift_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void *pitchshift_new(void)
{

    t_pitchshift *x = (t_pitchshift *)pd_new(pitchshift_class);
	// create one inlet for the pitch...
	floatinlet_new(&x->x_obj,&x->pitch);

	//output the signal
    outlet_new(&x->x_obj, gensym("signal"));
	// set a fixed delay time...for the pitch shift

	// fixed delay time, but CHANGES once we add in the phasor. HOW TO DO THIS?
	x->delay_time = 0.05f;
    x->delay_samples = 0.0f;
    x->pi = 4.0 * atan(1.0);
    x->delay_size = 16777216;
	x->head_pos = 0;
    x->delayline = (t_float *)malloc(sizeof(t_float) * x->delay_size);
	
	int i;

	for(i=0; i<x->delay_size;i++){
		*(x->delayline+i) = 0.0f;
	}
	//init the delayline

	// return normally.
    return (x);
}

// since we allocated some memory, we need a delete function
static void pitchshift_free(t_pitchshift *x)
{
	free(x->delayline);
}

    /* this routine, which must have exactly this name (with the "~" replaced
    by "_tilde) is called when the code is first loaded, and tells Pd how
    to build the "class". */
void pitchshift_tilde_setup(void)
{
    pitchshift_class = class_new(gensym("pitchshift~"), (t_newmethod)pitchshift_new, (t_method)pitchshift_free,
    	sizeof(t_pitchshift), 0, A_DEFFLOAT, 0);
	    /* this is magic to declare that the leftmost, "main" inlet
	    takes signals; other signal inlets are done differently... */
    CLASS_MAINSIGNALIN(pitchshift_class, t_pitchshift, x_f);
    	/* here we tell Pd about the "dsp" method, which is called back
	when DSP is turned on. */
    class_addmethod(pitchshift_class, (t_method)pitchshift_dsp, gensym("dsp"), 0);
}
