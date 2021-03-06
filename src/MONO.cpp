#include "cf.hpp"
#include "dsp/digital.hpp"


struct MONO : Module {
	enum ParamIds {
		PAN_PARAM,
	        GAIN_PARAM,
	        SOLO_PARAM,
		ON_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		SOLOT_INPUT,
		ONT_INPUT,
		PAN_INPUT,
		GAIN_INPUT,
		EXTSOLO_INPUT,
		LEFT_INPUT,
		RIGHT_INPUT,
		PANPOT_INPUT,
		IN1_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		EXTSOLO_OUTPUT,
		LEFT_OUTPUT,
		RIGHT_OUTPUT,
		OUT1_OUTPUT,
		NUM_OUTPUTS
	};
    enum LightIds {
		SOLO_LIGHT,
		ON_LIGHT,
		LEVEL_LIGHTS,
		NUM_LIGHTS = LEVEL_LIGHTS +11
	};


float SIGNAL = 0.0 ;
bool ON_STATE = false ;
bool SOLO_STATE = false ;
bool soloed = false;
int lightState[11] = {};
int cligno =0;
int retard =0;
int retard2 =0;
SchmittTrigger onTrigger;
SchmittTrigger oninTrigger;
SchmittTrigger soloTrigger;
SchmittTrigger soloinTrigger;


	MONO() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {reset();}
	void step() override;

void reset() override {
			ON_STATE = true;
			SOLO_STATE = false;
			}

json_t *toJson() override {
		json_t *rootJ = json_object();
		// solo
		json_object_set_new(rootJ, "solostate", json_integer(SOLO_STATE));

		// solo
		json_object_set_new(rootJ, "onstate", json_integer(ON_STATE));
		return rootJ;
		}

void fromJson(json_t *rootJ) override {
		// solo
		json_t *solostateJ = json_object_get(rootJ, "solostate");
		if (solostateJ)
			SOLO_STATE = json_integer_value(solostateJ);

		// solo
		json_t *onstateJ = json_object_get(rootJ, "onstate");
		if (onstateJ)
			ON_STATE = json_integer_value(onstateJ);
	
	}

};


void MONO::step() {

        SIGNAL = inputs[IN1_INPUT].value ;

	if (!inputs[GAIN_INPUT].active)
		SIGNAL = SIGNAL * params[GAIN_PARAM].value/5.0 ;
		else SIGNAL = SIGNAL * inputs[GAIN_INPUT].value/5.0 ;

	if (onTrigger.process(params[ON_PARAM].value)+oninTrigger.process(inputs[ONT_INPUT].value))
			{if (ON_STATE == 0) ON_STATE = 1; else ON_STATE = 0;}

	if (inputs[EXTSOLO_INPUT].value == 0) soloed = 0;
	if (inputs[EXTSOLO_INPUT].value == 10) soloed = 1;

	if (soloTrigger.process(params[SOLO_PARAM].value)+soloinTrigger.process(inputs[SOLOT_INPUT].value))
			{if (SOLO_STATE == 0) {SOLO_STATE = 1;} else {SOLO_STATE = 0;soloed=0;}}

	if ((!SOLO_STATE and !soloed) and (retard > 0)) retard = 0; else if (retard < 1000) retard = retard + 1;


	outputs[EXTSOLO_OUTPUT].value=round(10*retard/1000);

	if (!SOLO_STATE) SIGNAL = SIGNAL * ON_STATE ;
	if (soloed and !SOLO_STATE) SIGNAL = 0;

	outputs[OUT1_OUTPUT].value = SIGNAL;
	

	if (!inputs[PAN_INPUT].active) {
			outputs[LEFT_OUTPUT].value = inputs[LEFT_INPUT].value + SIGNAL*(1-clampf(params[PAN_PARAM].value,0,1));
			outputs[RIGHT_OUTPUT].value = inputs[RIGHT_INPUT].value + SIGNAL*(1-clampf(-params[PAN_PARAM].value,0,1));
		} else {
			outputs[LEFT_OUTPUT].value = inputs[LEFT_INPUT].value + SIGNAL*(1-clampf(inputs[PAN_INPUT].value,0,5)/5);
			outputs[RIGHT_OUTPUT].value = inputs[RIGHT_INPUT].value + SIGNAL*(1-clampf(-inputs[PAN_INPUT].value,0,5)/5);
		}

	if (ON_STATE==1) lights[ON_LIGHT].value=true; else lights[ON_LIGHT].value=false;
	
	if (SOLO_STATE==1) {if (cligno == 0) cligno =20000; else cligno=cligno-1;} else cligno = 0;
	if (cligno>5000) lights[SOLO_LIGHT].value =1; else lights[SOLO_LIGHT].value =0;

	for (int i = 0; i < 11; i++) {
		if (SIGNAL> i) {if (i<10) lightState[i]=5000;else lightState[i]=20000;}
	}
	for (int i = 0; i < 11; i++) {
		if (lightState[i]> 0) {lightState[i]=lightState[i]-1;lights[LEVEL_LIGHTS + i].value=true;} else lights[LEVEL_LIGHTS + i].value=false;
	}
}




MONOWidget::MONOWidget() {
	MONO *module = new MONO();
	setModule(module);
	box.size = Vec(15*6, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/MONO.svg")));
		addChild(panel);
	}

	addChild(createScrew<ScrewSilver>(Vec(15, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(createScrew<ScrewSilver>(Vec(15, 365)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x-30, 365)));


	addParam(createParam<Trimpot>(Vec(38, 127), module, MONO::PAN_PARAM, -1.0, 1.0, 0.0));
	addInput(createInput<PJ301MPort>(Vec(11, 131), module, MONO::PAN_INPUT));

    	addParam(createParam<RoundBlackKnob>(Vec(27, 247), module, MONO::GAIN_PARAM, 0.0, 10.0, 5.0));
	addInput(createInput<PJ301MPort>(Vec(11, 281), module, MONO::GAIN_INPUT));


   	addParam(createParam<LEDButton>(Vec(38, 167), module, MONO::SOLO_PARAM, 0.0, 1.0, 0.0));
 	addChild(createLight<MediumLight<BlueLight>>(Vec(42.4, 171.4), module, MONO::SOLO_LIGHT));
	addInput(createInput<PJ301MPort>(Vec(11, 171), module, MONO::SOLOT_INPUT));

     	addParam(createParam<LEDButton>(Vec(38, 208), module, MONO::ON_PARAM, 0.0, 1.0, 0.0));
	addChild(createLight<MediumLight<BlueLight>>(Vec(42.4, 212.4), module, MONO::ON_LIGHT));
	addInput(createInput<PJ301MPort>(Vec(11, 211), module, MONO::ONT_INPUT));
    

	addInput(createInput<PJ301MPort>(Vec(11, 321), module, MONO::IN1_INPUT));

	addOutput(createOutput<PJ301MPort>(Vec(54, 321), module, MONO::OUT1_OUTPUT));
	
	addOutput(createOutput<PJ301MPort>(Vec(54, 31), module, MONO::EXTSOLO_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(54, 61), module, MONO::LEFT_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(54, 91), module, MONO::RIGHT_OUTPUT));

	addInput(createInput<PJ301MPort>(Vec(11, 31), module, MONO::EXTSOLO_INPUT));
	addInput(createInput<PJ301MPort>(Vec(11, 61), module, MONO::LEFT_INPUT));
	addInput(createInput<PJ301MPort>(Vec(11, 91), module, MONO::RIGHT_INPUT));


	for (int i = 0; i < 11; i++) {
		if (i<10) addChild(createLight<MediumLight<BlueLight>>(Vec(70, 242-i*12), module, MONO::LEVEL_LIGHTS + i));
			else addChild(createLight<MediumLight<RedLight>>(Vec(70, 242-i*12), module, MONO::LEVEL_LIGHTS + i));
	}
	
	
}
