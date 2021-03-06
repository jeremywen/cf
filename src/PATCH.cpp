#include "cf.hpp"


struct PATCH : Module {

	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		L_INPUT,
		R_INPUT,
		M1_INPUT,
		M2_INPUT,
		M3_INPUT,
		IN_INPUT,
		NUM_INPUTS=IN_INPUT+8
	};
	enum OutputIds {
		L_OUTPUT,
		R_OUTPUT,
		M1_OUTPUT,
		M2_OUTPUT,
		M3_OUTPUT,
		OUT_OUTPUT,
		NUM_OUTPUTS=OUT_OUTPUT+8
	};
   	enum LightIds {
		NUM_LIGHTS
	};






PATCH() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

	void step() override;


};


void PATCH::step() {
	outputs[L_OUTPUT].value = inputs[L_INPUT].value ;
	outputs[R_OUTPUT].value = inputs[R_INPUT].value ;

	outputs[M1_OUTPUT].value = inputs[M1_INPUT].value+inputs[M2_INPUT].value+inputs[M3_INPUT].value ;
	outputs[M2_OUTPUT].value = inputs[M1_INPUT].value+inputs[M2_INPUT].value+inputs[M3_INPUT].value ;
	outputs[M3_OUTPUT].value = inputs[M1_INPUT].value+inputs[M2_INPUT].value+inputs[M3_INPUT].value ;

	for (int i = 0; i < 8; i++) {
		if (inputs[IN_INPUT + i].active) outputs[OUT_OUTPUT + i].value = inputs[IN_INPUT + i].value ;
			else outputs[OUT_OUTPUT + i].value = 0;	
		}
	
}

PATCHWidget::PATCHWidget() {
	PATCH *module = new PATCH();
	setModule(module);

	box.size = Vec(15*9, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/PATCH.svg")));
		addChild(panel);
	}

	addChild(createScrew<ScrewSilver>(Vec(15, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(createScrew<ScrewSilver>(Vec(15, 365)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x-30, 365)));

	addInput(createInput<PJ301MPort>(Vec(10, 171), module, PATCH::L_INPUT));
	addInput(createInput<PJ301MPort>(Vec(40, 171), module, PATCH::R_INPUT));
	addOutput(createOutput<PJ301MPort>(Vec(70, 171), module, PATCH::L_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(100, 171), module, PATCH::R_OUTPUT));

	addInput(createInput<PJ301MPort>(Vec(15, 61), module, PATCH::M1_INPUT));
	addInput(createInput<PJ301MPort>(Vec(55, 41), module, PATCH::M2_INPUT));
	addInput(createInput<PJ301MPort>(Vec(95, 71), module, PATCH::M3_INPUT));

	addOutput(createOutput<PJ301MPort>(Vec(40, 121), module, PATCH::M1_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(55, 81), module, PATCH::M2_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(80, 111), module, PATCH::M3_OUTPUT));

for (int i = 0; i < 8; i++) {

	addInput(createInput<PJ301MPort>(Vec(10+(i-4*floor(i/4))*30, 231+60*floor(i/4)), module, PATCH::IN_INPUT + i));
	addOutput(createOutput<PJ301MPort>(Vec(10+(i-4*floor(i/4))*30, 261+60*floor(i/4)), module, PATCH::OUT_OUTPUT + i));

	
}

	
}
