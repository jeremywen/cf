#include "cf.hpp"
#include "dsp/digital.hpp"

struct METRO : Module {
	enum ParamIds {
		BPM_PARAM,
		RST_PARAM,
		BEAT_PARAM,
		ON_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		ON_INPUT,
		BPM_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		MES_OUTPUT,
		BEAT_OUTPUT,
		START_OUTPUT,
		NUM_OUTPUTS
	};
    enum LightIds {
		ON_LIGHT,
		MES_LIGHT,
		BEAT_LIGHT,
		NUM_LIGHTS
	};

int max_METRO = 120 ;
int beatl = 0 ;
int mesl = 0 ;
int beats = 0 ;
int mess = 0 ;
int strt = 0 ;
int note ;
float toc_phase = 0.f;
uint32_t toc = 0u;
SchmittTrigger onTrigger;
SchmittTrigger oninTrigger;
SchmittTrigger rstTrigger;
bool ON_STATE = false;

	METRO() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {reset();}
	void step() override;
void reset() override {
			ON_STATE = true;
			
			}

json_t *toJson() override {
		json_t *rootJ = json_object();

		// on
		json_object_set_new(rootJ, "onstate", json_integer(ON_STATE));
		return rootJ;
		}

void fromJson(json_t *rootJ) override {

		// on
		json_t *onstateJ = json_object_get(rootJ, "onstate");
		if (onstateJ)
			ON_STATE = json_integer_value(onstateJ);
	
	}
};


void METRO::step() {
	if (!inputs[BPM_INPUT].active) max_METRO = floor(params[BPM_PARAM].value) ; else max_METRO = floor(inputs[BPM_INPUT].value *30) ;
	float bpm = max_METRO ;
	bool toced = false;

	if (onTrigger.process(params[ON_PARAM].value)+oninTrigger.process(inputs[ON_INPUT].value))
			{if (ON_STATE == 0) {ON_STATE = 1; strt = 5;} else ON_STATE = 0;}

	lights[ON_LIGHT].value = ON_STATE ;

	
	if (rstTrigger.process(params[RST_PARAM].value))
		{toc = 47u;toc_phase = 1.f; strt = 5;}
	

	if (ON_STATE) {
		toc_phase += ((bpm / 60.f) / engineGetSampleRate()) * 12.f;
		
		if(toc_phase >= 1.f) {
			toced = true;
			toc = (toc+1u) % 48u ;
			toc_phase -= 1.f;
			}

		if(toced) {
			beatl = toc % 12u ? 0 : 20000;
			mesl = toc % 48u ? 0 : 40000;
			beats = toc % 12u ? 0 : 200;
			mess = toc % 48u ? 0 : 200;
			note =5;
			}

		if (beatl>0) {
			lights[BEAT_LIGHT].value = true; 
			beatl = beatl -1;
		} else {
			lights[BEAT_LIGHT].value = false;
			}

		if (mesl>0) {
			lights[MES_LIGHT].value = true; 
			mesl = mesl -1;
		} else {
			lights[MES_LIGHT].value = false;
			}


		if (beats>0) {
			beats = beats -1;
			outputs[BEAT_OUTPUT].value = 2.5 * (beats- 150*round(beats/150))/150;
		} else {
			outputs[BEAT_OUTPUT].value = 0.0;
			}

		if (mess>0) { 
			mess = mess -1;
			outputs[BEAT_OUTPUT].value = 5.0 * (mess- 150*round(mess/150))/150;
			}

	} else {toc = 47u;toc_phase = 1.f;outputs[OUT_OUTPUT].value = 0.f;}
    

	if (strt > 0) {outputs[START_OUTPUT].value = 10.f;strt=strt-1;} else outputs[START_OUTPUT].value = 0.f;
	if (note > 0) {outputs[OUT_OUTPUT].value = 10.f;note=note-1;} else outputs[OUT_OUTPUT].value = 0.f;
}

struct NumDisplayWidget : TransparentWidget {

  int *value;
  std::shared_ptr<Font> font;

  NumDisplayWidget() {
    font = Font::load(assetPlugin(plugin, "res/Segment7Standard.ttf"));
  };

  void draw(NVGcontext *vg) {
    // Background
    NVGcolor backgroundColor = nvgRGB(0x44, 0x44, 0x44);
    NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 4.0);
    nvgFillColor(vg, backgroundColor);
    nvgFill(vg);
    nvgStrokeWidth(vg, 1.0);
    nvgStrokeColor(vg, borderColor);
    nvgStroke(vg);

    nvgFontSize(vg, 18);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 2.5);

    std::string to_display = std::to_string(*value);


    while(to_display.length()<3) to_display = ' ' + to_display;

    Vec textPos = Vec(6.0f, 17.0f);

    NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "~~~", NULL);

    textColor = nvgRGB(0xda, 0xe9, 0x29);

    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "\\\\\\", NULL);


    textColor = nvgRGB(0x28, 0xb0, 0xf3);
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x, textPos.y, to_display.c_str(), NULL);
  }
};

METROWidget::METROWidget() {
	METRO *module = new METRO();
	setModule(module);
	box.size = Vec(15*6, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/METRO.svg")));
		addChild(panel);
	}

	addChild(createScrew<ScrewSilver>(Vec(15, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(createScrew<ScrewSilver>(Vec(15, 365)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x-30, 365)));

	addParam(createParam<RoundBlackKnob>(Vec(27, 107), module, METRO::BPM_PARAM, 0.0, 301.0, 120.1));
	addInput(createInput<PJ301MPort>(Vec(11, 141), module, METRO::BPM_INPUT));

     	addParam(createParam<LEDButton>(Vec(38, 167), module, METRO::ON_PARAM, 0.0, 1.0, 0.0));
	addChild(createLight<MediumLight<BlueLight>>(Vec(42.4, 171.4), module, METRO::ON_LIGHT));
	addInput(createInput<PJ301MPort>(Vec(11, 171), module, METRO::ON_INPUT));

     	addParam(createParam<LEDButton>(Vec(38, 197), module, METRO::RST_PARAM, 0.0, 1.0, 0.0));
	addChild(createLight<MediumLight<BlueLight>>(Vec(42.4, 201.4), module, METRO::MES_LIGHT));
	//addOutput(createOutput<PJ301MPort>(Vec(54, 221), module, METRO::MES_OUTPUT));

     	addParam(createParam<LEDButton>(Vec(38, 227), module, METRO::BEAT_PARAM, 0.0, 1.0, 0.0));
	addChild(createLight<MediumLight<BlueLight>>(Vec(42.4, 231.4), module, METRO::BEAT_LIGHT));
	addOutput(createOutput<PJ301MPort>(Vec(54, 265), module, METRO::BEAT_OUTPUT));

	addOutput(createOutput<PJ301MPort>(Vec(11, 321), module, METRO::START_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(54, 321), module, METRO::OUT_OUTPUT));

	NumDisplayWidget *display = new NumDisplayWidget();
	display->box.pos = Vec(20,56);
	display->box.size = Vec(50, 20);
	display->value = &module->max_METRO;
	addChild(display);

	
}
